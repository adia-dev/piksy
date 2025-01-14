#include "command/export_animation_command.hpp"

#include <core/logger.hpp>
#include <fstream>
#include <utilities/json.hpp>

namespace piksy {
namespace commands {

namespace fs = std::filesystem;

void ExportAnimationsCommand::execute() {
    try {
        core::Logger::info("Exporting animation frames to: %s", m_output_path.c_str());

        // If necessary, create parent directories
        if (!fs::exists(m_output_path.parent_path())) {
            try {
                fs::create_directories(m_output_path.parent_path());
            } catch (const fs::filesystem_error& e) {
                core::Logger::error("Failed to create export directories: %s", e.what());
                return;
            }
        }

        // Open the file
        std::ofstream output_file(m_output_path);
        if (!output_file.is_open()) {
            core::Logger::error("Failed to export: could not open file: %s", m_output_path.c_str());
            return;
        }

        // Write only animations
        export_animations_json(output_file);

        output_file.close();

        core::Logger::info("Animations export complete!");
    } catch (const std::exception& ex) {
        core::Logger::error("Failed to export animations: %s", ex.what());
    }
}

void ExportAnimationsCommand::export_animations_json(std::ostream& output_stream) {
    // Build a minimal JSON, only for animations
    nlohmann::json j_animations = nlohmann::json::array();

    for (const auto& [animation_name, animation] : m_state.animation_state.animations) {
        nlohmann::json anim_json;
        anim_json["name"] = animation_name;

        anim_json["frames"] = nlohmann::json::array();
        for (const auto& frame : animation.frames) {
            // Basic frame info
            nlohmann::json frame_json = {
                {"x", frame.x}, {"y", frame.y}, {"w", frame.w}, {"h", frame.h}};

            // If the frame has custom data, add it
            // We can merge or store it separately. Example:
            if (!frame.data.empty()) {
                // merge the custom data into the frame's JSON
                frame_json.merge_patch(frame.data);
            }

            anim_json["frames"].push_back(frame_json);
        }

        j_animations.push_back(anim_json);
    }

    // Finally, write to the stream in a pretty format
    output_stream << std::setw(4) << j_animations << std::endl;
}

}  // namespace commands
}  // namespace piksy
