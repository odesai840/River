#include "Allocator.h"
#include <cstring>

RiverCore::Allocator::Allocator(int size, int count) : size(size), count(count), usedNum(0)
{
    memory = new char[size*count];

    used = new bool[count];
    std::memset(used, 0, count * sizeof(bool));
}

int RiverCore::Allocator::alloc()
{
    for (int i = 0; i < count; i++) {
        if (!used[i]) {
            used[i] = true;
            usedNum++;
            return i;
        }
    }
    return -1;
}

void RiverCore::Allocator::freeSlot(int id)
{
    if (id > -1 && id < count && used[id]) {
        used[id] = false;
        usedNum--;
    }
}

void RiverCore::Allocator::free()
{
    delete[] memory;
    delete[] used;
}

void *RiverCore::Allocator::getPointer(int id)
{
    return memory + (id * size);
}

int RiverCore::Allocator::getUsed() const
{
    return usedNum;
}

int RiverCore::Allocator::getTotal() const
{
    return count;
}

float RiverCore::Allocator::getUsedPercent() const
{
    if (count == 0) {
        return 0.0f;
    }

    return (float)usedNum/(float)count * 100.0f;
}
