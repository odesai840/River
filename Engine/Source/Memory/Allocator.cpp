#include "Allocator.h"
#include <cstring>

namespace RiverCore {

Allocator::Allocator(int size, int count) : size(size), count(count), usedNum(0) {
    memory = new char[size * count];
    used = new bool[count];
    std::memset(used, 0, count * sizeof(bool));
}

Allocator::~Allocator() {
    delete[] memory;
    delete[] used;
}

int Allocator::Alloc() {
    for (int i = 0; i < count; i++) {
        if (!used[i]) {
            used[i] = true;
            usedNum++;
            return i;
        }
    }
    return -1;
}

void Allocator::FreeSlot(int id) {
    if (id >= 0 && id < count && used[id]) {
        used[id] = false;
        usedNum--;
    }
}

void* Allocator::GetPointer(int id) {
    if (id < 0 || id >= count) {
        return nullptr;
    }
    return memory + (id * size);
}

int Allocator::GetUsed() const {
    return usedNum;
}

int Allocator::GetTotal() const {
    return count;
}

float Allocator::GetUsedPercent() const {
    if (count == 0) {
        return 0.0f;
    }
    return (float)usedNum / (float)count * 100.0f;
}

}
