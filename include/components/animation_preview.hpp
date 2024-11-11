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
    void render_controls();
    void render_frames();
    void render_frame_thumbnails();
    void render_frame_list();
    void adjust_pan_and_zoom_to_frame(int frame_index);

    void delete_frame(size_t frame_index);

   private:
    float _frame_display_size = 100.0f;
};

}  // namespace components
}  // namespace piksy
