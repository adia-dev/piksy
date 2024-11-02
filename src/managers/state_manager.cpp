#include <iostream>
#include <managers/state_manager.hpp>

namespace piksy {
namespace managers {

StateManager& StateManager::get() {
    static StateManager instance;
    return instance;
}
rendering::Sprite& StateManager::texture_sprite() {
    const auto& state_manager = get();
    return state_manager._state->texture_sprite;
}
void StateManager::init() {
    const auto& state_manager = get();
    state_manager._state->texture_sprite = rendering::Sprite(managers::ResourceManager::get_texture(
        core::Application::renderer().get(), std::string(RESOURCE_DIR) + "/textures/janemba.png"));
}
void StateManager::cleanup() {
    auto& state_manager = get();
    state_manager._state = nullptr;

    std::cout << "State manager cleaned up\n";
}
}  // namespace managers
}  // namespace piksy
