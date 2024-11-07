#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>
#include <map>
#include <rendering/renderer.hpp>
#include <string>

namespace piksy {
namespace components {

class Project : public UIComponent {
   public:
    explicit Project(managers::ResourceManager& resource_manager, core::State& state);

    void update(core::State& state) override;
    void render(core::State& state) override;

   private:
    struct DirectoryEntry {
        std::filesystem::path path;
        bool is_directory;
        bool is_open;
        std::vector<DirectoryEntry> children;
    };

    bool try_select_texture(const std::filesystem::path& file_path, core::State& state);
    void render_file_explorer(core::State& state);
    void build_directory_cache(const std::filesystem::path& root_path);
    void render_directory_entries(std::vector<DirectoryEntry>& entries, core::State& state);

    managers::ResourceManager& _resource_manager;
    std::map<std::string, bool> _directory_open_state;
    std::vector<DirectoryEntry> _directory_cache;
};

}  // namespace components
}  // namespace piksy
