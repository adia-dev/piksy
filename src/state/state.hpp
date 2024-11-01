#include "sprite/sprite.hpp"
#include <vector>

namespace piksy {
class State {
public:
  static State &get() {
    static State instance;
    return instance;
  }

  static std::vector<Sprite> &sprites() {
    auto &state = get();
    return state._sprites;
  }

  static void add_sprite(Sprite &&sprite) {
    auto &state = get();
    state._sprites.emplace_back(std::move(sprite));
  }

  static void add_sprite(const Sprite &sprite) {
    auto &state = get();
    state._sprites.push_back(sprite);
  }

private:
  State() = default;
  ~State() = default;

  State(const State &) = delete;
  State(State &&) = delete;
  State &operator=(const State &) = delete;
  State &operator=(State &&) = delete;

  std::vector<Sprite> _sprites;
};
} // namespace piksy
