
internal inline bool
IsObfuscating(World *world, int x, int y, int z)
{
    int chunk_x = x/CHUNKSIZE_X;
    int chunk_y = y/CHUNKSIZE_Y;
    if(z < 0 || z >= CHUNKSIZE_Z || 
            x < 0 || y < 0 || 
            chunk_x >= world->x_chunks || chunk_y >= world->y_chunks)
    {
        return false;
    }
    Chunk *chunk = ChunkAt(world, chunk_x, chunk_y);
    int x_in_chunk = x-chunk->x_offset;
    int y_in_chunk = y-chunk->y_offset;
    return BlockAt(chunk->blocks, x_in_chunk, y_in_chunk, z)!=0;
}

void
GenerateChunkMesh(World *world, Chunk *chunk)
{
    ClearMesh(chunk->mesh);

    IterBlocks(x, y, z)
    {
        U32 block = BlockAt(chunk->blocks, x, y, z);
        if(block)
        {
            int world_x = x+chunk->x_offset;
            int world_y = y+chunk->y_offset;
            int world_z = z+chunk->z_offset;
            Vec3_U16 dims = V3U16(CHUNK_UNIT, CHUNK_UNIT, CHUNK_UNIT);
            Vec3_U16 p000 = V3U16(CHUNK_UNIT*x, CHUNK_UNIT*y, CHUNK_UNIT*z);
            Vec3_U16 p001 = p000+V3U16(dims.x, 0,      0);
            Vec3_U16 p010 = p000+V3U16(0,      dims.y, 0);
            Vec3_U16 p011 = p000+V3U16(dims.x, dims.y, 0);
            Vec3_U16 p100 = p000+V3U16(0,      0,      dims.z);
            Vec3_U16 p101 = p000+V3U16(dims.x, 0,      dims.z);
            Vec3_U16 p110 = p000+V3U16(0,      dims.y, dims.z);
            Vec3_U16 p111 = p000+V3U16(dims.x, dims.y, dims.z);

            // Bottom, Top 
            if(!IsObfuscating(world, world_x, world_y, world_z-1))
            {
                PushQuad(chunk->mesh, p010, p011, p001, p000, V3I16(0,0, -I16MAX), block);
            }
            if(!IsObfuscating(world, world_x, world_y, world_z+1))
            {
                PushQuad(chunk->mesh, p100, p101, p111, p110, V3I16(0,0, I16MAX), block);
            }

            // Left Right
            if(!IsObfuscating(world, world_x-1, world_y, world_z))
            {
                PushQuad(chunk->mesh, p000, p100, p110, p010, V3I16(-I16MAX, 0, 0), block);
            }
            if(!IsObfuscating(world, world_x+1, world_y, world_z))
            {
                PushQuad(chunk->mesh, p011, p111, p101, p001, V3I16(I16MAX, 0, 0), block);
            }

            // Up Down
            if(!IsObfuscating(world, world_x, world_y-1, world_z))
            {
                PushQuad(chunk->mesh, p000, p001, p101, p100, V3I16(0, -I16MAX, 0), block);
            }
            if(!IsObfuscating(world, world_x, world_y+1, world_z))
            {
                PushQuad(chunk->mesh, p110, p111, p011, p010, V3I16(0, I16MAX, 0), block);
            }
        }
    }
}

void
GenerateChunk(World *world, Chunk *chunk)
{
    size_t n_blocks = CHUNKSIZE_X*CHUNKSIZE_Y*CHUNKSIZE_Z;
    memset(chunk->blocks, 0, n_blocks*sizeof(U32));

    U32 water_color = 0xd4f1f9ff;
    U32 sand_color = 0xc2b280ff;

    for(int y = 0; y < CHUNKSIZE_Y; y++)
    for(int x = 0; x < CHUNKSIZE_X; x++)
    {
        R32 xx = world->scale*(chunk->x_offset+x);
        R32 yy = world->scale*(chunk->y_offset+y);
        int height = world->mid_level+world->magnitude*fnlGetNoise2D(&world->noise_state, xx, yy);
        height = Min(height, CHUNKSIZE_Z);
        for(int z = 0; z < height; z++)
        {
            U32 color = 0U;
            if(z < 6)
            {
                color = COL32(
                        RandomI32(160, 200),
                        RandomI32(160, 200),
                        RandomI32(160, 200),
                        255);
            } 
            else if(z < world->water_level+1)
            {
                color = sand_color;
            }
            else
            {
                color = COL32(
                        RandomI32(0, 100),
                        RandomI32(200, 255),
                        RandomI32(0, 100),
                        255);
            }
            BlockAt(chunk->blocks, x, y, z) = color;
        }
        if(height > 0 && height < world->water_level)
        {
            for(int z = height; z < world->water_level; z++)
            {
                BlockAt(chunk->blocks, x, y, z) = water_color;
            }
        }
    }
    chunk->is_dirty = true;
}

Chunk *
CreateChunk(World *world, int x_idx, int y_idx)
{
    Chunk *chunk = PushZeroStruct(world->arena, Chunk);
    chunk->x_offset = x_idx*CHUNKSIZE_X;
    chunk->y_offset = y_idx*CHUNKSIZE_Y;
    size_t n_blocks = CHUNKSIZE_X * CHUNKSIZE_Y * CHUNKSIZE_Z;
    chunk->blocks = PushZeroArray(world->arena, U32, n_blocks);
    // Create mesh: TODO: calculate exact number of required vertices.
    chunk->mesh = CreateChunkMesh(world->arena, 150000, 150000);
    return chunk;
}

