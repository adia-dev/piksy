#pragma once

#include <filesystem>

namespace piksy {
namespace components {
class Project {
   public:
    void render();

   private:
    std::filesystem::path _current_path = RESOURCE_DIR;
};
}  // namespace components
}  // namespace piksy
