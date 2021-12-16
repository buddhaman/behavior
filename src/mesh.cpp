
void
PushVertex(Mesh *mesh, Vec2 pos, U32 color)
{
    Assert(mesh->n_vertices < mesh->max_vertices);
    size_t index = mesh->n_vertices*mesh->stride;
    mesh->vertex_buffer[index] = pos.x;
    mesh->vertex_buffer[index+1] = pos.y;
    mesh->vertex_buffer[index+2] = *((R32*)(&color));
    mesh->n_vertices++;
}

void
PushIndex(Mesh *mesh, U32 index)
{
    Assert(mesh->n_indices < mesh->max_indices);
    mesh->index_buffer[mesh->n_indices++] = index;
}

void
PushQuad(Mesh *mesh, 
        Vec2 p0,
        Vec2 p1,
        Vec2 p2,
        Vec2 p3,
        U32 color)
{
    U32 index = mesh->n_vertices;
    PushVertex(mesh, p0, color);
    PushVertex(mesh, p1, color);
    PushVertex(mesh, p2, color);
    PushVertex(mesh, p3, color);
    PushIndex(mesh, index);
    PushIndex(mesh, index+1);
    PushIndex(mesh, index+2);
    PushIndex(mesh, index+2);
    PushIndex(mesh, index+3);
    PushIndex(mesh, index);
}

void
PushRect(Mesh *mesh, Vec2 pos, Vec2 dims, U32 color)
{
    PushQuad(mesh, 
            pos, 
            V2(pos.x+dims.x,pos.y),
            V2(pos.x+dims.x, pos.y+dims.y),
            V2(pos.x, pos.y+dims.y), 
            color);
}

void
PushLineRect(Mesh *mesh, Vec2 pos, Vec2 dims, R32 thickness, U32 color)
{
    PushRect(mesh, pos, V2(dims.x, thickness), color);
    PushRect(mesh, V2(pos.x, pos.y+dims.y-thickness), V2(dims.x, thickness), color);
    PushRect(mesh, V2(pos.x+dims.x-thickness,pos.y+thickness), V2(thickness, dims.y-thickness*2), color);
    PushRect(mesh, V2(pos.x,pos.y+thickness), V2(thickness, dims.y-thickness*2), color);
}

void
PushLine(Mesh *mesh, Vec2 from, Vec2 to, R32 thickness, U32 color)
{
    Vec2 diff = V2Norm(V2Sub(to, from));
    Vec2 perp = V2MulS(V2(-diff.y, diff.x), thickness/2.0);
    PushQuad(mesh,
            V2Sub(from, perp),
            V2Add(from, perp),
            V2Add(to, perp),
            V2Sub(to, perp),
            color);
}

void
PushNGon(Mesh *mesh, Vec2 p, R32 r, int n, R32 d_angle, U32 color)
{
    U32 index = mesh->n_vertices;
    for(int atVertex = 0;
            atVertex < n;
            atVertex++)
    {
        R32 angle = M_PI*2.0f*atVertex/((R32)n)+d_angle;
        PushVertex(mesh, V2(p.x+r*cosf(angle), p.y+r*sinf(angle)), color);
    }
    for(int atVertex = 0;
            atVertex < n-2;
            atVertex++)
    {
        PushIndex(mesh, index);
        PushIndex(mesh, index+atVertex+1);
        PushIndex(mesh, index+atVertex+2);
    }
}

