#include <components/console.hpp>
#include <core/config.hpp>

namespace piksy {
namespace components {

Console::Console(core::State& state) : UIComponent(state) {}

void Console::update() {}

void Console::render() {
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) {
        core::Logger::clear_messages();
    }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::Button("Copy");
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_auto_scroll);

    ImGui::Separator();

    if (ImGui::Checkbox("Trace", &m_show_trace)) {
        m_scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Debug", &m_show_debug)) {
        m_scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Info", &m_show_info)) {
        m_scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Warn", &m_show_warn)) {
        m_scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Error", &m_show_error)) {
        m_scroll_to_bottom = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Fatal", &m_show_fatal)) {
        m_scroll_to_bottom = true;
    }

    m_filter.Draw("Filter", -100.0f);
    ImGui::Separator();

    ImGui::BeginChild("ConsoleArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (copy_to_clipboard) {
        ImGui::LogToClipboard();
    }

    const auto& messages = core::Logger::messages();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    for (const auto& [level, message] : messages) {
        bool show_message = false;
        switch (level) {
            case core::LogLevel::Trace:
                show_message = m_show_trace;
                break;
            case core::LogLevel::Debug:
                show_message = m_show_debug;
                break;
            case core::LogLevel::Info:
                show_message = m_show_info;
                break;
            case core::LogLevel::Warn:
                show_message = m_show_warn;
                break;
            case core::LogLevel::Error:
                show_message = m_show_error;
                break;
            case core::LogLevel::Fatal:
                show_message = m_show_fatal;
                break;
            default:
                show_message = true;
                break;
        }

        if (!show_message) {
            continue;
        }

        if (!m_filter.PassFilter(message.c_str())) {
            continue;
        }

        ImVec4 color = core::LogLevelToColor(level);

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar();

    if (m_scroll_to_bottom || (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
    }
    m_scroll_to_bottom = false;

    ImGui::EndChild();
    ImGui::End();
}

}  // namespace components
}  // namespace piksy
