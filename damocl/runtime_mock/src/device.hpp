#ifndef RUNTIME_MOCK_DEVICE_HPP
#define RUNTIME_MOCK_DEVICE_HPP

#include <vector>
#include <memory>

class Device {
};

std::vector<std::shared_ptr<Device>> getAllDevices();

#endif // RUNTIME_MOCK_DEVICE_HPP