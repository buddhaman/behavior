
#include <thread>

#include <SDL2/SDL.h>
#include "external_libs.h"

// TODO: Move to external? 
#define FNL_IMPL
#include "perlin.c"

#include "util.h"

#include "memory.h"
#include "memory_pool.h"
#include "geom.h"
#include "darray.h"
#include "mesh.h"
#include "shader.h"
#include "input_handler.h"
#include "chunk.h"
#include "camera.h"
#include "behavior.h"
#include "world.h"

#include "mesh.cpp"
#include "shader.cpp"
#include "input_handler.cpp"
#include "chunk.cpp"
#include "camera.cpp"
#include "behavior.cpp"
#include "world.cpp"

void
ChunkMeshGeneratorLoop(World *world, int y_min, int y_max)
{
    for(;;)
    {
        int dirty_count = 0;
        for(int y_chunk = y_min; y_chunk < y_max; y_chunk++)
        for(int x_chunk = 0; x_chunk < world->x_chunks; x_chunk++)
        {
            Chunk *chunk = ChunkAt(world, x_chunk, y_chunk);
            if(chunk->is_dirty)
            {
                dirty_count++;
                GenerateChunkMesh(world, chunk);
                chunk->is_dirty = false;
                chunk->is_mesh_dirty = true;
                DebugOut("just generated chunk mesh");
            }
        }
        std::this_thread::yield();
    }
}

