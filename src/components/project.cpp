#include <imgui.h>

#include <components/project.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace piksy {
namespace components {

void Project::render() {
    ImGui::Begin("Project");
    ImGui::Text("Current: %s",
                _current_path.lexically_relative(fs::path(std::string(RESOURCE_DIR)).parent_path())
                    .c_str());
    ImVec2 button_sz(125, 125);

    if (_current_path.has_parent_path()) {
        ImGui::PushID(0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.3f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.19f, 0.19, 0.58f, 0.54f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.88f, 0.85f));

        if (ImGui::Button("..", button_sz)) {
            _current_path = _current_path.parent_path();
        }

        ImGui::PopID();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }

    size_t n = _current_path.has_parent_path() ? 1 : 0;
    for (const fs::directory_entry& entry : fs::directory_iterator(_current_path)) {
        ImGuiStyle& style = ImGui::GetStyle();

        if (entry.is_directory()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.3f, 0.54f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.19f, 0.19, 0.58f, 0.54f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.88f, 0.85f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.22f, 0.23f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.19f, 0.19f, 0.19f, 0.54f));
        }

        int buttons_count = 20;
        float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

        ImGui::PushID(n);
        if (ImGui::Button(entry.path().filename().string().substr(0, 24).c_str(), button_sz)) {
            if (entry.is_directory()) {
                _current_path = entry.path();
                ImGui::PopID();
                ImGui::PopStyleColor(3);
                break;
            }
        }
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + style.ItemSpacing.x +
                               button_sz.x;  // Expected position if next button was on same line
        if (n + 1 < buttons_count && next_button_x2 < window_visible_x2) ImGui::SameLine();
        ImGui::PopID();
        ImGui::PopStyleColor(3);
        n++;
    }

    ImGui::End();
}
}  // namespace components
}  // namespace piksy
