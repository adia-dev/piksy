#include <core/state.hpp>

namespace piksy {
namespace core {

State &State::get() {
    static State instance;
    return instance;
}

std::vector<rendering::Sprite> &State::sprites() {
    auto &state = get();
    return state._sprites;
}

void State::add_sprite(rendering::Sprite &&sprite) {
    auto &state = get();
    state._sprites.emplace_back(std::move(sprite));
}

void State::add_sprite(const rendering::Sprite &sprite) {
    auto &state = get();
    state._sprites.push_back(sprite);
}

}  // namespace core
}  // namespace piksy
