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
#include <utils/dump.hpp>
#include <utils/maths.hpp>
#include <vector>

#include "icons/IconsFontAwesome4.h"

namespace piksy {
namespace components {

Viewport::Viewport(core::State& state, rendering::Renderer& renderer,
                   managers::ResourceManager& resource_manager)
    : UIComponent(state),
      _renderer(renderer),
      _resource_manager(resource_manager),
      _render_texture(nullptr),
      _viewport_size(800, 600) {
    create_render_texture(static_cast<int>(_viewport_size.x), static_cast<int>(_viewport_size.y));

    // NOTE: DON'T COMMIT
    // UPADTE: You commited it dumbass...
    if (_state.texture_sprite.texture() != nullptr) {
        commands::SwapTextureCommand command(
            {255, 255, 255, 255},
            SDL_Color{
                static_cast<Uint8>(_state.replacement_color[0] * 255),
                static_cast<Uint8>(_state.replacement_color[1] * 255),
                static_cast<Uint8>(_state.replacement_color[2] * 255),
                static_cast<Uint8>(_state.replacement_color[3] * 255),
            },
            _state.texture_sprite.texture());
        command.execute();
    }
}

Viewport::~Viewport() {
    if (_render_texture) {
        SDL_DestroyTexture(_render_texture);
        _render_texture = nullptr;
    }
}

void Viewport::create_render_texture(int width, int height) {
    if (_render_texture) {
        SDL_DestroyTexture(_render_texture);
        _render_texture = nullptr;
    }

    _render_texture = SDL_CreateTexture(_renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, width, height);

    if (!_render_texture) {
        core::Logger::fatal("Failed to create render texture: %s", SDL_GetError());
    }
}

void Viewport::update() {
    update_zoom();
    update_pan();

    if (_state.mouse_state.is_pressed && !_state.mouse_state.is_panning) {
        process_selection();
    }

    if (ImGui::IsKeyDown(ImGuiKey_Backspace)) {
        if (!_state.selected_frames.empty()) {
            for (size_t i : _state.selected_frames) {
                if (i < _state.frames.size()) {
                    _state.frames.erase(_state.frames.begin() + i);
                }
            }
            _state.selected_frames.clear();
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
        switch (_state.current_tool) {
            case core::Tool::SELECT:
                _state.selected_frames.clear();
                break;
            case core::Tool::EXTRACT:
                _state.animation_state.current_frame = 0;
                _state.frames.clear();
                _state.selected_frames.clear();
                break;
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
    if (viewport_size.x != _viewport_size.x || viewport_size.y != _viewport_size.y) {
        _state.viewport_state.size = viewport_size;
        _viewport_size = viewport_size;
        create_render_texture(static_cast<int>(_viewport_size.x),
                              static_cast<int>(_viewport_size.y));
    }

    SDL_SetRenderTarget(_renderer.get(), _render_texture);

    SDL_SetRenderDrawColor(_renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(_renderer.get());

    render_grid_background();
    render_texture();

    if (_state.mouse_state.is_pressed && !_state.mouse_state.is_panning) {
        render_selection_rect();
    }

    render_frames();

    SDL_SetRenderTarget(_renderer.get(), nullptr);

    ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

    process_mouse_input();

    render_cursor_hud();

    /* ImGuiAxis toolbar_axis = ImGuiAxis_Y; */
    /* DockingToolbar("Toolbar", &toolbar_axis); */

    ImGui::End();
}

void Viewport::notify_dropped_file(const std::string& dropped_file_path) {
    core::Logger::debug("Dropped file into the Viewport: %s", dropped_file_path.c_str());
    try {
        _state.texture_sprite.set_texture(_resource_manager.get_texture(dropped_file_path));
    } catch (const std::runtime_error& ex) {
        core::Logger::error("Failed to select a texture in the project: %s", ex.what());
    }
}

void Viewport::process_mouse_input() {
    if (ImGui::IsWindowHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 image_pos = ImGui::GetItemRectMin();
        ImVec2 relative_pos = {mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y};
        _state.mouse_state.current_pos = relative_pos;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _state.mouse_state.start_pos = _state.mouse_state.current_pos;
            _state.mouse_state.is_pressed = true;
            handle_click(_state.mouse_state.current_pos.x, _state.mouse_state.current_pos.y);
        }

        _state.mouse_state.is_pressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);

        process_zoom();
        process_panning();

    } else {
        _state.mouse_state.is_pressed = false;
        _state.mouse_state.is_panning = false;
    }
}

void Viewport::process_zoom() {
    float wheel = ImGui::GetIO().MouseWheel;

    float zoom_accel = wheel;
    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || _state.current_tool == core::Tool::ZOOM) {
        float drag_y = _state.mouse_state.is_pressed *
                       (_state.mouse_state.current_pos.y - _state.mouse_state.start_pos.y);

        // TODO: Make the drag y factor a variable
        if (std::fabs(drag_y * 0.01f) > std::fabs(wheel)) {
            zoom_accel = drag_y * 0.01f;
        }
    }

    if (zoom_accel != 0.0f) {
        _state.zoom_state.target_scale += zoom_accel * _state.zoom_state.zoom_speed;
        _state.zoom_state.target_scale = std::clamp(_state.zoom_state.target_scale, 0.1f, 10.0f);
    }
}

void Viewport::process_panning() {
    if (_state.mouse_state.is_pressed &&
        (_state.current_tool == core::Tool::PAN || ImGui::IsKeyDown(ImGuiKey_LeftShift))) {
        ImVec2 delta = {(_state.mouse_state.current_pos.x - _state.mouse_state.start_pos.x) /
                            _state.zoom_state.current_scale,
                        (_state.mouse_state.current_pos.y - _state.mouse_state.start_pos.y) /
                            _state.zoom_state.current_scale};

        _state.pan_state.target_offset.x += delta.x;
        _state.pan_state.target_offset.y += delta.y;

        _state.mouse_state.start_pos = _state.mouse_state.current_pos;
        _state.mouse_state.is_panning = true;
    } else {
        _state.mouse_state.is_panning = false;
    }
}

void Viewport::update_zoom() {
    _state.zoom_state.current_scale =
        utils::maths::lerp(_state.zoom_state.current_scale, _state.zoom_state.target_scale, 0.1f);
}

void Viewport::update_pan() {
    _state.pan_state.current_offset =
        utils::maths::lerp(_state.pan_state.current_offset, _state.pan_state.target_offset, 0.1f);
}

void Viewport::process_selection() {
    float x0 = (_state.mouse_state.start_pos.x / _state.zoom_state.current_scale) -
               _state.pan_state.current_offset.x;
    float y0 = (_state.mouse_state.start_pos.y / _state.zoom_state.current_scale) -
               _state.pan_state.current_offset.y;
    float x1 = (_state.mouse_state.current_pos.x / _state.zoom_state.current_scale) -
               _state.pan_state.current_offset.x;
    float y1 = (_state.mouse_state.current_pos.y / _state.zoom_state.current_scale) -
               _state.pan_state.current_offset.y;

    SDL_Rect selection_world_rect = {
        static_cast<int>(std::min(x0, x1)), static_cast<int>(std::min(y0, y1)),
        static_cast<int>(std::abs(x1 - x0)), static_cast<int>(std::abs(y1 - y0))};

    switch (_state.current_tool) {
        case core::Tool::EXTRACT: {
            if (!_state.texture_sprite.texture()) return;

            commands::FrameExtractionCommand command(
                selection_world_rect, _state.texture_sprite.texture(), _state.frames);
            command.execute();
        } break;
        case core::Tool::SELECT: {
            _state.selected_frames.clear();

            for (size_t i = 0; i < _state.frames.size(); ++i) {
                const SDL_Rect& frame = _state.frames[i];
                if (SDL_HasIntersection(&frame, &selection_world_rect)) {
                    _state.selected_frames.insert(i);
                }
            }
        } break;
        default:
            break;
    }
}

void Viewport::render_texture() {
    if (_state.texture_sprite.texture() != nullptr) {
        _state.texture_sprite.render(_renderer.get(), _state.zoom_state.current_scale,
                                     _state.pan_state.current_offset.x,
                                     _state.pan_state.current_offset.y);
    } else {
        render_placeholder_text();
    }
}

void Viewport::render_cursor_hud() {
    ImVec2 mouse_pos = ImGui::GetMousePos();

    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
        std::fabs(_state.zoom_state.target_scale - _state.zoom_state.current_scale) >= 0.001f) {
        ImVec2 text_offset(5, -10);
        ImVec2 text_pos = ImVec2(mouse_pos.x + text_offset.x, mouse_pos.y + text_offset.y);
        ImGui::SetCursorScreenPos(text_pos);
        ImGui::Text("%s %.2f", ICON_FA_SEARCH, _state.zoom_state.current_scale);
    }

    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
        (std::fabs(_state.pan_state.current_offset.x - _state.pan_state.target_offset.x) >=
             0.001f &&
         std::fabs(_state.pan_state.current_offset.y - _state.pan_state.target_offset.y) >=
             0.001f)) {
        ImVec2 text_offset(7, 10);
        ImVec2 text_pos = ImVec2(mouse_pos.x + text_offset.x, mouse_pos.y + text_offset.y);
        ImGui::SetCursorScreenPos(text_pos);
        ImGui::Text("%s x: %.2f, y: %.2f", ICON_FA_ARROWS, _state.pan_state.current_offset.x,
                    _state.pan_state.current_offset.y);
    }
}

void Viewport::render_placeholder_text() {
    auto font =
        _resource_manager.get_font(std::string(RESOURCE_DIR) + "/fonts/PixelifySans-Regular.ttf");
    if (font != nullptr) {
        const char* placeholder_text = "No texture loaded. Please insert a texture.";
        SDL_Color text_color{255, 255, 255, 255};
        SDL_Surface* text_surface =
            TTF_RenderText_Blended(font.get()->get(), placeholder_text, text_color);
        if (text_surface != nullptr) {
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(_renderer.get(), text_surface);
            if (text_texture != nullptr) {
                int text_width = text_surface->w;
                int text_height = text_surface->h;
                SDL_Rect dest_rect = {static_cast<int>((_viewport_size.x - text_width) / 2),
                                      static_cast<int>((_viewport_size.y - text_height) / 2),
                                      text_width, text_height};
                SDL_RenderCopy(_renderer.get(), text_texture, nullptr, &dest_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

void Viewport::render_selection_rect() {
    int start_x = static_cast<int>(_state.mouse_state.start_pos.x);
    int start_y = static_cast<int>(_state.mouse_state.start_pos.y);
    int current_x = static_cast<int>(_state.mouse_state.current_pos.x);
    int current_y = static_cast<int>(_state.mouse_state.current_pos.y);

    _selection_rect = {std::min(start_x, current_x), std::min(start_y, current_y),
                       std::abs(current_x - start_x), std::abs(current_y - start_y)};

    SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 255);
    SDL_RenderDrawRect(_renderer.get(), &_selection_rect);
    SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 25);
    SDL_RenderFillRect(_renderer.get(), &_selection_rect);
}

void Viewport::render_grid_background() {
    SDL_SetRenderDrawColor(_renderer.get(), 33, 33, 33, 155);

    float scaled_grid_cell_size =
        std::max(_state.viewport_state.grid_cell_size * _state.zoom_state.current_scale, 1.0f);

    float offset_x = fmod(_state.pan_state.current_offset.x * _state.zoom_state.current_scale,
                          scaled_grid_cell_size);
    float offset_y = fmod(_state.pan_state.current_offset.y * _state.zoom_state.current_scale,
                          scaled_grid_cell_size);

    int num_vertical_lines =
        static_cast<int>(std::ceil(_viewport_size.x / scaled_grid_cell_size)) + 1;
    int num_horizontal_lines =
        static_cast<int>(std::ceil(_viewport_size.y / scaled_grid_cell_size)) + 1;

    for (int i = 0; i < num_vertical_lines; ++i) {
        float x = offset_x + i * scaled_grid_cell_size;
        if (x >= 0 && x <= _viewport_size.x) {
            SDL_RenderDrawLine(_renderer.get(), static_cast<int>(x), 0, static_cast<int>(x),
                               static_cast<int>(_viewport_size.y));
        }
    }

    for (int j = 0; j < num_horizontal_lines; ++j) {
        float y = offset_y + j * scaled_grid_cell_size;
        if (y >= 0 && y <= _viewport_size.y) {
            SDL_RenderDrawLine(_renderer.get(), 0, static_cast<int>(y),
                               static_cast<int>(_viewport_size.x), static_cast<int>(y));
        }
    }
}

void Viewport::render_frames() const {
    // Pre-compute colors to reduce redundant operations inside the loop
    const SDL_Color current_frame_color{206, 2, 65, 155};
    const SDL_Color selected_frame_color{255, 135, 177, 155};
    const SDL_Color default_frame_color{135, 235, 177, 155};

    // Pre-compute common scaling factor for efficiency
    const float scale = _state.zoom_state.current_scale;
    const ImVec2 offset = _state.pan_state.current_offset;

    // Render loop with reduced computation
    for (size_t i = 0; i < _state.frames.size(); ++i) {
        const SDL_Rect& frame = _state.frames[i];
        SDL_Rect render_frame_rect{static_cast<int>((frame.x + offset.x) * scale),
                                   static_cast<int>((frame.y + offset.y) * scale),
                                   static_cast<int>(frame.w * scale),
                                   static_cast<int>(frame.h * scale)};

        // Set color based on the current context
        if (i == _state.animation_state.current_frame) {
            SDL_SetRenderDrawColor(_renderer.get(), current_frame_color.r, current_frame_color.g,
                                   current_frame_color.b, current_frame_color.a);
        } else if (_state.selected_frames.count(i)) {
            SDL_SetRenderDrawColor(_renderer.get(), selected_frame_color.r, selected_frame_color.g,
                                   selected_frame_color.b, selected_frame_color.a);
        } else {
            SDL_SetRenderDrawColor(_renderer.get(), default_frame_color.r, default_frame_color.g,
                                   default_frame_color.b, default_frame_color.a);
        }

        SDL_RenderDrawRect(_renderer.get(), &render_frame_rect);
    }
}

void Viewport::handle_click(float x, float y) {
    float world_x = (x / _state.zoom_state.current_scale) - _state.pan_state.current_offset.x;
    float world_y = (y / _state.zoom_state.current_scale) - _state.pan_state.current_offset.y;

    auto& sprite = _state.texture_sprite;
    SDL_Rect rect = sprite.rect();

    float texture_x = world_x - rect.x;
    float texture_y = world_y - rect.y;

    if (texture_x >= 0 && texture_x < rect.w && texture_y >= 0 && texture_y < rect.h) {
        switch (_state.current_tool) {
            case core::Tool::SELECT:
                sprite.set_selected(true);
                break;
            case core::Tool::COLOR_SWAP: {
                SDL_Color pixel_color = get_texture_pixel_color(
                    static_cast<int>(texture_x), static_cast<int>(texture_y), sprite);
                commands::SwapTextureCommand command(
                    pixel_color,
                    SDL_Color{
                        static_cast<Uint8>(_state.replacement_color[0] * 255),
                        static_cast<Uint8>(_state.replacement_color[1] * 255),
                        static_cast<Uint8>(_state.replacement_color[2] * 255),
                        static_cast<Uint8>(_state.replacement_color[3] * 255),
                    },
                    _state.texture_sprite.texture());
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImVec4(0.1f, 0.1f, 0.1f, 0.5f));  // Adjust color and transparency
    ImGui::SetNextWindowBgAlpha(0.5f);  // Set background transparency for the window

    // Create a dockable toolbar with offset
    ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_FirstUseEver);    // Offset from the border
    ImGui::SetNextWindowSize(ImVec2(300, 50), ImGuiCond_FirstUseEver);  // Initial size
    ImGui::Begin("Toolbar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

    // Begin a horizontal layout
    ImGui::BeginGroup();

    // Render the toolbar buttons
    int num_tools = static_cast<int>(core::Tool::COUNT);
    for (int tool_idx = 0; tool_idx < num_tools; tool_idx++) {
        core::Tool tool = static_cast<core::Tool>(tool_idx);
        const char* label = core::tool_to_string(tool);
        const char* icon = core::tool_to_icon(tool);

        if (tool_idx > 0) ImGui::SameLine();

        if (tool == _state.current_tool) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 125 / 255.0f, 155 / 255.0f, 0.73f));
            if (ImGui::Button((std::string(icon) + " " + label).c_str())) {
                _state.current_tool = tool;
            }
            ImGui::PopStyleColor();
        } else {
            if (ImGui::Button(label)) {
                _state.current_tool = tool;
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
