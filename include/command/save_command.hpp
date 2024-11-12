#pragma once

#include <command/command.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <ostream>

namespace fs = std::filesystem;

namespace piksy {
namespace commands {
class SaveCommand : public Command {
   public:
    SaveCommand(core::State& state, fs::path save_path);

    virtual void execute() override;

   private:
    void save(std::ostream& save_file_stream);

   private:
    core::State& m_state;
    fs::path m_save_path;
};
}  // namespace commands
}  // namespace piksy
