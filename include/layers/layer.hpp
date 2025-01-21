#pragma once

#include <SDL_events.h>

#include <core/state.hpp>
#include <string>

namespace piksy {
namespace layers {
class Layer {
   public:
    Layer(core::State& state) : m_state(state) {}
    Layer(core::State& state, const std::string& debug_name)
        : m_state(state), m_debug_name(debug_name) {}
    virtual ~Layer() = default;

    virtual void on_attach() {}
    virtual void on_detach() {}
    virtual void on_update(float dt) {}
    virtual void on_render() {}
    // TODO: RAII of the SDL_Event
    virtual void on_event(SDL_Event& event, bool& event_handled) {}

   protected:
    // TODO: Ideally each layer could own its own state.
    core::State& m_state;
    std::string m_debug_name = "<Unnamed Layer>";
};
}  // namespace layers
}  // namespace piksy
