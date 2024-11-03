#include <imgui.h>

#include <components/project.hpp>
#include <iostream>
#include <managers/resource_manager.hpp>

namespace fs = std::filesystem;

namespace piksy {
namespace components {

void Project::render(rendering::Renderer& renderer, core::State& state) {
    ImGui::Begin("Project");
    render_file_browser(renderer, state);
    ImGui::End();
}

void Project::render_file_browser(rendering::Renderer& renderer, core::State& state) {
    static ImGuiTextFilter filter;
    filter.Draw("Filter");

    ImGui::Separator();

    ImGui::Text("Current Path: %s", state.current_path.string().c_str());

    if (ImGui::Button("Up")) {
        if (state.current_path.has_parent_path()) {
            state.current_path = state.current_path.parent_path();
        }
    }

    ImGui::Separator();

    ImGui::BeginChild("File Browser", ImVec2(0, 0), false);

    for (const fs::directory_entry& entry : fs::directory_iterator(state.current_path)) {
        const auto& path = entry.path();
        std::string filename = path.filename().string();

        if (!filter.PassFilter(filename.c_str())) continue;

        if (entry.is_directory()) {
            if (ImGui::Selectable((filename + "/").c_str(), false,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                state.current_path = path;
                break;
            }
        } else if (entry.is_regular_file()) {
            if (ImGui::Selectable(filename.c_str())) {
                if (!try_select_texture(renderer, path, state)) {
                    ImGui::OpenPopup("Error Loading Texture");
                }
            }
        }
    }

    ImGui::EndChild();

    if (ImGui::BeginPopupModal("Error Loading Texture", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to load the selected texture.");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool Project::try_select_texture(rendering::Renderer& renderer,
                                 const std::filesystem::path& file_path, core::State& state) {
    try {
        state.texture_sprite.set_texture(managers::ResourceManager::get().get_texture(
            renderer.mutable_get(), file_path.string()));
        return true;
    } catch (const std::runtime_error& ex) {
        std::cerr << "Failed ot select a texture in the project: " << ex.what() << std::endl;
        return false;
    }
}

}  // namespace components
}  // namespace piksy
