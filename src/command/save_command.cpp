#include <command/save_command.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <utilities/json.hpp>

#include "core/logger.hpp"
#include "core/state.hpp"

namespace piksy {
namespace commands {

SaveCommand::SaveCommand(core::State& state, fs::path save_path)
    : m_state(state), m_save_path(std::move(save_path)) {}

void SaveCommand::execute() {
    try {
        core::Logger::info("Saving the application state...");

        if (!fs::exists(m_save_path)) {
            core::Logger::warn("Save file does not exists yet, creating it... (path: %s)",
                               m_save_path.c_str());
            try {
                if (std::filesystem::create_directories(m_save_path.parent_path())) {
                    core::Logger::info("Save directories created at: %s",
                                       m_save_path.parent_path().c_str());
                } else {
                    core::Logger::debug("Save directories already exist: %s", m_save_path.c_str());
                }
            } catch (const std::filesystem::filesystem_error& e) {
                core::Logger::error("Error creating directories: %s", e.what());
            }
        }

        std::ofstream save_file(m_save_path);
        if (!save_file.is_open()) {
            core::Logger::error("Failed to save: failed to open the save file.");
            return;
        }

        save(save_file);

        save_file.close();
    } catch (const std::exception& ex) {
        core::Logger::error("Failed to save: %s", ex.what());
    }
}

void SaveCommand::save(std::ostream& save_file_stream) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char timestamp_buf[100];
    std::strftime(timestamp_buf, sizeof(timestamp_buf), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&now_c));

    nlohmann::json j;
    j["metadata"] = {{"version", 1}, {"timestamp", timestamp_buf}};
    j["tool"] = m_state.current_tool;

    j["animations"] = nlohmann::json::array();
    for (const auto& [name, animation] : m_state.animation_state.animations) {
        nlohmann::json animation_json;
        animation_json["name"] = name;

        animation_json["frames"] = nlohmann::json::array();
        for (const auto& frame : animation.frames) {
            nlohmann::json frame_json = {
                {"x", frame.x}, {"y", frame.y}, {"w", frame.w}, {"h", frame.h}};

            if (!frame.data.empty()) {
                frame_json.merge_patch(frame.data);
            }

            animation_json["frames"].push_back(frame_json);
        }

        j["animations"].push_back(animation_json);
    }
    j["current_animation"] = m_state.animation_state.current_animation;

    nlohmann::json texture_json({});
    if (m_state.texture_sprite.texture() != nullptr) {
        texture_json["path"] = m_state.texture_sprite.texture()->path();
    }

    j["sprite"] = {
        {
            {"texture", texture_json},
        },
    };

    // Write to a temporary file first for atomic saving
    auto temp_path = m_save_path;
    temp_path.replace_extension(".tmp");

    try {
        std::ofstream ofs(temp_path);
        if (!ofs) throw std::ios_base::failure("Failed to open temporary file for writing");

        ofs << std::setw(4) << j;
        ofs.close();
        core::Logger::info("Temporary save file created at: %s", temp_path.c_str());

        std::filesystem::rename(temp_path, m_save_path);
        core::Logger::info("Save file created at: %s", m_save_path.c_str());

        if (std::filesystem::exists(m_save_path)) {
            auto backup_path = m_save_path;
            backup_path.replace_extension(".bak");

            std::filesystem::copy_file(m_save_path, backup_path,
                                       std::filesystem::copy_options::overwrite_existing);
            core::Logger::info("Backup created at: %s", backup_path.c_str());
        } else {
            core::Logger::error("Save file does not exist after rename operation: %s",
                                m_save_path.c_str());
        }

    } catch (const std::filesystem::filesystem_error& e) {
        core::Logger::error("Filesystem error during save: %s", e.what());
    } catch (const std::exception& e) {
        core::Logger::error("General error during save: %s", e.what());
    }
}

}  // namespace commands
}  // namespace piksy