int
main(int argc, char **argv)
{
    const char* glsl_version = "#version 150";
    int screen_width = 1280;
    int screen_height = 720;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "%s", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Behavior", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if(err)
    {
        fprintf(stderr, "Error loading opengl");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings for opengl.
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontFromFileTTF("assets/DejaVuSansMono.ttf", 16.0f);
    
    //Vec4 clear_color = HexToVec4(0xcae9f6ff);
    Vec4 clear_color = V4(53.0f/100.0f, 80.0f/100.0f, 99.0f/100.0f, 1.0f);

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // SDL timing
    U32 last_time = SDL_GetTicks(); 
    (void)last_time;
    U32 currentTime;

    // Custom rendering init

    Memory_Arena *arena = CreateMemoryArena(1024L*1024L*512L);

    Shader *chunk_shader = CreateShaderProgram(arena, 
            "assets/chunk_shader.vert", "assets/solid.frag");

    Shader *model_shader = CreateShaderProgram(arena,
            "assets/solid.vert", "assets/solid.frag");

    Shader *flat_shader = CreateShaderProgram(arena,
            "assets/solid.vert", "assets/flat.frag");

    Input *input = PushZeroStruct(arena, Input);

    World *world = PushZeroStruct(arena, World);
    InitWorld(arena, world, 8, 8);

    Camera3 *cam = PushZeroStruct(arena, Camera3);
    cam->phi = -M_PI/4.0f;
    cam->theta = M_PI/2.0f;
    cam->radius = 10.0f;

    cam->look_at = V3(world->x_size/2, world->y_size/2, world->mid_level+10.0f);

    Mesh *mesh = CreateMesh3D(arena, 10000, 10000);
    Mesh *line_mesh = CreateMesh3D(arena, 10000, 10000);

    AddAgent(world, V3(40, 40, 40));

    int first_y_half = world->y_chunks/2;
    std::thread render_thread0(ChunkMeshGeneratorLoop, world, 0, first_y_half);
    std::thread render_thread1(ChunkMeshGeneratorLoop, world, first_y_half, world->y_chunks);

    bool done = false;
    while (!done)
    {
        memcpy(input->was_down, input->is_down, ArraySize(input->was_down));
        input->scroll = 0;
        input->is_mouse_disabled = io.WantCaptureMouse;
        input->mouse_diff = V2(0,0);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch(event.type) {
                
            case SDL_MOUSEWHEEL:
            {
                input->scroll = event.wheel.y;
            } break;

            case SDL_MOUSEMOTION:
            {
                input->mouse_pos = V2(event.motion.x, event.motion.y);
                input->mouse_diff = V2(event.motion.xrel, event.motion.yrel);
            } break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {   
                bool is_down = event.button.state==SDL_PRESSED;
                if(event.button.button==SDL_BUTTON_LEFT)
                {
                    if(is_down)
                    {
                        // just clicked
                        input->dragged_from = input->mouse_pos;
                    }
                    input->is_down[Action_LeftMouse] = is_down;
                }
            } break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                HandleKeyInput(input, event.key.keysym.sym, event.type==SDL_KEYDOWN);
            } break;

            case SDL_QUIT:
            {
                done = true;
            } break;

            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE 
                        && event.window.windowID == SDL_GetWindowID(window)) 
                {
                    done = true;
                }
            } break;
            
            default:
            {
            } break;

            }
        }

        if(input->is_down[Action_LeftMouse])
        {
            input->drag_diff = input->mouse_pos - input->dragged_from;
            R32 len = V2Len(input->drag_diff);
            if(len > 4.0)
            {
                input->is_dragging = true;
            }
        }
        else
        {
            input->is_dragging = false;
        }

        // Update real display size and mouse button state
        SDL_GL_GetDrawableSize(window, &screen_width, &screen_height);
        input->mouse_pos_normalized = (input->mouse_pos / V2(screen_width, screen_height))*2.0f-1.0f;

        // SDL timing
        currentTime = SDL_GetTicks();
        last_time = currentTime;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Create central dockspace for imgui.
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dockspace Main_", NULL, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("DockSpace Main Window_");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        ImGui::End();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);

        UpdateCamera3(cam, screen_width, screen_height, input);

        for(int chunk_x = 0; chunk_x < world->x_chunks; chunk_x++)
        for(int chunk_y = 0; chunk_y < world->x_chunks; chunk_y++)
        {
            Chunk *chunk = ChunkAt(world, chunk_x, chunk_y);

            if(chunk->is_mesh_dirty)
            {
                chunk->is_mesh_dirty = false;
                BufferMesh(chunk->mesh, GL_STATIC_DRAW);
            }
        }

        // Render chunks
        local_persist R32 time = 0.0f;
        time+=1.0f/60.0f;
        glUseProgram(chunk_shader->program);

        glUniformMatrix4fv(chunk_shader->transform_loc, 1, GL_FALSE, (const R32 *)cam->transform.m);
        
        for(int chunk_x = 0; chunk_x < world->x_chunks; chunk_x++)
        for(int chunk_y = 0; chunk_y < world->x_chunks; chunk_y++)
        {
            Chunk *chunk = ChunkAt(world, chunk_x, chunk_y);

            Mat4 model_matrix = M4Translation(V3(chunk->x_offset, chunk->y_offset, 0));
            glUniformMatrix4fv(chunk_shader->model_loc, 1, GL_FALSE, (const R32*)model_matrix.m);

            Render(chunk->mesh, GL_TRIANGLES);
        }

        // Render normal models
        
        ClearMesh(mesh);
        ClearMesh(line_mesh);

        Ray pick_ray = GetPickRay(cam, input->mouse_pos_normalized.x, -input->mouse_pos_normalized.y);

        Vec3_I16 hit;
        Vec3 intersect;
        Vec3 normal;
        if(IntersectBlock(world, pick_ray, &hit, &intersect, &normal))
        {
            R32 outline = 0.01f;
            PushCube(mesh, 
                    V3(hit.x-outline, hit.y-outline, hit.z-outline), 
                    V3(1+outline*2, 1+outline*2, 1+outline*2), 
                    0xff22ffff);
            if(IsJustReleased(input, Action_LeftMouse))
            {
                //SetBlockSphereAt(world, hit.x, hit.y, hit.z, 2, 0xff0000ff);
                int width = 12;
                int height = 12;
                int depth = 60;
                U32 color = 0xaaaaaaff;
                GenerateXWall(world, hit.x, hit.y, hit.y+height, hit.z, hit.z+depth, color);
                GenerateYWall(world, hit.y, hit.x+1, hit.x+width, hit.z, hit.z+depth, color);
                GenerateXWall(world, hit.x+width-1, hit.y, hit.y+height, hit.z, hit.z+depth, color);
                GenerateYWall(world, hit.y+height-1, hit.x, hit.x+width, hit.z, hit.z+depth, color);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::SetTooltip("(%d %d %d)", hit.x, hit.y, hit.z);
            ImGui::PopStyleVar();
        }

        for(int agent_idx = 0;
                agent_idx < world->agents.size;
                agent_idx++)
        {
            Entity *agent = world->agents[agent_idx];
            PushLineSphere(line_mesh, agent->pos, agent->radius, 0xff22ffff);
        }

        BufferMesh(mesh, GL_DYNAMIC_DRAW);
        BufferMesh(line_mesh, GL_DYNAMIC_DRAW);

        glUseProgram(model_shader->program);

        glUniformMatrix4fv(model_shader->transform_loc, 1, GL_FALSE, (const R32 *)cam->transform.m);
        Mat4 model_matrix = M4Identity();
        glUniformMatrix4fv(model_shader->model_loc, 1, GL_FALSE, (const R32*)model_matrix.m);

        Render(mesh, GL_TRIANGLES);

        glUseProgram(flat_shader->program);

        glUniformMatrix4fv(flat_shader->transform_loc, 1, GL_FALSE, (const R32 *)cam->transform.m);
        model_matrix = M4Identity();
        glUniformMatrix4fv(flat_shader->model_loc, 1, GL_FALSE, (const R32*)model_matrix.m);

        Render(line_mesh, GL_LINES);

        ImGui::Begin("Info");
        ImGui::Text("Game arena used: %zu / %zu kilobytes.", arena->used/1024, arena->size/1024);
#if 0
        if(ImGui::Button("1x")) {sim_speed = 1;} ImGui::SameLine();
        if(ImGui::Button("10x")) {sim_speed = 10;} ImGui::SameLine();
        if(ImGui::Button("50x")) {sim_speed = 50;} ImGui::SameLine();
        ImGui::Text("%dx", sim_speed);
#endif
        ImGui::DragFloat("Scale", &world->scale, 0.1f, 0.0f, 10.0f, "%.2f", 1.0f);
        ImGui::DragFloat("Magnitude", &world->magnitude, 1.0f, 0.0f, 100.0, "%.2f", 1.0f);
        ImGui::DragFloat("Water level", &world->water_level, 1.0f, 0.0f, 100.0, "%.2f", 1.0f);
        ImGui::DragFloat("Mid level", &world->mid_level, 1.0f, 0.0f, 100.0, "%.2f", 1.0f);
        const char * const noise_type_names[4]{
            "Open simplex 2", 
            "Open simplex 2s", 
            "cellular", 
            "perlin", 
        };
        const fnl_noise_type noise_types[]{
            FNL_NOISE_OPENSIMPLEX2,
            FNL_NOISE_OPENSIMPLEX2S,
            FNL_NOISE_CELLULAR,
            FNL_NOISE_PERLIN,
        };
        static int current_noise = world->noise_state.noise_type;

        ImGui::Text("Advanced noise options");
        ImGui::Separator();
        ImGui::Combo("Noise type", 
                &current_noise, 
                noise_type_names,
                4);
        ImGui::InputInt("Seed", &world->noise_state.seed);
        ImGui::DragFloat("Frequency", &world->noise_state.frequency, 0.001f, 0.0f, 1.0f);
        ImGui::InputInt("Octaves", &world->noise_state.octaves);
        ImGui::DragFloat("Lacunarity", &world->noise_state.lacunarity, 0.02f, 0.0f, 10.0f);

        world->noise_state.noise_type = noise_types[current_noise];
        if(ImGui::Button("Generate new world"))
        {
            GenerateChunks(world);
        }
        ImGui::End();

        // Render Dear ImGui frame
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

