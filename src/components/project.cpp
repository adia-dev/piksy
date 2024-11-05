#include <imgui.h>

#include <components/project.hpp>
#include <core/logger.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>

#include "core/state.hpp"

namespace fs = std::filesystem;

namespace piksy {
namespace components {

Project::Project(managers::ResourceManager& resource_manager, core::State& state)
    : _resource_manager(resource_manager) {
    build_directory_cache(state.current_path);
}

void Project::update() {}

void Project::render(core::State& state) {
    ImGui::Begin("Project");
    render_file_explorer(state);
    ImGui::End();
}

void Project::render_directory_entries(std::vector<DirectoryEntry>& entries, core::State& state) {
    for (auto& entry : entries) {
        const std::string& name = entry.path.filename().string();
        if (entry.is_directory) {
            if (ImGui::Selectable(("##" + entry.path.string() + "/").c_str(), entry.is_open,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                    state.current_path = entry.path;
                    build_directory_cache(state.current_path);
                    return;
                } else {
                    entry.is_open = !entry.is_open;
                }
            }
            ImGui::SameLine();

            if (entry.is_open) {
                ImGui::TextColored({0.49f, 1.0f, 0.83f, 1.0f}, "> %s/", name.c_str());
                ImGui::Indent();
                render_directory_entries(entry.children, state);
                ImGui::Unindent();
            } else {
                ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "v %s/", name.c_str());
            }
        } else {
            if (ImGui::Selectable(("##" + entry.path.string()).c_str(), false,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                try_select_texture(entry.path, state);
            }
            ImGui::SameLine();
            ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "%s", name.c_str());
        }
    }
}

void Project::render_file_explorer(core::State& state) {
    static ImGuiTextFilter filter;
    filter.Draw("Filter");

    ImGui::Separator();

    ImGui::Text("Current Path: %s", state.current_path.string().c_str());

    if (state.current_path.has_parent_path() && ImGui::Button("..")) {
        if (state.current_path.has_parent_path()) {
            state.current_path = state.current_path.parent_path();
            build_directory_cache(state.current_path);
        }
    }

    if (ImGui::Button("Reload")) {
        build_directory_cache(state.current_path);
    }

    ImGui::Separator();

    ImGui::BeginChild("File Browser", ImVec2(0, 0), false);
    render_directory_entries(_directory_cache, state);
    ImGui::EndChild();
}

bool Project::try_select_texture(const std::filesystem::path& file_path, core::State& state) {
    try {
        state.texture_sprite.set_texture(_resource_manager.get_texture(file_path.string()));
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
