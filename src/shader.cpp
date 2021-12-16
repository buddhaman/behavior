
U32
CreateShader(const char *path_to_src, GLenum type)
{
    const char *shader_src = ReadEntireFile(path_to_src);
    U32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_src, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation failed (%s): \n%s\n", path_to_src, infoLog);
        return 0;
    }
    else
    {
        return shader;
    }
}

void
FindDefaultShaderLocations(Shader *shader)
{
    shader->model_loc = glGetUniformLocation(shader->program, "model");
    shader->transform_loc = glGetUniformLocation(shader->program, "transform");
}

Shader *
CreateShaderProgram(Memory_Arena *arena, const char *source_v, const char *source_f)
{
    Shader *shader = PushStruct(arena, Shader);
    U32 vertex_shader = CreateShader(source_v, GL_VERTEX_SHADER);
    U32 fragment_shader = CreateShader(source_f, GL_FRAGMENT_SHADER);
    shader->program = glCreateProgram();
    glAttachShader(shader->program, vertex_shader);
    glAttachShader(shader->program, fragment_shader);
    glLinkProgram(shader->program);

    int success;
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if(!success)
    {
        char info[512];
        glGetProgramInfoLog(shader->program, 512, NULL, info);
        fprintf(stderr, "Can't link shader: \n%s\n", info);
        return NULL;
    }
    else
    {
        FindDefaultShaderLocations(shader);
        return shader;
    }
}
