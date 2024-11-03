#pragma once

#include <core/state.hpp>

namespace piksy {
namespace components {
class UIComponent {
   public:
    virtual void update() = 0;
    virtual void render(core::State& state) = 0;

    virtual ~UIComponent() {}

   private:
};
}  // namespace components
}  // namespace piksy
