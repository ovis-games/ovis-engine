#pragma once

#if OVIS_EMSCRIPTEN

#include <string>
#include <functional>
#include <map>

#include <ovis/core/file.hpp>

namespace ovis {

enum class RequestMethod { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

struct FetchProgress {
  uint64_t num_bytes;
  uint64_t data_offset;
  uint64_t total_bytes;
};

struct FetchOptions {
  RequestMethod method = RequestMethod::GET;
  std::map<std::string, std::string> headers;
  bool with_credentials = false;

  using SuccessCallback = std::function<void()>;
  using ErrorCallback = std::function<void()>;
  using ProgressCallback = std::function<void(const FetchProgress&)>;
  using ReadyStateChangeCallback = std::function<void()>;

  SuccessCallback on_success;
  ErrorCallback on_error;
  ProgressCallback on_progress;
  ReadyStateChangeCallback on_ready_state_change;
};

void Fetch(const std::string& url, const FetchOptions& options, Blob body = {});

}  // namespace ovis

#endif
