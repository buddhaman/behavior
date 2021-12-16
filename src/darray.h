
template <typename T> 
struct DArray
{
    int capacity;
    int size;
    T *data;

    inline T& operator[](int idx) { Assert(idx>=0 && idx < size); return data[idx]; }
    inline T *GetRef(int idx) { Assert(idx >= 0 && idx < size); return data+idx; }
    inline T Get(int idx) { Assert(idx >= 0 && idx < size); return data[idx]; }
    inline bool IsEmpty() { return size==0; }
    inline void PushBack(T value) { Assert(size < capacity); data[size++] = value; }
    inline T* PushBack() { Assert(size < capacity); return data+size++; }

    inline void RemoveIdxUnordered(int idx) 
    { 
        Assert(idx >= 0 && idx < size);
        if(idx!=size-1)
        {
            data[idx] = data[size-1];
        }
        size--;
    }

    inline void RemoveIdxOrdered(int idx)
    {
        Assert(idx >= 0 && idx < size);
        if(idx!=size-1)
        {
            memmove(data+idx, data+idx+1, sizeof(T)*(size-1-idx));
        }
        size--;
    }

    inline int IndexOf(T value)
    {
        for(int idx = 0; idx < size; idx++)
        {
            if(data[idx] == value) return idx;
        }
        return -1;
    }

    inline void RemoveUnordered(T value)
    {
        int idx = IndexOf(value);
        RemoveIdxUnordered(idx);
    }

};

template <typename T> DArray<T> 
CreateDArray(Memory_Arena *arena, int capacity)
{
    DArray<T> array = {0};
    array.capacity = capacity;
    array.data = PushArray(arena, T, capacity);
    return array;
}

template <typename T> DArray<T> *
PushDArray(Memory_Arena *arena, int capacity)
{
    DArray<T> *array = PushStruct(arena, DArray<T>);
    *array = (DArray<T>){0};
    array->capacity = capacity;
    array->data = PushArray(arena, T, capacity);
    return array;
}

template <typename T> DArray<T> *
CreateDArrayMalloc(int capacity)
{
    DArray<T> *array = (DArray<T> *)malloc(sizeof(DArray<T>) + sizeof(T)*capacity);
    *array = (DArray<T>){};
    array->capacity = capacity;
    array->data = (T *)(array+1);
    return array;
}

template <typename T> DArray<T> *
CreateDArrayFromMemory(void *memory, int capacity)
{
    DArray<T> *array = (DArray<T> *)memory;
    *array = (DArray<T>){};
    array->capacity = capacity;
    array->data = (T *)(array+1);
    return array;
}


