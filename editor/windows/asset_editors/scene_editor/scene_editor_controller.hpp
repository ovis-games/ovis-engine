#include <ovis/core/scene_controller.hpp>

namespace ovis {
namespace editor {

class SceneEditorController : public SceneController {
 public:
  bool ProcessEvent(const SDL_Event& event) override;

 private:
};


    // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    //   const Vector2 mouse_position = ImGui::GetMousePos();
    //   SceneObject* object =
    //       GetObjectAtPosition(scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left));
    //   if (object == nullptr) {
    //     selected_object_.reset();
    //   } else {
    //     selected_object_ = object->name();
    //   }
    // }

    // SceneObject* selected_object = GetSelectedObject();
    // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && selected_object != nullptr) {
    //   if (selected_object->HasComponent("Transform")) {
    //     move_state_.emplace();
    //     move_state_->object_name = selected_object->name();

    //     const Vector2 mouse_position = ImGui::GetMousePos();
    //     move_state_->drag_start_mouse_position =
    //         scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left);

    //     move_state_->original_position = selected_object->GetComponent<TransformComponent>("Transform")->translation();
    //   }
    // }
    // if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && move_state_.has_value()) {
    //   const Vector2 mouse_position = ImGui::GetMousePos();
    //   const Vector3 current_mouse_pos = scene_viewport_->DeviceCoordinatesToWorldSpace(mouse_position - top_left);
    //   const Vector3 position_delta = current_mouse_pos - move_state_->drag_start_mouse_position;
    //   const Vector3 object_position = move_state_->original_position + position_delta;

    //   if (selected_object->HasComponent("Transform")) {
    //     TransformComponent* transform_component = selected_object->GetComponent<TransformComponent>("Transform");
    //     transform_component->SetTranslation(object_position);
    //     serialized_scene_working_copy_[GetComponentPath(selected_object->name(), "Transform")] =
    //         transform_component->Serialize();
    //   }
    // }
    // if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && move_state_.has_value()) {
    //   move_state_.reset();
    //   SubmitJsonFile(serialized_scene_working_copy_);
    // }

}  // namespace editor

}  // namespace ovis
