#include "ovis/sdl/sdl_event_processor.hpp"
#include "SDL_events.h"
#include "ovis/core/application.hpp"
#include "ovis/sdl/sdl_window.hpp"
#include "ovis/utils/log.hpp"

namespace ovis {

Result<> SDLEventProcessor::Execute(const double& delta_time) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        LogI("Application quit requested via SDL");
        QuitApplicationLoop();
        break;

      case SDL_WINDOWEVENT:
        if (auto window = SDLWindow::GetWindowById(event.window.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (auto window = SDLWindow::GetWindowById(event.key.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_TEXTEDITING:
        if (auto window = SDLWindow::GetWindowById(event.edit.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_TEXTINPUT:
        if (auto window = SDLWindow::GetWindowById(event.text.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_MOUSEMOTION:
        if (auto window = SDLWindow::GetWindowById(event.motion.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (auto window = SDLWindow::GetWindowById(event.button.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_MOUSEWHEEL:
        if (auto window = SDLWindow::GetWindowById(event.wheel.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_JOYAXISMOTION:
      case SDL_JOYBALLMOTION:
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
      case SDL_JOYDEVICEADDED:
      case SDL_JOYDEVICEREMOVED:
      case SDL_CONTROLLERAXISMOTION:
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
      case SDL_CONTROLLERDEVICEADDED:
      case SDL_CONTROLLERDEVICEREMOVED:
      case SDL_CONTROLLERDEVICEREMAPPED:
      case SDL_AUDIODEVICEADDED:
      case SDL_AUDIODEVICEREMOVED:
      case SDL_USEREVENT:
      case SDL_SYSWMEVENT:
        // TODO: handle this event correctly
        break;

      case SDL_FINGERMOTION:
      case SDL_FINGERDOWN:
      case SDL_FINGERUP:
        if (auto window = SDLWindow::GetWindowById(event.tfinger.windowID); window) {
          window->ProcessEvent(event);
        }
        break;

      case SDL_MULTIGESTURE:
      case SDL_DOLLARGESTURE:
      case SDL_DOLLARRECORD:
        break;

      case SDL_DROPFILE:
      case SDL_DROPTEXT:
      case SDL_DROPBEGIN:
      case SDL_DROPCOMPLETE:
        if (auto window = SDLWindow::GetWindowById(event.drop.windowID); window) {
          window->ProcessEvent(event);
        }
        break;
    }
  }

  return Success;
}

}  // namespace ovis
