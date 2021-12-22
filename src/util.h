
#define DebugOut(args...) printf(args); printf("\n");

#define Assert(statement) if(!(statement)) \
    {printf(__FILE__ ":%d Assert failed ("#statement")\n", __LINE__);__builtin_trap();}

#define InvalidCodePath\
    {printf(__FILE__ ":%d  Invalid code path.\n", __LINE__);__builtin_trap();}

#define ArraySize(array) \
    sizeof(array)/sizeof(array[0])

typedef float R32;
typedef double R64;

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

#define local_persist static
#define global_variable static
#define internal static

#define I16MAX 32767

#define GLError() {GLenum error = glGetError(); if(error) DebugOut("OpenGl error %d", error);

#define Sign(x) ((x) < 0 ? -1 : 1)

char *
ReadEntireFile(const char *path)
{
    FILE *file = fopen(path, "r");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *buffer = (char *)malloc(size+1);
        fread(buffer, 1, size, file);
        buffer[size] = 0;
        fclose(file);
        return buffer;
    }
    else
    {
        fprintf(stderr, "Cant open file %s.", path);
        return 0;
    }
}

R32
RandomR32(float min, float max)
{
    R32 r = rand()/(R32)RAND_MAX;
    return min+r*(max-min);
}

U32
RandomI32(I32 min, I32 max)
{
    U32 r = min+(rand() % (max-min));
    return r;
}

#define COL32(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | a)

R32 
MinR32(int n, int *idx, ...)
{
    va_list args;
    va_start(args, idx);
    R32 min = FLT_MAX;
    for(int i = 0; i < n; i++)
    {
        R64 value = va_arg(args, R64);
        if(value < min)
        {
            min = value;
            *idx = i;
        }
    }
    va_end(args);
    return min;
}

internal inline R32 Min(R32 x, R32 y) { return x < y ? x : y; }
internal inline I32 Min(I32 x, I32 y) { return x < y ? x : y; }

internal inline R32 Max(R32 x, R32 y) { return x > y ? x : y; }
internal inline I32 Max(I32 x, I32 y) { return x > y ? x : y; }

R32 
GetSmallestPositiveNumber(int n, int *idx, ...)
{
    va_list args;
    va_start(args, idx);
    R32 min = FLT_MAX;
    for(int i = 0; i < n; i++)
    {
        R64 value = va_arg(args, R64);
        if(value > 0.0 && value < min)
        {
            min = value;
            *idx = i;
        }
    }
    va_end(args);
    return min;
}




