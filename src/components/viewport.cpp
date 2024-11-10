#include <SDL_error.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <imgui.h>

#include <components/viewport.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <exception>
#include <opencv2/opencv.hpp>
#include <rendering/sprite.hpp>
#include <utils/dump.hpp>
#include <utils/maths.hpp>

namespace piksy {
namespace components {

Viewport::Viewport(rendering::Renderer& renderer, managers::ResourceManager& resource_manager)
    : _renderer(renderer),
      _resource_manager(resource_manager),
      _render_texture(nullptr),
      _viewport_size(800, 600) {
    create_render_texture(static_cast<int>(_viewport_size.x), static_cast<int>(_viewport_size.y));
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

void Viewport::update(core::State& state) {
    update_zoom();
    update_pan();

    if (_mouse_state.is_pressed && !_mouse_state.is_panning) {
        process_selection(state);
    }
}

void Viewport::render(core::State& state) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    if (viewport_size.x != _viewport_size.x || viewport_size.y != _viewport_size.y) {
        _viewport_size = viewport_size;
        create_render_texture(static_cast<int>(_viewport_size.x),
                              static_cast<int>(_viewport_size.y));
    }

    SDL_SetRenderTarget(_renderer.get(), _render_texture);

    SDL_SetRenderDrawColor(_renderer.get(), 0, 0, 0, 255);
    SDL_RenderClear(_renderer.get());

    render_grid_background(state);
    render_texture(state);

    if (_mouse_state.is_pressed && !_mouse_state.is_panning) {
        render_selection_rect();
    }

    render_frames(state.frames);

    SDL_SetRenderTarget(_renderer.get(), nullptr);

    // Display the rendered texture
    ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

    // Handle mouse input here, after ImGui::Begin and before ImGui::End
    handle_mouse_input(state);

    ImGui::End();
}

void Viewport::notify_dropped_file(core::State& state, const std::string& dropped_file_path) {
    core::Logger::debug("Dropped file into the Viewport: %s", dropped_file_path.c_str());
    try {
        state.texture_sprite.set_texture(_resource_manager.get_texture(dropped_file_path));
    } catch (const std::runtime_error& ex) {
        core::Logger::error("Failed to select a texture in the project: %s", ex.what());
    }
}

// Processing methods

void Viewport::handle_mouse_input(core::State& state) {
    if (ImGui::IsWindowHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 image_pos = ImGui::GetItemRectMin();
        ImVec2 relative_pos = {mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y};
        _mouse_state.current_pos = relative_pos;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _mouse_state.start_pos = _mouse_state.current_pos;
            _mouse_state.is_pressed = true;
            handle_viewport_click(_mouse_state.current_pos.x, _mouse_state.current_pos.y, state);
        }

        _mouse_state.is_pressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);

        process_zoom();
        process_panning();
    } else {
        _mouse_state.is_pressed = false;
        _mouse_state.is_panning = false;
    }
}

void Viewport::process_zoom() {
    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f) {
        _zoom_state.target_scale += wheel * _zoom_state.zoom_speed;
        _zoom_state.target_scale = std::clamp(_zoom_state.target_scale, 0.1f, 10.0f);
    }
}

void Viewport::process_panning() {
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && _mouse_state.is_pressed) {
        ImVec2 delta = {
            (_mouse_state.current_pos.x - _mouse_state.start_pos.x) / _zoom_state.current_scale,
            (_mouse_state.current_pos.y - _mouse_state.start_pos.y) / _zoom_state.current_scale};

        _pan_state.target_offset.x += delta.x;
        _pan_state.target_offset.y += delta.y;

        _mouse_state.start_pos = _mouse_state.current_pos;
        _mouse_state.is_panning = true;
    } else {
        _mouse_state.is_panning = false;
    }
}

void Viewport::update_zoom() {
    _zoom_state.current_scale =
        utils::maths::lerp(_zoom_state.current_scale, _zoom_state.target_scale, 0.1f);
}

void Viewport::update_pan() {
    _pan_state.current_offset =
        utils::maths::lerp(_pan_state.current_offset, _pan_state.target_offset, 0.1f);
}

