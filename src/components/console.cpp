#include <components/console.hpp>
#include <core/config.hpp>

namespace piksy {
namespace components {

void Console::update() {}

void Console::render(core::State& state) {
    ImGui::Begin("Console");

    // Options menu
    if (ImGui::Button("Clear")) {
        core::Logger::get().clear_messages();
    }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::Button("Copy");
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &_auto_scroll);

    ImGui::Separator();

    // Log level filters
    if (ImGui::Checkbox("Trace", &_show_trace)) {
        _scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Debug", &_show_debug)) {
        _scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Info", &_show_info)) {
        _scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Warn", &_show_warn)) {
        _scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Error", &_show_error)) {
        _scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Fatal", &_show_fatal)) {
        _scroll_to_bottom = true;
    }

    _filter.Draw("Filter", -100.0f);
    ImGui::Separator();

    ImGui::BeginChild("ConsoleArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (copy_to_clipboard) {
        ImGui::LogToClipboard();
    }

    const auto& messages = core::Logger::get().messages();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    for (const auto& [level, message] : messages) {
        // Apply log level filters
        bool show_message = false;
        switch (level) {
            case core::LogLevel::Trace:
                show_message = _show_trace;
                break;
            case core::LogLevel::Debug:
                show_message = _show_debug;
                break;
            case core::LogLevel::Info:
                show_message = _show_info;
                break;
            case core::LogLevel::Warn:
                show_message = _show_warn;
                break;
            case core::LogLevel::Error:
                show_message = _show_error;
                break;
            case core::LogLevel::Fatal:
                show_message = _show_fatal;
                break;
            default:
                show_message = true;
                break;
        }

        if (!show_message) {
            continue;
        }

        // Apply text filter
        if (!_filter.PassFilter(message.c_str())) {
            continue;
        }

        // Color coding based on log level
        ImVec4 color = core::LogLevelToColor(level);

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar();

    if (_scroll_to_bottom || (_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
    }
    _scroll_to_bottom = false;

    ImGui::EndChild();
    ImGui::End();
}

}  // namespace components
}  // namespace piksy
