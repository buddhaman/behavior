
bool
IsDown(Input *input, Input_Action action)
{
    return input->is_down[action];
}

bool
IsJustDown(Input *input, Input_Action action)
{
    bool is_disabled = (IsMouseType(action) && input->is_mouse_disabled);
    return !is_disabled && input->is_down[action] && !input->was_down[action];
}

bool
IsJustReleased(Input *input, Input_Action action)
{
    bool is_disabled = (IsMouseType(action) && input->is_mouse_disabled);
    return !is_disabled && !input->is_down[action] && input->was_down[action];
}

bool
IsClicked(Input *input, Input_Action mouse_action)
{
    R32 drag_diff = V2Len(input->drag_diff);
    return IsJustReleased(input, mouse_action) && (drag_diff < 10.0f);
}

void
HandleKeyInput(Input *input, SDL_Keycode sym, bool is_down)
{
    Input_Action action = Action_None; 
    switch(sym)
    {   

    case SDLK_w:
    {
        action = Action_W;
    } break;

    case SDLK_s:
    {
        action = Action_S;
    } break;

    case SDLK_a:
    {
        action = Action_A;
    } break;

    case SDLK_d:
    {
        action = Action_D;
    } break;

    case SDLK_UP:
    {
        action = Action_Up;
    } break;

    case SDLK_DOWN:
    {
        action = Action_Down;
    } break;

    case SDLK_RIGHT:
    {
        action = Action_Right;
    } break;

    case SDLK_LEFT:
    {
        action = Action_Left;
    } break;

    case SDLK_z:
    {
        action = Action_Z;
    } break;

    case SDLK_x:
    {
        action = Action_X;
    } break;

    case SDLK_e:
    {
        action = Action_E;
    } break;

    case SDLK_q:
    {
        action = Action_Q;
    } break;
    }
    input->is_down[action] = is_down;
}

