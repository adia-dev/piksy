#pragma once
#include <core/application.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <memory>
#include <rendering/sprite.hpp>

namespace piksy {
namespace managers {
class StateManager {
   public:
    static StateManager& get();

    static rendering::Sprite& texture_sprite();
    static void init();
    static void cleanup();

   private:
    StateManager() { _state = std::make_shared<core::State>(); }
    std::shared_ptr<core::State> _state{nullptr};
};
}  // namespace managers
}  // namespace piksy
