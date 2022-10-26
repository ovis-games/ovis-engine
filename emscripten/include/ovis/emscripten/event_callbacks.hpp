#pragma once

#if OVIS_EMSCRIPTEN
#include <emscripten/html5.h>

namespace ovis {

EM_BOOL HandleWheelEvent(int event_type, const EmscriptenWheelEvent* wheel_event, void* viewport);
EM_BOOL HandleKeyDownEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport);
// TODO: the keypress event is actually deprecated and should be replaced by the keydown event. But there is no easy way
// to check whether
EM_BOOL HandleKeyPressEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport);
EM_BOOL HandleKeyUpEvent(int event_type, const EmscriptenKeyboardEvent* keyboard_event, void* viewport);
EM_BOOL HandleMouseMoveEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport);
EM_BOOL HandleMouseDownEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport);
EM_BOOL HandleMouseUpEvent(int event_type, const EmscriptenMouseEvent* mouse_event, void* viewport);

// It is related to input in the sense that we need to reset all key states when the canvas loses focus.
EM_BOOL HandleBlurEvent(int event_type, const EmscriptenFocusEvent* focus_event, void* viewport);

}  // namespace ovis

#endif
