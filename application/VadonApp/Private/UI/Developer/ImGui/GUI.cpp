#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/UI/Developer/ImGui/GUI.hpp>

#include <VadonApp/Private/Core/Application.hpp>
#include <VadonApp/Private/UI/Developer/ImGui/GUIElements.hpp>
#include <VadonApp/Private/UI/Developer/ImGui/Fonts/fontawesome-webfont.inl>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/Keyboard.hpp>

#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <Vadon/Core/Task/TaskSystem.hpp>

// FIXME: could separate the renderer backend to make this file less cluttered?
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Utilities/Data/DataUtilities.hpp>
#include <Vadon/Utilities/Data/Visitor.hpp>
#include <Vadon/Utilities/Math/Matrix.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#ifndef IM_ASSERT
#define IM_ASSERT(Expr) VADON_ASSERT(Expr, "Dear ImGui error!")
#endif

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h> // NOTE: required to allow using std::string with input text

namespace VadonApp::Private::UI::Developer::ImGUI
{
    namespace
    {
        ImGuiMouseButton_ get_imgui_mouse_button(VadonApp::Platform::MouseButton mouse_button)
        {
            switch (mouse_button)
            {
            case VadonApp::Platform::MouseButton::LEFT:
                return ImGuiMouseButton_::ImGuiMouseButton_Left;
            case VadonApp::Platform::MouseButton::MIDDLE:
                return ImGuiMouseButton_::ImGuiMouseButton_Middle;
            case VadonApp::Platform::MouseButton::RIGHT:
                return ImGuiMouseButton_::ImGuiMouseButton_Right;
            }

            return ImGuiMouseButton_::ImGuiMouseButton_COUNT;
        }

        ImGuiKey get_imgui_key(VadonApp::Platform::KeyCode key_code)
        {
            switch (key_code)
            {
            case VadonApp::Platform::KeyCode::RETURN:
                return ImGuiKey_Enter;
            case VadonApp::Platform::KeyCode::ESCAPE:
                return ImGuiKey_Escape;
            case VadonApp::Platform::KeyCode::BACKSPACE:
                return ImGuiKey_Backspace;
            case VadonApp::Platform::KeyCode::SPACE:
                return ImGuiKey_Space;
            case VadonApp::Platform::KeyCode::KEY_0:
                return ImGuiKey_0;
            case VadonApp::Platform::KeyCode::KEY_1:
                return ImGuiKey_1;
            case VadonApp::Platform::KeyCode::KEY_2:
                return ImGuiKey_2;
            case VadonApp::Platform::KeyCode::KEY_3:
                return ImGuiKey_3;
            case VadonApp::Platform::KeyCode::KEY_4:
                return ImGuiKey_4;
            case VadonApp::Platform::KeyCode::KEY_5:
                return ImGuiKey_5;
            case VadonApp::Platform::KeyCode::KEY_6:
                return ImGuiKey_6;
            case VadonApp::Platform::KeyCode::KEY_7:
                return ImGuiKey_7;
            case VadonApp::Platform::KeyCode::KEY_8:
                return ImGuiKey_8;
            case VadonApp::Platform::KeyCode::KEY_9:
                return ImGuiKey_9;
            case VadonApp::Platform::KeyCode::KEY_a:
                return ImGuiKey_A;
            case VadonApp::Platform::KeyCode::KEY_b:
                return ImGuiKey_B;
            case VadonApp::Platform::KeyCode::KEY_c:
                return ImGuiKey_C;
            case VadonApp::Platform::KeyCode::KEY_d:
                return ImGuiKey_D;
            case VadonApp::Platform::KeyCode::KEY_e:
                return ImGuiKey_E;
            case VadonApp::Platform::KeyCode::KEY_f:
                return ImGuiKey_F;
            case VadonApp::Platform::KeyCode::KEY_g:
                return ImGuiKey_G;
            case VadonApp::Platform::KeyCode::KEY_h:
                return ImGuiKey_H;
            case VadonApp::Platform::KeyCode::KEY_i:
                return ImGuiKey_I;
            case VadonApp::Platform::KeyCode::KEY_j:
                return ImGuiKey_J;
            case VadonApp::Platform::KeyCode::KEY_k:
                return ImGuiKey_K;
            case VadonApp::Platform::KeyCode::KEY_l:
                return ImGuiKey_L;
            case VadonApp::Platform::KeyCode::KEY_m:
                return ImGuiKey_M;
            case VadonApp::Platform::KeyCode::KEY_n:
                return ImGuiKey_N;
            case VadonApp::Platform::KeyCode::KEY_o:
                return ImGuiKey_O;
            case VadonApp::Platform::KeyCode::KEY_p:
                return ImGuiKey_P;
            case VadonApp::Platform::KeyCode::KEY_q:
                return ImGuiKey_Q;
            case VadonApp::Platform::KeyCode::KEY_r:
                return ImGuiKey_R;
            case VadonApp::Platform::KeyCode::KEY_s:
                return ImGuiKey_S;
            case VadonApp::Platform::KeyCode::KEY_t:
                return ImGuiKey_T;
            case VadonApp::Platform::KeyCode::KEY_u:
                return ImGuiKey_U;
            case VadonApp::Platform::KeyCode::KEY_v:
                return ImGuiKey_V;
            case VadonApp::Platform::KeyCode::KEY_w:
                return ImGuiKey_W;
            case VadonApp::Platform::KeyCode::KEY_x:
                return ImGuiKey_X;
            case VadonApp::Platform::KeyCode::KEY_y:
                return ImGuiKey_Y;
            case VadonApp::Platform::KeyCode::KEY_z:
                return ImGuiKey_Z;
            case VadonApp::Platform::KeyCode::INSERT:
                return ImGuiKey_Insert;
            case VadonApp::Platform::KeyCode::HOME:
                return ImGuiKey_Home;
            case VadonApp::Platform::KeyCode::PAGE_UP:
                return ImGuiKey_PageUp;
            case VadonApp::Platform::KeyCode::DELETE_KEY:
                return ImGuiKey_Delete;
            case VadonApp::Platform::KeyCode::END:
                return ImGuiKey_End;
            case VadonApp::Platform::KeyCode::PAGE_DOWN:
                return ImGuiKey_PageDown;
            case VadonApp::Platform::KeyCode::RIGHT:
                return ImGuiKey_RightArrow;
            case VadonApp::Platform::KeyCode::LEFT:
                return ImGuiKey_LeftArrow;
            case VadonApp::Platform::KeyCode::DOWN:
                return ImGuiKey_DownArrow;
            case VadonApp::Platform::KeyCode::UP:
                return ImGuiKey_UpArrow;
            }
            return ImGuiKey_None;
        }

        VadonApp::Platform::Cursor get_platform_cursor(ImGuiMouseCursor_ imgui_cursor)
        {
            switch (imgui_cursor)
            {
            case ImGuiMouseCursor_Arrow:
                return VadonApp::Platform::Cursor::ARROW;
            case ImGuiMouseCursor_TextInput:
                return VadonApp::Platform::Cursor::TEXT_INPUT;
            case ImGuiMouseCursor_ResizeAll:
                return VadonApp::Platform::Cursor::RESIZE_ALL;
            case ImGuiMouseCursor_ResizeNS:
                return VadonApp::Platform::Cursor::RESIZE_NS;
            case ImGuiMouseCursor_ResizeEW:
                return VadonApp::Platform::Cursor::RESIZE_EW;
            case ImGuiMouseCursor_ResizeNESW:
                return VadonApp::Platform::Cursor::RESIZE_NESW;
            case ImGuiMouseCursor_ResizeNWSE:
                return VadonApp::Platform::Cursor::RESIZE_NWSE;
            case ImGuiMouseCursor_Hand:
                return VadonApp::Platform::Cursor::HAND;
            case ImGuiMouseCursor_NotAllowed:
                return VadonApp::Platform::Cursor::NOT_ALLOWED;
            }

            return VadonApp::Platform::Cursor::ARROW;
        }

