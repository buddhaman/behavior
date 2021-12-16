
void
UpdateWorld(World *world)
{
}

internal inline bool
IsChunkCoordinateInBounds(World *world, int x_chunk, int y_chunk)
{
    return x_chunk >= 0 && y_chunk >= 0 && 
        x_chunk < world->x_chunks && y_chunk < world->y_chunks;
}

internal inline bool
IsInWorldBounds(World *world, Vec3 pos)
{
    return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
        pos.x <= world->x_size && pos.y <= world->y_size && pos.z <= world->z_size;
}

internal inline bool
IsBlockInBounds(World *world, int x, int y, int z)
{
    return x>=0 && y>=0 && z>=0 &&
        x < world->x_size && y < world->y_size && z < world->z_size;
}

// No check whether in bounds. Even in debug mode.
internal inline U32
GetBlockAtNoCheck(World *world, int x, int y, int z)
{
    int chunk_x = x/CHUNKSIZE_X;
    int chunk_y = y/CHUNKSIZE_Y;
    Chunk *chunk = ChunkAt(world, chunk_x, chunk_y);
    int x_in_chunk = x-chunk->x_offset;
    int y_in_chunk = y-chunk->y_offset;
    return BlockAt(chunk->blocks, x_in_chunk, y_in_chunk, z);
}

internal inline void
SetBlockAt(World *world, int x, int y, int z, U32 block)
{
    Assert(IsBlockInBounds(world, x, y, z));
    int chunk_x = x/CHUNKSIZE_X;
    int chunk_y = y/CHUNKSIZE_Y;
    Chunk *chunk = ChunkAt(world, chunk_x, chunk_y);
    int x_in_chunk = x-chunk->x_offset;
    int y_in_chunk = y-chunk->y_offset;
    BlockAt(chunk->blocks, x_in_chunk, y_in_chunk, z) = block;
    chunk->is_dirty = true;
    if(x_in_chunk == 0 && chunk_x > 0) 
        { ChunkAt(world, chunk_x-1, chunk_y)->is_dirty = true; }
    if(y_in_chunk == 0 && chunk_y > 0) 
        { ChunkAt(world, chunk_x, chunk_y-1)->is_dirty = true; }
    if(x_in_chunk == (CHUNKSIZE_X-1) && chunk_x < world->x_chunks-1) 
        { ChunkAt(world, chunk_x+1, chunk_y)->is_dirty = true; }
    if(y_in_chunk == (CHUNKSIZE_Y-1) && chunk_y < world->y_chunks-1) 
        { ChunkAt(world, chunk_x, chunk_y+1)->is_dirty = true; }
}

// Returns the axis or -1 on failure
int 
MoveToNextBoundingPlane(World *world, Ray *ray, R32 epsilon=0.001f)
{
    R32 x0 = -ray->pos.x/ray->dir.x;
    R32 x1 = -(ray->pos.x-world->x_size)/ray->dir.x;

    R32 y0 = -ray->pos.y/ray->dir.y;
    R32 y1 = -(ray->pos.y-world->y_size)/ray->dir.y;

    R32 z0 = -ray->pos.z/ray->dir.z;
    R32 z1 = -(ray->pos.z-CHUNKSIZE_Z)/ray->dir.z;
    int option = -1;
    R32 traversal = GetSmallestPositiveNumber(6, &option, x0, x1, y0, y1, z0, z1);
    if(option >= 0)
    {
        ray->pos += (traversal+epsilon)*ray->dir;
        return option/2;
    }
    return -1;
}

internal inline bool
IsInXBounds(World *world, Vec3 pos)
{
    return pos.x >= 0 && pos.x < world->x_size;
}

internal inline bool
IsInYBounds(World *world, Vec3 pos)
{
    return pos.y >= 0 && pos.y < world->y_size;
}

internal inline bool
IsInZBounds(World *world, Vec3 pos)
{
    return pos.z >= 0 && pos.z < world->z_size;
}

// Is in bounds in the infinite beam along this axis. 
internal inline bool
IsInBoundsExceptForAxis(World *world, Vec3 pos, Axis axis)
{
    switch(axis)
    {
    case X_Axis: return IsInYBounds(world, pos) && IsInZBounds(world, pos);
    case Y_Axis: return IsInXBounds(world, pos) && IsInZBounds(world, pos);
    case Z_Axis: return IsInXBounds(world, pos) && IsInYBounds(world, pos);
    }
    return false;
}