void Viewport::process_selection(core::State& state) {
    if (!state.texture_sprite.texture()) return;

    // Convert screen coordinates to world coordinates
    float x0 = (_mouse_state.start_pos.x / _zoom_state.current_scale) - _pan_state.current_offset.x;
    float y0 = (_mouse_state.start_pos.y / _zoom_state.current_scale) - _pan_state.current_offset.y;
    float x1 =
        (_mouse_state.current_pos.x / _zoom_state.current_scale) - _pan_state.current_offset.x;
    float y1 =
        (_mouse_state.current_pos.y / _zoom_state.current_scale) - _pan_state.current_offset.y;

    SDL_Rect selection_world_rect = {
        static_cast<int>(std::min(x0, x1)), static_cast<int>(std::min(y0, y1)),
        static_cast<int>(std::abs(x1 - x0)), static_cast<int>(std::abs(y1 - y0))};

    SDL_Rect texture_rect = state.texture_sprite.rect();

    SDL_Rect intersection_world;
    if (SDL_IntersectRect(&selection_world_rect, &texture_rect, &intersection_world) == SDL_FALSE) {
        return;
    }

    if (intersection_world.w > 0 && intersection_world.h > 0) {
        void* pixels;
        int pitch;
        auto texture = state.texture_sprite.texture();

        if (SDL_LockTexture(texture->get(), &intersection_world, &pixels, &pitch) < 0) {
            core::Logger::error("Failed to lock the texture: %s", SDL_GetError());
            return;
        }

        try {
            cv::Mat mat(intersection_world.h, intersection_world.w, CV_8UC4, pixels, pitch);

            cv::Mat mat_gray;
            cv::cvtColor(mat, mat_gray, cv::COLOR_RGBA2GRAY);

            int threshold_value = 1;
            cv::Mat thresholded;
            cv::threshold(mat_gray, thresholded, threshold_value, 255, cv::THRESH_BINARY);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(thresholded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            state.frames.clear();
            for (const auto& contour : contours) {
                cv::Rect bounding_rect = cv::boundingRect(contour);

                SDL_Rect frame_rect{bounding_rect.x + intersection_world.x,
                                    bounding_rect.y + intersection_world.y, bounding_rect.width,
                                    bounding_rect.height};

                state.frames.push_back(frame_rect);
            }
        } catch (const std::exception& ex) {
            core::Logger::error("Failed to process the selected area: %s", ex.what());
        }

        SDL_UnlockTexture(texture->get());
    }
}

// Rendering methods

void Viewport::render_texture(core::State& state) {
    if (state.texture_sprite.texture() != nullptr) {
        state.texture_sprite.render(_renderer.get(), _zoom_state.current_scale,
                                    _pan_state.current_offset.x, _pan_state.current_offset.y);
    } else {
        render_placeholder_text();
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
    // Render selection rectangle in screen coordinates
    int start_x = static_cast<int>(_mouse_state.start_pos.x);
    int start_y = static_cast<int>(_mouse_state.start_pos.y);
    int current_x = static_cast<int>(_mouse_state.current_pos.x);
    int current_y = static_cast<int>(_mouse_state.current_pos.y);

    _selection_rect = {std::min(start_x, current_x), std::min(start_y, current_y),
                       std::abs(current_x - start_x), std::abs(current_y - start_y)};

    SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 255);
    SDL_RenderDrawRect(_renderer.get(), &_selection_rect);
    SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 25);
    SDL_RenderFillRect(_renderer.get(), &_selection_rect);
}

