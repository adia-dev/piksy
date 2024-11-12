#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

class AnimationPreview : public UIComponent {
   public:
    explicit AnimationPreview(core::State& state);
    void update() override;
    void render() override;

   private:
    void adjust_pan_and_zoom_to_frame(int frame_index);
    void delete_frame(size_t frame_index);

    float target_frame_offset = 0.0f;   // Target frame offset for scrolling
    float current_frame_offset = 0.0f;  // Current frame offset for lerping
};

}  // namespace components
}  // namespace piksy