void
PushLineNGon(Mesh *mesh, Vec2 p, R32 r, int n, R32 d_angle, R32 thickness, U32 color)
{
    U32 index = mesh->n_vertices;
    for(int atVertex = 0;
            atVertex < n;
            atVertex++)
    {
        R32 angle = M_PI*2.0f*atVertex/((R32)n)+d_angle;
        R32 r_inner = r-thickness/2.0f;
        R32 r_outer = r+thickness/2.0f;
        PushVertex(mesh, V2(p.x+r_inner*cosf(angle), p.y+r_inner*sinf(angle)), color);
        PushVertex(mesh, V2(p.x+r_outer*cosf(angle), p.y+r_outer*sinf(angle)), color);
    }
    for(int atVertex = 0;
            atVertex < n-1;
            atVertex++)
    {
        PushIndex(mesh, index+atVertex*2);
        PushIndex(mesh, index+atVertex*2+1);
        PushIndex(mesh, index+atVertex*2+3);
        PushIndex(mesh, index+atVertex*2+3);
        PushIndex(mesh, index+atVertex*2+2);
        PushIndex(mesh, index+atVertex*2);
    }
    PushIndex(mesh, index+(n-1)*2);
    PushIndex(mesh, index+(n-1)*2+1);
    PushIndex(mesh, index+1);
    PushIndex(mesh, index+1);
    PushIndex(mesh, index);
    PushIndex(mesh, index+(n-1)*2);
}

void
ClearMesh(Mesh *mesh)
{
    mesh->n_vertices = 0;
    mesh->n_indices = 0;
}

void
BufferMesh(Mesh *mesh, GLenum mode)
{
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
            mesh->stride*mesh->n_vertices*sizeof(R32),
            mesh->vertex_buffer,
            mode);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            mesh->n_indices*sizeof(U32),
            mesh->index_buffer,
            mode);
}

void
Render(Mesh *mesh, GLenum mode)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
}