void Viewport::render_grid_background(core::State& state) {
    SDL_SetRenderDrawColor(_renderer.get(), 33, 33, 33, 155);

    float scaled_grid_cell_size =
        std::max(state.viewport_grid_cell_size * _zoom_state.current_scale, 1.0f);

    float offset_x =
        fmod(_pan_state.current_offset.x * _zoom_state.current_scale, scaled_grid_cell_size);
    float offset_y =
        fmod(_pan_state.current_offset.y * _zoom_state.current_scale, scaled_grid_cell_size);

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

void Viewport::render_frames(const std::vector<SDL_Rect>& frames) const {
    SDL_SetRenderDrawColor(_renderer.get(), 0, 255, 0, 255);

    for (const SDL_Rect& frame : frames) {
        // Transform frame coordinates from world space to screen space
        SDL_Rect render_frame_rect{
            static_cast<int>((frame.x + _pan_state.current_offset.x) * _zoom_state.current_scale),
            static_cast<int>((frame.y + _pan_state.current_offset.y) * _zoom_state.current_scale),
            static_cast<int>(frame.w * _zoom_state.current_scale),
            static_cast<int>(frame.h * _zoom_state.current_scale),
        };
        SDL_RenderDrawRect(_renderer.get(), &render_frame_rect);
    }
}

// Utility methods

void Viewport::handle_viewport_click(float x, float y, core::State& state) {
    float world_x = (x / _zoom_state.current_scale) - _pan_state.current_offset.x;
    float world_y = (y / _zoom_state.current_scale) - _pan_state.current_offset.y;

    auto& sprite = state.texture_sprite;
    SDL_Rect rect = sprite.rect();

    float texture_x = world_x - rect.x;
    float texture_y = world_y - rect.y;

    if (texture_x >= 0 && texture_x < rect.w && texture_y >= 0 && texture_y < rect.h) {
        sprite.set_selected(true);

        SDL_Color pixel_color = get_texture_pixel_color(static_cast<int>(texture_x),
                                                        static_cast<int>(texture_y), sprite);

        if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && !_mouse_state.is_panning) {
            swap_texture_color(pixel_color,
                               SDL_Color{
                                   static_cast<Uint8>(state.replacement_color[0] * 255),
                                   static_cast<Uint8>(state.replacement_color[1] * 255),
                                   static_cast<Uint8>(state.replacement_color[2] * 255),
                                   static_cast<Uint8>(state.replacement_color[3] * 255),
                               },
                               state);
        }
    } else {
        sprite.set_selected(false);
    }
}

void Viewport::swap_texture_color(const SDL_Color& from, const SDL_Color& to, core::State& state) {
    auto texture = state.texture_sprite.texture();
    void* pixels;
    int pitch;

    if (SDL_LockTexture(state.texture_sprite.texture()->get(), nullptr, &pixels, &pitch) < 0) {
        core::Logger::error("Failed to lock the texture to remove the background: %s",
                            SDL_GetError());
        return;
    }

    Uint32 format;
    SDL_QueryTexture(texture->get(), &format, nullptr, nullptr, nullptr);
    SDL_PixelFormat* pixel_format = SDL_AllocFormat(format);
    Uint32* pixel_data = static_cast<Uint32*>(pixels);

    Uint32 to_u32 = SDL_MapRGBA(pixel_format, to.r, to.g, to.b, to.a);
    int num_replaced = 0;
    for (int y = 0; y < texture->height(); ++y) {
        for (int x = 0; x < texture->width(); ++x) {
            Uint32* current_pixel = pixel_data + y * (pitch / 4) + x;
            Uint8 pr, pg, pb, pa;
            SDL_GetRGBA(*current_pixel, pixel_format, &pr, &pg, &pb, &pa);
            if (pr == from.r && pg == from.g && pb == from.b && pa == from.a) {
                *current_pixel = to_u32;
                ++num_replaced;
            }
        }
    }

    core::Logger::debug("Number of pixels replaced: %d", num_replaced);
    core::Logger::info("Replaced the color (%d, %d, %d, %d) with the color (%d, %d, %d, %d)",
                       from.r, from.g, from.b, from.a, to.r, to.g, to.b, to.a);

    SDL_UnlockTexture(state.texture_sprite.texture()->get());
    SDL_FreeFormat(pixel_format);
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

    Uint8* pixel_ptr =
        static_cast<Uint8*>(pixels) + y * pitch + x * 4;  // Assuming 4 bytes per pixel
    Uint32 pixel_value = *(reinterpret_cast<Uint32*>(pixel_ptr));

    SDL_Color color;
    SDL_GetRGBA(pixel_value, pixel_format, &color.r, &color.g, &color.b, &color.a);

    SDL_UnlockTexture(texture);
    SDL_FreeFormat(pixel_format);

    return color;
}

}  // namespace components
}  // namespace piksy
