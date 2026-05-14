#ifndef RUNTIME_MOCK_CONTEXT_HPP
#define RUNTIME_MOCK_CONTEXT_HPP

#include <vector>
#include <memory>
#include <set>

#include "device.hpp"
#include "stream.hpp"

class Context{
  Stream * streamCreate(uint32_t flags,
            Stream::Priority priority);  // bareStreamCreate + save stream
  struct StreamPriorityCompare {
    bool operator()(const std::shared_ptr<Stream> left, const std::shared_ptr<Stream> right) const;
  };
  std::set<std::shared_ptr<Stream>, StreamPriorityCompare> streamSet_;
  void ctxAddStream(std::shared_ptr<Stream> s);
};

class UserContext{
};

UserContext* userContextCreate(Device *dev);

int userContextDestroy(UserContext *ctx);

#endif // RUNTIME_MOCK_DEVICE_HPP