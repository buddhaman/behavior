
#define CHUNK_UNIT 256

#define CHUNKSIZE_X 16
#define CHUNKSIZE_Y 16
#define CHUNKSIZE_Z 128

#define BlockAt(array, x, y, z) (array)[(z)*CHUNKSIZE_X*CHUNKSIZE_Y+(y)*CHUNKSIZE_X+(x)]

#define IterBlocks(x, y, z) \
    for(int z = 0; z < CHUNKSIZE_Z; z++) \
    for(int y = 0; y < CHUNKSIZE_Y; y++) \
    for(int x = 0; x < CHUNKSIZE_X; x++) 

struct Chunk
{
    volatile bool is_dirty;
    volatile bool is_mesh_dirty;
    int x_offset;
    int y_offset;
    int z_offset;
    U32 *blocks;
    Mesh *mesh;
};

