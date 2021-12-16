
struct Rect
{
    union
    {
        struct
        {
            Vec2 pos;
            Vec2 dims;
        };
        struct
        {
            R32 x;
            R32 y;
            R32 width;
            R32 height;
        };
    };
};

internal inline bool 
PointRectIntersect(Vec2 p, Rect rect)
{
    if(p.x < rect.x || p.y < rect.y) { return false; }
    if(p.x > rect.x+rect.width || p.y > rect.y+rect.height) { return false; }
    return true;
}

internal inline bool
PointCircleIntersect(Vec2 p, Vec2 center, R32 radius)
{
    Vec2 diff = p-center;
    R32 len2 = V2Len2(diff);
    return (len2 <= radius*radius);
}

struct Ray
{
    Vec3 pos;
    Vec3 dir;
};

enum Axis
{
    X_Axis,
    Y_Axis,
    Z_Axis,
};



