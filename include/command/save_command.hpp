#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <ostream>

#include "managers/animation_manager.hpp"

namespace fs = std::filesystem;

namespace piksy {
namespace commands {
class SaveCommand : public Command {
   public:
    SaveCommand(fs::path save_path, core::State& state,
                managers::AnimationManager& animation_manager);

    virtual void execute() override;

   private:
    void save(std::ostream& save_file_stream);

   private:
    fs::path m_save_path;
    core::State& m_state;
    managers::AnimationManager& m_animation_manager;
};
}  // namespace commands
}  // namespace piksy
