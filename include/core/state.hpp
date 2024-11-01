#include <rendering/sprite.hpp>
#include <vector>

namespace piksy {
class State {
   public:
    static State &get();

    static std::vector<Sprite> &sprites();
    static void add_sprite(Sprite &&sprite);
    static void add_sprite(const Sprite &sprite);

   private:
    State() = default;
    ~State() = default;

    State(const State &) = delete;
    State(State &&) = delete;
    State &operator=(const State &) = delete;
    State &operator=(State &&) = delete;

    std::vector<Sprite> _sprites;
};
}  // namespace piksy
