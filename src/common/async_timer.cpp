#include <aerial_autonomy/common/async_timer.h>

#include <stdexcept>

AsyncTimer::AsyncTimer(std::function<void()> function,
                       std::chrono::duration<double> timer_duration)
    : function_(function), timer_duration_(timer_duration), running_(false) {}

AsyncTimer::~AsyncTimer() { stop(); }

void AsyncTimer::stop() {
  running_ = false;
  if (timer_thread_.joinable()) {
    timer_thread_.join();
  }
}

void AsyncTimer::start() {
  if (!running_) {
    running_ = true;
    timer_thread_ = std::thread(std::bind(&AsyncTimer::functionTimer, this));
  } else {
    throw std::logic_error("Cannot start AsyncTimer twice!");
  }
}

void AsyncTimer::functionTimer() {
  while (running_) {
    auto start = std::chrono::high_resolution_clock::now();
    function_();
    std::this_thread::sleep_until(start + timer_duration_);
  }
}

void AsyncTimer::setDuration(std::chrono::duration<double> duration) {
  if (!running_) {
    timer_duration_ = duration;
  } else {
    stop();
    timer_duration_ = duration;
    start();
  }
}
