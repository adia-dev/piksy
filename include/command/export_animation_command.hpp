#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <ostream>

namespace piksy {
namespace commands {

/**
 * Command to export only the animations (frames, custom data).
 */
class ExportAnimationsCommand : public Command {
   public:
    ExportAnimationsCommand(core::State& state, std::filesystem::path output_path)
        : m_state(state), m_output_path(std::move(output_path)) {}

    void execute() override;

   private:
    void export_animations_json(std::ostream& output_stream);

   private:
    core::State& m_state;
    std::filesystem::path m_output_path;
};

}  // namespace commands
}  // namespace piksy
