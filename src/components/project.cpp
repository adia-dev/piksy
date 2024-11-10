#include <imgui.h>

#include <components/project.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>

namespace fs = std::filesystem;

namespace piksy {
namespace components {

Project::Project(core::State& state, managers::ResourceManager& resource_manager)
    : UIComponent(state), _resource_manager(resource_manager) {
    build_directory_cache(_state.current_path);
}

void Project::update() {}

void Project::render() {
    ImGui::Begin("Project");
    render_file_explorer();
    ImGui::End();
}

void Project::render_directory_entries(std::vector<DirectoryEntry>& entries) {
    for (auto& entry : entries) {
        const std::string& name = entry.path.filename().string();
        if (entry.is_directory) {
            if (ImGui::Selectable(("##" + entry.path.string() + "/").c_str(), entry.is_open,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                    _state.current_path = entry.path;
                    build_directory_cache(_state.current_path);
                    return;
                } else {
                    entry.is_open = !entry.is_open;
                }
            }
            ImGui::SameLine();

            if (entry.is_open) {
                ImGui::TextColored({0.49f, 1.0f, 0.83f, 1.0f}, "v %s/", name.c_str());
                ImGui::Indent();
                render_directory_entries(entry.children);
                ImGui::Unindent();
            } else {
                ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "> %s/", name.c_str());
            }
        } else {
            if (ImGui::Selectable(("##" + entry.path.string()).c_str(), false,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                try_select_texture(entry.path);
            }
            ImGui::SameLine();
            ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "%s", name.c_str());
        }
    }
}

void Project::render_file_explorer() {
    ImGui::BeginChild("File Browser", ImVec2(0, 0), false);
    render_directory_entries(_directory_cache);
    ImGui::EndChild();
}

bool Project::try_select_texture(const std::filesystem::path& file_path) {
    try {
        _state.texture_sprite.set_texture(_resource_manager.get_texture(file_path.string()));
        return true;
    } catch (const std::runtime_error& ex) {
        core::Logger::error("Failed to select a texture in the project: %s", ex.what());
        return false;
    }
}

void Project::build_directory_cache(const fs::path& root_path) {
    _directory_cache.clear();

    std::function<void(const fs::path&, std::vector<DirectoryEntry>&)> populate_cache =
        [&](const fs::path& path, std::vector<DirectoryEntry>& entries) {
            for (const auto& entry : fs::directory_iterator(path)) {
                DirectoryEntry dir_entry{entry.path(), entry.is_directory(), false};
                if (entry.is_directory()) {
                    populate_cache(entry.path(), dir_entry.children);
                }
                entries.push_back(std::move(dir_entry));
            }
        };

    populate_cache(root_path, _directory_cache);
}

}  // namespace components
}  // namespace piksy
