
Vec2
ScreenToWorld(Camera2 *camera, Vec2 screen_pos)
{
    Vec2 norm = (screen_pos/V2(camera->screen_width, camera->screen_height))-0.5f;
    norm.y = -norm.y;
    Vec2 world_pos = camera->pos+norm*camera->dims;
    return world_pos;
}

void
UpdateCamera2(Camera2 *camera, int screen_width, int screen_height, Input *input)
{
    camera->screen_width = screen_width;
    camera->screen_height = screen_height;
    camera->dims = V2(screen_width/camera->scale, screen_height/camera->scale);
    if(input->is_dragging)
    {
        if(camera->is_dragging)
        {
            Vec2 diff = V2(-input->drag_diff.x/camera->scale, input->drag_diff.y/camera->scale);
            camera->pos = camera->drag_from+diff;
        }
        else
        {
            camera->is_dragging = true;
            camera->drag_from = camera->pos;
        }
    }
    else
    {
        camera->is_dragging = false;
    }
    if(input->scroll)
    {
        camera->scale*=powf(1.1f, input->scroll);
    }
    camera->mouse_pos = ScreenToWorld(camera, input->mouse_pos);
}

Ray
GetPickRay(Camera3 *cam, R32 nx, R32 ny)
{
    Ray result;
    Mat4 inverse = M4Invert(cam->transform);
    Vec3 from = M4MulPos(inverse, V3(nx, ny, -1));
    Vec3 to = M4MulPos(inverse, V3(nx, ny, 1));
    result.pos = from;
    result.dir = V3Norm(to-from);
    return result;
}

void
UpdateCamera3(Camera3 *cam, int screen_width, int screen_height, Input *input)
{
    R32 rotation_speed = 0.03f;
    R32 zoom_factor = 0.98f;
    R32 speed = 0.03f*cam->radius;

    R32 ct = cosf(cam->theta);
    R32 st = sinf(cam->theta);
    R32 cp = cosf(cam->phi);
    R32 sp = sinf(cam->phi);

    Vec3 forward = V3(ct, st, 0);
    Vec3 right = V3(st, -ct, 0);

    if(IsDown(input, Action_Up))
    {
        cam->phi-=rotation_speed;
    }
    if(IsDown(input, Action_Down))
    {
        cam->phi+=rotation_speed;
    }
    if(IsDown(input, Action_Right))
    {
        cam->theta+=rotation_speed;
    }
    if(IsDown(input, Action_Left))
    {
        cam->theta-=rotation_speed;
    }
    if(IsDown(input, Action_W))
    {
        cam->look_at+=(speed*forward);
    }
    if(IsDown(input, Action_S))
    {
        cam->look_at-=(speed*forward);
    }
    if(IsDown(input, Action_D))
    {
        cam->look_at+=(speed*right);
    }
    if(IsDown(input, Action_A))
    {
        cam->look_at-=(speed*right);
    }
    if(IsDown(input, Action_E))
    {
        cam->look_at.z+=speed;
    }
    if(IsDown(input, Action_Q))
    {
        cam->look_at.z-=speed;
    }
    if(IsDown(input, Action_X))
    {
        cam->radius/=zoom_factor;
    }
    if(IsDown(input, Action_Z))
    {
        cam->radius*=zoom_factor;
    }
    if(IsDown(input, Action_X))
    {
        cam->radius/=zoom_factor;
    }

    R32 epsilon = 0.001;
    if(cam->phi > M_PI/2-epsilon)
    {
        cam->phi = M_PI/2-epsilon;
    }
    if(cam->phi < -M_PI/2+epsilon)
    {
        cam->phi = -M_PI/2+epsilon;
    }
    cam->dir = V3(ct * cp, st *cp, sp);
    cam->pos = cam->look_at-cam->radius*cam->dir;

    R32 aspect = ((R32)screen_width)/((R32)screen_height);
    
    cam->transform = M4Mul(
            M4Perspective(70.0f, aspect, 0.1f, 300.0f),
            M4LookAt(cam->pos, cam->pos+cam->dir, V3(0,0,1)));
}
