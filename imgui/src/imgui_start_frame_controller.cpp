#include <imgui.h>

#include <ovis/utils/down_cast.hpp>
#include <ovis/utils/log.hpp>
#include <ovis/utils/platform.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/input/key_events.hpp>
#include <ovis/input/mouse_button.hpp>
#include <ovis/input/mouse_events.hpp>
#include <ovis/input/text_input_event.hpp>
#include <ovis/imgui/imgui_render_pass.hpp>
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
  LoadFont("OpenSans-Regular");
  LoadFont("Inconsolata-Regular");

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
  io.DeltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(delta_time).count(); 

  if (font_added_) {
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&font_atlas_pixels_, &font_atlas_width_, &font_atlas_height_);
    font_added_ = false;
    reload_font_atlas_ = true;
    // TODO: can I release the font memory now? (the one in the map)
    RenderingViewport* viewport = dynamic_cast<RenderingViewport*>(scene()->main_viewport());
    SDL_assert(viewport != nullptr);
    auto render_pass = viewport->GetRenderPass<ImGuiRenderPass>(std::string(ImGuiRenderPass::Name()));
    if (render_pass != nullptr) {
      render_pass->ReloadFontAtlas(this);
      SDL_assert(reload_font_atlas_ == false);
    }
  }

  auto* viewport = scene()->main_viewport();
  const Vector2 viewport_dimensions = viewport != nullptr ? viewport->GetDimensions() : Vector2::Zero();
  if (io.Fonts->Fonts.Size > 0 && viewport_dimensions != Vector2::Zero()) {
    io.DisplaySize = viewport_dimensions;
    ImGui::NewFrame();
    frame_started_ = true;
  } else {
    frame_started_ = false;
    LogV("Skip imgui update resources not loaded yet or invalid viewport!");
  }
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
    if (io.WantTextInput) {
      event->StopPropagation();
    }
  } else if (event->type() == KeyPressEvent::TYPE) {
    KeyPressEvent* key_event = down_cast<KeyPressEvent*>(event);
    SDL_assert(key_event->key().code < IM_ARRAYSIZE(io.KeysDown));
    io.KeysDown[key_event->key().code] = true;
    io.KeyShift = IsKeyPressed(Key::ShiftLeft()) || IsKeyPressed(Key::ShiftRight());
    io.KeyCtrl = IsKeyPressed(Key::ControlLeft()) || IsKeyPressed(Key::ControlRight());
    io.KeyAlt = IsKeyPressed(Key::AltLeft()) || IsKeyPressed(Key::AltRight());
    io.KeySuper = IsKeyPressed(Key::MetaLeft()) || IsKeyPressed(Key::MetaRight());
    if (io.WantCaptureKeyboard && !io.WantTextInput) {
      event->StopPropagation();
    }
  } else if (event->type() == KeyReleaseEvent::TYPE) {
    KeyReleaseEvent* key_event = down_cast<KeyReleaseEvent*>(event);
    SDL_assert(key_event->key().code < IM_ARRAYSIZE(io.KeysDown));
    io.KeysDown[key_event->key().code] = false;
    io.KeyShift = IsKeyPressed(Key::ShiftLeft()) || IsKeyPressed(Key::ShiftRight());
    io.KeyCtrl = IsKeyPressed(Key::ControlLeft()) || IsKeyPressed(Key::ControlRight());
    io.KeyAlt = IsKeyPressed(Key::AltLeft()) || IsKeyPressed(Key::AltRight());
    io.KeySuper = IsKeyPressed(Key::MetaLeft()) || IsKeyPressed(Key::MetaRight());
    if (io.WantCaptureKeyboard && !io.WantTextInput) {
      event->StopPropagation();
    }
  }
}

ImFont* ImGuiStartFrameController::LoadFont(std::string_view asset, float size, std::vector<std::pair<ImWchar, ImWchar>> ranges) {
  const auto map_key = std::make_pair(std::string(asset), size);
  if (auto font = fonts_.find(map_key); font != fonts_.end()) {
    return font->second.font;
  }

  AssetLibrary* asset_library = GetApplicationAssetLibrary()->Contains(asset) ? GetApplicationAssetLibrary() : GetEngineAssetLibrary();

  if (const auto asset_type = asset_library->GetAssetType(asset); !asset_type || *asset_type != "font") {
    LogE("Could not load font: {}", asset);
    return nullptr;
  }

  const auto asset_file_types = asset_library->GetAssetFileTypes(asset);
  const bool is_ttf_file = std::find(asset_file_types.begin(), asset_file_types.end(), "ttf") != asset_file_types.end();

  auto font_data = GetEngineAssetLibrary()->LoadAssetBinaryFile(asset, is_ttf_file ? "ttf" : "otf");
  if (font_data) {
    ImGui::SetCurrentContext(imgui_context_.get());
    ImGuiIO& io = ImGui::GetIO();

    auto inserted = fonts_.insert(std::make_pair(map_key, FontData{std::move(*font_data), {}, nullptr}));
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;

    if (ranges.size() > 0) {
      std::vector<ImWchar>* glyph_ranges = &inserted.first->second.glyph_ranges;

      glyph_ranges->reserve(ranges.size() * 2 + 1);
      for (auto range : ranges) {
        glyph_ranges->push_back(range.first);
        glyph_ranges->push_back(range.second);
      }
      glyph_ranges->push_back(0);
      config.GlyphRanges = glyph_ranges->data();
    }

    font_added_ = true;
    return inserted.first->second.font = io.Fonts->AddFontFromMemoryTTF(
      inserted.first->second.font_file.data(),
      inserted.first->second.font_file.size(),
      size,
      &config);
  } else {
    LogE("Could not load font {}: only ttf and otf files are supported!", asset);
    return nullptr;
  }
}

ImFont* ImGuiStartFrameController::GetFont(std::string_view name, float size) {
  const auto map_key = std::make_pair(std::string(name), size);
  if (auto font = fonts_.find(map_key); font != fonts_.end()) {
    return font->second.font;
  } else {
    return nullptr;
  }
}

}  // namespace ovis
