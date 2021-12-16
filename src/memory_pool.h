
// Very very very naive implementation. Improve by using bit fields.

#define PoolAlloc(pool, type)\
    (type *)pool->Alloc()

struct Memory_Pool
{
    int size;
    int capacity;
    bool *used_slots;
    int last_used;
    size_t slot_size;
    U8 *data;

    void *Alloc()
    {
        Assert(size < capacity);
        int idx = last_used;
        for(int checkIdx = 0; 
                checkIdx < capacity;
                checkIdx++)
        {
            if(++idx >= capacity)
            {
                idx = 0;
            }
            if(!used_slots[idx])
            {
                break;
            }
        }
        used_slots[idx] = true;
        last_used = idx;
        size++;
        return data+idx*slot_size;
    }

    Memory_Arena AllocMemoryBlock()
    {
        Memory_Arena block = {};
        block.size = slot_size;
        block.base = (U8 *)Alloc();
        return block;
    }

    void Free(void *element)
    {
        ssize_t diff = (ssize_t)((ssize_t)element-(ssize_t)data);
        int idx = (int)(diff/slot_size);
        printf(__FILE__ " Free idx = %d\n", idx);
        Assert(idx >= 0 && idx < capacity);
        Assert(used_slots[idx]==true);
        used_slots[idx] = false;
        size--;
    }
};

Memory_Pool *
PushMemoryPool(Memory_Arena *arena, int capacity, size_t element_size)
{
    Memory_Pool *pool = PushStruct(arena, Memory_Pool);
    *pool = (Memory_Pool){0};
    pool->slot_size = element_size;
    pool->capacity = capacity;
    pool->used_slots = PushZeroArray(arena, bool, capacity);
    pool->data = PushZeroArray(arena, U8, capacity*element_size);
    return pool;
}

