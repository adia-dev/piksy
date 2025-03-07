#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>
#include <map>
#include <rendering/renderer.hpp>
#include <string>
#include <unordered_map>

namespace piksy {
namespace components {

class Project : public UIComponent {
   public:
    Project(core::State& state, managers::ResourceManager& resource_manager);

    void update() override;
    void render() override;

   private:
    struct DirectoryEntry {
        std::filesystem::path path;
        bool is_directory;
        bool is_open;
        std::vector<DirectoryEntry> children;
    };

   private:
    bool try_select_texture(const std::filesystem::path& file_path);
    void render_file_explorer();
    void build_directory_cache(const std::filesystem::path& root_path);
    void build_file_extension_icons_map();
    void render_directory_entries(std::vector<DirectoryEntry>& entries);

   private:
    managers::ResourceManager& m_resource_manager;
    std::map<std::string, bool> m_directory_open_state;
    std::vector<DirectoryEntry> m_directory_cache;
    std::unordered_map<std::string, const char*> m_file_extension_icons;
};

}  // namespace components
}  // namespace piksy
