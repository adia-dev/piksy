#pragma once

namespace piksy {
namespace commands {
class Command {
   public:
    Command() = default;
    virtual ~Command() = default;

    virtual void execute() = 0;
};
}  // namespace commands
}  // namespace piksy
