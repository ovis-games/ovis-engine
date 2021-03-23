#include <imgui.h>
#include <ovis/base/imgui_scene_controller.hpp>

#include <ovis/core/down_cast.hpp>
#include <ovis/engine/input.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/window.hpp>

namespace ovis {

namespace {

// TODO: maybe make these public as they are partly used in scene_view_editor.cpp

int GetImGuiButtonIndex(MouseButton button) {
  // clang-format off
  switch (button) {
    case MouseButton::LEFT: return ImGuiMouseButton_Left;
    case MouseButton::MIDDLE: return ImGuiMouseButton_Middle;
    case MouseButton::RIGHT: return ImGuiMouseButton_Right;
    case MouseButton::EXTRA1: return 3;
    case MouseButton::EXTRA2: return 4;
    default: SDL_assert(false);
  }
  // clang-format off
}


MouseButton GetMouseButtonFromImGuiIndex(int button) {
  // clang-format off
  switch (button) {
    case ImGuiMouseButton_Left: return MouseButton::LEFT;
    case ImGuiMouseButton_Middle: return MouseButton::MIDDLE;
    case ImGuiMouseButton_Right: return MouseButton::RIGHT;
    case 3: return MouseButton::EXTRA1;
    case 4: return MouseButton::EXTRA2;
    default: SDL_assert(false); return MouseButton::LEFT; // Keep clang happy
  }
  // clang-format off
}


}  // namespace

ImGuiSceneController::ImGuiSceneController(ImGuiContext* context) : SceneController("ImGui"), context_(context) {
  ImGui::SetCurrentContext(context_);

  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
  SubscribeToEvent(MouseWheelEvent::TYPE);
  SubscribeToEvent(TextInputEvent::TYPE);
  SubscribeToEvent(KeyPressEvent::TYPE);
  SubscribeToEvent(KeyReleaseEvent::TYPE);
}

void ImGuiSceneController::DrawImGui() {
  ImGuiIO& io = ImGui::GetIO();
  ImGui::SetCurrentContext(context_);
  for (int i = 0; i < 5; ++i) {
    io.MouseDown[i] = mouse_button_pressed_[i] || input()->GetMouseButtonState(GetMouseButtonFromImGuiIndex(i));
    mouse_button_pressed_[i] = false;
  }
}

void ImGuiSceneController::ProcessEvent(Event* event) {
  ImGuiIO& io = ImGui::GetIO();
  ImGui::SetCurrentContext(context_);

  if (event->type() == MouseMoveEvent::TYPE) {
    MouseMoveEvent* mouse_event = down_cast<MouseMoveEvent*>(event);
    io.MousePos.x = mouse_event->device_coordinates().x;
    io.MousePos.y = mouse_event->device_coordinates().y;
    if (io.WantCaptureMouse) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseButtonPressEvent::TYPE) {
    MouseButtonPressEvent* button_event = down_cast<MouseButtonPressEvent*>(event);
    const int button_index = GetImGuiButtonIndex(button_event->button());
    mouse_button_pressed_[button_index] = true;
    if (io.WantCaptureMouse) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseButtonReleaseEvent::TYPE) {
    MouseButtonReleaseEvent* button_event = down_cast<MouseButtonReleaseEvent*>(event);
    if (io.WantCaptureMouse) {
      event->StopPropagation();
    }
  } else if (event->type() == MouseWheelEvent::TYPE) {
    MouseWheelEvent* mouse_event = down_cast<MouseWheelEvent*>(event);
    if (mouse_event->delta().x > 0) {
      io.MouseWheelH += 1;
    }
    if (mouse_event->delta().x < 0) {
      io.MouseWheelH -= 1;
    }
    if (mouse_event->delta().y > 0) {
      io.MouseWheel += 1;
    }
    if (mouse_event->delta().y < 0) {
      io.MouseWheel -= 1;
    }
    if (io.WantCaptureMouse) {
      event->StopPropagation();
    }
  } else if (event->type() == TextInputEvent::TYPE) {
    TextInputEvent* text_event = down_cast<TextInputEvent*>(event);
    io.AddInputCharactersUTF8(text_event->text().c_str());
    if (io.WantCaptureKeyboard) {
      event->StopPropagation();
    }
  } else if (event->type() == KeyPressEvent::TYPE) {
    KeyPressEvent* key_event = down_cast<KeyPressEvent*>(event);
    SDL_assert(key_event->key().code < IM_ARRAYSIZE(io.KeysDown));
    io.KeysDown[key_event->key().code] = true;
    io.KeyShift = input()->GetKeyState(Key::SHIFT_LEFT) || input()->GetKeyState(Key::SHIFT_RIGHT);
    io.KeyCtrl = input()->GetKeyState(Key::CONTROL_LEFT) || input()->GetKeyState(Key::CONTROL_RIGHT);
    io.KeyAlt = input()->GetKeyState(Key::ALT_LEFT) || input()->GetKeyState(Key::ALT_RIGHT);
    io.KeySuper = input()->GetKeyState(Key::META_LEFT) || input()->GetKeyState(Key::META_RIGHT);
    if (io.WantCaptureKeyboard) {
      event->StopPropagation();
    }
  } else if (event->type() == KeyReleaseEvent::TYPE) {
    KeyReleaseEvent* key_event = down_cast<KeyReleaseEvent*>(event);
    SDL_assert(key_event->key().code < IM_ARRAYSIZE(io.KeysDown));
    io.KeysDown[key_event->key().code] = false;
    io.KeyShift = input()->GetKeyState(Key::SHIFT_LEFT) || input()->GetKeyState(Key::SHIFT_RIGHT);
    io.KeyCtrl = input()->GetKeyState(Key::CONTROL_LEFT) || input()->GetKeyState(Key::CONTROL_RIGHT);
    io.KeyAlt = input()->GetKeyState(Key::ALT_LEFT) || input()->GetKeyState(Key::ALT_RIGHT);
    io.KeySuper = input()->GetKeyState(Key::META_LEFT) || input()->GetKeyState(Key::META_RIGHT);
    if (io.WantCaptureKeyboard) {
      event->StopPropagation();
    }
  }
}

}  // namespace ovis
