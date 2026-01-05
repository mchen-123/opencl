#ifndef RUNTIME_MOCK_STREAM_HPP
#define RUNTIME_MOCK_STREAM_HPP

#include <vector>
#include <memory>

class Stream {
public:
  enum Priority : int {
    High = -1,
    Normal = 0,
    Low = 1
  };
  Priority getPriority() const { return priority_; }
  Priority priority_;
};


#endif // RUNTIME_MOCK_DEVICE_HPP