#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

class FrameViewer : public UIComponent {
   public:
    explicit FrameViewer(core::State& state);
    void update() override;
    void render() override;

   private:
    void adjust_pan_and_zoom_to_frame(int frame_index);
    void render_background_grid(const ImVec2& size) const;
    void render_custom_data_editor(nlohmann::json& data_json) const;
    void render_frame_data() const;
    void delete_frame(size_t frame_index);
};

}  // namespace components
}  // namespace piksy
