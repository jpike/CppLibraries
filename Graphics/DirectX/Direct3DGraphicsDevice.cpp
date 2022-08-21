#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/Direct3DGraphicsDevice.h"
#include "Graphics/DirectX/DisplayMode.h"
#include "Graphics/DirectX/LightBuffer.h"
#include "Graphics/DirectX/Shader.h"
#include "Graphics/DirectX/TransformationMatrixBuffer.h"
#include "Graphics/DirectX/VertexInputBuffer.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Windowing/Win32Window.h"

namespace GRAPHICS::DIRECT_X
{
    /// Attempts to connect a graphics device to the specified window for rendering via Direct3D.
    /// @param[in,out]  window - The window in which to do rendering.  Non-const since non-const access is sometimes needed.
    /// return  The Direct3D graphics device, if successfully connected to the window; null if an error occurs.
    std::unique_ptr<Direct3DGraphicsDevice> Direct3DGraphicsDevice::ConnectTo(WINDOWING::IWindow& window)
    {
        // GET THE SMALLEST MATCHING DISPLAY MODE FOR THE WINDOW.
        unsigned int width_in_pixels = window.GetWidthInPixels();
        unsigned int height_in_pixels = window.GetHeightInPixels();
        std::optional<DXGI_MODE_DESC> smallest_matching_display_mode = DisplayMode::GetSmallestMatching(width_in_pixels, height_in_pixels);
        ASSERT_THEN_IF_NOT(smallest_matching_display_mode)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }
        
        DXGI_MODE_DESC backbuffer_display_mode_description = *smallest_matching_display_mode;
        backbuffer_display_mode_description.Width = width_in_pixels;
        backbuffer_display_mode_description.Height = height_in_pixels;

