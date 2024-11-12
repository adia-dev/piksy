#pragma once

#include <core/state.hpp>

namespace piksy {
namespace components {
class UIComponent {
   public:
    UIComponent(core::State& state) : m_state(state) {}
    virtual void update() = 0;
    virtual void render() = 0;

    virtual ~UIComponent() = default;

   protected:
    core::State& m_state;
};
}  // namespace components
}  // namespace piksy
