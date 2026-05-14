#include "device.hpp"

std::vector<std::shared_ptr<Device>> getAllDevices() {
  Device *dev = new Device();
  // Device *dev2 = new Device();
  std::vector<std::shared_ptr<Device>> v;
  v.push_back(std::shared_ptr<Device>(dev));
  // v.push_back(std::shared_ptr<Device>(dev2));
  return v;
}