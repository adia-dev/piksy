#include <core/logger.hpp>
#include <managers/animation_manager.hpp>
#include <string>

#include "rendering/animation.hpp"

namespace piksy {
namespace managers {

void AnimationManager::new_default_animation() {
    size_t new_animation_index = 1;
    while (m_animations.find("New Animation " + std::to_string(new_animation_index)) !=
           m_animations.end()) {
        new_animation_index++;
    }

    std::string conflict_free_animation_name =
        "New Animation " + std::to_string(new_animation_index);

    add_animation(conflict_free_animation_name, rendering::Animation(conflict_free_animation_name));
}

void AnimationManager::add_animation(const std::string& name, rendering::Animation&& animation) {
    std::string animation_name = name;
    if (m_animations.count(animation_name)) {
        size_t index = 1;
        while (m_animations.count(name + std::to_string(index))) {
            index++;
        }
        animation_name = name + " " + std::to_string(index);
    }

    m_animations.emplace(animation_name, std::move(animation));

    core::Logger::info("Added animation '%s'.", animation_name.c_str());

    set_current_animation(animation_name);
}

void AnimationManager::remove_animation(const std::string& name) {
    auto it = m_animations.find(name);
    if (it == m_animations.end()) {
        core::Logger::warn("Animation '%' not found. Cannot delete it.", name.c_str());
        return;
    }

    if (m_current_animation == &it->second) {
        m_current_animation = nullptr;
    }

    core::Logger::info("Deleted animation '%s'.", name.c_str());
    m_animations.erase(it);
}

void AnimationManager::set_current_animation(const std::string& name) {
    if (!m_animations.count(name)) {
        core::Logger::info("Animation '%s' not found. Cannot set as current animation.",
                           name.c_str());
        return;
    }

    m_current_animation = &m_animations.at(name);
}

bool AnimationManager::update_animation_name(const std::string& name) {
    auto it = m_animations.find(name);
    if (it != m_animations.end()) {
        core::Logger::warn("Animation '%s' already exists. Cannot rename the animation.",
                           name.c_str());
        return false;
    }

    rendering::Animation copy = it->second;
    m_animations.erase(it);
    m_animations.emplace(name, std::move(copy));
    m_current_animation = &m_animations.at(name);

    core::Logger::info("Renamed animation '%s' to '%s'.", it->first.c_str(), name.c_str());
    return true;
}

void AnimationManager::clear() {
    m_animations.clear();
    m_current_animation = nullptr;
    core::Logger::info("Cleared all the animations. (TODO: Make it undo-able)");
}

}  // namespace managers
}  // namespace piksy