        int32_t get_imgui_treenode_flags(GUISystem::TreeNodeFlags flags)
        {
            int32_t imgui_flags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_None;
            VADON_START_BITMASK_SWITCH(flags)
            {
            case GUISystem::TreeNodeFlags::SELECTED:
                imgui_flags |= ImGuiTreeNodeFlags_Selected;
                break;
            case GUISystem::TreeNodeFlags::FRAMED:
                imgui_flags |= ImGuiTreeNodeFlags_Framed;
                break;
            case GUISystem::TreeNodeFlags::DEFAULT_OPEN:
                imgui_flags |= ImGuiTreeNodeFlags_DefaultOpen;
                break;
            case GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK:
                imgui_flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
                break;
            case GUISystem::TreeNodeFlags::OPEN_ON_ARROW:
                imgui_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
                break;
            case GUISystem::TreeNodeFlags::LEAF:
                imgui_flags |= ImGuiTreeNodeFlags_Leaf;
                break;
            case GUISystem::TreeNodeFlags::BULLET:
                imgui_flags |= ImGuiTreeNodeFlags_Bullet;
                break;
            }

            return imgui_flags;
        }

        GUISystem* dev_gui_instance = nullptr;

        constexpr int32_t INIT_VERTEX_BUFFER_CAPACITY = 5000;
        constexpr int32_t INIT_INDEX_BUFFER_CAPACITY = 5000;

        constexpr const char* HLSL_VERTEX_SHADER_SOURCE =
            R"(cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

struct VS_INPUT
{
    float2 pos : POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
})";