        // CREATE THE DIRECT 3D DEVICE WITH A SWAP CHAIN.
        // For now, only a single sample with the lowest quality is used for better performance.
        DXGI_SAMPLE_DESC multisampling_parameters =
        {
            .Count = 1,
            .Quality = 0
        };
        // Had to tweak a lot of these parameters to get it to work.
        WINDOWING::Win32Window& win32_window = dynamic_cast<WINDOWING::Win32Window&>(window);
        DXGI_SWAP_CHAIN_DESC swap_chain_description =
        {
            .BufferDesc = backbuffer_display_mode_description,
            .SampleDesc = multisampling_parameters,
            // Only a single buffer for rendering to the output window is used.
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1,
            .OutputWindow = win32_window.WindowHandle,
            .Windowed = TRUE,
            // The backbuffer can be discarded each time after being presented.
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
            // No special flags needed as of yet.
            .Flags = 0
        };
        constexpr IDXGIAdapter* USE_DEFAULT_DISPLAY_ADAPTER = nullptr;
        constexpr HMODULE NO_SOFTWARE_RASTERIZER = nullptr;
        constexpr D3D_FEATURE_LEVEL* USE_MOST_RECENT_FEATURE_LEVEL_POSSIBLE = nullptr;
        IDXGISwapChain* swap_chain = nullptr;;
        ID3D11Device* device = nullptr;
        D3D_FEATURE_LEVEL feature_level = (D3D_FEATURE_LEVEL)0;
        ID3D11DeviceContext* device_context = nullptr;
        HRESULT create_device_and_swap_chain_result = D3D11CreateDeviceAndSwapChain(
            USE_DEFAULT_DISPLAY_ADAPTER,
            D3D_DRIVER_TYPE_HARDWARE,
            NO_SOFTWARE_RASTERIZER,
            // No particular flags as of now but might want to use those commented out below.
            0, // D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE,
            USE_MOST_RECENT_FEATURE_LEVEL_POSSIBLE,
            0, // No feature levels provided.
            D3D11_SDK_VERSION,
            &swap_chain_description,
            &swap_chain,
            &device,
            &feature_level,
            &device_context);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_device_and_swap_chain_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // GET THE BACK BUFFER.
        ID3D11Texture2D* back_buffer = nullptr;
        constexpr UINT SINGLE_BUFFER_INDEX = 0;
        HRESULT get_back_buffer_result = swap_chain->GetBuffer(SINGLE_BUFFER_INDEX, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_back_buffer_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // GET THE RENDER TARGET VIEW.
        ID3D11RenderTargetView* render_target_view = nullptr;
        constexpr D3D11_RENDER_TARGET_VIEW_DESC* USE_MIPMAP_LEVEL_0 = NULL;
        HRESULT get_render_target_view_result = device->CreateRenderTargetView(back_buffer, USE_MIPMAP_LEVEL_0, &render_target_view);
        // The back buffer handle is no longer needed after the above call.
        back_buffer->Release();
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(get_render_target_view_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // CREATE THE DEPTH BUFFER TEXTURE.
        D3D11_TEXTURE2D_DESC depth_buffer_description =
        {
            .Width = width_in_pixels,
            .Height = height_in_pixels,
            // Minimum mipmapping so far.
            .MipLevels = 1,
            // Only creating a single texture.
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            // This texture will specifically be used for depth/stencil buffering.
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            // No CPU access needed.
            .CPUAccessFlags = 0,
            // No extra special behavior needed.
            .MiscFlags = 0,
        };
        constexpr D3D11_SUBRESOURCE_DATA* NO_INITIAL_DATA = nullptr;
        ID3D11Texture2D* depth_stencil_buffer = nullptr;
        HRESULT create_depth_texture_result = device->CreateTexture2D(&depth_buffer_description, NO_INITIAL_DATA, &depth_stencil_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_depth_texture_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // CREATE DEPTH STENCIL STATE.
        D3D11_DEPTH_STENCIL_DESC depth_stencil_description =
        {
            .DepthEnable = TRUE,
            // Writing is needed for the depth-stencil buffer.
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            // Assume that lower depth values are nearer.
            .DepthFunc = D3D11_COMPARISON_LESS,
            .StencilEnable = TRUE,
            .StencilReadMask = 0xFF,
            .StencilWriteMask = 0xFF,
            .FrontFace =
            {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_INCR,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS
            },
            .BackFace =
            {
                .StencilFailOp = D3D11_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D11_STENCIL_OP_DECR,
                .StencilPassOp = D3D11_STENCIL_OP_KEEP,
                .StencilFunc = D3D11_COMPARISON_ALWAYS
            },
        };

        ID3D11DepthStencilState* depth_stencil_state = nullptr;
        HRESULT create_depth_stencil_state_result = device->CreateDepthStencilState(&depth_stencil_description, &depth_stencil_state);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_depth_stencil_state_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }
        device_context->OMSetDepthStencilState(depth_stencil_state, 1);

        // CREATE THE DEPTH STENCIL VIEW.
        D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_description =
        {
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MipSlice = 0
            }
        };
        ID3D11DepthStencilView* depth_stencil_view = nullptr;
        HRESULT create_depth_stencil_view_result = device->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_description, &depth_stencil_view);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_depth_stencil_view_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // SET A SINGLE RENDER TARGET WITH DEPTH BUFFERING.
        constexpr UINT SINGLE_RENDER_TARGET = 1;
        device_context->OMSetRenderTargets(SINGLE_RENDER_TARGET, &render_target_view, depth_stencil_view);

