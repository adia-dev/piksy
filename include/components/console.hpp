#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>

namespace piksy {
namespace components {

class Console : public UIComponent {
   public:
    void update() override;
    void render(core::State& state) override;

   private:
    ImGuiTextFilter _filter;
    bool _auto_scroll = true;
    bool _scroll_to_bottom = false;

    bool _show_trace = false;
    bool _show_debug = true;
    bool _show_info = true;
    bool _show_warn = true;
    bool _show_error = true;
    bool _show_fatal = true;

    void render_console();
};

}  // namespace components
}  // namespace piksy