        constexpr const char* HLSL_PIXEL_SHADER_SOURCE =
            R"(struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

sampler sampler0;
Texture2D texture0;
            
float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
})";

        struct VertexConstantBuffer
        {
            Vadon::Utilities::Matrix4 model_view_projection;
        };

        struct DrawCommand
        {
            Vadon::Render::DrawCommand draw_command;

            ImVec2 clip_min;
            ImVec2 clip_max;

            Vadon::Render::SRVHandle texture_srv_handle;
        };

        using DrawCommandBuffer = std::vector<DrawCommand>;

        struct DrawCommandList
        {
            int32_t vertex_count = 0;
            int32_t index_count = 0;

            DrawCommandBuffer command_buffer;
        };

        using DrawCommandListBuffer = std::vector<DrawCommandList>;

        struct DrawData
        {
            std::vector<ImDrawVert> vertex_data;
            std::vector<ImDrawIdx> index_data;

            ImVec2 display_position;
            ImVec2 display_size = ImVec2(-1, -1); // Start with empty

            // FIXME: optimize further using a fully flattened array?
            DrawCommandListBuffer command_lists;

            void swap(DrawData& other)
            {
                vertex_data.swap(other.vertex_data);
                index_data.swap(other.index_data);
                command_lists.swap(other.command_lists);

                // Use swap to make sure the order doesn't matter
                std::swap(display_position, other.display_position);
                std::swap(display_size, other.display_size);
            }

            bool is_valid() const
            {
                if (display_size.x <= 0.0f || display_size.y <= 0.0f)
                {
                    return false;
                }

                return true;
            }
        };
    }

    struct GUISystem::Internal
    {
        GUISystem& m_gui_system;

        DrawData m_draw_data;

        Internal(GUISystem& gui_system)
            : m_gui_system(gui_system)
        {

        }

        bool write_draw_data(DrawData& draw_data)
        {
            ImDrawData* imgui_draw_data = ImGui::GetDrawData();
            draw_data.display_size = imgui_draw_data->DisplaySize;

            if (!draw_data.is_valid())
            {
                // Nothing to draw
                return false;
            }

            draw_data.display_position = imgui_draw_data->DisplayPos;

            // Gather the vertex data and commands
            // FIXME: cache might prefer it if we're only poking specific buffers at a time?
            std::vector<ImDrawVert>& vertex_data = draw_data.vertex_data;
            vertex_data.clear();
            vertex_data.reserve(imgui_draw_data->TotalVtxCount);

            std::vector<ImDrawIdx>& index_data = draw_data.index_data;
            index_data.clear();
            index_data.reserve(imgui_draw_data->TotalIdxCount);

            DrawCommandListBuffer& command_lists = draw_data.command_lists;
            command_lists.clear();
            command_lists.reserve(imgui_draw_data->CmdListsCount);

            for (const ImDrawList* current_imgui_command_list : imgui_draw_data->CmdLists)
            {
                DrawCommandList& current_command_list = command_lists.emplace_back();

                // Gather vertices
                {
                    current_command_list.vertex_count = current_imgui_command_list->VtxBuffer.Size;

                    ImDrawVert* vertex_begin = current_imgui_command_list->VtxBuffer.Data;
                    ImDrawVert* vertex_end = vertex_begin + current_imgui_command_list->VtxBuffer.Size;

                    vertex_data.insert(vertex_data.end(), vertex_begin, vertex_end);
                }

                // Gather indices
                {
                    current_command_list.index_count = current_imgui_command_list->IdxBuffer.Size;

                    ImDrawIdx* index_begin = current_imgui_command_list->IdxBuffer.Data;
                    ImDrawIdx* index_end = index_begin + current_imgui_command_list->IdxBuffer.Size;

                    index_data.insert(index_data.end(), index_begin, index_end);
                }

                // Gather commands
                current_command_list.command_buffer.reserve(current_imgui_command_list->CmdBuffer.Size);

                ImVec2 clip_off = draw_data.display_position;
                for (const ImDrawCmd& current_imgui_command : current_imgui_command_list->CmdBuffer)
                {
                    DrawCommand& current_command = current_command_list.command_buffer.emplace_back();

                    // Project scissor/clipping rectangles into framebuffer space
                    current_command.clip_min = ImVec2(current_imgui_command.ClipRect.x - clip_off.x, current_imgui_command.ClipRect.y - clip_off.y);
                    current_command.clip_max = ImVec2(current_imgui_command.ClipRect.z - clip_off.x, current_imgui_command.ClipRect.w - clip_off.y);

                    // Get the texture
                    // FIXME: improve the texture API
                    const size_t texture_id = (size_t)current_imgui_command.GetTexID();
                    auto texture_it = m_gui_system.m_texture_lookup.find(texture_id);

                    if (texture_it != m_gui_system.m_texture_lookup.end())
                    {
                        current_command.texture_srv_handle = texture_it->second;
                    }

                    // Store the command offsets and element count
                    current_command.draw_command.type = Vadon::Render::DrawCommandType::INDEXED;
                    current_command.draw_command.vertices.offset = current_imgui_command.VtxOffset;
                    current_command.draw_command.indices.offset = current_imgui_command.IdxOffset;
                    current_command.draw_command.indices.count = current_imgui_command.ElemCount;
                }
            }

            return true;
        }

        void buffer_draw_data(const DrawData& draw_data)
        {
            if (!draw_data.is_valid())
            {
                // Nothing to draw
                return;
            }

            Vadon::Render::BufferSystem& buffer_system = m_gui_system.m_application.get_engine_core().get_system<Vadon::Render::BufferSystem>();

            // Update the buffers
            {
                const int32_t vertex_count = static_cast<int32_t>(draw_data.vertex_data.size());
                const int32_t index_count = static_cast<int32_t>(draw_data.index_data.size());
                m_gui_system.update_buffers(vertex_count, index_count);

                // Buffer the data
                const Vadon::Render::BufferWriteData vertex_buffer_write_data{ .range = {.offset = 0, .count = vertex_count }, .data = draw_data.vertex_data.data() };
                buffer_system.buffer_data(m_gui_system.m_vertex_buffer.buffer_handle, vertex_buffer_write_data);

                const Vadon::Render::BufferWriteData index_buffer_write_data{ .range = {.offset = 0, .count = index_count }, .data = draw_data.index_data.data() };
                buffer_system.buffer_data(m_gui_system.m_index_buffer.buffer_handle, index_buffer_write_data);
            }

            // Update constant buffer
            {
                // Setup orthographic projection matrix into our constant buffer
                // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
                float L = draw_data.display_position.x;
                float R = draw_data.display_position.x + draw_data.display_size.x;
                float T = draw_data.display_position.y;
                float B = draw_data.display_position.y + draw_data.display_size.y;

                VertexConstantBuffer constant_buffer_data;
                constant_buffer_data.model_view_projection =
                {
                    { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
                    { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
                    { 0.0f,         0.0f,           0.5f,       0.0f },
                    { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
                };

                const Vadon::Render::BufferWriteData constant_buffer_write_data{ .range = { 0, 1 }, .data = &constant_buffer_data };
                buffer_system.buffer_data(m_gui_system.m_constant_buffer, constant_buffer_write_data);
            }
        }

        void render_draw_data(const DrawData& draw_data)
        {
            // Render GUI based on draw data
            Vadon::Core::EngineCoreInterface& engine_core = m_gui_system.m_application.get_engine_core();

            Vadon::Render::GraphicsAPI& graphics_api = engine_core.get_system<Vadon::Render::GraphicsAPI>();
            Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
            Vadon::Render::PipelineSystem& pipeline_system = engine_core.get_system<Vadon::Render::PipelineSystem>();
            Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
            Vadon::Render::ResourceSystem& resource_system = engine_core.get_system<Vadon::Render::ResourceSystem>();
            Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
            Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();

            // Apply shaders and pipeline state
            pipeline_system.apply_blend_state(m_gui_system.m_pipeline_state.blend_update);
            pipeline_system.apply_depth_stencil_state(m_gui_system.m_pipeline_state.depth_stencil_update);
            pipeline_system.apply_rasterizer_state(m_gui_system.m_pipeline_state.rasterizer_state);

            pipeline_system.set_primitive_topology(Vadon::Render::PrimitiveTopology::TRIANGLE_LIST);

            shader_system.apply_shader(m_gui_system.m_vertex_shader);
            shader_system.apply_shader(m_gui_system.m_pixel_shader);
            shader_system.set_vertex_layout(m_gui_system.m_vertex_layout);

            buffer_system.set_vertex_buffer(m_gui_system.m_vertex_buffer.buffer_handle, 0);
            buffer_system.set_index_buffer(m_gui_system.m_index_buffer.buffer_handle, sizeof(ImDrawIdx) == 2 ? Vadon::Render::GraphicsAPIDataFormat::R16_UINT : Vadon::Render::GraphicsAPIDataFormat::R32_UINT);
            buffer_system.set_constant_buffer(Vadon::Render::ShaderType::VERTEX, m_gui_system.m_constant_buffer, 0);

            // Setup viewport
            Vadon::Render::Viewport viewport;
            viewport.dimensions.size.x = draw_data.display_size.x;
            viewport.dimensions.size.y = draw_data.display_size.y;
            viewport.depth_min_max.x = 0.0f;
            viewport.depth_min_max.y = 1.0f;
            viewport.dimensions.position.x = 0.0f;
            viewport.dimensions.position.y = 0.0f;

            rt_system.apply_viewport(viewport);

            texture_system.set_sampler(Vadon::Render::ShaderType::PIXEL, m_gui_system.m_sampler, 0);

            // Render command lists
            // (Because we merged all buffers into a single one, we maintain our own offset into them)
            int32_t global_idx_offset = 0;
            int32_t global_vtx_offset = 0;
            ImVec2 clip_off = draw_data.display_position;
            for (const DrawCommandList& current_command_list : draw_data.command_lists)
            {
                // FIXME: implement user callback for commands?
                for (const DrawCommand& current_command : current_command_list.command_buffer)
                {
                    if (current_command.clip_max.x <= current_command.clip_min.x || current_command.clip_max.y <= current_command.clip_min.y)
                    {
                        // Command not valid
                        continue;
                    }

                    // Apply scissor/clipping rectangle
                    pipeline_system.set_scissor(Vadon::Utilities::RectangleInt
                        {
                            Vadon::Utilities::Vector2i(current_command.clip_min.x, current_command.clip_min.y),
                                Vadon::Utilities::Vector2i(current_command.clip_max.x - current_command.clip_min.x, current_command.clip_max.y - current_command.clip_min.y)
                        }
                    );

                    // Bind texture
                    resource_system.apply_shader_resource(Vadon::Render::ShaderType::PIXEL, current_command.texture_srv_handle, 0);

                    // Draw the vertices
                    Vadon::Render::DrawCommand draw_command = current_command.draw_command;
                    draw_command.indices.offset += global_idx_offset;
                    draw_command.vertices.offset += global_vtx_offset;

                    graphics_api.draw(draw_command);
                }

                // Update offsets
                global_idx_offset += current_command_list.index_count;
                global_vtx_offset += current_command_list.vertex_count;
            }
        }
    };

    bool GUISystem::Buffer::is_valid(int32_t new_capacity) const
    {
        return (buffer_handle.is_valid() && (new_capacity <= capacity));
    }

    GUISystem::GUISystem(VadonApp::Core::Application& application)
        : VadonApp::Private::UI::Developer::GUISystem(application)
        , m_texture_counter(0)
        , m_internal(std::make_unique<Internal>(*this))
    {
        dev_gui_instance = this;
    }

    GUISystem::~GUISystem() = default;

    bool GUISystem::initialize()
    {
        // Setup Dear ImGui context
        // TODO: config options?
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        if (!init_platform())
        {
            return false;
        }

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        io.Fonts->AddFontDefault();

        // Set up FontAwesome fonts
        {
            const float base_font_size = 13.0f; // 13.0f is the size of the default font. Change to the font size you use.
            const float icon_font_size_pixels = base_font_size * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
                
            // merge in icons from Font Awesome
            static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            icons_config.GlyphMinAdvanceX = icon_font_size_pixels;
            io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_webfont_compressed_data, font_awesome_webfont_compressed_size, icon_font_size_pixels, &icons_config, icons_ranges);
            // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
        }

        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

        if (!init_renderer())
        {
            return false;
        }

        // TODO: use configuration?

        log_message("ImGui initialized successfully!\n");
        return true;
    }

    void GUISystem::shutdown()
    {
        ImGui::DestroyContext();
        dev_gui_instance = nullptr;
        log_message("ImGui shut down successfully!\n");
    }

    GUISystem::IOFlags GUISystem::get_io_flags() const
    {
        IOFlags flags = IOFlags::NONE;
        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse == true)
        {
            flags |= IOFlags::MOUSE_CAPTURE;
        }

        if (io.WantCaptureKeyboard == true)
        {
            flags |= IOFlags::KEYBOARD_CAPTURE;
        }

        if (io.WantTextInput == true)
        {
            flags |= IOFlags::TEXT_INPUT;
        }

        return flags;
    }

    GUIStyle GUISystem::get_style() const
    {
        ImGuiStyle& style = ImGui::GetStyle();
        return GUIStyle{
            .frame_padding = { style.FramePadding.x, style.FramePadding.y }
        };
    }

    void GUISystem::set_platform_window(VadonApp::Platform::WindowHandle window_handle)
    {
        m_platform_data.window_handle = window_handle;

#ifdef _WIN32
        // Set platform dependent data in viewport
        VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();
        ImGui::GetMainViewport()->PlatformHandleRaw = platform_interface.get_platform_window_handle(window_handle);
#endif
    }

    void GUISystem::start_frame()
    {
        update_platform();

        // Start the new ImGui frame
        // FIXME: some way to make sure user correctly used begin & end before this?
        ImGui::NewFrame();
    }

    void GUISystem::end_frame()
    {
        // Prepare the draw data, thus ending the frame
        ImGui::Render();
    }

    void GUISystem::render()
    {
        // Write, buffer, and then render the draw data directly
        if (!m_internal->write_draw_data(m_internal->m_draw_data))
        {
            // Nothing to draw
            return;
        }

        m_internal->buffer_draw_data(m_internal->m_draw_data);
        m_internal->render_draw_data(m_internal->m_draw_data);
    }

    void GUISystem::push_id(std::string_view string_id)
    {
        ImGui::PushID(string_id.data());
    }

    void GUISystem::push_id(const void* pointer_id)
    {
        ImGui::PushID(pointer_id);
    }

    void GUISystem::push_id(int32_t int_id)
    {
        ImGui::PushID(int_id);
    }

    void GUISystem::pop_id()
    {
        ImGui::PopID();
    }

    GUISystem::ID GUISystem::get_id(std::string_view string_id)
    {
        return ImGui::GetID(string_id.data());
    }

    GUISystem::ID GUISystem::get_id(const void* pointer_id)
    {
        return ImGui::GetID(pointer_id);
    }

    void GUISystem::begin_disabled(bool disabled)
    {
        ImGui::BeginDisabled(disabled);
    }

    void GUISystem::end_disabled()
    {
        ImGui::EndDisabled();
    }

    Vadon::Utilities::Vector2 GUISystem::get_available_content_region() const
    {
        const ImVec2 content_region = ImGui::GetContentRegionAvail();
        return Vadon::Utilities::Vector2{ content_region.x, content_region.y };
    }

    Vadon::Utilities::Vector2 GUISystem::calculate_text_size(std::string_view text, std::string_view text_end, bool hide_after_double_hash, float wrap_width) const
    {
        const ImVec2 text_size = ImGui::CalcTextSize(text.data(), text_end.empty() ? nullptr : text_end.data(), hide_after_double_hash, wrap_width);
        return Vadon::Utilities::Vector2{ text_size.x, text_size.y };
    }

    void GUISystem::push_item_width(float item_width)
    {
        ImGui::PushItemWidth(item_width);
    }

    void GUISystem::pop_item_width()
    {
        ImGui::PopItemWidth();
    }

    void GUISystem::set_next_item_width(float item_width)
    {
        ImGui::SetNextItemWidth(item_width);
    }

    bool GUISystem::begin_window(Window& window)
    {
        // TODO: add window initialization?
        //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        //ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        const bool enable_close = Vadon::Utilities::to_bool(window.flags & WindowFlags::ENABLE_CLOSE);
        if (enable_close == false)
        {
            return ImGui::Begin(window.title.c_str());
        }

        return ImGui::Begin(window.title.c_str(), &window.open);
    }

    void GUISystem::end_window()
    {
        ImGui::End();
    }

    bool GUISystem::is_window_focused() const
    {
        return ImGui::IsWindowFocused();
    }

    bool GUISystem::is_window_hovered() const
    {
        return ImGui::IsWindowHovered();
    }

    bool GUISystem::begin_child_window(const ChildWindow& window)
    {
        // TODO: flags!
        return ImGui::BeginChild(window.id.c_str(), ImVec2(window.size.x, window.size.y), window.border);
    }

    void GUISystem::end_child_window()
    {
        ImGui::EndChild();
    }

    bool GUISystem::begin_popup(Window& popup)
    {
        return ImGui::BeginPopup(popup.title.c_str());
    }

    bool GUISystem::begin_popup_modal(Window& popup)
    {
        // TODO: flags to set whether to pass in pointer to open flag
        return ImGui::BeginPopupModal(popup.title.c_str(), nullptr);
    }

    void GUISystem::end_popup()
    {
        ImGui::EndPopup();
    }

    void GUISystem::open_popup(std::string_view id)
    {
        ImGui::OpenPopup(id.data());
    }

    void GUISystem::close_current_popup()
    {
        ImGui::CloseCurrentPopup();
    }

    bool GUISystem::begin_popup_context_item(std::string_view id)
    {
        return ImGui::BeginPopupContextItem(id.empty() == false ? id.data() : nullptr);
    }

    bool GUISystem::begin_main_menu_bar()
    {
        return ::ImGui::BeginMainMenuBar();
    }

    void GUISystem::end_main_menu_bar()
    {
        ::ImGui::EndMainMenuBar();
    }

    bool GUISystem::begin_menu_bar()
    {
        return ::ImGui::BeginMenuBar();
    }

    void GUISystem::end_menu_bar()
    {
        ::ImGui::EndMenuBar();
    }

    bool GUISystem::begin_menu(std::string_view label, bool enabled)
    {
        return ::ImGui::BeginMenu(label.data(), enabled);
    }

    void GUISystem::end_menu()
    {
        ::ImGui::EndMenu();
    }

    bool GUISystem::add_menu_item(const MenuItem& menu_item)
    {
        return ImGui::MenuItem(menu_item.label.c_str(), nullptr, menu_item.selected, menu_item.enabled);
    }

    bool GUISystem::push_tree_node(std::string_view label, TreeNodeFlags flags)
    {
        return ImGui::TreeNodeEx(label.data(), get_imgui_treenode_flags(flags));
    }

    bool GUISystem::push_tree_node(std::string_view id, std::string_view label, TreeNodeFlags flags)
    {
        return ImGui::TreeNodeEx(id.data(), get_imgui_treenode_flags(flags), label.data());
    }

    bool GUISystem::push_tree_node(const void* id, std::string_view label, TreeNodeFlags flags)
    {
        return ImGui::TreeNodeEx(id, get_imgui_treenode_flags(flags), label.data());
    }

    void GUISystem::pop_tree_node()
    {
        ImGui::TreePop();
    }

    bool GUISystem::draw_input_int(InputInt& input_int)
    {
        return ImGui::InputInt(input_int.label.c_str(), &input_int.input);
    }

    bool GUISystem::draw_input_int2(InputInt2& input_int)
    {
        // FIXME: is this safe to use this way?
        return ImGui::InputInt2(input_int.label.c_str(), &input_int.input.x);
    }

    bool GUISystem::draw_input_float(InputFloat& input_float)
    {
        return ImGui::InputFloat(input_float.label.c_str(), &input_float.input);
    }

    bool GUISystem::draw_input_float2(InputFloat2& input_float)
    {
        // FIXME: is this safe to use this way?
        return ImGui::InputFloat2(input_float.label.c_str(), &input_float.input.x);
    }

    bool GUISystem::draw_color3_picker(InputFloat3& color)
    {
        // FIXME: is this safe to use this way?
        return ImGui::ColorPicker3(color.label.c_str(), &color.input.x);
    }

    bool GUISystem::draw_input_text(InputText& input_text) 
    {
        if (input_text.multiline)
        {
            return ImGui::InputTextMultiline(input_text.label.c_str(), &input_text.input);
        }
        else
        {
            return ImGui::InputText(input_text.label.c_str(), &input_text.input, ImGuiInputTextFlags_EnterReturnsTrue);
        }
    }

    bool GUISystem::draw_slider_int(SliderInt& slider)
    {
        return ImGui::SliderInt(slider.label.c_str(), &slider.value, slider.min, slider.max, slider.format.empty() ? "%d" : slider.format.c_str());
    }

    bool GUISystem::draw_slider_int2(SliderInt2& slider)
    {
        // FIXME: is this safe to use this way?
        return ImGui::SliderInt2(slider.label.c_str(), &slider.value.x, slider.min, slider.max, slider.format.empty() ? "%d" : slider.format.c_str());
    }

    bool GUISystem::draw_slider_float(SliderFloat& slider)
    {
        return ImGui::SliderFloat(slider.label.c_str(), &slider.value, slider.min, slider.max, slider.format.empty() ? "%.3f" : slider.format.c_str());
    }

    bool GUISystem::draw_slider_float2(SliderFloat2& slider)
    {
        // FIXME: is this safe to use this way?
        return ImGui::SliderFloat2(slider.label.c_str(), &slider.value.x, slider.min, slider.max, slider.format.empty() ? "%.3f" : slider.format.c_str());
    }

    bool GUISystem::draw_button(const Button& button)
    {
        return ImGui::Button(button.label.c_str());
    }

    bool GUISystem::draw_checkbox(Checkbox& checkbox)
    {
        return ImGui::Checkbox(checkbox.label.c_str(), &checkbox.checked);
    }

    bool GUISystem::draw_list_box(ListBox& list_box, bool* double_clicked)
    {
        if (double_clicked)
        {
            *double_clicked = false;
        }

        if (ImGui::BeginListBox(list_box.label.c_str(), ImVec2{ list_box.size.x, list_box.size.y }))
        {
            int32_t current_item_index = 0;
            for (const std::string& current_item : list_box.items)
            {
                const bool is_selected = (current_item_index == list_box.selected_item);
                if (ImGui::Selectable(current_item.c_str(), is_selected))
                {
                    list_box.selected_item = current_item_index;
                }

                if (double_clicked)
                {
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        *double_clicked = true;
                    }
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }

                ++current_item_index;
            }
            ImGui::EndListBox();
            return true;
        }

        return false;
    }

    bool GUISystem::draw_combo_box(ComboBox& combo_box)
    {
        const char* preview_item = combo_box.items.empty() ? nullptr : combo_box.items[combo_box.selected_item].c_str();
        if (ImGui::BeginCombo(combo_box.label.c_str(), preview_item) == true)
        {
            int32_t current_item_index = 0;
            for (const std::string& current_item : combo_box.items)
            {
                const bool is_selected = (current_item_index == combo_box.selected_item);
                if (ImGui::Selectable(current_item.c_str(), is_selected) == true)
                {
                    combo_box.selected_item = current_item_index;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                }
                ++current_item_index;
            }
            ImGui::EndCombo();
            return true;
        }

        return false;
    }

    bool GUISystem::begin_table(const Table& table)
    {
        // FIXME: make flags modifiable!
        return ImGui::BeginTable(table.label.c_str(), table.column_count, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY);
    }

    void GUISystem::next_table_column()
    {
        ImGui::TableNextColumn();
    }

    void GUISystem::end_table()
    {
        ImGui::EndTable();
    }

    void GUISystem::add_separator()
    {
        ImGui::Separator();
    }

    void GUISystem::add_text(std::string_view text)
    {
        ImGui::Text(text.data());
    }

    void GUISystem::add_text_unformatted(std::string_view text)
    {
        ImGui::TextUnformatted(text.data(), std::to_address(text.end()));
    }

    void GUISystem::add_text_wrapped(std::string_view text)
    {
        ImGui::TextWrapped(text.data());
    }

    void GUISystem::add_separator_text(std::string_view text)
    {
        ImGui::SeparatorText(text.data());
    }

    void GUISystem::set_item_tooltip(std::string_view tooltip_text)
    {
        ImGui::SetItemTooltip(tooltip_text.data());
    }

    void GUISystem::same_line()
    {
        ImGui::SameLine();
    }

    void GUISystem::set_scroll_x(float ratio)
    {
        ImGui::SetScrollHereX(ratio);
    }

    void GUISystem::set_scroll_y(float ratio)
    {
        ImGui::SetScrollHereY(ratio);
    }

    bool GUISystem::is_item_hovered() const
    {
        return ImGui::IsItemHovered();
    }

    bool GUISystem::is_item_active() const
    {
        return ImGui::IsItemActive();
    }

    bool GUISystem::is_item_focused() const
    {
        return ImGui::IsItemFocused();
    }

    bool GUISystem::is_item_clicked(VadonApp::Platform::MouseButton mouse_button) const
    {
        return ImGui::IsItemClicked(get_imgui_mouse_button(mouse_button));
    }

    bool GUISystem::is_item_toggled_open() const
    {
        return ImGui::IsItemToggledOpen();
    }

    bool GUISystem::is_item_edited() const
    {
        return ImGui::IsItemEdited();
    }

    bool GUISystem::is_key_down(VadonApp::Platform::KeyCode key) const
    {
        return ImGui::IsKeyDown(get_imgui_key(key));
    }

    bool GUISystem::is_key_pressed(VadonApp::Platform::KeyCode key, bool repeat) const
    {
        return ImGui::IsKeyPressed(get_imgui_key(key), repeat);
    }

    bool GUISystem::is_key_released(VadonApp::Platform::KeyCode key) const
    {
        return ImGui::IsKeyReleased(get_imgui_key(key));
    }

    bool GUISystem::is_mouse_down(VadonApp::Platform::MouseButton button) const
    {
        return ImGui::IsMouseDown(get_imgui_mouse_button(button));
    }

    bool GUISystem::is_mouse_clicked(VadonApp::Platform::MouseButton button, bool repeat) const
    {
        return ImGui::IsMouseClicked(get_imgui_mouse_button(button), repeat);
    }

    bool GUISystem::is_mouse_released(VadonApp::Platform::MouseButton button) const
    {
        return ImGui::IsMouseReleased(get_imgui_mouse_button(button));
    }

    bool GUISystem::is_mouse_double_clicked(VadonApp::Platform::MouseButton button) const
    {
        return ImGui::IsMouseDoubleClicked(get_imgui_mouse_button(button));
    }

    bool GUISystem::init_platform()
    {
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.BackendPlatformUserData == nullptr);

        VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();

        platform_interface.register_event_callback(
            [this](const VadonApp::Platform::PlatformEventList& platform_events)
            {
                process_platform_events(platform_events);
            }
        );

        // Setup backend capabilities flags
        io.BackendPlatformUserData = nullptr;
        io.BackendPlatformName = "imgui_impl_platform_engineapp";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)

        io.SetClipboardTextFn = &GUISystem::set_clipboard_text;
        io.GetClipboardTextFn = &GUISystem::get_clipboard_text;
        io.ClipboardUserData = nullptr;

        // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
        // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
        // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
        // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
        // you can ignore SDL_MOUSEBUTTONDOWN events coming right after a SDL_WINDOWEVENT_FOCUS_GAINED)
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

        // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
        SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

        m_platform_data.performance_frequency = platform_interface.get_performance_frequency();

        const VadonApp::Platform::FeatureFlags platform_feature_flags = platform_interface.get_feature_flags();
        m_platform_data.mouse_global_state = Vadon::Utilities::to_bool(platform_feature_flags & VadonApp::Platform::FeatureFlags::MOUSE_GLOBAL_STATE);

        return true;
    }

    bool GUISystem::init_renderer()
    {
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.BackendRendererUserData == nullptr);

        // Setup backend capabilities flags
        io.BackendRendererUserData = nullptr;
        io.BackendRendererName = "imgui_impl_renderer_vadon";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

        Vadon::Core::EngineCoreInterface& engine_core = m_application.get_engine_core();
        {
            // Create shaders
            Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();

            // Initialize shader and vertex layout
            {
                Vadon::Render::ShaderInfo vertex_shader_info;
                vertex_shader_info.source = HLSL_VERTEX_SHADER_SOURCE;
                vertex_shader_info.entrypoint = "main";
                vertex_shader_info.name = "ImGuiVShader";
                vertex_shader_info.type = Vadon::Render::ShaderType::VERTEX;

                m_vertex_shader = shader_system.create_shader(vertex_shader_info);
                VADON_ASSERT(m_vertex_shader.is_valid(), "Failed to create vertex shader!");
                if (m_vertex_shader.is_valid() == false)
                {
                    return false;
                }

                // Create vertex layout
                Vadon::Render::VertexLayoutInfo vertex_layout_info;

                {
                    Vadon::Render::VertexLayoutElement& position_element = vertex_layout_info.emplace_back();
                    position_element.format = Vadon::Render::GraphicsAPIDataFormat::R32G32_FLOAT;
                    position_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
                    position_element.name = "POSITION";
                }

                {
                    Vadon::Render::VertexLayoutElement& texcoord_element = vertex_layout_info.emplace_back();
                    texcoord_element.format = Vadon::Render::GraphicsAPIDataFormat::R32G32_FLOAT;
                    texcoord_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
                    texcoord_element.name = "TEXCOORD";
                }

                {
                    Vadon::Render::VertexLayoutElement& color_element = vertex_layout_info.emplace_back();
                    color_element.format = Vadon::Render::GraphicsAPIDataFormat::R8G8B8A8_UNORM;
                    color_element.type = Vadon::Render::VertexElementType::PER_VERTEX;
                    color_element.name = "COLOR";
                }

                m_vertex_layout = shader_system.create_vertex_layout(m_vertex_shader, vertex_layout_info);
                VADON_ASSERT(m_vertex_layout.is_valid(), "Failed to create vertex layout!");
                if (m_vertex_layout.is_valid() == false)
                {
                    return false;
                }
            }

            {
                Vadon::Render::ShaderInfo pixel_shader_info;
                pixel_shader_info.source = HLSL_PIXEL_SHADER_SOURCE;
                pixel_shader_info.entrypoint = "main";
                pixel_shader_info.name = "ImGuiPShader";
                pixel_shader_info.type = Vadon::Render::ShaderType::PIXEL;

                m_pixel_shader = shader_system.create_shader(pixel_shader_info);
                VADON_ASSERT(m_pixel_shader.is_valid(), "Failed to create pixel shader!");
                if (m_pixel_shader.is_valid() == false)
                {
                    return false;
                }
            }

            update_buffers(INIT_VERTEX_BUFFER_CAPACITY, INIT_INDEX_BUFFER_CAPACITY);

            // Create constant buffer
            {
                Vadon::Render::BufferInfo constant_buffer_info;
                constant_buffer_info.type = Vadon::Render::BufferType::CONSTANT;
                constant_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
                constant_buffer_info.element_size = sizeof(VertexConstantBuffer);
                constant_buffer_info.capacity = 1;

                VertexConstantBuffer constant_buffer_init_data;

                Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();
                m_constant_buffer = buffer_system.create_buffer(constant_buffer_info, &constant_buffer_init_data);
                VADON_ASSERT(m_constant_buffer.is_valid(), "Failed to create constant buffer!");
                if (m_constant_buffer.is_valid() == false)
                {
                    return false;
                }
            }
        }

        // Create fonts texture
        {
            Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();

            // Build texture atlas
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            // Upload texture to graphics system
            {
                Vadon::Render::TextureInfo texture_info;
                texture_info.dimensions.x = width;
                texture_info.dimensions.y = height;
                texture_info.mip_levels = 1;
                texture_info.array_size = 1;
                texture_info.format = Vadon::Render::GraphicsAPIDataFormat::R8G8B8A8_UNORM;
                texture_info.sample_info.count = 1;
                texture_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
                texture_info.flags = Vadon::Render::TextureFlags::SHADER_RESOURCE;

                // Create texture view
                Vadon::Render::TextureSRVInfo texture_srv_info;
                texture_srv_info.format = Vadon::Render::GraphicsAPIDataFormat::R8G8B8A8_UNORM;
                texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
                texture_srv_info.mip_levels = texture_info.mip_levels;
                texture_srv_info.most_detailed_mip = 0;

                m_fonts_texture = texture_system.create_texture(texture_info, pixels);

                // Store texture in the lookup
                Vadon::Render::SRVHandle fonts_texture_resource = texture_system.create_shader_resource_view(m_fonts_texture, texture_srv_info);

                const size_t fonts_texture_id = m_texture_counter++;
                m_texture_lookup[fonts_texture_id] = fonts_texture_resource;

                // Store our identifier
                io.Fonts->SetTexID((ImTextureID)fonts_texture_id);
            }

            // Create texture sampler
            // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
            {
                Vadon::Render::TextureSamplerInfo sampler_info;
                sampler_info.filter = Vadon::Render::TextureFilter::MIN_MAG_MIP_LINEAR;
                sampler_info.address_u = Vadon::Render::TextureAddressMode::WRAP;
                sampler_info.address_v = Vadon::Render::TextureAddressMode::WRAP;
                sampler_info.address_w = Vadon::Render::TextureAddressMode::WRAP;
                sampler_info.mip_lod_bias = 0.0f;
                sampler_info.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;
                sampler_info.min_lod = 0.0f;
                sampler_info.max_lod = 0.0f;

                m_sampler = texture_system.create_sampler(sampler_info);
            }
        }

        Vadon::Render::PipelineSystem& pipeline_system = engine_core.get_system<Vadon::Render::PipelineSystem>();

        // Create the blending setup
        {
            Vadon::Render::BlendInfo blend_info;

            blend_info.alpha_to_coverage_enable = false;
            Vadon::Render::RenderTargetBlendInfo& rt_blend_info = blend_info.render_target[0];

            rt_blend_info.blend_enable = true;
            rt_blend_info.source_blend = Vadon::Render::BlendType::SRC_ALPHA;
            rt_blend_info.dest_blend = Vadon::Render::BlendType::INV_SRC_ALPHA;
            rt_blend_info.blend_operation = Vadon::Render::BlendOperation::ADD;
            rt_blend_info.source_blend_alpha = Vadon::Render::BlendType::ONE;
            rt_blend_info.dest_blend_alpha = Vadon::Render::BlendType::INV_SRC_ALPHA;
            rt_blend_info.blend_operation_alpha = Vadon::Render::BlendOperation::ADD;
            rt_blend_info.write_mask = Vadon::Render::ColorWriteEnable::ALL;

            m_pipeline_state.blend_update.blend_state = pipeline_system.get_blend_state(blend_info);

            m_pipeline_state.blend_update.blend_factor.fill(0.0f);
            m_pipeline_state.blend_update.sample_mask = 0xffffffff;
        }

        // Create the rasterizer state
        {
            Vadon::Render::RasterizerInfo rasterizer_info;

            rasterizer_info.fill_mode = Vadon::Render::RasterizerFillMode::SOLID;
            rasterizer_info.cull_mode = Vadon::Render::RasterizerCullMode::NONE;
            rasterizer_info.scissor_enable = true;
            rasterizer_info.depth_clip_enable = true;

            m_pipeline_state.rasterizer_state = pipeline_system.get_rasterizer_state(rasterizer_info);
        }

        // Create depth-stencil State
        {
            Vadon::Render::DepthStencilInfo depth_stencil_info;

            Vadon::Render::DepthInfo& depth_info = depth_stencil_info.depth;

            depth_info.enable = false;
            depth_info.write_mask = Vadon::Render::DepthWriteMask::ALL;
            depth_info.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;

            Vadon::Render::StencilInfo& stencil_info = depth_stencil_info.stencil;

            stencil_info.enable = false;
            stencil_info.front_face.fail = stencil_info.front_face.depth_fail = stencil_info.front_face.pass = Vadon::Render::StencilOperation::KEEP;
            stencil_info.front_face.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;
            stencil_info.back_face = stencil_info.front_face;

            m_pipeline_state.depth_stencil_update.depth_stencil = pipeline_system.get_depth_stencil_state(depth_stencil_info);
            m_pipeline_state.depth_stencil_update.stencil_ref = 0;
        }

        return true;
    }

    void GUISystem::update_platform()
    {
        ImGuiIO& io = ImGui::GetIO();
        VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();
        const VadonApp::Platform::WindowFlags platform_window_flags = platform_interface.get_window_flags(m_platform_data.window_handle);

        // Setup display size (every frame to accommodate for window resizing)
        int width, height;
        if (Vadon::Utilities::to_bool(platform_window_flags & VadonApp::Platform::WindowFlags::MINIMIZED))
        {
            width = height = 0;
        }
        else
        {
            const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_size(m_platform_data.window_handle);
            width = window_size.x;
            height = window_size.y;
        }

        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        if ((width > 0) && (height > 0))
        {
            const Vadon::Utilities::Vector2i drawable_size = platform_interface.get_window_drawable_size(m_platform_data.window_handle);
            io.DisplayFramebufferScale = ImVec2(static_cast<float>(drawable_size.x) / width, static_cast<float>(drawable_size.y) / height);
        }

        // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
        uint64_t current_time = platform_interface.get_performance_counter();
        io.DeltaTime = m_platform_data.current_time > 0 ? (float)((double)(current_time - m_platform_data.current_time) / m_platform_data.performance_frequency) : (float)(1.0f / 60.0f);
        m_platform_data.current_time = current_time;

        if (m_platform_data.pending_mouse_leave_frame && (m_platform_data.pending_mouse_leave_frame >= ImGui::GetFrameCount()) && (m_platform_data.mouse_buttons_down == 0))
        {
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            m_platform_data.pending_mouse_leave_frame = 0;
        }

        update_mouse_data();
        update_mouse_cursor();

        // Update game controllers (if enabled and available)
        update_gamepads();
    }

    void GUISystem::process_platform_events(const VadonApp::Platform::PlatformEventList& platform_events)
    {
        auto platform_event_visitor = Vadon::Utilities::VisitorOverloadList{
            [this](const VadonApp::Platform::WindowEvent& window_event)
            {
                if (window_event.type == VadonApp::Platform::WindowEventType::ENTER)
                {
                    m_platform_data.pending_mouse_leave_frame = 0;
                }
                else if (window_event.type == VadonApp::Platform::WindowEventType::LEAVE)
                {
                    m_platform_data.pending_mouse_leave_frame = ImGui::GetFrameCount() + 1;
                }

                ImGuiIO& io = ImGui::GetIO();
                if (window_event.type == VadonApp::Platform::WindowEventType::FOCUS_GAINED)
                {
                    io.AddFocusEvent(true);
                }
                else if (window_event.type == VadonApp::Platform::WindowEventType::FOCUS_LOST)
                {
                    io.AddFocusEvent(false);
                }
            },
            [](const VadonApp::Platform::MouseMotionEvent& mouse_motion)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddMousePosEvent(static_cast<float>(mouse_motion.position.x), static_cast<float>(mouse_motion.position.y));
            },
            [this](const VadonApp::Platform::MouseButtonEvent& mouse_button)
            {
                ImGuiIO& io = ImGui::GetIO();
                const ImGuiMouseButton_ imgui_mouse_button = get_imgui_mouse_button(mouse_button.button);

                io.AddMouseButtonEvent(imgui_mouse_button, mouse_button.down); // TODO: map application button code to ImGui button code!
                m_platform_data.mouse_buttons_down = mouse_button.down ? (m_platform_data.mouse_buttons_down | (1 << imgui_mouse_button)) : (m_platform_data.mouse_buttons_down & ~(1 << imgui_mouse_button));
            },
            [](const VadonApp::Platform::MouseWheelEvent& mouse_wheel)
            {
                const float wheel_x = -mouse_wheel.precise_x;
                const float wheel_y = mouse_wheel.precise_y;

                ImGuiIO& io = ImGui::GetIO();
                io.AddMouseWheelEvent(wheel_x, wheel_y);
            },
            [](const VadonApp::Platform::KeyboardEvent& keyboard_event)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddKeyEvent(ImGuiMod_Ctrl, Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::CTRL));
                io.AddKeyEvent(ImGuiMod_Shift, Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::SHIFT));
                io.AddKeyEvent(ImGuiMod_Alt, Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::ALT));
                io.AddKeyEvent(ImGuiMod_Super, Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::GUI));

                ImGuiKey key = get_imgui_key(keyboard_event.key);
                io.AddKeyEvent(key, keyboard_event.down);

                io.SetKeyEventNativeData(key, keyboard_event.native_code, keyboard_event.native_scancode, keyboard_event.native_scancode); // To support legacy indexing (<1.87 user code).
            },
            [](const VadonApp::Platform::TextInputEvent& text_event)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.AddInputCharactersUTF8(text_event.text.c_str());
            },
            [](auto) { /* Default, do nothing */}
        };

        for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
        {
            std::visit(platform_event_visitor, current_event);
        }
    }

    void GUISystem::update_mouse_data()
    {
        VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();
        platform_interface.capture_mouse((m_platform_data.mouse_buttons_down != 0) ? true : false);

        // We forward mouse input when focused
        if (platform_interface.is_window_focused(m_platform_data.window_handle))
        {
            ImGuiIO& io = ImGui::GetIO();
            // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
            if (io.WantSetMousePos)
            {
                platform_interface.warp_mouse(m_platform_data.window_handle, Vadon::Utilities::Vector2i(static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)));
            }

            // (Optional) Fallback to provide mouse position when focused
            if (m_platform_data.mouse_global_state && (m_platform_data.mouse_buttons_down == 0))
            {
                const Vadon::Utilities::Vector2i mouse_position = platform_interface.get_mouse_position();
                const Vadon::Utilities::Vector2i window_position = platform_interface.get_window_position(m_platform_data.window_handle);

                const Vadon::Utilities::Vector2i mouse_window_position = mouse_position - window_position;

                io.AddMousePosEvent(static_cast<float>(mouse_window_position.x), static_cast<float>(mouse_window_position.y));
            }
        }
    }

    void GUISystem::update_mouse_cursor()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        {
            return;
        }

        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();
        if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            platform_interface.show_cursor(false);
        }
        else
        {
            // Show OS mouse cursor
            platform_interface.set_cursor(get_platform_cursor(Vadon::Utilities::to_enum<ImGuiMouseCursor_>(imgui_cursor)));
            platform_interface.show_cursor(true);
        }
    }

    void GUISystem::update_gamepads()
    {
        // TODO!!!
#if 0
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0) // FIXME: Technically feeding gamepad shouldn't depend on this now that they are regular inputs.
            return;

        // Get gamepad
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        SDL_GameController* game_controller = SDL_GameControllerOpen(0);
        if (!game_controller)
            return;
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

        // Update gamepad inputs