        // ALLOCATE A VIEWPORT MATCHING THE WINDOW.
        D3D11_VIEWPORT viewport =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<float>(width_in_pixels),
            .Height = static_cast<float>(height_in_pixels),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };
        constexpr UINT SINGLE_VIEWPORT = 1;
        device_context->RSSetViewports(SINGLE_VIEWPORT, &viewport);

        // DEFINE THE RASTERIZER.
        D3D11_RASTERIZER_DESC rasterizer_description =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_BACK,
            .FrontCounterClockwise = TRUE,
            .DepthBias = 0,
            .DepthBiasClamp = 0.0f,
            .SlopeScaledDepthBias = 0.0f,
            .DepthClipEnable = FALSE, //TRUE,
            // The additional features below are not currently needed.
            .ScissorEnable = FALSE,
            .MultisampleEnable = FALSE,
            .AntialiasedLineEnable = FALSE,
        };
        ID3D11RasterizerState* rasterizer_state = nullptr;
        HRESULT create_rasterizer_result = device->CreateRasterizerState(&rasterizer_description, &rasterizer_state);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_rasterizer_result)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }
        device_context->RSSetState(rasterizer_state);

        // CREATE THE DEFAULT SHADER.
        std::unique_ptr<ShaderProgram> default_shader_program = ShaderProgram::CreateDefault(*device);
        ASSERT_THEN_IF_NOT(default_shader_program)
        {
            // INDICATE THAT NO DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // SET THE SHADER FOR RENDERING.
        device_context->IASetInputLayout(*default_shader_program->VertexInputLayout);
        ID3D11ClassInstance* const* NO_CLASS_INSTANCE = nullptr;
        constexpr UINT NO_CLASS_INSTANCE_COUNT = 0;
        device_context->VSSetShader(*default_shader_program->VertexShader, NO_CLASS_INSTANCE, NO_CLASS_INSTANCE_COUNT);
        device_context->PSSetShader(*default_shader_program->PixelShader, NO_CLASS_INSTANCE, NO_CLASS_INSTANCE_COUNT);

        // FINAL GRAPHICS DEVICE SETUP.
        auto graphics_device = std::make_unique<Direct3DGraphicsDevice>();

        graphics_device->Window = &window;

        graphics_device->Device = device;
        graphics_device->DeviceContext = device_context;
        graphics_device->SwapChain = swap_chain;
        graphics_device->RenderTargetView = render_target_view;
        graphics_device->DepthStencilBuffer = depth_stencil_buffer;
        graphics_device->DepthStencilState = depth_stencil_state;
        graphics_device->DepthStencilView = depth_stencil_view;
        graphics_device->RasterizerState = rasterizer_state;
        graphics_device->DefaultShaderProgram = std::move(default_shader_program);

        return graphics_device;
    }

    /// Shutdowns the graphics device, freeing up allocated resources.
    void Direct3DGraphicsDevice::Shutdown()
    {
        // FREE ANY TEXTURES.
        Textures.clear();

        // FREE VERTEX BUFFERS.
        for (ID3D11Buffer* vertex_buffer : VertexBuffers)
        {
            vertex_buffer->Release();
        }
        VertexBuffers.clear();

        // FREE OTHER RESOURCES.
        DefaultShaderProgram.reset();
        RasterizerState.Release();
        DepthStencilView.Release();
        DepthStencilState.Release();
        DepthStencilBuffer.Release();
        RenderTargetView.Release();
        SwapChain.Release();
        DeviceContext.Release();
        Device.Release();
    }

    /// Shuts down the graphics device to ensure all resources are freed.
    Direct3DGraphicsDevice::~Direct3DGraphicsDevice()
    {
        Shutdown();
    }

    /// Gets the type of the graphics device.
    /// @return The type of the graphics device.
    GRAPHICS::HARDWARE::GraphicsDeviceType Direct3DGraphicsDevice::Type() const
    {
        return GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D;
    }

    /// Loads the 3D object into the graphics device.
    /// @param[in,out]  object_3D - The object to load.
    void Direct3DGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // SKIP OVER ANY OBJECTS THAT DO NOT HAVE MESHES.
        // Only meshes can fill vertex buffers.
        bool object_has_meshes = !object_3D.Model.MeshesByName.empty();
        if (!object_has_meshes)
        {
            return;
        }

        // FILL A VERTEX BUFFER FOR THE OBJECT.
        ID3D11Buffer* vertex_buffer = VertexInputBuffer::Fill(object_3D.Model, **Device);
        ASSERT_THEN_IF_NOT(vertex_buffer)
        {
            return;
        }

        // STORE THE VERTEX BUFFER IN APPROPRIATE PLACES.
        // Since the object itself might move around in memory, the vertex buffer must be associated with it.
        object_3D.Model.Direct3DVertexBuffer = vertex_buffer;
        // The vertex buffer should be stored in this graphics device for proper memory management.
        VertexBuffers.emplace_back(vertex_buffer);

        // LOAD ANY TEXTURES.
        for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
        {
            // LOAD TEXTURES FOR ALL TRIANGLES.
            for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
            {
                // SKIP OVER ANY TRIANGLES WITHOUT MATERIALS.
                if (!triangle.Material)
                {
                    continue;
                }

                /// @todo   LOAD ANY AMBIENT TEXTURES.

                // LOAD ANY DIFFUSE TEXTURES.
                if (triangle.Material->DiffuseProperties.Texture)
                {
                    // ALLOCATE THE TEXTURE.
                    std::shared_ptr<Direct3DTexture> texture = std::make_shared<Direct3DTexture>();
                    D3D11_TEXTURE2D_DESC texture_description =
                    {
                        .Width = triangle.Material->DiffuseProperties.Texture->GetWidthInPixels(),
                        .Height = triangle.Material->DiffuseProperties.Texture->GetHeightInPixels(),
                        .MipLevels = 0,
                        .ArraySize = 1,
                        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                        .SampleDesc = {.Count = 1, .Quality = 0 },
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
                        .CPUAccessFlags = 0,
                        .MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS
                    };
                    constexpr D3D11_SUBRESOURCE_DATA* NO_INITIAL_DATA = nullptr;
                    HRESULT texture_creation_result = Device->CreateTexture2D(&texture_description, NO_INITIAL_DATA, &texture->Texture.Resource);
                    ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(texture_creation_result)
                    {
                        // CONTINUE PROCESSING OTHER TRIANGLES.
                        continue;
                    }

                    // POPULATE THE TEXTURE DATA.
                    constexpr UINT FIRST_SUBRESOURCE_INDEX = 0;
                    const D3D11_BOX* COPY_ENTIRE_TEXTURE_BOX = nullptr;
                    constexpr UINT COLOR_COMPONENT_COUNT_PER_PIXEL = 4;
                    UINT texture_row_pitch = (COLOR_COMPONENT_COUNT_PER_PIXEL * texture_description.Width) * sizeof(uint8_t);
                    constexpr UINT NO_DEPTH_PITCH = 0;
                    DeviceContext->UpdateSubresource(
                        texture->Texture.Resource,
                        FIRST_SUBRESOURCE_INDEX,
                        COPY_ENTIRE_TEXTURE_BOX,
                        triangle.Material->DiffuseProperties.Texture->GetRawData(),
                        texture_row_pitch,
                        NO_DEPTH_PITCH);

                    // ALLOW THE SHADERS TO ACCESS THE TEXTURE RESOURCE.
                    D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_description =
                    {
                        .Format = texture_description.Format,
                        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
                        .Texture2D =
                        {
                            .MostDetailedMip = 0,
                            .MipLevels = 1
                        }
                    };
                    HRESULT create_texture_shader_resource_view_result = Device->CreateShaderResourceView(*texture->Texture, &texture_shader_resource_description, &texture->TextureView.Resource);
                    ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_texture_shader_resource_view_result)
                    {
                        // CONTINUE PROCESSING OTHER TRIANGLES.
                        continue;
                    }

                    // ENSURE APPROPRIATE MIPMAPS ARE GENERATED.
                    DeviceContext->GenerateMips(*texture->TextureView);

                    // STORE THE TEXTURE IN APPROPRIATE PLACES.
                    Textures.push_back(texture);
                    triangle.Material->DiffuseProperties.Direct3DTextureResource = texture;
                }

                /// @todo   LOAD ANY SPECULAR TEXTURES.
            }
        }
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    /// @param[in]  color - The background color to clear to.
    void Direct3DGraphicsDevice::ClearBackground(const GRAPHICS::Color& color)
    {
        // CLEAR THE COLOR BUFFER.
        float background_color[] =
        {
            color.Red,
            color.Green,
            color.Blue,
            color.Alpha,
        };
        DeviceContext->ClearRenderTargetView(*RenderTargetView, background_color);

        // CLEAR THE DEPTH-STENCIL BUFFER.
        constexpr float MAX_DEPTH = 1.0f;
        constexpr UINT8 STENCIL_CLEAR_VALUE = 0;
        DeviceContext->ClearDepthStencilView(
            *DepthStencilView,
            D3D11_CLEAR_DEPTH,
            MAX_DEPTH,
            STENCIL_CLEAR_VALUE);
    }

    /// Renders the specified scene using the graphics device.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  camera - The camera through which the scene is being viewed.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @todo   Use all of rendering settings.
    void Direct3DGraphicsDevice::Render(
        const GRAPHICS::Scene& scene,
        const VIEWING::Camera& camera,
        const GRAPHICS::RenderingSettings& rendering_settings)
    {
        // CLEAR THE BACKGROUND FOR THE SCENE.
        ClearBackground(scene.BackgroundColor);

        // COMPUTE CAMERA MATRICES.
        unsigned int width_in_pixels = Window->GetWidthInPixels();
        unsigned int height_in_pixels = Window->GetHeightInPixels();
        float aspect_ratio = static_cast<float>(width_in_pixels) / static_cast<float>(height_in_pixels);
        DirectX::XMMATRIX perspective_matrix = DirectX::XMMatrixPerspectiveFovLH(
            MATH::Angle<float>::DegreesToRadians(camera.FieldOfView).Value,
            aspect_ratio,
            camera.NearClipPlaneViewDistance,
            camera.FarClipPlaneViewDistance);
        DirectX::XMMATRIX orthographic_matrix = DirectX::XMMatrixOrthographicLH(
            (float)width_in_pixels,
            (float)height_in_pixels,
            camera.NearClipPlaneViewDistance,
            camera.FarClipPlaneViewDistance);

#define TRANSPOSE 0
#if TRANSPOSE
        MATH::Matrix4x4f camera_view_transform = camera.ViewTransform();
        DirectX::XMMATRIX view_matrix = DirectX::XMMATRIX(camera_view_transform.ElementsInRowMajorOrder());

        MATH::Matrix4x4f projection_transform = camera.ProjectionTransform();
        DirectX::XMMATRIX projection_matrix = DirectX::XMMATRIX(projection_transform.ElementsInRowMajorOrder());
#else
        MATH::Matrix4x4f camera_view_transform = camera.ViewTransform();
        DirectX::XMMATRIX view_matrix = DirectX::XMMATRIX(camera_view_transform.Elements.ValuesInColumnMajorOrder().data());

        MATH::Matrix4x4f projection_transform = camera.ProjectionTransform();
        DirectX::XMMATRIX projection_matrix = DirectX::XMMATRIX(projection_transform.Elements.ValuesInColumnMajorOrder().data());
#endif

        // MAP THE TRANSFORMATION MATRIX BUFFER INTO MEMORY.
        D3D11_MAPPED_SUBRESOURCE mapped_matrix_buffer;
        constexpr UINT FIRST_SUBRESOURCE_INDEX = 0;
        constexpr UINT NO_WAIT_FLAGS = 0;
        HRESULT transformation_matrix_buffer_map_result = DeviceContext->Map(
            *DefaultShaderProgram->TransformMatrixBuffer,
            FIRST_SUBRESOURCE_INDEX, 
            D3D11_MAP_WRITE_DISCARD, 
            NO_WAIT_FLAGS, 
            &mapped_matrix_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(transformation_matrix_buffer_map_result)
        {
            // RETURN SINCE RENDERING CANNOT CONTINUE.
            return;
        }
        TransformationMatrixBuffer* matrix_buffer = (TransformationMatrixBuffer*)mapped_matrix_buffer.pData;

        // POPULATE CAMERA TRANSFORMATION MATRICES IN THE BUFFER.
#if TRANSPOSE
        matrix_buffer->ViewMatrix = XMMatrixTranspose(view_matrix);
        matrix_buffer->ProjectionMatrix = XMMatrixTranspose(projection_matrix);
#else
        matrix_buffer->ViewMatrix = view_matrix;
        matrix_buffer->ProjectionMatrix = projection_matrix;
#endif

        // MAP THE LIGHTING BUFFER INTO MEMORY.
        D3D11_MAPPED_SUBRESOURCE mapped_light_buffer;
        HRESULT light_buffer_map_result = DeviceContext->Map(
            *DefaultShaderProgram->LightingBuffer,
            FIRST_SUBRESOURCE_INDEX,
            D3D11_MAP_WRITE_DISCARD,
            NO_WAIT_FLAGS,
            &mapped_light_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(light_buffer_map_result)
        {
            // RETURN SINCE RENDERING CANNOT CONTINUE.
            return;
        }
        LightBuffer* light_buffer = (LightBuffer*)mapped_light_buffer.pData;

        // POPULATE ANY LIGHTING IN THE LIGHTING BUFFER.
        constexpr float HOMOGENOUS_POSITION_W_COORDINATE = 1.0f;
        bool is_lit = !scene.Lights.empty();
        light_buffer->IsTexturedAndLit.y = is_lit;
        if (is_lit)
        {
            const GRAPHICS::SHADING::LIGHTING::Light& first_light = scene.Lights.at(0);
            light_buffer->LightPosition = DirectX::XMFLOAT4(
                first_light.PointLightWorldPosition.X,
                first_light.PointLightWorldPosition.Y,
                first_light.PointLightWorldPosition.Z,
                HOMOGENOUS_POSITION_W_COORDINATE);
            light_buffer->InputLightColor = DirectX::XMFLOAT4(
                first_light.Color.Red,
                first_light.Color.Green,
                first_light.Color.Blue,
                first_light.Color.Alpha);
        }

        // RENDER EACH OBJECT.
        for (const Object3D& object_3D : scene.Objects)
        {
            // POPULATE THE OBJECT'S WORLD TRANSFORMATION MATRIX.
#if TRANSPOSE
            MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
            DirectX::XMMATRIX world_matrix = DirectX::XMMATRIX(world_transform.ElementsInRowMajorOrder());
            matrix_buffer->WorldMatrix = XMMatrixTranspose(world_matrix);
#else
            MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
            DirectX::XMMATRIX world_matrix = DirectX::XMMATRIX(world_transform.Elements.ValuesInColumnMajorOrder().data());
            matrix_buffer->WorldMatrix = world_matrix;
#endif
            // UPDATE THE TRANSFORMATION MATRIX BUFFER ON THE GPU.
            DeviceContext->Unmap(*DefaultShaderProgram->TransformMatrixBuffer, FIRST_SUBRESOURCE_INDEX);
            constexpr UINT FIRST_CONSTANT_BUFFER_SLOT = 0;
            constexpr UINT SINGLE_BUFFER = 1;
            DeviceContext->VSSetConstantBuffers(FIRST_CONSTANT_BUFFER_SLOT, SINGLE_BUFFER, &DefaultShaderProgram->TransformMatrixBuffer.Resource);

            // SET THE VERTEX BUFFER.
            constexpr UINT FIRST_VERTEX_BUFFER_SLOT = 0;
            constexpr UINT SINGLE_VERTEX_BUFFER = 1;
            unsigned int stride = sizeof(VertexInputBuffer);
            unsigned int offset = 0;
            DeviceContext->IASetVertexBuffers(FIRST_VERTEX_BUFFER_SLOT, SINGLE_VERTEX_BUFFER, &object_3D.Model.Direct3DVertexBuffer, &stride, &offset);
            DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // RENDER EACH MESH.
            // Since all model vertices share the same vertex buffer, the current triangle
            // out of the entire model must be tracked.
            UINT current_triangle_index = 0;
            for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
            {
                // RENDER EACH TRIANGLE.
                for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
                {
                    // CHECK IF THE TRIANGLE IS TEXTURED.
                    bool is_textured = static_cast<bool>(triangle.Material->DiffuseProperties.Texture);
                    light_buffer->IsTexturedAndLit.x = is_textured;

                    // UPDATE THE LIGHT BUFFER ON THE GPU.
                    DeviceContext->Unmap(*DefaultShaderProgram->LightingBuffer, FIRST_SUBRESOURCE_INDEX);
                    constexpr UINT SECOND_CONSTANT_BUFFER_SLOT = 1;
                    DeviceContext->VSSetConstantBuffers(SECOND_CONSTANT_BUFFER_SLOT, SINGLE_BUFFER, &DefaultShaderProgram->LightingBuffer.Resource);

                    // SET ANY TEXTURE SAMPLING FOR THE SHADER.
                    if (triangle.Material)
                    {
                        std::shared_ptr<Direct3DTexture> texture = triangle.Material->DiffuseProperties.Direct3DTextureResource.lock();
                        if (texture)
                        {
                            constexpr UINT FIRST_TEXTURE_SAMPLER_SLOT = 0;
                            constexpr UINT SINGLE_TEXTURE_SAMPLER = 1;
                            DeviceContext->PSSetSamplers(FIRST_TEXTURE_SAMPLER_SLOT, SINGLE_TEXTURE_SAMPLER, &DefaultShaderProgram->SamplerState.Resource);
                            DeviceContext->PSSetShaderResources(FIRST_TEXTURE_SAMPLER_SLOT, SINGLE_TEXTURE_SAMPLER, &texture->TextureView.Resource);
                        }
                    }

                    // DRAW THE CURRENT TRIANGLE.
                    // Since texture information is currently on a per-triangle basis, only a single triangle can be drawn at once.
                    UINT vertex_count = GEOMETRY::Triangle::VERTEX_COUNT;
                    UINT start_vertex_index = (current_triangle_index * GEOMETRY::Triangle::VERTEX_COUNT);
                    DeviceContext->Draw(vertex_count, start_vertex_index);

                    // MOVE TO THE NEXT TRIANGLE.
                    ++current_triangle_index;
                }
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    void Direct3DGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
        // Referenced to avoid compiler warnings for an unused parameter.
        window;

        constexpr UINT PRESENT_AFTER_VERTICAL_BLANK = 1;
        constexpr UINT PRESENT_FRAME_FROM_EACH_BUFFER = 0;
        SwapChain->Present(PRESENT_AFTER_VERTICAL_BLANK, PRESENT_FRAME_FROM_EACH_BUFFER);
    }
}
