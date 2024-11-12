#include <rendering/sprite.hpp>
#include <stdexcept>

namespace piksy {

namespace rendering {
Sprite::Sprite() : m_texture(nullptr), m_rect({}), m_frame_rect({}), m_selected(false) {}

Sprite::Sprite(std::shared_ptr<Texture2D> texture, const SDL_Rect &rect, const SDL_Rect &frame_rect)
    : m_texture(texture), m_rect(rect), m_frame_rect(frame_rect) {
    if (texture == nullptr) return;

    if (m_frame_rect.w == 0) {
        m_frame_rect.w = texture->width();
    }

    if (m_rect.w == 0) {
        m_rect.w = texture->width();
    }

    if (m_frame_rect.h == 0) {
        m_frame_rect.h = texture->height();
    }

    if (m_rect.h == 0) {
        m_rect.h = texture->height();
    }
}

void Sprite::set_size(int w, int h) {
    m_rect.w = w;
    m_rect.h = h;
}

void Sprite::set_position(int x, int y) {
    m_rect.x = x;
    m_rect.y = y;
}

void Sprite::set_selected(bool selected) { m_selected = selected; }

bool Sprite::is_selected() const { return m_selected; }

void Sprite::set_texture(std::shared_ptr<Texture2D> texture) {
    m_texture = texture;

    if (texture == nullptr) return;

    m_frame_rect.w = texture->width();
    m_frame_rect.h = texture->height();

    m_rect.w = texture->width();
    m_rect.h = texture->height();
}

std::shared_ptr<Texture2D> Sprite::texture() const { return m_texture; }

void Sprite::set_frame_rect(const SDL_Rect &frame_rect) { m_frame_rect = frame_rect; }

int Sprite::x() const { return m_rect.x; };

int Sprite::y() const { return m_rect.y; };

int Sprite::width() const { return m_rect.w; }

int Sprite::height() const { return m_rect.h; }

const SDL_Rect &Sprite::frame_rect() const { return m_frame_rect; }

const SDL_Rect &Sprite::rect() const { return m_rect; }

void Sprite::move(int dx, int dy) {
    m_rect.x += dx;
    m_rect.y += dy;
}

void Sprite::render(SDL_Renderer *renderer, float scale, float offset_x, int offset_y) const {
    if (renderer == nullptr) {
        throw std::runtime_error("Cannot render a sprite if the renderer is null.");
    }
    if (m_texture == nullptr) {
        throw std::runtime_error("Cannot render a sprite if the texture is null.");
    }

    SDL_Rect scaled_rect = {static_cast<int>((m_rect.x + offset_x) * scale),
                            static_cast<int>((m_rect.y + offset_y) * scale),
                            static_cast<int>(m_rect.w * scale), static_cast<int>(m_rect.h * scale)};

    SDL_RenderCopy(renderer, m_texture->get(), &m_frame_rect, &scaled_rect);

    if (m_selected) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &scaled_rect);
    }
}
}  // namespace rendering
}  // namespace piksy
