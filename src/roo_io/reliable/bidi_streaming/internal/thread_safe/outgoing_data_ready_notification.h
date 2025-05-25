#pragma once

#include "roo_io/reliable/bidi_streaming/internal/thread_safe/compile_guard.h"
#ifdef ROO_USE_THREADS

#include <condition_variable>
#include <thread>

namespace roo_io {
namespace internal {

class OutgoingDataReadyNotification {
 public:
  OutgoingDataReadyNotification() : mutex_(), has_data_to_send_(false), cv_() {}

  void notify() {
    std::unique_lock<std::mutex> guard(mutex_);
    has_data_to_send_ = true;
    cv_.notify_all();
  }

  bool await(long micros) {
    std::unique_lock<std::mutex> guard(mutex_);
    cv_.wait_for(guard, std::chrono::microseconds(micros),
                 [this] { return has_data_to_send_; });
    if (has_data_to_send_) {
      has_data_to_send_ = false;
      return true;
    }
    return false;
  }

 private:
  std::mutex mutex_;
  bool has_data_to_send_;
  std::condition_variable cv_;
};

}  // namespace internal
}  // namespace roo_io

#endif  // ROO_USE_THREADS