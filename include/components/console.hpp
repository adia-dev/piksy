#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

class Console : public UIComponent {
   public:
    explicit Console(core::State& state);
    void update() override;
    void render() override;

   private:
    void render_console();

   private:
    ImGuiTextFilter m_filter;
    bool m_auto_scroll = true;
    bool m_scroll_to_bottom = false;

    // TODO: Make this a bit field
    bool m_show_trace = false;
    bool m_show_debug = true;
    bool m_show_info = true;
    bool m_show_warn = true;
    bool m_show_error = true;
    bool m_show_fatal = true;
};

}  // namespace components
}  // namespace piksy
