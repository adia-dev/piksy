#include <rendering/sprite.hpp>
#include <vector>

namespace piksy {
namespace core {
class State {
   public:
    static State &get();

    static std::vector<rendering::Sprite> &sprites();
    static void add_sprite(rendering::Sprite &&sprite);
    static void add_sprite(const rendering::Sprite &sprite);

   private:
    State() = default;
    ~State() = default;

    State(const State &) = delete;
    State(State &&) = delete;
    State &operator=(const State &) = delete;
    State &operator=(State &&) = delete;

    std::vector<rendering::Sprite> _sprites;
};
}  // namespace core
}  // namespace piksy
