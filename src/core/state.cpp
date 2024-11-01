#include <core/state.hpp>

namespace piksy {
State &State::get() {
    static State instance;
    return instance;
}

std::vector<Sprite> &State::sprites() {
    auto &state = get();
    return state._sprites;
}

void State::add_sprite(Sprite &&sprite) {
    auto &state = get();
    state._sprites.emplace_back(std::move(sprite));
}

void State::add_sprite(const Sprite &sprite) {
    auto &state = get();
    state._sprites.push_back(sprite);
}

}  // namespace piksy
