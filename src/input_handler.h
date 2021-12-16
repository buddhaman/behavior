
enum Input_Action
{
    Action_None = 0,
    Action_Up = 1,
    Action_Down,
    Action_Left,
    Action_Right,
    Action_W,
    Action_A,
    Action_S,
    Action_D,
    Action_Z,
    Action_X,
    Action_E,
    Action_Q,
    Action_MouseBegin,
    Action_LeftMouse,
    Action_MouseEnd,
    Action_ActionsPlusOne,
};
bool IsMouseType(Input_Action action)
{
    return action > Action_MouseBegin && action < Action_MouseEnd;
}

struct Input
{
    bool is_mouse_disabled;
    Vec2 mouse_pos;
    Vec2 mouse_diff;
    Vec2 dragged_from;
    Vec2 drag_diff;
    Vec2 mouse_pos_normalized;

    int scroll;
    bool is_dragging;
    bool is_down[Action_ActionsPlusOne];
    bool was_down[Action_ActionsPlusOne];
};


