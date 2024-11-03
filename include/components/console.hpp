#include <components/ui_component.hpp>

namespace piksy {
namespace components {
class Console : public UIComponent {
   public:
    void update() override;
    void render(core::State &state) override;

   private:
};
}  // namespace components
}  // namespace piksy
