#include <imgui.h>

#include <ovis/utils/down_cast.hpp>
#include <ovis/utils/platform.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>
#include <ovis/input/text_input_event.hpp>
#include <ovis/imgui/imgui_start_frame_controller.hpp>

namespace ovis {

namespace {

// TODO: maybe make these public as they are partly used in scene_view_editor.cpp

int GetImGuiButtonIndex(MouseButton button) {
  // clang-format off
  switch (button.code) {
    case MouseButton::Left().code: return ImGuiMouseButton_Left;
    case MouseButton::Middle().code: return ImGuiMouseButton_Middle;
    case MouseButton::Right().code: return ImGuiMouseButton_Right;
    case MouseButton::Four().code: return 3;
    case MouseButton::Five().code: return 4;
    default: SDL_assert(false); return ImGuiMouseButton_Left;
  }
  // clang-format off
}


MouseButton GetMouseButtonFromImGuiIndex(int button) {
  // clang-format off
  switch (button) {
    case ImGuiMouseButton_Left: return MouseButton::Left();
    case ImGuiMouseButton_Middle: return MouseButton::Middle();
    case ImGuiMouseButton_Right: return MouseButton::Right();
    case 3: return MouseButton::Four();
    case 4: return MouseButton::Five();
    default: SDL_assert(false); return MouseButton::Left(); // Keep clang happy
  }
  // clang-format off
}


}  // namespace

ImGuiStartFrameController::ImGuiStartFrameController()
    : SceneController(Name())
    , imgui_context_(ImGui::CreateContext(), [](ImGuiContext* context) { ImGui::DestroyContext(context); }) {
  IMGUI_CHECKVERSION();

  SubscribeToEvent(MouseMoveEvent::TYPE);
  SubscribeToEvent(MouseButtonPressEvent::TYPE);
  SubscribeToEvent(MouseButtonReleaseEvent::TYPE);
  SubscribeToEvent(MouseWheelEvent::TYPE);
  SubscribeToEvent(TextInputEvent::TYPE);
  SubscribeToEvent(KeyPressEvent::TYPE);
  SubscribeToEvent(KeyReleaseEvent::TYPE);

  ImGuiIO& io = ImGui::GetIO();

#if !OVIS_EMSCRIPTEN
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  io.IniFilename = "/user/imgui.ini";
#endif
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup back-end capabilities flags
  io.ConfigMacOSXBehaviors = GetPlatform() == Platform::MACOS;
  //   io.BackendFlags |=
  //       ImGuiBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor()
  //                                           // values (optional)
  //   io.BackendFlags |=
  //       ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos
  //                                          // requests (optional, rarely
  //                                          used)

  // Set imgui key codes
  io.KeyMap[ImGuiKey_Tab] = Key::Tab().code;
  io.KeyMap[ImGuiKey_LeftArrow] = Key::ArrowLeft().code;
  io.KeyMap[ImGuiKey_RightArrow] = Key::ArrowRight().code;
  io.KeyMap[ImGuiKey_UpArrow] = Key::ArrowUp().code;
  io.KeyMap[ImGuiKey_DownArrow] = Key::ArrowDown().code;
  io.KeyMap[ImGuiKey_PageUp] = Key::PageUp().code;
  io.KeyMap[ImGuiKey_PageDown] = Key::PageDown().code;
  io.KeyMap[ImGuiKey_Home] = Key::Home().code;
  io.KeyMap[ImGuiKey_End] = Key::End().code;
  io.KeyMap[ImGuiKey_Insert] = Key::Insert().code;
  io.KeyMap[ImGuiKey_Delete] = Key::Delete().code;
  io.KeyMap[ImGuiKey_Backspace] = Key::Backspace().code;
  io.KeyMap[ImGuiKey_Space] = Key::Space().code;
  io.KeyMap[ImGuiKey_Enter] = Key::Enter().code;
  io.KeyMap[ImGuiKey_Escape] = Key::Escape().code;
  io.KeyMap[ImGuiKey_KeyPadEnter] = Key::NumpadEnter().code;
  io.KeyMap[ImGuiKey_A] = Key::A().code;
  io.KeyMap[ImGuiKey_C] = Key::C().code;
  io.KeyMap[ImGuiKey_V] = Key::V().code;
  io.KeyMap[ImGuiKey_X] = Key::X().code;
  io.KeyMap[ImGuiKey_Y] = Key::Y().code;
  io.KeyMap[ImGuiKey_Z] = Key::Z().code;
}

void ImGuiStartFrameController::Update(std::chrono::microseconds delta_time) {
  ImGui::SetCurrentContext(imgui_context_.get());
  ImGuiIO& io = ImGui::GetIO();
  for (int i = 0; i < 5; ++i) {
    io.MouseDown[i] = mouse_button_pressed_[i] || GetMouseButtonState(GetMouseButtonFromImGuiIndex(i));
    mouse_button_pressed_[i] = false;
  }
  ImGui::NewFrame();
}

void ImGuiStartFrameController::ProcessEvent(Event* event) {
  ImGui::SetCurrentContext(imgui_context_.get());
  ImGuiIO& io = ImGui::GetIO();

  if (event->type() == MouseMoveEvent::TYPE) {
    MouseMoveEvent* mouse_event = down_cast<MouseMoveEvent*>(event);
    io.MousePos.x = mouse_event->screen_space_position().x;
    io.MousePos.y = mouse_event->screen_space_position().y;
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
    io.KeyShift = GetKeyState(Key::ShiftLeft()) || GetKeyState(Key::ShiftRight());
    io.KeyCtrl = GetKeyState(Key::ControlLeft()) || GetKeyState(Key::ControlRight());
    io.KeyAlt = GetKeyState(Key::AltLeft()) || GetKeyState(Key::AltRight());
    io.KeySuper = GetKeyState(Key::MetaLeft()) || GetKeyState(Key::MetaRight());
    if (io.WantCaptureKeyboard) {
      event->StopPropagation();
    }
  } else if (event->type() == KeyReleaseEvent::TYPE) {
    KeyReleaseEvent* key_event = down_cast<KeyReleaseEvent*>(event);
    SDL_assert(key_event->key().code < IM_ARRAYSIZE(io.KeysDown));
    io.KeysDown[key_event->key().code] = false;
    io.KeyShift = GetKeyState(Key::ShiftLeft()) || GetKeyState(Key::ShiftRight());
    io.KeyCtrl = GetKeyState(Key::ControlLeft()) || GetKeyState(Key::ControlRight());
    io.KeyAlt = GetKeyState(Key::AltLeft()) || GetKeyState(Key::AltRight());
    io.KeySuper = GetKeyState(Key::MetaLeft()) || GetKeyState(Key::MetaRight());
    if (io.WantCaptureKeyboard) {
      event->StopPropagation();
    }
  }
}

}  // namespace ovis
