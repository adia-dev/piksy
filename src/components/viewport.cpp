#include <SDL_error.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <imgui.h>

#include <algorithm>
#include <command/frame_extraction_command.hpp>
#include <command/swap_texture_color_command.hpp>
#include <components/viewport.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <opencv2/opencv.hpp>
#include <rendering/sprite.hpp>
#include <utils/maths.hpp>
#include <vector>

#include "icons/IconsFontAwesome4.h"
#include "managers/animation_manager.hpp"
#include "rendering/frame.hpp"
#include "tools/tool.hpp"

namespace piksy {
namespace components {

Viewport::Viewport(core::State& state, rendering::Renderer& renderer,
                   managers::ResourceManager& resource_manager,
                   managers::AnimationManager& animation_manager)
    : UIComponent(state),
      m_renderer(renderer),
      m_resource_manager(resource_manager),
      m_animation_manager(animation_manager),
      m_render_texture(nullptr),
      m_viewport_size(800, 600) {
    create_render_texture(static_cast<int>(m_viewport_size.x), static_cast<int>(m_viewport_size.y));
}

Viewport::~Viewport() {
    if (m_render_texture) {
        SDL_DestroyTexture(m_render_texture);
        m_render_texture = nullptr;
    }
}

void Viewport::create_render_texture(int width, int height) {
    if (m_render_texture) {
        SDL_DestroyTexture(m_render_texture);
        m_render_texture = nullptr;
    }

    m_render_texture = SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET, width, height);

    if (!m_render_texture) {
        core::Logger::fatal("Failed to create render texture: %s", SDL_GetError());
    }
}

void Viewport::update() {
    update_zoom();
    update_pan();

    auto animation = m_animation_manager.current_animation();
    if (animation == nullptr) {
        return;
    }

    // Handle frame extraction commit when mouse is released
    if (m_state.current_tool == tools::Tool::EXTRACT && !m_state.mouse_state.is_pressed &&
        m_is_previewing) {
        // Mouse was released and we were previewing - commit the frames

        // Check if we should append (shift key) or replace frames
        bool should_append = ImGui::IsKeyDown(ImGuiKey_LeftShift);

        // Commit the preview frames
        if (!m_preview_frames.empty()) {
            if (!should_append) {
                animation->frames.clear();
            }
            animation->frames.insert(animation->frames.end(), m_preview_frames.begin(),
                                     m_preview_frames.end());

            core::Logger::debug("Committed %zu frames to animation", m_preview_frames.size());
        }

        m_preview_frames.clear();
        m_is_previewing = false;
    }

    // Handle selection and preview
    if (m_state.mouse_state.is_pressed && !m_state.mouse_state.is_panning) {
        process_selection();
    }

    // Rest of the update code...
    if (ImGui::IsKeyDown(ImGuiKey_Backspace)) {
        if (!m_state.animation_state.selected_frames.empty()) {
            for (size_t i : m_state.animation_state.selected_frames) {
                if (i < animation->frames.size()) {
                    animation->frames.erase(animation->frames.begin() + i);
                }
            }
            m_state.animation_state.selected_frames.clear();
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
        switch (m_state.current_tool) {
            case tools::Tool::SELECT:
                m_state.animation_state.selected_frames.clear();
                break;
            case tools::Tool::EXTRACT: {
                auto animation = m_animation_manager.current_animation();
                if (animation == nullptr) {
                    break;
                }
                animation->frames.clear();
                m_preview_frames.clear();
                m_is_previewing = false;
                m_state.animation_state.current_frame = 0;
                m_state.animation_state.selected_frames.clear();
            } break;
            default:
                break;
        }
    }
}

void Viewport::render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    render_toolbar();

    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    if (viewport_size.x != m_viewport_size.x || viewport_size.y != m_viewport_size.y) {
        m_state.viewport_state.size = viewport_size;
        m_viewport_size = viewport_size;
        create_render_texture(static_cast<int>(m_viewport_size.x),
                              static_cast<int>(m_viewport_size.y));
    }

    SDL_SetRenderTarget(m_renderer.get(), m_render_texture);

    SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(m_renderer.get());

    render_grid_background();
    render_texture();

    if (m_state.mouse_state.is_pressed && !m_state.mouse_state.is_panning) {
        render_selection_rect();
    }

    render_frames();

    SDL_SetRenderTarget(m_renderer.get(), nullptr);

    ImGui::Image((ImTextureID)(intptr_t)m_render_texture, m_viewport_size);

    process_mouse_input();

    render_cursor_hud();

    /* ImGuiAxis toolbar_axis = ImGuiAxis_Y; */
    /* DockingToolbar("Toolbar", &toolbar_axis); */

    ImGui::End();
}

void Viewport::notify_dropped_file(const std::string& dropped_file_path) {
    core::Logger::debug("NotImplemented (file: %s)", dropped_file_path.c_str());
}

void Viewport::process_mouse_input() {
    if (ImGui::IsWindowHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 image_pos = ImGui::GetItemRectMin();
        ImVec2 relative_pos = {mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y};
        m_state.mouse_state.current_pos = relative_pos;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_state.mouse_state.start_pos = m_state.mouse_state.current_pos;
            m_state.mouse_state.is_pressed = true;
            handle_click(m_state.mouse_state.current_pos.x, m_state.mouse_state.current_pos.y);
        }

        m_state.mouse_state.is_dragging = m_state.mouse_state.is_pressed;
        m_state.mouse_state.is_pressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);

