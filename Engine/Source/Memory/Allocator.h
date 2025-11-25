#ifndef ALLOCATOR_H
#define ALLOCATOR_H

namespace RiverCore {
    class Allocator {
        
        public:
            Allocator(int size, int count);
            int alloc();
            void freeSlot(int id);
            void free();
            void* getPointer(int id);

            int getUsed() const;
            int getTotal() const;
            float getUsedPercent() const;
        private:
            int size;
            int count;
            char* memory;
            bool* used;
            int usedNum;
    };
}

#endif