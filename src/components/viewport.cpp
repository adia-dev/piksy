#include <SDL_render.h>
#include <SDL_stdinc.h>

#include <components/viewport.hpp>
#include <core/state.hpp>

#include "core/application.hpp"

namespace piksy {
namespace components {

Viewport::Viewport(std::shared_ptr<SDL_Renderer> renderer)
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

    _render_texture = SDL_CreateTexture(_renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, width, height);

    if (!_render_texture) {
        throw std::runtime_error("Failed to create render texture: " + std::string(SDL_GetError()));
    }
}

void Viewport::render(SDL_Renderer* renderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != _viewport_size.x || viewportSize.y != _viewport_size.y) {
        _viewport_size = viewportSize;
        create_render_texture(static_cast<int>(_viewport_size.x),
                              static_cast<int>(_viewport_size.y));
    }

    SDL_SetRenderTarget(renderer, _render_texture);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    core::Application::get().state().texture_sprite.render(renderer);

    if (_is_dragging) {
        render_selection_rect();
    }

    SDL_SetRenderTarget(renderer, nullptr);

    ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

    if (ImGui::IsItemHovered()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 relativePos = ImVec2(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

        float viewportX = relativePos.x;
        float viewportY = relativePos.y;

        _mouse_position = {viewportX, viewportY};

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _start_dragging_position = _mouse_position;
            _is_dragging = true;
            handle_viewport_click(viewportX, viewportY);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            _is_dragging = false;
        }
    } else {
        _is_dragging = false;
    }

    ImGui::End();
}

void Viewport::render_selection_rect() {
    _selection_rect = {
        static_cast<int>(_start_dragging_position.x),
        static_cast<int>(_start_dragging_position.y),
        std::abs(static_cast<int>(_mouse_position.x - _start_dragging_position.x)),
        std::abs(static_cast<int>(_mouse_position.y - _start_dragging_position.y)),
    };

    if (_start_dragging_position.x > _mouse_position.x) {
        _selection_rect.x = _mouse_position.x;
    }
    if (_start_dragging_position.y > _mouse_position.y) {
        _selection_rect.y = _mouse_position.y;
    }

    SDL_SetRenderDrawColor(_renderer.get(), 255, 0, 0, 255);
    SDL_RenderDrawRect(_renderer.get(), &_selection_rect);
}

void Viewport::handle_viewport_click(float x, float y) {
    {
        SDL_Color pixel_color = get_pixel_color(static_cast<int>(x), static_cast<int>(y));

        printf("Clicked on a pixel of color (%u, %u, %u, %u)\n", pixel_color.r, pixel_color.g,
               pixel_color.b, pixel_color.a);
    }

    auto& sprite = core::Application::get().mutable_state().texture_sprite;
    SDL_Rect rect = sprite.rect();
    if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h) {
        sprite.set_selected(true);
    } else {
        sprite.set_selected(false);
    }
}

SDL_Color Viewport::get_pixel_color(int x, int y) {
    SDL_Rect pixel_rect{static_cast<int>(x), static_cast<int>(y), 1, 1};
    Uint32 pixel;
    SDL_Color pixel_color = SDL_Color{0, 0, 0, 0};

    SDL_SetRenderTarget(_renderer.get(), _render_texture);
    if (SDL_RenderReadPixels(_renderer.get(), &pixel_rect, SDL_PIXELFORMAT_RGBA8888, &pixel,
                             sizeof(pixel)) < 0) {
        SDL_Log("SDL_RenderReadPixels failed: %s", SDL_GetError());
        return pixel_color;
    }

    SDL_GetRGBA(pixel, SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), &pixel_color.r, &pixel_color.g,
                &pixel_color.b, &pixel_color.a);

    SDL_SetRenderTarget(_renderer.get(), nullptr);

    return pixel_color;
}

}  // namespace components
}  // namespace piksy
