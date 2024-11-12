#include <icons/IconsFontAwesome4.h>
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
    : UIComponent(state), m_resource_manager(resource_manager) {
    build_file_extension_icons_map();
    build_directory_cache(m_state.current_path);
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
                    m_state.current_path = entry.path;
                    build_directory_cache(m_state.current_path);
                    return;
                } else {
                    entry.is_open = !entry.is_open;
                }
            }
            ImGui::SameLine();

            if (entry.is_open) {
                ImGui::TextColored({0.49f, 1.0f, 0.83f, 1.0f}, "%s%s/", ICON_FA_FOLDER_OPEN,
                                   name.c_str());
                ImGui::Indent();
                render_directory_entries(entry.children);
                ImGui::Unindent();
            } else {
                ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "%s%s/", ICON_FA_FOLDER, name.c_str());
            }
        } else {
            if (ImGui::Selectable(("##" + entry.path.string()).c_str(), false,
                                  ImGuiSelectableFlags_DontClosePopups)) {
                try_select_texture(entry.path);
            }
            ImGui::SameLine();
            const char* file_icon = m_file_extension_icons.count(entry.path.extension())
                                        ? m_file_extension_icons[entry.path.extension()]
                                        : ICON_FA_FILE;
            ImGui::TextColored({0.8f, 0.8f, 0.8f, 0.8f}, "%s %s", file_icon, name.c_str());
        }
    }
}

void Project::render_file_explorer() {
    ImGui::BeginChild("File Browser", ImVec2(0, 0), false);
    render_directory_entries(m_directory_cache);
    ImGui::EndChild();
}

bool Project::try_select_texture(const std::filesystem::path& file_path) {
    try {
        m_state.texture_sprite.set_texture(m_resource_manager.get_texture(file_path.string()));

        // TODO: Refactor this, this isn't right
        m_state.animation_state.selected_frames.clear();
        m_state.animation_state.animations.clear();
        m_state.animation_state.current_animation = "Untitled";
        m_state.animation_state.animations["Untitled"];
        return true;
    } catch (const std::runtime_error& ex) {
        core::Logger::error("Failed to select a texture in the project: %s", ex.what());
        return false;
    }
}

void Project::build_directory_cache(const fs::path& root_path) {
    m_directory_cache.clear();

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

    populate_cache(root_path, m_directory_cache);
}

void Project::build_file_extension_icons_map() {
    m_file_extension_icons = {
        // Images
        {".jpeg", ICON_FA_FILE_IMAGE_O},
        {".png", ICON_FA_FILE_IMAGE_O},
        {".bmp", ICON_FA_FILE_IMAGE_O},
        {".jpg", ICON_FA_FILE_IMAGE_O},
        {".gif", ICON_FA_FILE_IMAGE_O},
        {".tiff", ICON_FA_FILE_IMAGE_O},

        // Video
        {".mp4", ICON_FA_FILE_VIDEO_O},
        {".avi", ICON_FA_FILE_VIDEO_O},
        {".mov", ICON_FA_FILE_VIDEO_O},
        {".mkv", ICON_FA_FILE_VIDEO_O},
        {".wmv", ICON_FA_FILE_VIDEO_O},

        // Audio
        {".mp3", ICON_FA_FILE_AUDIO_O},
        {".wav", ICON_FA_FILE_AUDIO_O},
        {".flac", ICON_FA_FILE_AUDIO_O},
        {".aac", ICON_FA_FILE_AUDIO_O},
        {".ogg", ICON_FA_FILE_AUDIO_O},

        // Documents
        {".pdf", ICON_FA_FILE_PDF_O},
        {".doc", ICON_FA_FILE_WORD_O},
        {".docx", ICON_FA_FILE_WORD_O},
        {".xls", ICON_FA_FILE_EXCEL_O},
        {".xlsx", ICON_FA_FILE_EXCEL_O},
        {".ppt", ICON_FA_FILE_POWERPOINT_O},
        {".pptx", ICON_FA_FILE_POWERPOINT_O},
        {".txt", ICON_FA_FILE_TEXT_O},

        // Code
        {".cpp", ICON_FA_FILE_CODE_O},
        {".h", ICON_FA_FILE_CODE_O},
        {".hpp", ICON_FA_FILE_CODE_O},
        {".py", ICON_FA_FILE_CODE_O},
        {".js", ICON_FA_FILE_CODE_O},
        {".html", ICON_FA_FILE_CODE_O},
        {".css", ICON_FA_FILE_CODE_O},
        {".java", ICON_FA_FILE_CODE_O},
        {".cs", ICON_FA_FILE_CODE_O},
        {".php", ICON_FA_FILE_CODE_O},

        // Archives
        {".zip", ICON_FA_FILE_ARCHIVE_O},
        {".rar", ICON_FA_FILE_ARCHIVE_O},
        {".7z", ICON_FA_FILE_ARCHIVE_O},
        {".tar", ICON_FA_FILE_ARCHIVE_O},
        {".gz", ICON_FA_FILE_ARCHIVE_O},
    };
}
}  // namespace components
}  // namespace piksy
