#include "viewport.hpp"
#include "SDL_render.h"
#include <state/state.hpp>
#include <stdexcept>

namespace piksy {

Viewport::Viewport(SDL_Renderer *renderer)
    : _render_texture(nullptr), _viewport_size(800, 600) {
  create_render_texture(renderer, static_cast<int>(_viewport_size.x),
                        static_cast<int>(_viewport_size.y));
}

Viewport::~Viewport() {
  if (_render_texture) {
    SDL_DestroyTexture(_render_texture);
    _render_texture = nullptr;
  }
}

void Viewport::create_render_texture(SDL_Renderer *renderer, int width,
                                     int height) {
  if (_render_texture) {
    SDL_DestroyTexture(_render_texture);
    _render_texture = nullptr;
  }

  _render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET, width, height);

  if (!_render_texture) {
    throw std::runtime_error("Failed to create render texture: " +
                             std::string(SDL_GetError()));
  }
}

void Viewport::render(SDL_Renderer *renderer) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(0, 0)); // Remove padding
  ImGui::Begin("Viewport", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::PopStyleVar();

  // Get the size of the viewport
  ImVec2 viewportSize = ImGui::GetContentRegionAvail();
  if (viewportSize.x != _viewport_size.x ||
      viewportSize.y != _viewport_size.y) {
    _viewport_size = viewportSize;
    create_render_texture(renderer, static_cast<int>(_viewport_size.x),
                          static_cast<int>(_viewport_size.y));
  }

  // Set the render target to the texture
  SDL_SetRenderTarget(renderer, _render_texture);

  // Clear the render target
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // Render sprites to the render texture
  for (const Sprite &sprite : State::sprites()) {
    sprite.render(renderer);
  }

  if (_is_dragging) {
    render_selection_rect(renderer);
  }

  // Reset the render target to the default (window)
  SDL_SetRenderTarget(renderer, nullptr);

  // Display the render texture in ImGui
  ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

  // Detect clicks on the viewport
  if (ImGui::IsItemHovered()) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 imagePos = ImGui::GetItemRectMin();
    ImVec2 relativePos =
        ImVec2(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

    // Convert to viewport coordinates if necessary
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

void Viewport::render_selection_rect(SDL_Renderer *renderer) {
  _selection_rect = {
      static_cast<int>(_start_dragging_position.x),
      static_cast<int>(_start_dragging_position.y),
      std::abs(
          static_cast<int>(_mouse_position.x - _start_dragging_position.x)),
      std::abs(
          static_cast<int>(_mouse_position.y - _start_dragging_position.y)),
  };

  if (_start_dragging_position.x > _mouse_position.x) {
    _selection_rect.x = _mouse_position.x;
  }
  if (_start_dragging_position.y > _mouse_position.y) {
    _selection_rect.y = _mouse_position.y;
  }

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderDrawRect(renderer, &_selection_rect);
}

void Viewport::handle_viewport_click(float x, float y) {
  for (Sprite &sprite : State::sprites()) {
    SDL_Rect rect = sprite.rect();
    if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y &&
        y <= rect.y + rect.h) {
      sprite.set_selected(true);
    } else {
      sprite.set_selected(false);
    }
  }
}

} // namespace piksy
