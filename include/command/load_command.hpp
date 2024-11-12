#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <istream>
#include <string>

#include "managers/resource_manager.hpp"

namespace fs = std::filesystem;

namespace piksy {
namespace commands {
class LoadCommand : public Command {
   public:
    LoadCommand(core::State& m_state, managers::ResourceManager& resource_manager,
                const fs::path& load_path);

    virtual void execute() override;

   private:
    void load(std::istream& load_file_stream);

   private:
    core::State& m_state;
    managers::ResourceManager m_resource_manager;
    fs::path m_load_path;
};
}  // namespace commands
}  // namespace piksy
