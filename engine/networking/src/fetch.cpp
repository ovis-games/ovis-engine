#include <array>
#include <vector>

#include <SDL2/SDL_assert.h>
#if OVIS_EMSCRIPTEN
#include <emscripten/fetch.h>
#endif

#include <ovis/networking/fetch.hpp>

namespace ovis {

#if OVIS_EMSCRIPTEN
namespace {

const std::array<const char*, 9> MethodStrings = {"GET",     "HEAD",    "POST",  "PUT",  "DELETE",
                                                  "CONNECT", "OPTIONS", "TRACE", "PATCH"};

struct FetchContext {
  Blob body;
  FetchOptions::SuccessCallback on_success;
  FetchOptions::ErrorCallback on_error;
  FetchOptions::ProgressCallback on_progress;
  FetchOptions::ReadyStateChangeCallback on_ready_state_change;

  static void OnSuccess(emscripten_fetch_t* fetch) {
    FetchContext* context = reinterpret_cast<FetchContext*>(fetch->userData);
    if (context->on_success) {
      FetchResponse response;
      response.status_code = fetch->status;
      response.body = fetch->data;
      response.content_length = fetch->totalBytes;
      context->on_success(response);
    }
    delete context;
    emscripten_fetch_close(fetch);
  }

  static void OnError(emscripten_fetch_t* fetch) {
    FetchContext* context = reinterpret_cast<FetchContext*>(fetch->userData);
    if (context->on_error) {
      FetchResponse response;
      response.status_code = fetch->status;
      response.body = fetch->data;
      response.content_length = fetch->totalBytes;
      context->on_error(response);
    }
    delete context;
    emscripten_fetch_close(fetch);
  }

  static void OnProgress(emscripten_fetch_t* fetch) {
    FetchContext* context = reinterpret_cast<FetchContext*>(fetch->userData);
    if (context->on_progress) {
      FetchProgress progress;
      progress.num_bytes = fetch->numBytes;
      progress.data_offset = fetch->dataOffset;
      progress.total_bytes = fetch->totalBytes;
      context->on_progress(progress);
    }
  }

  static void OnReadyStateChange(emscripten_fetch_t* fetch) {
    FetchContext* context = reinterpret_cast<FetchContext*>(fetch->userData);
    if (context->on_ready_state_change) {
      context->on_ready_state_change();
    }
  }
};

}  // namespace

void Fetch(const std::string& url, const FetchOptions& options, Blob body) {
  SDL_assert(static_cast<int>(options.method) < MethodStrings.size());

  FetchContext* context = new FetchContext();
  context->on_success = options.on_success;
  context->on_error = options.on_error;
  context->on_progress = options.on_progress;
  context->on_ready_state_change = options.on_ready_state_change;
  context->body = std::move(body);

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, MethodStrings[static_cast<int>(options.method)]);

  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

  attr.onsuccess = &FetchContext::OnSuccess;
  attr.onerror = &FetchContext::OnError;
  attr.onprogress = &FetchContext::OnProgress;
  attr.onreadystatechange = &FetchContext::OnReadyStateChange;

  std::vector<const char*> additional_headers;
  additional_headers.reserve(options.headers.size() * 2 + 1);
  for (const auto& header : options.headers) {
    additional_headers.push_back(header.first.c_str());
    additional_headers.push_back(header.second.c_str());
  }
  additional_headers.push_back(nullptr);
  attr.requestHeaders = additional_headers.data();

  attr.userData = context;
  attr.requestData = reinterpret_cast<const char*>(context->body.data());
  attr.requestDataSize = context->body.size();

  attr.withCredentials = options.with_credentials;

  emscripten_fetch(&attr, url.c_str());
}
#else
void Fetch(const std::string& url, const FetchOptions& options, Blob body) {
  FetchResponse response;
  response.status_code = 501;
  options.on_error(response);
}
#endif

}  // namespace ovis

