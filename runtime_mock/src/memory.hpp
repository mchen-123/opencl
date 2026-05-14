#ifndef RUNTIME_MOCK_MEMORY_HPP
#define RUNTIME_MOCK_MEMORY_HPP

#include <vector>
#include <memory>

class mem {
public:
    explicit mem(int d) : data_(d){}
private:
    int data_;
};

#endif