        process_zoom();
        process_panning();

    } else {
        m_state.mouse_state.is_dragging = false;
        m_state.mouse_state.is_pressed = false;
        m_state.mouse_state.is_panning = false;
    }
}

void Viewport::process_zoom() {
    float wheel = ImGui::GetIO().MouseWheel;

    float zoom_accel = wheel;
    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt)) {
        float drag_y = m_state.mouse_state.is_pressed *
                       (m_state.mouse_state.current_pos.y - m_state.mouse_state.start_pos.y);

        // TODO: Make the drag y factor a variable
        if (std::fabs(drag_y * 0.01f) > std::fabs(wheel)) {
            zoom_accel = drag_y * 0.01f;
        }
    }

    if (zoom_accel != 0.0f) {
        m_state.zoom_state.target_scale += zoom_accel * m_state.zoom_state.zoom_speed;
        m_state.zoom_state.target_scale = std::clamp(m_state.zoom_state.target_scale, 0.1f, 10.0f);
    }
}

void Viewport::process_panning() {
    if (m_state.mouse_state.is_pressed &&
        (m_state.current_tool == tools::Tool::PAN ||
         (!m_state.mouse_state.is_dragging && ImGui::IsKeyDown(ImGuiKey_LeftShift)))) {
        ImVec2 delta = {(m_state.mouse_state.current_pos.x - m_state.mouse_state.start_pos.x) /
                            m_state.zoom_state.current_scale,
                        (m_state.mouse_state.current_pos.y - m_state.mouse_state.start_pos.y) /
                            m_state.zoom_state.current_scale};

        m_state.pan_state.target_offset.x += delta.x;
        m_state.pan_state.target_offset.y += delta.y;

        m_state.mouse_state.start_pos = m_state.mouse_state.current_pos;
        m_state.mouse_state.is_panning = true;
    } else {
        m_state.mouse_state.is_panning = false;
    }
}

void Viewport::update_zoom() {
    m_state.zoom_state.current_scale =
        utils::maths::lerp(m_state.zoom_state.current_scale, m_state.zoom_state.target_scale, 0.1f);
}

void Viewport::update_pan() {
    m_state.pan_state.current_offset =
        utils::maths::lerp(m_state.pan_state.current_offset, m_state.pan_state.target_offset, 0.1f);
}