Mesh *
CreateMesh(Memory_Arena *arena, 
        int stride, 
        size_t max_vertices, 
        size_t max_indices)
{
    Mesh *mesh = PushStruct(arena, Mesh);
    mesh->vertex_buffer = PushArray(arena, R32, stride*max_vertices);
    mesh->index_buffer = PushArray(arena, U32, max_vertices);
    mesh->stride = stride;
    mesh->max_vertices = max_vertices;
    mesh->max_indices = max_indices;

    // Setup vao, vbo and ebo
    glGenVertexArrays(1, &mesh->vao);

    glBindVertexArray(mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glGenBuffers(1, &mesh->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    return mesh;
}

Mesh *
CreateMesh2D(Memory_Arena *arena, size_t max_vertices, size_t max_indices)
{
    Mesh *mesh = CreateMesh(arena, 3, max_vertices, max_indices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mesh->stride*sizeof(R32), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, mesh->stride*sizeof(R32), 
            (void *)(sizeof(R32)*2));

    return mesh;
}

// Chunk mesh stuff

struct Vec3_U16 
{
    U16 x;
    U16 y;
    U16 z;
};
internal inline Vec3_U16 V3U16(U16 x, U16 y, U16 z) { return {x, y, z}; }
internal inline Vec3_U16 V3Add(Vec3_U16 a, Vec3_U16 b) { return V3U16(a.x+b.x, a.y+b.y, a.z+b.z); }
internal inline Vec3_U16 operator+(Vec3_U16 left, Vec3_U16 right) { return V3Add(left, right); }

struct Vec3_I16 
{
    I16 x;
    I16 y;
    I16 z;
};
internal inline Vec3_I16 V3I16(I16 x, I16 y, I16 z) { return {x, y, z}; }

#define CastBits(v, toType) *((toType *)&v)
void
PushVertex(Mesh *mesh, Vec3_U16 v, Vec3_I16 normal, U32 color)
{
    Assert(mesh->n_vertices < mesh->max_vertices);
    U32 first = ((v.y << 16) | v.x);
    U32 second = (v.z) | (CastBits(normal.x, U16) << 16);
    U32 third = (CastBits(normal.z, U16) << 16) | CastBits(normal.y, U16);
    int idx = mesh->n_vertices*mesh->stride;
    mesh->vertex_buffer[idx] = CastBits(first, R32);
    mesh->vertex_buffer[idx+1] = CastBits(second, R32);
    mesh->vertex_buffer[idx+2] = CastBits(third, R32);
    mesh->vertex_buffer[idx+3] = CastBits(color, R32);
    mesh->n_vertices++;
}

void
PushQuad(Mesh *mesh, 
        Vec3_U16 p0,
        Vec3_U16 p1,
        Vec3_U16 p2,
        Vec3_U16 p3,
        Vec3_I16 normal, 
        U32 color)
{
    int idx = mesh->n_vertices;
    PushVertex(mesh, p0, normal, color);
    PushVertex(mesh, p1, normal, color);
    PushVertex(mesh, p2, normal, color);
    PushVertex(mesh, p3, normal, color);
    PushIndex(mesh, idx);
    PushIndex(mesh, idx+1);
    PushIndex(mesh, idx+2);
    PushIndex(mesh, idx+2);
    PushIndex(mesh, idx+3);
    PushIndex(mesh, idx);
}

void
PushCube(Mesh *mesh,
        Vec3_U16 pos,
        Vec3_U16 dims,
        U32 color)
{
    Vec3_U16 p000 = pos;
    Vec3_U16 p001 = pos+V3U16(dims.x, 0,      0);
    Vec3_U16 p010 = pos+V3U16(0,      dims.y, 0);
    Vec3_U16 p011 = pos+V3U16(dims.x, dims.y, 0);
    Vec3_U16 p100 = pos+V3U16(0,      0,      dims.z);
    Vec3_U16 p101 = pos+V3U16(dims.x, 0,      dims.z);
    Vec3_U16 p110 = pos+V3U16(0,      dims.y, dims.z);
    Vec3_U16 p111 = pos+V3U16(dims.x, dims.y, dims.z);

    // Bottom, Top 
    PushQuad(mesh, p010, p011, p001, p000, V3I16(0,0, I16MAX), color);
    PushQuad(mesh, p100, p101, p111, p110, V3I16(0,0,-I16MAX), color);

    // Left Right
    PushQuad(mesh, p000, p100, p110, p010, V3I16(-I16MAX, 0, 0), color);
    PushQuad(mesh, p011, p111, p101, p001, V3I16(I16MAX, 0, 0), color);

    // Up Down
    PushQuad(mesh, p000, p001, p101, p100, V3I16(0, -I16MAX, 0), color);
    PushQuad(mesh, p110, p111, p011, p010, V3I16(0, I16MAX, 0), color);
}

Mesh *
CreateChunkMesh(Memory_Arena *arena, size_t max_vertices, size_t max_indices)
{
    // Create voxel color world.
    // x, y, z. position (6 bytes)
    // normal (xyz) 6 bytes
    // color (4 bytes)
    // 16 bytes thats it ? 
    Mesh *mesh = CreateMesh(arena, 4, max_vertices, max_indices);

    size_t total_stride = mesh->stride*sizeof(R32);
    size_t offset = 0;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_UNSIGNED_SHORT, GL_TRUE, total_stride, 
            (void *)(offset));
    offset+=(3*sizeof(U16));  

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_SHORT, GL_TRUE, total_stride, 
            (void *)(offset));
    offset+=(3*sizeof(I16));  

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, total_stride, 
            (void *)(offset));
    offset+=sizeof(U32);
    Assert(offset==total_stride);

    return mesh;
}

// Normal 3d mesh

void
PushVertex(Mesh *mesh, Vec3 pos, Vec3 normal, U32 color)
{
    Assert(mesh->n_vertices < mesh->max_vertices);
    size_t index = mesh->n_vertices*mesh->stride;
    mesh->vertex_buffer[index] = pos.x;
    mesh->vertex_buffer[index+1] = pos.y;
    mesh->vertex_buffer[index+2] = pos.z;
    mesh->vertex_buffer[index+3] = normal.x;
    mesh->vertex_buffer[index+4] = normal.y;
    mesh->vertex_buffer[index+5] = normal.z;
    mesh->vertex_buffer[index+6] = CastBits(color, R32);
    mesh->n_vertices++;
}

