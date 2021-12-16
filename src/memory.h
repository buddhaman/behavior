
typedef struct
{
    size_t size;
    size_t used;
    uint8_t *base;
} Memory_Arena;

internal inline void *
PushMemory_(Memory_Arena *arena, size_t size)
{
    Assert(arena->used+size <= arena->size);
    void *result = arena->base+arena->used;
    arena->used+=size;
    return result;
}

internal inline void *
PushAndZeroMemory_(Memory_Arena *arena, size_t size)
{
    void *mem = PushMemory_(arena, size);
    memset(mem, 0, size);
    return mem;
}

Memory_Arena *
CreateMemoryArena(size_t size)
{
    Memory_Arena *arena = (Memory_Arena *)malloc(sizeof(Memory_Arena)+size);
    *arena = (Memory_Arena){0};
    arena->base = (uint8_t *)(arena+1);
    arena->size = size;
    return arena;
}

void
ClearMemoryArena(Memory_Arena *arena)
{
    memset(arena->base, 0, arena->used);
    arena->used = 0;
}

#define PushStruct(arena, type)\
    (type *)PushMemory_(arena, sizeof(type))
#define PushArray(arena, type, n)\
    (type *)PushMemory_(arena, sizeof(type)*n);

#define PushZeroStruct(arena, type)\
    (type *)PushAndZeroMemory_(arena, sizeof(type))
#define PushZeroArray(arena, type, n)\
    (type *)PushAndZeroMemory_(arena, sizeof(type)*n);