void Viewport::process_selection() {
    float x0 = (m_state.mouse_state.start_pos.x / m_state.zoom_state.current_scale) -
               m_state.pan_state.current_offset.x;
    float y0 = (m_state.mouse_state.start_pos.y / m_state.zoom_state.current_scale) -
               m_state.pan_state.current_offset.y;
    float x1 = (m_state.mouse_state.current_pos.x / m_state.zoom_state.current_scale) -
               m_state.pan_state.current_offset.x;
    float y1 = (m_state.mouse_state.current_pos.y / m_state.zoom_state.current_scale) -
               m_state.pan_state.current_offset.y;

    SDL_Rect selection_world_rect = {
        static_cast<int>(std::min(x0, x1)), static_cast<int>(std::min(y0, y1)),
        static_cast<int>(std::abs(x1 - x0)), static_cast<int>(std::abs(y1 - y0))};

    switch (m_state.current_tool) {
        case tools::Tool::EXTRACT: {
            if (!m_state.texture_sprite.texture()) return;

            // Update preview while dragging
            m_is_previewing = true;
            commands::FrameExtractionCommand preview_command(
                selection_world_rect, m_state.texture_sprite.texture(), m_preview_frames,
                false,  // don't append in preview
                true    // preview mode
            );
            preview_command.execute();
        } break;

        case tools::Tool::SELECT: {
            m_state.animation_state.selected_frames.clear();

            auto* animation = m_animation_manager.current_animation();
            if (animation == nullptr) {
                break;
            }

            for (size_t i = 0; i < animation->frames.size(); ++i) {
                const rendering::Frame& frame = animation->frames[i];
                const SDL_Rect frame_rect{frame.x, frame.y, frame.w, frame.h};
                if (SDL_HasIntersection(&frame_rect, &selection_world_rect)) {
                    m_state.animation_state.selected_frames.insert(i);
                }
            }
        } break;

        default:
            break;
    }
}

void Viewport::render_texture() {
    if (m_state.texture_sprite.texture() != nullptr) {
        m_state.texture_sprite.render(m_renderer.get(), m_state.zoom_state.current_scale,
                                      m_state.pan_state.current_offset.x,
                                      m_state.pan_state.current_offset.y);
    } else {
        render_placeholder_text();
    }
}

void Viewport::render_cursor_hud() {
    ImVec2 mouse_pos = ImGui::GetMousePos();

    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
        std::fabs(m_state.zoom_state.target_scale - m_state.zoom_state.current_scale) >= 0.001f) {
        ImVec2 text_offset(5, -10);
        ImVec2 text_pos = ImVec2(mouse_pos.x + text_offset.x, mouse_pos.y + text_offset.y);
        ImGui::SetCursorScreenPos(text_pos);
        ImGui::Text("%s %.2f", ICON_FA_SEARCH, m_state.zoom_state.current_scale);
    }

    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
        (std::fabs(m_state.pan_state.current_offset.x - m_state.pan_state.target_offset.x) >=
             0.001f &&
         std::fabs(m_state.pan_state.current_offset.y - m_state.pan_state.target_offset.y) >=
             0.001f)) {
        ImVec2 text_offset(7, 10);
        ImVec2 text_pos = ImVec2(mouse_pos.x + text_offset.x, mouse_pos.y + text_offset.y);
        ImGui::SetCursorScreenPos(text_pos);
        ImGui::Text("%s x: %.2f, y: %.2f", ICON_FA_ARROWS, m_state.pan_state.current_offset.x,
                    m_state.pan_state.current_offset.y);
    }
}

