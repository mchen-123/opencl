#include "context.hpp"
#include "device.hpp"

UserContext* userContextCreate(Device *dev) {
    return new UserContext();
}

int userContextDestroy(UserContext *ctx) {
    delete ctx;
    return 0;
}

void Context::ctxAddStream(std::shared_ptr<Stream> s) {
  streamSet_.insert(s);
}

Stream *Context::streamCreate(uint32_t flags, Stream::Priority priority) {
    auto s = std::make_shared<Stream>();
    ctxAddStream(s);
    return s.get();
}

bool Context::StreamPriorityCompare::operator()(const std::shared_ptr<Stream> left,
                                                const std::shared_ptr<Stream> right) const {
  if (left == nullptr || right == nullptr) return left < right;

  if (left->getPriority() != right->getPriority()) {
    return left->getPriority() < right->getPriority();
  }

  return left < right;
}

// int contextPushCurrent(Context *ctx) {
//   ctxStackPush(ctx)
//   return ;
// }