void
PushQuad(Mesh *mesh, 
        Vec3 p0,
        Vec3 p1,
        Vec3 p2,
        Vec3 p3,
        Vec3 normal, 
        U32 color)
{
    U32 index = mesh->n_vertices;
    PushVertex(mesh, p0, normal, color);
    PushVertex(mesh, p1, normal, color);
    PushVertex(mesh, p2, normal, color);
    PushVertex(mesh, p3, normal, color);
    PushIndex(mesh, index);
    PushIndex(mesh, index+1);
    PushIndex(mesh, index+2);
    PushIndex(mesh, index+2);
    PushIndex(mesh, index+3);
    PushIndex(mesh, index);
}

void
PushCube(Mesh *mesh,
        Vec3 pos,
        Vec3 dims,
        U32 color)
{
    Vec3 p000 = pos;
    Vec3 p001 = pos+V3(dims.x, 0,      0);
    Vec3 p010 = pos+V3(0,      dims.y, 0);
    Vec3 p011 = pos+V3(dims.x, dims.y, 0);
    Vec3 p100 = pos+V3(0,      0,      dims.z);
    Vec3 p101 = pos+V3(dims.x, 0,      dims.z);
    Vec3 p110 = pos+V3(0,      dims.y, dims.z);
    Vec3 p111 = pos+V3(dims.x, dims.y, dims.z);

    // Bottom, Top 
    PushQuad(mesh, p010, p011, p001, p000, V3(0,0, 1), color);
    PushQuad(mesh, p100, p101, p111, p110, V3(0,0,-1), color);

    // Left Right
    PushQuad(mesh, p000, p100, p110, p010, V3(-1, 0, 0), color);
    PushQuad(mesh, p011, p111, p101, p001, V3(1, 0, 0), color);

    // Up Down
    PushQuad(mesh, p000, p001, p101, p100, V3(0, -1, 0), color);
    PushQuad(mesh, p110, p111, p011, p010, V3(0, 1, 0), color);
}

// Make sure up is normalized
void
PushCubeLine(Mesh *mesh, 
        Vec3 from,
        Vec3 to, 
        Vec3 up,
        R32 size,
        U32 color)
{
    Vec3 dir = V3Norm(to-from);
    Vec3 right = V3Norm(V3Cross(up, dir));
    Vec3 up2 = V3Norm(V3Cross(dir, right));

    R32 s = 0.5f*size;
    Vec3 p000 = from + s*( -right + -up2);
    Vec3 p001 = from + s*(  right + -up2);
    Vec3 p010 = from + s*(  -right +  up2);
    Vec3 p011 = from + s*(  right +  up2);
    Vec3 p100 = to + s*( -right + -up2);
    Vec3 p101 = to + s*(  right + -up2);
    Vec3 p110 = to + s*(  -right +  up2);
    Vec3 p111 = to + s*(  right +  up2);

    // Bottom, Top 
    PushQuad(mesh, p010, p011, p001, p000, -dir, color);
    PushQuad(mesh, p100, p101, p111, p110, dir, color);

    // Left Right
    PushQuad(mesh, p000, p100, p110, p010, -right, color);
    PushQuad(mesh, p011, p111, p101, p001, right, color);

    // Up Down
    PushQuad(mesh, p000, p001, p101, p100, -up2, color);
    PushQuad(mesh, p110, p111, p011, p010, up2, color);
}

Mesh *
CreateMesh3D(Memory_Arena *arena, size_t max_vertices, size_t max_indices)
{
    // pos, normal, color (4 bytes) 
    Mesh *mesh = CreateMesh(arena, 7, max_vertices, max_indices);

    size_t total_stride = sizeof(R32)*mesh->stride;
    size_t offset = 0;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, total_stride, 
            (void *)(offset));
    offset+=(3*sizeof(R32));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, total_stride, 
            (void *)(offset));
    offset+=(3*sizeof(R32));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, total_stride, 
            (void *)(offset));
    offset+=(sizeof(R32));

    Assert(offset==total_stride);

    return mesh;
}

