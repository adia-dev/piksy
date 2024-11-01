#include "viewport.hpp"
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

  // Reset the render target to the default (window)
  SDL_SetRenderTarget(renderer, nullptr);

  // Display the render texture in ImGui
  ImGui::Image((ImTextureID)(intptr_t)_render_texture, _viewport_size);

  // Detect clicks on the viewport
  if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 imagePos = ImGui::GetItemRectMin();
    ImVec2 relativePos =
        ImVec2(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

    // Convert to viewport coordinates if necessary
    float viewportX = relativePos.x;
    float viewportY = relativePos.y;

    // Handle the click event (e.g., select a sprite)
    handle_viewport_click(viewportX, viewportY);
  }

  ImGui::End();
}

void Viewport::handle_viewport_click(float x, float y) {
  // Iterate over sprites to check if the click is within any sprite
  for (Sprite &sprite : State::sprites()) {
    SDL_Rect rect = sprite.rect();
    if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y &&
        y <= rect.y + rect.h) {
      // Sprite was clicked
      sprite.set_selected(true);
      // Perform additional actions as needed
    } else {
      sprite.set_selected(false);
    }
  }
}

} // namespace piksy
