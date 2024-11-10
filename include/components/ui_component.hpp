#pragma once

#include <core/state.hpp>

namespace piksy {
namespace components {
class UIComponent {
   public:
    UIComponent(core::State& state) : _state(state) {}
    virtual void update() = 0;
    virtual void render() = 0;

    virtual ~UIComponent() = default;

   protected:
    core::State& _state;
};
}  // namespace components
}  // namespace piksy