void Viewport::render_placeholder_text() {
    auto font =
        m_resource_manager.get_font(std::string(RESOURCE_DIR) + "/fonts/PixelifySans-Regular.ttf");
    if (font != nullptr) {
        const char* placeholder_text = "No texture loaded. Please insert a texture.";
        SDL_Color text_color{255, 255, 255, 255};
        SDL_Surface* text_surface =
            TTF_RenderText_Blended(font.get()->get(), placeholder_text, text_color);
        if (text_surface != nullptr) {
            SDL_Texture* text_texture =
                SDL_CreateTextureFromSurface(m_renderer.get(), text_surface);
            if (text_texture != nullptr) {
                int text_width = text_surface->w;
                int text_height = text_surface->h;
                SDL_Rect dest_rect = {static_cast<int>((m_viewport_size.x - text_width) / 2),
                                      static_cast<int>((m_viewport_size.y - text_height) / 2),
                                      text_width, text_height};
                SDL_RenderCopy(m_renderer.get(), text_texture, nullptr, &dest_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

void Viewport::render_selection_rect() {
    int start_x = static_cast<int>(m_state.mouse_state.start_pos.x);
    int start_y = static_cast<int>(m_state.mouse_state.start_pos.y);
    int current_x = static_cast<int>(m_state.mouse_state.current_pos.x);
    int current_y = static_cast<int>(m_state.mouse_state.current_pos.y);

    m_selection_rect = {std::min(start_x, current_x), std::min(start_y, current_y),
                        std::abs(current_x - start_x), std::abs(current_y - start_y)};

    SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 0, 255);
    SDL_RenderDrawRect(m_renderer.get(), &m_selection_rect);
    SDL_SetRenderDrawColor(m_renderer.get(), 255, 255, 0, 25);
    SDL_RenderFillRect(m_renderer.get(), &m_selection_rect);
}

void Viewport::render_grid_background() {
    SDL_SetRenderDrawColor(m_renderer.get(), 33, 33, 33, 155);

    float scaled_grid_cell_size =
        std::max(m_state.viewport_state.grid_cell_size * m_state.zoom_state.current_scale, 1.0f);

    float offset_x = fmod(m_state.pan_state.current_offset.x * m_state.zoom_state.current_scale,
                          scaled_grid_cell_size);
    float offset_y = fmod(m_state.pan_state.current_offset.y * m_state.zoom_state.current_scale,
                          scaled_grid_cell_size);

    int num_vertical_lines =
        static_cast<int>(std::ceil(m_viewport_size.x / scaled_grid_cell_size)) + 1;
    int num_horizontal_lines =
        static_cast<int>(std::ceil(m_viewport_size.y / scaled_grid_cell_size)) + 1;

    for (int i = 0; i < num_vertical_lines; ++i) {
        float x = offset_x + i * scaled_grid_cell_size;
        if (x >= 0 && x <= m_viewport_size.x) {
            SDL_RenderDrawLine(m_renderer.get(), static_cast<int>(x), 0, static_cast<int>(x),
                               static_cast<int>(m_viewport_size.y));
        }
    }

    for (int j = 0; j < num_horizontal_lines; ++j) {
        float y = offset_y + j * scaled_grid_cell_size;
        if (y >= 0 && y <= m_viewport_size.y) {
            SDL_RenderDrawLine(m_renderer.get(), 0, static_cast<int>(y),
                               static_cast<int>(m_viewport_size.x), static_cast<int>(y));
        }
    }
}

void Viewport::render_frames() const {
    auto animation = m_animation_manager.current_animation();
    if (animation == nullptr) {
        return;
    }

    // Pre-compute colors to reduce redundant operations inside the loop
    static const SDL_Color current_frame_color{206, 2, 65, 155};
    static const SDL_Color selected_frame_color{255, 135, 177, 155};
    static const SDL_Color default_frame_color{135, 235, 177, 155};
    static const SDL_Color preview_frame_color{255, 215, 0, 155};

    // Pre-compute common scaling factor for efficiency
    const float scale = m_state.zoom_state.current_scale;
    const ImVec2 offset = m_state.pan_state.current_offset;

    // Render loop with reduced computation
    for (size_t i = 0; i < animation->frames.size(); ++i) {
        const rendering::Frame& frame = animation->frames[i];
        SDL_Rect render_frame_rect{static_cast<int>((frame.x + offset.x) * scale),
                                   static_cast<int>((frame.y + offset.y) * scale),
                                   static_cast<int>(frame.w * scale),
                                   static_cast<int>(frame.h * scale)};

        // Set color based on the current context
        if (i == m_state.animation_state.current_frame) {
            SDL_SetRenderDrawColor(m_renderer.get(), current_frame_color.r, current_frame_color.g,
                                   current_frame_color.b, current_frame_color.a);
        } else if (m_state.animation_state.selected_frames.count(i)) {
            SDL_SetRenderDrawColor(m_renderer.get(), selected_frame_color.r, selected_frame_color.g,
                                   selected_frame_color.b, selected_frame_color.a);
        } else {
            SDL_SetRenderDrawColor(m_renderer.get(), default_frame_color.r, default_frame_color.g,
                                   default_frame_color.b, default_frame_color.a);
        }

        SDL_RenderDrawRect(m_renderer.get(), &render_frame_rect);
    }

    if (m_is_previewing) {
        SDL_SetRenderDrawColor(m_renderer.get(), preview_frame_color.r, preview_frame_color.g,
                               preview_frame_color.b, preview_frame_color.a);

        for (const auto& frame : m_preview_frames) {
            SDL_Rect render_frame_rect{static_cast<int>((frame.x + offset.x) * scale),
                                       static_cast<int>((frame.y + offset.y) * scale),
                                       static_cast<int>(frame.w * scale),
                                       static_cast<int>(frame.h * scale)};
            SDL_RenderDrawRect(m_renderer.get(), &render_frame_rect);
        }
    }
}

void Viewport::handle_click(float x, float y) {
    float world_x = (x / m_state.zoom_state.current_scale) - m_state.pan_state.current_offset.x;
    float world_y = (y / m_state.zoom_state.current_scale) - m_state.pan_state.current_offset.y;

    auto& sprite = m_state.texture_sprite;
    SDL_Rect rect = sprite.rect();

    float texture_x = world_x - rect.x;
    float texture_y = world_y - rect.y;

    if (texture_x >= 0 && texture_x < rect.w && texture_y >= 0 && texture_y < rect.h) {
        switch (m_state.current_tool) {
            case tools::Tool::SELECT:
                sprite.set_selected(true);
                break;
            case tools::Tool::COLOR_SWAP: {
                SDL_Color pixel_color = get_texture_pixel_color(
                    static_cast<int>(texture_x), static_cast<int>(texture_y), sprite);
                commands::SwapTextureCommand command(
                    pixel_color,
                    SDL_Color{
                        static_cast<Uint8>(m_state.replacement_color[0] * 255),
                        static_cast<Uint8>(m_state.replacement_color[1] * 255),
                        static_cast<Uint8>(m_state.replacement_color[2] * 255),
                        static_cast<Uint8>(m_state.replacement_color[3] * 255),
                    },
                    m_state.texture_sprite.texture());
                command.execute();
            } break;
            default:
                break;
        }
    } else {
        sprite.set_selected(false);
    }
}

SDL_Color Viewport::get_texture_pixel_color(int x, int y, const rendering::Sprite& sprite) {
    SDL_Texture* texture = sprite.texture()->get();
    void* pixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
        core::Logger::error("Failed to lock texture: %s", SDL_GetError());
        return SDL_Color{0, 0, 0, 0};
    }

    Uint32 format;
    SDL_QueryTexture(texture, &format, nullptr, nullptr, nullptr);
    SDL_PixelFormat* pixel_format = SDL_AllocFormat(format);

    Uint8* pixel_ptr = static_cast<Uint8*>(pixels) + y * pitch + x * 4;
    Uint32 pixel_value = *(reinterpret_cast<Uint32*>(pixel_ptr));

    SDL_Color color;
    SDL_GetRGBA(pixel_value, pixel_format, &color.r, &color.g, &color.b, &color.a);

    SDL_UnlockTexture(texture);
    SDL_FreeFormat(pixel_format);

    return color;
}

void Viewport::render_toolbar() {
    // Make toolbar transparent
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.0f);

    // Create a dockable toolbar with offset
    ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 50), ImGuiCond_FirstUseEver);
    ImGui::Begin("Toolbar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

    // Begin a horizontal layout
    ImGui::BeginGroup();

    // Render the toolbar buttons
    int num_tools = static_cast<int>(tools::Tool::COUNT);
    for (int tool_idx = 0; tool_idx < num_tools; tool_idx++) {
        tools::Tool tool = static_cast<tools::Tool>(tool_idx);
        const char* icon = tools::tool_to_icon(tool);

        if (tool_idx > 0) ImGui::SameLine();

        if (tool == m_state.current_tool) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 125 / 255.0f, 155 / 255.0f, 0.73f));
            if (ImGui::Button(icon)) {
                m_state.current_tool = tool;
            }
            ImGui::PopStyleColor();
        } else {
            if (ImGui::Button(icon)) {
                m_state.current_tool = tool;
            }
        }
    }

    ImGui::EndGroup();
    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleColor();
}

}  // namespace components
}  // namespace piksy
