#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <istream>
#include <string>

#include "managers/animation_manager.hpp"
#include "managers/resource_manager.hpp"

namespace fs = std::filesystem;

namespace piksy {
namespace commands {
class LoadCommand : public Command {
   public:
    LoadCommand(const fs::path& load_path, core::State& m_state,
                managers::ResourceManager& resource_manager,
                managers::AnimationManager& animation_manager);

    virtual void execute() override;

   private:
    void load(std::istream& load_file_stream);

   private:
    fs::path m_load_path;
    core::State& m_state;
    managers::ResourceManager& m_resource_manager;
    managers::AnimationManager& m_animation_manager;
};
}  // namespace commands
}  // namespace piksy
