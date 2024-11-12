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
    void draw_background_grid(const ImVec2& pos, const ImVec2& size) const;
    void delete_frame(size_t frame_index);
};

}  // namespace components
}  // namespace piksy
