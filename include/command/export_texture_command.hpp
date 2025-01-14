#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>

namespace piksy {
namespace commands {

/**
 * Command to export the current Sprite's texture as a PNG file.
 */
class ExportTextureCommand : public Command {
   public:
    ExportTextureCommand(core::State& state, std::filesystem::path output_path);

    void execute() override;

   private:
    core::State& m_state;
    std::filesystem::path m_output_path;
};

}  // namespace commands
}  // namespace piksy
