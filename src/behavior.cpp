
internal inline Behavior_Node_Instance *
GetBehaviorNodeInstance(Entity *agent, Behavior_Node *node)
{
    return &agent->behavior->nodes[node->idx];
}

internal inline Behavior *
PushBehavior(World *world, 
        Behavior_Type type, 
        Update_Behavior_Function *UpdateFunction)
{
    Behavior *behavior = world->behavior_table.PushBack();
    behavior->type = type;
    behavior->Update = UpdateFunction;
    return behavior;
}

internal inline Behavior_Node *
PushNode(World *world, Behavior_Tree *tree, Behavior_Node *parent)
{
    Behavior_Node *node = world->behavior_nodes.PushBack();
    node->children = CreateDArray<Behavior_Node*>(world->arena, 32);
    tree->nodes.PushBack(node);
    node->idx = tree->nodes.size-1;
    if(parent)
    {
        parent->children.PushBack(node);
    }
    else
    {
        Assert(tree->root==NULL);
        tree->root = node;
    }
    return node;
}

internal inline Behavior_Node *
PushControlFlowNode(World *world, Behavior_Tree *tree, Behavior_Node *parent, Behavior_Node_Type type)
{
    Assert(type==BehaviorNodeType_Selector || type==BehaviorNodeType_Sequence);
    Behavior_Node *node = PushNode(world, tree, parent);
    node->type = type;
    return node;
}

internal inline Behavior_Node *
PushLeafNode(World *world, Behavior_Tree *tree, Behavior_Node *parent, Behavior_Type type)
{
    Behavior_Node *node = PushNode(world, tree, parent);
    node->type = BehaviorNodeType_Leaf;
    Behavior *behavior = &world->behavior_table[type];
    Assert(behavior->type==type);
    node->behavior = behavior;
    return node;
}

Behavior_Tree *
PushBehaviorTree(World *world)
{
    Behavior_Tree *tree = PushZeroStruct(world->arena, Behavior_Tree);
    tree->nodes = CreateDArray<Behavior_Node*>(world->arena, 32);
    return tree;
}

internal inline void
ClearBehaviorState(Entity *agent, Behavior_Node *node)
{
    Behavior_Node_Instance *instance = GetBehaviorNodeInstance(agent, node);
    instance->state = (Behavior_State)0;
    for(int child_idx = 0;
            child_idx < node->children.size;
            child_idx++)
    {
        Behavior_Node *child = node->children[child_idx];
        ClearBehaviorState(agent, child);
    }
}

internal inline Behavior_State
UpdateBehaviorNodeInstance(World *world, Entity *agent, Behavior_Node *node)
{
    Behavior_Node_Instance *instance = &agent->behavior->nodes[node->idx];
    Behavior_State result = BehaviorState_Failed;
    switch(node->type)
    {

    case BehaviorNodeType_Selector:
    {
        result = BehaviorState_Updating;
        for(int child_idx = 0;
                child_idx < node->children.size;
                child_idx++)
        {
            Behavior_Node *child = node->children[child_idx];
            Behavior_State state = UpdateBehaviorNodeInstance(world, agent, child);
            if(state!=BehaviorState_Failed)
            {
                result = state;
            }
        }
    } break;

    case BehaviorNodeType_Sequence:
    {
        result = BehaviorState_Updating;
        bool is_completed = true;
        for(int child_idx = 0;
                child_idx < node->children.size;
                child_idx++)
        {
            Behavior_Node *child = node->children[child_idx];
            Behavior_Node_Instance *child_instance = &agent->behavior->nodes[child->idx];
            if(child_instance->state!=BehaviorState_Succeeded)
            {
                Behavior_State child_state = UpdateBehaviorNodeInstance(world, agent, child);
                if(child_state==BehaviorState_Failed)
                {
                    result = BehaviorState_Failed;
                }
                is_completed = false;
                break;
            }
        }
        if(is_completed)
        {
            result = BehaviorState_Succeeded;
            if(node->repeat)
            {
                ClearBehaviorState(agent, node);
            }
        }
    } break;

    case BehaviorNodeType_Leaf:
    {
        result = node->behavior->Update(world, agent, node->behavior);
    } break;

    default:
    {
        InvalidCodePath;
    } break;

    }
    instance->state = result;
    return result;
}

internal inline void
UpdateBehaviorTree(World *world, Entity *agent)
{
    Behavior_Node *node = agent->behavior->tree->root;
    UpdateBehaviorNodeInstance(world, agent, node);
}

Behavior_Tree_Instance *
AddBehaviorTreeInstance(Behavior_Tree *tree)
{
    Memory_Arena arena = tree->instance_pool->AllocMemoryBlock();
    Behavior_Tree_Instance *instance = PushStruct(&arena, Behavior_Tree_Instance);
    instance->tree = tree;
    instance->nodes = CreateDArray<Behavior_Node_Instance>(&arena, tree->nodes.size);
    instance->nodes.size = instance->nodes.capacity;
    return instance;
}

void
CreateBehaviorTreeInstanceMemoryPool(World *world, Behavior_Tree *tree)
{
    size_t instance_size = sizeof(Behavior_Tree_Instance)
            +sizeof(Behavior_Node_Instance)*tree->nodes.size;
    DebugOut("Creating pool of size %zu for tree", instance_size);
    tree->instance_pool = PushMemoryPool(world->arena, 2048, instance_size);
}