bool
IntersectBlock(World *world, Ray ray, Vec3_I16 *result, Vec3 *intersect, Vec3 *normal)
{
    int planes_hit;
    if(!IsInWorldBounds(world, ray.pos))
    {
        int axis;
        for(planes_hit = 0; planes_hit < 3; planes_hit++)
        {
            if((axis=MoveToNextBoundingPlane(world, &ray))>=0)
            {
                Assert(axis >= 0 && axis < 3);
                if(IsInBoundsExceptForAxis(world, ray.pos, (Axis)axis))
                {
                    break;
                }
            }
        }
    }

    int x_dir = Sign(ray.dir.x);
    int y_dir = Sign(ray.dir.y);
    int z_dir = Sign(ray.dir.z);
    //DebugOut("%d %d %d", x_dir, y_dir, z_dir);

    result->x = (int)floor(ray.pos.x);
    result->y = (int)floor(ray.pos.y);
    result->z = (int)floor(ray.pos.z);

    for(;;)
    {
        if(IsBlockInBounds(world, result->x, result->y, result->z))
        {
            U32 block = GetBlockAtNoCheck(world, result->x, result->y, result->z);
            if(block)
            {
                *intersect = ray.pos;
                return true;
            }
            else
            {
                // Again, find minimum intersection. Now only for 3 axes.
                int x = result->x + (x_dir==1 ? 1 : 0);
                int y = result->y + (y_dir==1 ? 1 : 0);
                int z = result->z + (z_dir==1 ? 1 : 0);

                int x_step = 1;
                int y_step = 0;
                int z_step = 0;
                R32 traversal = -(ray.pos.x-x)/ray.dir.x;

                R32 y_trav = -(ray.pos.y-y)/ray.dir.y;
                if(y_trav < traversal) { traversal = y_trav; x_step = 0; y_step = 1; z_step = 0; }

                R32 z_trav = -(ray.pos.z-z)/ray.dir.z;
                if(z_trav < traversal) { traversal = z_trav; x_step = 0; y_step = 0; z_step = 1; }

                Assert(traversal >= 0.0f);
                //DebugOut("traverse by %f", traversal);

                ray.pos += (traversal*ray.dir);
                result->x+=x_step*x_dir;
                result->y+=y_step*y_dir;
                result->z+=z_step*z_dir;
            }
        }
        else
        {
            break;
        }
    }

    return false;
}

void
InitWorld(Memory_Arena *arena, World *world, int x_chunks, int y_chunks)
{
    world->arena = arena;
    world->x_chunks = x_chunks;
    world->y_chunks = y_chunks;
    world->chunks = PushZeroArray(arena, Chunk*, world->x_chunks*world->y_chunks);
    for(int x = 0; x < world->x_chunks; x++)
    for(int y = 0; y < world->y_chunks; y++)
    {
        ChunkAt(world, x, y) = CreateChunk(world, x, y);
    }
    for(int x = 0; x < world->x_chunks; x++)
    for(int y = 0; y < world->y_chunks; y++)
    {
        Chunk *chunk = ChunkAt(world, x, y);
        GenerateChunkMesh(world, chunk);
    }
    world->x_size = x_chunks*CHUNKSIZE_X;
    world->y_size = y_chunks*CHUNKSIZE_Y;
    world->z_size = CHUNKSIZE_Z;

    world->entity_pool = PushMemoryPool(arena, 2048, sizeof(Entity));
    world->brain_pool = PushMemoryPool(arena, 2048, sizeof(Brain));
    world->agents = CreateDArray<Entity*>(arena, 2048);

    world->behavior_table = CreateDArray<Behavior>(arena, 128);
    world->behavior_nodes = CreateDArray<Behavior_Node>(arena, 256);
    world->behavior_trees = CreateDArray<Behavior_Tree>(arena, 256);
}

// Imgui debug rendering

bool
ImGuiTreeNodeColored(char *id, U32 color, ImGuiTreeNodeFlags flags, char *format, ...)
{
    ImGui::PushStyleColor(ImGuiCol_Text, color);

    va_list args;
    va_start(args, format);
    bool result = ImGui::TreeNodeExV(id, flags, format, args);
    va_end(args);

    ImGui::PopStyleColor();
    return result;
}

void
ImGuiBehaviorNode(World *world, Entity *agent, Behavior_Node *node)
{
    Behavior_Node_Instance *instance = &agent->behavior->nodes[node->idx];
    Behavior_State state = instance->state;
    U32 color = state==BehaviorState_Updating ? IM_COL32(100, 100, 255, 255) : 
        state==BehaviorState_Failed ? IM_COL32(255, 50, 50, 255) : IM_COL32(100, 255, 100, 255);
    char id[32];
    sprintf(id, "bn%p", node);
    if(node->type==BehaviorNodeType_Leaf)
    {
        if(ImGuiTreeNodeColored(id, 
                    color, 
                    ImGuiTreeNodeFlags_Leaf, 
                    "%s: %s", 
                    BehaviorTypeToString(node->behavior->type),
                    BehaviorStateToString(instance->state)))
        {
            ImGui::TreePop();
        }
    }
    else
    {
        if(ImGuiTreeNodeColored(id, 
                    color, 
                    0, 
                    "%s: %s",
                    node->type==BehaviorNodeType_Selector ? "selector" : "sequence",
                    BehaviorStateToString(instance->state)))
        {
            for(int child_idx = 0;
                    child_idx < node->children.size;
                    child_idx++)
            {
                Behavior_Node *child = node->children[child_idx];
                ImGuiBehaviorNode(world, agent, child);
            }
            ImGui::TreePop();
        }
    }
}

