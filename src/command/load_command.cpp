#include <command/load_command.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utilities/json.hpp>

#include "core/logger.hpp"
#include "core/state.hpp"
#include "managers/animation_manager.hpp"

namespace piksy {
namespace commands {

LoadCommand::LoadCommand(const fs::path& load_path, core::State& m_state,
                         managers::ResourceManager& resource_manager,
                         managers::AnimationManager& animation_manager)
    : m_load_path(load_path),
      m_state(m_state),
      m_resource_manager(resource_manager),
      m_animation_manager(animation_manager) {}

void LoadCommand::execute() {
    try {
        core::Logger::info("Loading the application state...");

        if (!fs::exists(m_load_path)) {
            core::Logger::error("Save file does not exist (path: %s)", m_load_path.c_str());
            throw std::runtime_error("Failed to load save file: File does not exist");
        }

        std::ifstream load_file(m_load_path);
        if (!load_file.is_open()) {
            core::Logger::error("Failed to load: failed to open the load file.");
            return;
        }

        load(load_file);
        load_file.close();
    } catch (const std::exception& ex) {
        core::Logger::error("Failed to load: %s", ex.what());
    }
}

void LoadCommand::load(std::istream& load_file_stream) {
    nlohmann::json j;
    try {
        load_file_stream >> j;

        // Load metadata and application-specific fields if they exist
        if (j.contains("metadata")) {
            const auto& metadata = j["metadata"];
            if (metadata.contains("version")) {
                int version = metadata["version"].get<int>();
                core::Logger::info("Loaded save file version: %d", version);
            }
            if (metadata.contains("timestamp")) {
                std::string timestamp = metadata["timestamp"].get<std::string>();
                core::Logger::info("Loaded save file timestamp: %s", timestamp.c_str());
            }
        }

        // Load current tool
        if (j.contains("tool")) {
            m_state.current_tool = j["tool"].get<tools::Tool>();
        }

        // Load animations
        if (j.contains("animations")) {
            m_animation_manager.clear();
            for (const auto& anim_json : j["animations"]) {
                std::string name = anim_json["name"];
                rendering::Animation animation(name);

                for (const auto& frame_json : anim_json["frames"]) {
                    rendering::Frame frame{
                        frame_json["x"].get<int>(),
                        frame_json["y"].get<int>(),
                        frame_json["w"].get<int>(),
                        frame_json["h"].get<int>(),
                    };

                    frame.data = frame_json;
                    frame.data.erase("x");
                    frame.data.erase("y");
                    frame.data.erase("w");
                    frame.data.erase("h");

                    animation.frames.push_back(frame);
                }

                m_animation_manager.add_animation(name, std::move(animation));
            }
        }

        if (j.contains("current_animation")) {
            m_animation_manager.set_current_animation(j["current_animation"]);
        }

        // Load texture sprite
        if (j.contains("sprite") && j["sprite"].size() > 0) {
            const auto& texture_json = j["sprite"][0]["texture"];
            if (texture_json.contains("path")) {
                std::string texture_path = texture_json["path"].get<std::string>();
                m_state.texture_sprite.set_texture(m_resource_manager.get_texture(texture_path));
                core::Logger::info("Loaded texture from path: %s", texture_path.c_str());
            }
        }

    } catch (const nlohmann::json::exception& e) {
        core::Logger::error("JSON error during load: %s", e.what());
    } catch (const std::filesystem::filesystem_error& e) {
        core::Logger::error("Filesystem error during load: %s", e.what());
    } catch (const std::exception& e) {
        core::Logger::error("General error during load: %s", e.what());
    }
}

}  // namespace commands
}  // namespace piksy
