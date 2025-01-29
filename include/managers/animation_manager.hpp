#pragma once

#include <string>
#include <unordered_map>

#include "core/logger.hpp"
#include "rendering/animation.hpp"

namespace piksy {
namespace managers {
class AnimationManager {
   public:
    AnimationManager() = default;

    /// Create a new default animation
    /// The name of the new animation will default to:
    /// New Animation {{highest available number}}
    void new_default_animation();

    /// Add a new animation
    /// If an animation with the same name already exists,
    /// a number suffix will be added to represent the index of that animation
    void add_animation(const std::string& name, rendering::Animation&& animation);

    /// Remove an animation given the name of it
    /// If the animation removed was the last, a new default one is created
    void remove_animation(const std::string& name);

    // Set the current animation given a name
    // No-Op if the name is not found in the animations
    void set_current_animation(const std::string& name);

    // Rename the current animation
    // Returns `false` if the name is already taken
    // Return `true` if not
    bool update_animation_name(const std::string& name);

    /// Clears all the animations
    void clear();

   public:
    /// Returns the animations
    const std::unordered_map<std::string, rendering::Animation>& animations() const {
        return m_animations;
    }

    /// Returns the current animation
    rendering::Animation* current_animation() { return m_current_animation; }

   private:
    rendering::Animation* m_current_animation = nullptr;
    std::unordered_map<std::string, rendering::Animation> m_animations;
};

}  // namespace managers
}  // namespace piksy
