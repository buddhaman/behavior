
enum Entity_Type
{
    Entity_Agent,
};

enum Entity_Flag
{
    EntityFlag_IsClaimed = 1 << 0,
};

struct Brain
{
    Entity *selected[1];
};

struct Entity
{
    Entity_Type type;
    U32 flags;
    bool is_removed;

    Brain *brain;
    Behavior_Tree_Instance *behavior;
};

#define ChunkAt(world, x, y) (world)->chunks[(x)+(y)*(world)->x_chunks]

struct World
{
    Memory_Arena *arena;

    DArray<Entity *> agents;
    Memory_Pool *entity_pool;
    Memory_Pool *brain_pool;
    Memory_Pool *behavior_tree_instance_pool;

    Behavior_Tree *agent_tree;

    DArray<Behavior> behavior_table;
    DArray<Behavior_Tree> behavior_trees;
    DArray<Behavior_Node> behavior_nodes;

    int x_chunks;
    int y_chunks;
    Chunk **chunks;

    int x_size;
    int y_size;
    int z_size;
};

