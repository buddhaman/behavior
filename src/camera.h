
struct Camera2
{
    Vec2 pos;
    Vec2 dims;
    R32 scale;
    int screen_width;
    int screen_height;
    
    bool is_dragging;
    Vec2 drag_from;
    Vec2 mouse_pos;
};

struct Camera3
{
    Vec3 look_at;
    Vec3 dir;
    Vec3 pos;
    R32 theta;
    R32 phi;
    R32 radius;
    Mat4 transform;
};

