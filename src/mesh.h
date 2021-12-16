
typedef struct
{
    int stride;
    U32 vao;
    U32 vbo;
    U32 ebo;

    int max_vertices;
    int n_vertices;
    R32 *vertex_buffer;

    int max_indices;
    int n_indices;
    U32 *index_buffer;
} Mesh;