#define IM_SATURATE(V)                      (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_NO)       { io.AddKeyEvent(KEY_NO, SDL_GameControllerGetButton(game_controller, BUTTON_NO) != 0); }
#define MAP_ANALOG(KEY_NO, AXIS_NO, V0, V1) { float vn = (float)(SDL_GameControllerGetAxis(game_controller, AXIS_NO) - V0) / (float)(V1 - V0); vn = IM_SATURATE(vn); io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn); }
        const int thumb_dead_zone = 8000;           // SDL_gamecontroller.h suggests using this value.
        MAP_BUTTON(ImGuiKey_GamepadStart, SDL_CONTROLLER_BUTTON_START);
        MAP_BUTTON(ImGuiKey_GamepadBack, SDL_CONTROLLER_BUTTON_BACK);
        MAP_BUTTON(ImGuiKey_GamepadFaceLeft, SDL_CONTROLLER_BUTTON_X);              // Xbox X, PS Square
        MAP_BUTTON(ImGuiKey_GamepadFaceRight, SDL_CONTROLLER_BUTTON_B);              // Xbox B, PS Circle
        MAP_BUTTON(ImGuiKey_GamepadFaceUp, SDL_CONTROLLER_BUTTON_Y);              // Xbox Y, PS Triangle
        MAP_BUTTON(ImGuiKey_GamepadFaceDown, SDL_CONTROLLER_BUTTON_A);              // Xbox A, PS Cross
        MAP_BUTTON(ImGuiKey_GamepadDpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        MAP_BUTTON(ImGuiKey_GamepadDpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        MAP_BUTTON(ImGuiKey_GamepadDpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
        MAP_BUTTON(ImGuiKey_GamepadDpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        MAP_BUTTON(ImGuiKey_GamepadL1, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        MAP_BUTTON(ImGuiKey_GamepadR1, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        MAP_ANALOG(ImGuiKey_GamepadL2, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 0.0f, 32767);
        MAP_ANALOG(ImGuiKey_GamepadR2, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.0f, 32767);
        MAP_BUTTON(ImGuiKey_GamepadL3, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        MAP_BUTTON(ImGuiKey_GamepadR3, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        MAP_ANALOG(ImGuiKey_GamepadLStickLeft, SDL_CONTROLLER_AXIS_LEFTX, -thumb_dead_zone, -32768);
        MAP_ANALOG(ImGuiKey_GamepadLStickRight, SDL_CONTROLLER_AXIS_LEFTX, +thumb_dead_zone, +32767);
        MAP_ANALOG(ImGuiKey_GamepadLStickUp, SDL_CONTROLLER_AXIS_LEFTY, -thumb_dead_zone, -32768);
        MAP_ANALOG(ImGuiKey_GamepadLStickDown, SDL_CONTROLLER_AXIS_LEFTY, +thumb_dead_zone, +32767);
        MAP_ANALOG(ImGuiKey_GamepadRStickLeft, SDL_CONTROLLER_AXIS_RIGHTX, -thumb_dead_zone, -32768);
        MAP_ANALOG(ImGuiKey_GamepadRStickRight, SDL_CONTROLLER_AXIS_RIGHTX, +thumb_dead_zone, +32767);
        MAP_ANALOG(ImGuiKey_GamepadRStickUp, SDL_CONTROLLER_AXIS_RIGHTY, -thumb_dead_zone, -32768);
        MAP_ANALOG(ImGuiKey_GamepadRStickDown, SDL_CONTROLLER_AXIS_RIGHTY, +thumb_dead_zone, +32767);
#undef MAP_BUTTON
#undef MAP_ANALOG
#endif
    }

    void GUISystem::update_buffers(int32_t vertex_count, int32_t index_count)
    {
        if (m_vertex_buffer.is_valid(vertex_count) && m_index_buffer.is_valid(index_count))
        {
            return;
        }

        Vadon::Core::EngineCoreInterface& engine_core = m_application.get_engine_core();
        Vadon::Render::BufferSystem& buffer_system = engine_core.get_system<Vadon::Render::BufferSystem>();

        // Prepare vertex buffer
        if (m_vertex_buffer.capacity < vertex_count)
        {
            if (m_vertex_buffer.buffer_handle.is_valid())
            {
                // Clean up the previous buffer
                buffer_system.remove_buffer(m_vertex_buffer.buffer_handle);
            }

            m_vertex_buffer.capacity = vertex_count + 5000;

            // Prepare vertex buffer
            Vadon::Render::BufferInfo vertex_buffer_info;
            vertex_buffer_info.type = Vadon::Render::BufferType::VERTEX;
            vertex_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
            vertex_buffer_info.element_size = sizeof(ImDrawVert);
            vertex_buffer_info.capacity = m_vertex_buffer.capacity;

            m_vertex_buffer.buffer_handle = buffer_system.create_buffer(vertex_buffer_info);
        }

        // Prepare index buffer
        if (m_index_buffer.capacity < index_count)
        {
            if (m_index_buffer.buffer_handle.is_valid())
            {
                // Clean up the previous buffer
                buffer_system.remove_buffer(m_index_buffer.buffer_handle);
            }

            m_index_buffer.capacity = index_count + 5000;

            // Prepare vertex buffer
            Vadon::Render::BufferInfo index_buffer_info;
            index_buffer_info.type = Vadon::Render::BufferType::INDEX;
            index_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
            index_buffer_info.element_size = sizeof(ImDrawIdx);
            index_buffer_info.capacity = m_index_buffer.capacity;

            m_index_buffer.buffer_handle = buffer_system.create_buffer(index_buffer_info);
        }
    }

    const char* GUISystem::get_clipboard_text(void* /*user_data*/)
    {
        VadonApp::Platform::PlatformInterface& platform_interface = dev_gui_instance->m_application.get_system<VadonApp::Platform::PlatformInterface>();
        return platform_interface.get_clipboard_text();
    }

    void GUISystem::set_clipboard_text(void* /*user_data*/, const char* text)
    {
        VadonApp::Platform::PlatformInterface& platform_interface = dev_gui_instance->m_application.get_system<VadonApp::Platform::PlatformInterface>();
        platform_interface.set_clipboard_text(text);
    }
}