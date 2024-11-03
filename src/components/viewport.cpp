#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <imgui.h>

#include <components/viewport.hpp>
#include <core/state.hpp>
#include <utils/dump.hpp>

namespace piksy {
namespace components {

Viewport::Viewport(rendering::Renderer& renderer)
    : _renderer(renderer), _render_texture(nullptr), _viewport_size(800, 600) {
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

    _render_texture = SDL_CreateTexture(_renderer.mutable_get(), SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, width, height);

    if (!_render_texture) {
        throw std::runtime_error("Failed to create render texture: " + std::string(SDL_GetError()));
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

    // Update zoom and pan
    update_zoom();
    update_pan();

    SDL_SetRenderTarget(_renderer.mutable_get(), _render_texture);

    SDL_SetRenderDrawColor(_renderer.mutable_get(), 0, 0, 0, 255);
    SDL_RenderClear(_renderer.mutable_get());

    render_grid_background();

    if (state.texture_sprite.texture() != nullptr) {
        state.texture_sprite.render(_renderer.mutable_get(), _zoom_state.current_scale,
                                    _pan_state.current_offset.x, _pan_state.current_offset.y);
    }

    if (_mouse_state.is_pressed && !_mouse_state.is_panning) {
        render_selection_rect();
    }

    SDL_SetRenderTarget(_renderer.mutable_get(), nullptr);

    ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

    // Handle mouse input
    handle_mouse_input(state);

    ImGui::End();
}

void Viewport::handle_mouse_input(core::State& state) {
    if (ImGui::IsItemHovered()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 relativePos = {mousePos.x - imagePos.x, mousePos.y - imagePos.y};
        _mouse_state.current_pos = relativePos;

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
        ImVec2 delta = {_mouse_state.current_pos.x - _mouse_state.start_pos.x,
                        _mouse_state.current_pos.y - _mouse_state.start_pos.y};

        _pan_state.target_offset.x += delta.x * _pan_state.pan_speed / _zoom_state.current_scale;
        _pan_state.target_offset.y += delta.y * _pan_state.pan_speed / _zoom_state.current_scale;

        _mouse_state.start_pos = _mouse_state.current_pos;
        _mouse_state.is_panning = true;
    } else {
        _mouse_state.is_panning = false;
    }
}

void Viewport::update_zoom() {
    _zoom_state.current_scale = lerp(_zoom_state.current_scale, _zoom_state.target_scale, 0.1f);
}

void Viewport::update_pan() {
    _pan_state.current_offset = lerp(_pan_state.current_offset, _pan_state.target_offset, 0.1f);
}

void Viewport::render_selection_rect() {
    int start_x = static_cast<int>(_mouse_state.start_pos.x);
    int start_y = static_cast<int>(_mouse_state.start_pos.y);
    int current_x = static_cast<int>(_mouse_state.current_pos.x);
    int current_y = static_cast<int>(_mouse_state.current_pos.y);

    _selection_rect = {std::min(start_x, current_x), std::min(start_y, current_y),
                       std::abs(current_x - start_x), std::abs(current_y - start_y)};

    SDL_SetRenderDrawColor(_renderer.mutable_get(), 255, 255, 0, 255);
    SDL_RenderDrawRect(_renderer.mutable_get(), &_selection_rect);

    SDL_SetRenderDrawColor(_renderer.mutable_get(), 255, 255, 0, 25);
    SDL_RenderFillRect(_renderer.mutable_get(), &_selection_rect);
}

void Viewport::handle_viewport_click(float x, float y, core::State& state) {
    // Map screen coordinates to world coordinates
    float world_x = (x - _pan_state.current_offset.x) / _zoom_state.current_scale;
    float world_y = (y - _pan_state.current_offset.y) / _zoom_state.current_scale;

    SDL_Color pixel_color = get_pixel_color(static_cast<int>(world_x), static_cast<int>(world_y));
    printf("Clicked on a pixel of color (%u, %u, %u, %u)\n", pixel_color.r, pixel_color.g,
           pixel_color.b, pixel_color.a);

    auto& sprite = state.texture_sprite;
    SDL_Rect rect = sprite.rect();
    if (world_x >= rect.x && world_x <= rect.x + rect.w && world_y >= rect.y &&
        world_y <= rect.y + rect.h) {
        sprite.set_selected(true);
    } else {
        sprite.set_selected(false);
    }
}

SDL_Color Viewport::get_pixel_color(int x, int y) {
    SDL_Rect pixel_rect{static_cast<int>(x), static_cast<int>(y), 1, 1};
    Uint32 pixel;
    SDL_Color pixel_color = SDL_Color{0, 0, 0, 0};

    SDL_SetRenderTarget(_renderer.mutable_get(), _render_texture);
    if (SDL_RenderReadPixels(_renderer.mutable_get(), &pixel_rect, SDL_PIXELFORMAT_RGBA8888, &pixel,
                             sizeof(pixel)) < 0) {
        SDL_Log("SDL_RenderReadPixels failed: %s", SDL_GetError());
        return pixel_color;
    }

    SDL_GetRGBA(pixel, SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), &pixel_color.r, &pixel_color.g,
                &pixel_color.b, &pixel_color.a);

    SDL_SetRenderTarget(_renderer.mutable_get(), nullptr);

    return pixel_color;
}

void Viewport::render_grid_background() {
    SDL_SetRenderDrawColor(_renderer.mutable_get(), 33, 33, 33, 155);

    float scaled_grid_cell_size = std::max(_grid_cell_size * _zoom_state.current_scale, 1.0f);

    float start_x =
        fmod(-_pan_state.current_offset.x * _zoom_state.current_scale, scaled_grid_cell_size);
    float start_y =
        fmod(-_pan_state.current_offset.y * _zoom_state.current_scale, scaled_grid_cell_size);

    int num_vertical_lines =
        static_cast<int>(std::ceil(_viewport_size.x / scaled_grid_cell_size)) + 1;
    int num_horizontal_lines =
        static_cast<int>(std::ceil(_viewport_size.y / scaled_grid_cell_size)) + 1;

    for (int i = 0; i < num_vertical_lines; ++i) {
        float x = start_x + i * scaled_grid_cell_size;
        if (x >= 0 && x <= _viewport_size.x) {
            SDL_RenderDrawLine(_renderer.mutable_get(), static_cast<int>(x), 0, static_cast<int>(x),
                               static_cast<int>(_viewport_size.y));
        }
    }

    for (int j = 0; j < num_horizontal_lines; ++j) {
        float y = start_y + j * scaled_grid_cell_size;
        if (y >= 0 && y <= _viewport_size.y) {
            SDL_RenderDrawLine(_renderer.mutable_get(), 0, static_cast<int>(y),
                               static_cast<int>(_viewport_size.x), static_cast<int>(y));
        }
    }
}

// Lerp utility function
ImVec2 Viewport::lerp(const ImVec2& lhs, const ImVec2& rhs, float t) {
    return {lhs.x + t * (rhs.x - lhs.x), lhs.y + t * (rhs.y - lhs.y)};
}

float Viewport::lerp(float lhs, float rhs, float t) { return lhs + t * (rhs - lhs); }

}  // namespace components
}  // namespace piksy
