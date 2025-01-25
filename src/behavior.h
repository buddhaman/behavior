
typedef struct World World;
typedef struct Entity Entity;

typedef struct Behavior_Node Behavior_Node;
typedef struct Behavior_Tree Behavior_Tree;
typedef struct Behavior Behavior;

enum Behavior_Node_Type
{
    BehaviorNodeType_Selector,
    BehaviorNodeType_Sequence,
    BehaviorNodeType_Leaf,
};

enum Behavior_Type
{
    BehaviorType_WalkTo = 0,
};

const char *
BehaviorTypeToString(Behavior_Type type)
{
    switch(type)
    {
    case BehaviorType_WalkTo: return "walk to";
    }
    return "unknown";
}

enum Behavior_State
{
    BehaviorState_Updating,
    BehaviorState_Failed,
    BehaviorState_Succeeded,
};

const char *
BehaviorStateToString(Behavior_State state)
{
    switch(state)
    {
    case BehaviorState_Updating: return "BehaviorState_Updating";
    case BehaviorState_Failed: return "BehaviorState_Failed";
    case BehaviorState_Succeeded: return "BehaviorState_Succeeded";
    };
    return "unknown";
}

#define UpdateBehaviorFunction(name) Behavior_State name(World *world,\
        Entity *agent,\
        Behavior *behavior)
typedef UpdateBehaviorFunction(Update_Behavior_Function);

struct Behavior
{
    Behavior_Type type;
    Update_Behavior_Function *Update;
};

struct Behavior_Node
{
    int idx;        // Idx in tree;
    Behavior_Node_Type type;
    Behavior *behavior;
    DArray<Behavior_Node *> children;
    bool repeat;
};

struct Behavior_Tree
{
    DArray<Behavior_Node*> nodes;
    Behavior_Node *root;
    Memory_Pool *instance_pool;
};

struct Behavior_Node_Instance
{
    Behavior_State state;
};

struct Behavior_Tree_Instance
{
    Behavior_Tree *tree;
    DArray<Behavior_Node_Instance> nodes;
};



