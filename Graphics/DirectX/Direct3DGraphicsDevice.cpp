#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/Direct3DGraphicsDevice.h"
#include "Graphics/Lighting/Light.h"
#include "Windowing/Win32Window.h"

static std::string VERTEX_SHADER = R"HLSL( 
cbuffer TransformationMatrices
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    float4 LightPosition;
    float4 InputLightColor;
    int2 IsTexturedAndLit;
};

struct VertexInput
{
    float4 Position: POSITION;
    float4 Color: COLOR;
    float4 Normal: NORMAL;
    float2 TextureCoordinates: TEXCOORD0;
};

struct PixelInput
{
    float4 Position: SV_POSITION;
    float4 Color: COLOR;
    float2 TextureCoordinates: TEXCOORD0;
    bool IsTextured: BOOL;
    float4 LightColor: COLOR1;
};

PixelInput VertexShaderEntryPoint(VertexInput vertex_input)
{
    PixelInput pixel_input;

    float4 world_position = mul(WorldMatrix, vertex_input.Position);
    float4 view_position = mul(ViewMatrix, world_position);
    float4 projected_position = mul(ProjectionMatrix, view_position);
    
    //pixel_input.Position = world_position;
    //pixel_input.Position = view_position;
    //pixel_input.Position = projected_position;

    //pixel_input.Position = vertex_input.Position;
    pixel_input.Position = float4(
        projected_position.x / projected_position.w,
        projected_position.y / projected_position.w, 
        -projected_position.z / projected_position.w, 
        1.0);

    pixel_input.TextureCoordinates = vertex_input.TextureCoordinates;
    pixel_input.IsTextured = (IsTexturedAndLit.x == 1);

    pixel_input.Color = vertex_input.Color;

    if (IsTexturedAndLit.y == 1)
    {
        float3 direction_from_vertex_to_light = LightPosition.xyz - world_position.xyz;
        float3 unit_direction_from_point_to_light = normalize(direction_from_vertex_to_light);
        float illumination_proportion = dot(vertex_input.Normal.xyz, unit_direction_from_point_to_light);
        float clamped_illumination = max(0, illumination_proportion);
        float3 scaled_light_color = clamped_illumination * InputLightColor.rgb;
        pixel_input.LightColor = float4(scaled_light_color.rgb, 1.0);
    }
    else
    {
        pixel_input.LightColor = float4(1.0, 1.0, 1.0, 1.0);
    }
    

    return pixel_input;
}
)HLSL";

static std::string PIXEL_SHADER = R"HLSL( 
Texture2D texture_image;
SamplerState texture_sampler_state;

struct PixelInput
{
    float4 Position: SV_POSITION;
    float4 Color: COLOR;
    float2 TextureCoordinates: TEXCOORD0;
    bool IsTextured: BOOL;
    float4 LightColor: COLOR1;
};

float4 PixelShaderEntryPoint(PixelInput pixel_input): SV_TARGET
{
    if (pixel_input.IsTextured)
    {
        float4 texture_color = texture_image.Sample(texture_sampler_state, pixel_input.TextureCoordinates);
        float4 lit_texture_color = texture_color * pixel_input.LightColor;
        return float4(lit_texture_color.rgb, 1.0);
    }
    else
    {
        float4 lit_color = pixel_input.Color * pixel_input.LightColor;
        return float4(lit_color.rgb, 1.0);
    }
}
)HLSL";

void PrintResultIfFailed(const HRESULT result)
{
    if (FAILED(result))
    {
        std::string error_message = "\nHRESULT = " + std::to_string(result);
        OutputDebugString(error_message.c_str());
    }
};

namespace GRAPHICS::DIRECT_X
{
    /// Attempts to connect a graphics device to the specified window for rendering via Direct3D.
    /// @param[in,out]  window - The window in which to do CPU rendering.  Non-const since non-const access is sometimes needed.
    /// return  The Direct3D graphics device, if successfully connected to the window; null if an error occurs.
    std::unique_ptr<Direct3DGraphicsDevice> Direct3DGraphicsDevice::ConnectTo(WINDOWING::IWindow& window)
    {
        unsigned int width_in_pixels = window.GetWidthInPixels();
        unsigned int height_in_pixels = window.GetHeightInPixels();

        // DIRECT 3D ADAPTER SETUP.
        IDXGIFactory* factory = nullptr;
        HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
        PrintResultIfFailed(result);

        std::optional<DXGI_MODE_DESC> smallest_matching_display_mode;
        std::vector<IDXGIAdapter*> adapters;
        IDXGIAdapter* adapter = nullptr;
        for (UINT adapter_index = 0;
            (result = factory->EnumAdapters(adapter_index, &adapter)) != DXGI_ERROR_NOT_FOUND;
            ++adapter_index)
        {
            if (FAILED(result))
            {
                PrintResultIfFailed(result);
                continue;
            }

            adapters.emplace_back(adapter);
            std::string adapter_message = "\nAdapter index = " + std::to_string(adapter_index);
            OutputDebugString(adapter_message.c_str());

            std::vector<IDXGIOutput*> adapter_outputs;
            IDXGIOutput* adapter_output = nullptr;
            for (UINT output_index = 0;
                (result = adapter->EnumOutputs(output_index, &adapter_output)) != DXGI_ERROR_NOT_FOUND;
                ++output_index)
            {
                if (FAILED(result))
                {
                    PrintResultIfFailed(result);
                    continue;
                }

                adapter_outputs.emplace_back(adapter_output);
                std::string output_message = "\nOutput index = " + std::to_string(output_index);
                OutputDebugString(output_message.c_str());

                std::vector<DXGI_FORMAT> display_formats =
                {
                    //DXGI_FORMAT_UNKNOWN, // 0
                    //DXGI_FORMAT_R8G8B8A8_TYPELESS, // 27 (41 results)
                    DXGI_FORMAT_R8G8B8A8_UNORM, // 28 (41 results)
                    //DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // 29
                    //DXGI_FORMAT_R8G8B8A8_UINT, // 30
                    //DXGI_FORMAT_R8G8B8A8_SNORM, // 31
                    //DXGI_FORMAT_R8G8B8A8_SINT, // 32 (41 results)
    #if BGRA_FORMATS
                    DXGI_FORMAT_B8G8R8A8_UNORM, // 87
                    DXGI_FORMAT_B8G8R8X8_UNORM, // 88
                    DXGI_FORMAT_B8G8R8A8_TYPELESS, // 90 (41 results)
                    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, // 91
                    DXGI_FORMAT_B8G8R8X8_TYPELESS, // 92
                    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, // 93
    #endif
                };

                for (const auto& display_format : display_formats)
                {
                    UINT mode_count = 0;
                    result = adapter_output->GetDisplayModeList(
                        display_format,
                        DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING,
                        &mode_count,
                        NULL // Get all
                    );
                    if (FAILED(result))
                    {
                        PrintResultIfFailed(result);
                        continue;
                    }
                    std::string mode_count_message = "\nDisplay mode count = " + std::to_string(mode_count);
                    OutputDebugString(mode_count_message.c_str());

                    std::unique_ptr<DXGI_MODE_DESC[]> display_mode_descriptions = std::make_unique<DXGI_MODE_DESC[]>(mode_count);
                    result = adapter_output->GetDisplayModeList(
                        display_format,
                        DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING,
                        &mode_count,
                        display_mode_descriptions.get());
                    if (FAILED(result))
                    {
                        PrintResultIfFailed(result);
                        continue;
                    }
                    for (std::size_t display_mode_index = 0; display_mode_index < mode_count; ++display_mode_index)
                    {
                        const DXGI_MODE_DESC& display_mode_description = display_mode_descriptions[display_mode_index];
                        std::string display_mode_message =
                            "\nDisplay mode: " + std::to_string(display_mode_index) +
                            "\n\tWidth = " + std::to_string(display_mode_description.Width) +
                            "\n\tHeight = " + std::to_string(display_mode_description.Height) +
                            "\n\tRefreshRate = " +
                            std::to_string(display_mode_description.RefreshRate.Numerator) +
                            "/" +
                            std::to_string(display_mode_description.RefreshRate.Denominator) +
                            "\n\tFormat = " + std::to_string(display_mode_description.Format) +
                            "\n\tScanlineOrdering = " + std::to_string(display_mode_description.ScanlineOrdering) +
                            "\n\tScaling = " + std::to_string(display_mode_description.Scaling);
                        OutputDebugString(display_mode_message.c_str());

                        if (!smallest_matching_display_mode)
                        {
                            bool current_display_mode_big_enough = (
                                display_mode_description.Width >= width_in_pixels &&
                                display_mode_description.Height >= height_in_pixels);
                            if (current_display_mode_big_enough)
                            {
                                smallest_matching_display_mode = display_mode_description;
                            }
                        }
                    }
                }
                if (smallest_matching_display_mode)
                {
                    std::string display_mode_message = std::string("\nMatching display mode: ") +
                        "\n\tWidth = " + std::to_string(smallest_matching_display_mode->Width) +
                        "\n\tHeight = " + std::to_string(smallest_matching_display_mode->Height) +
                        "\n\tRefreshRate = " +
                        std::to_string(smallest_matching_display_mode->RefreshRate.Numerator) +
                        "/" +
                        std::to_string(smallest_matching_display_mode->RefreshRate.Denominator) +
                        "\n\tFormat = " + std::to_string(smallest_matching_display_mode->Format) +
                        "\n\tScanlineOrdering = " + std::to_string(smallest_matching_display_mode->ScanlineOrdering) +
                        "\n\tScaling = " + std::to_string(smallest_matching_display_mode->Scaling);
                    OutputDebugString(display_mode_message.c_str());
                }

                adapter_output->Release();
            }

            DXGI_ADAPTER_DESC adapter_description;
            result = adapter->GetDesc(&adapter_description);
            PrintResultIfFailed(result);
            // https://en.cppreference.com/w/cpp/locale/wstring_convert
            OutputDebugStringW(L"\n");
            OutputDebugStringW(adapter_description.Description);
            std::string adapter_description_message =
                "\n\tVendorId = " + std::to_string(adapter_description.VendorId) +
                "\n\tDeviceId = " + std::to_string(adapter_description.DeviceId) +
                "\n\tSubSysId = " + std::to_string(adapter_description.SubSysId) +
                "\n\tDedicatedVideoMemory = " + std::to_string(adapter_description.DedicatedVideoMemory) +
                "\n\tDedicatedSystemMemory = " + std::to_string(adapter_description.DedicatedSystemMemory) +
                "\n\tSharedSystemMemory = " + std::to_string(adapter_description.SharedSystemMemory) +
                "\n\tAdapterLuid high = " + std::to_string(adapter_description.AdapterLuid.HighPart) +
                "\n\tAdapterLuid low = " + std::to_string(adapter_description.AdapterLuid.LowPart);
            OutputDebugString(adapter_description_message.c_str());

            adapter->Release();
        }

        factory->Release();

        // DIRECT 3D - MISCELLANEOUS CORE SETUP.
        DXGI_MODE_DESC backbuffer_display_mode_description = *smallest_matching_display_mode;
        backbuffer_display_mode_description.Width = width_in_pixels;
        backbuffer_display_mode_description.Height = height_in_pixels;

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
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1,
            .OutputWindow = win32_window.WindowHandle,
            .Windowed = TRUE,
            .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
            .Flags = 0
        };
        IDXGISwapChain* swap_chain = nullptr;;
        ID3D11Device* device = nullptr;
        D3D_FEATURE_LEVEL feature_level = (D3D_FEATURE_LEVEL)0;
        ID3D11DeviceContext* device_context = nullptr;
        result = D3D11CreateDeviceAndSwapChain(
            NULL, // default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            NULL, // No software renderer
            0, // D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE,
            NULL, // Basically use whatever feature levels we can.
            0, // No feature levels provided.
            D3D11_SDK_VERSION,
            &swap_chain_description,
            &swap_chain,
            &device,
            &feature_level,
            &device_context);
        PrintResultIfFailed(result);

        ID3D11Texture2D* back_buffer = nullptr;
        constexpr UINT SINGLE_BUFFER_INDEX = 0;
        result = swap_chain->GetBuffer(SINGLE_BUFFER_INDEX, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
        PrintResultIfFailed(result);

        ID3D11RenderTargetView* render_target_view = nullptr;
        constexpr D3D11_RENDER_TARGET_VIEW_DESC* USE_MIPMAP_LEVEL_0 = NULL;
        result = device->CreateRenderTargetView(back_buffer, USE_MIPMAP_LEVEL_0, &render_target_view);
        PrintResultIfFailed(result);
        back_buffer->Release();

#if DEPTH_BUFFER
        D3D11_TEXTURE2D_DESC depth_buffer_description =
        {
            .Width = width_in_pixels,
            .Height = height_in_pixels,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        ID3D11Texture2D* depth_stencil_buffer = nullptr;
        result = device->CreateTexture2D(&depth_buffer_description, NULL, &depth_stencil_buffer);
        PrintResultIfFailed(result);

        D3D11_DEPTH_STENCIL_DESC depth_stencil_description =
        {
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
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
        result = device->CreateDepthStencilState(&depth_stencil_description, &depth_stencil_state);
        PrintResultIfFailed(result);
        device_context->OMSetDepthStencilState(depth_stencil_state, 1);

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
        result = device->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_description, &depth_stencil_view);
        PrintResultIfFailed(result);
#else
        ID3D11DepthStencilView* depth_stencil_view = nullptr;
#endif

        device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
        D3D11_VIEWPORT viewport =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<float>(width_in_pixels),
            .Height = static_cast<float>(height_in_pixels),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };
        device_context->RSSetViewports(1, &viewport);

        D3D11_RASTERIZER_DESC rasterizer_description =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_BACK,
            .FrontCounterClockwise = TRUE,
            .DepthBias = 0,
            .DepthBiasClamp = 0.0f,
            .SlopeScaledDepthBias = 0.0f,
            .DepthClipEnable = FALSE, //TRUE,
            .ScissorEnable = FALSE,
            .MultisampleEnable = FALSE,
            .AntialiasedLineEnable = FALSE,
        };
        ID3D11RasterizerState* rasterizer_state = nullptr;
        result = device->CreateRasterizerState(&rasterizer_description, &rasterizer_state);
        PrintResultIfFailed(result);
        device_context->RSSetState(rasterizer_state);

        // DIRECT 3D SHADER COMPILATION.
        ID3DBlob* vertex_shader_compiled_code = nullptr;
        ID3DBlob* vertex_shader_error_messages = nullptr;
        result = D3DCompile(
            VERTEX_SHADER.data(),
            VERTEX_SHADER.size(),
            NULL,
            NULL,
            NULL,
            "VertexShaderEntryPoint",
            "vs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
            0,
            &vertex_shader_compiled_code,
            &vertex_shader_error_messages);
        PrintResultIfFailed(result);
        if (FAILED(result))
        {
            if (vertex_shader_error_messages)
            {
                OutputDebugString((char*)vertex_shader_error_messages->GetBufferPointer());
            }
        }

        ID3DBlob* pixel_shader_compiled_code = nullptr;
        ID3DBlob* pixel_shader_error_messages = nullptr;
        result = D3DCompile(
            PIXEL_SHADER.data(),
            PIXEL_SHADER.size(),
            NULL,
            NULL,
            NULL,
            "PixelShaderEntryPoint",
            "ps_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
            0,
            &pixel_shader_compiled_code,
            &pixel_shader_error_messages);
        PrintResultIfFailed(result);
        if (FAILED(result))
        {
            if (pixel_shader_error_messages)
            {
                OutputDebugString((char*)pixel_shader_error_messages->GetBufferPointer());
            }
        }

        ID3D11VertexShader* vertex_shader = nullptr;
        result = device->CreateVertexShader(
            vertex_shader_compiled_code->GetBufferPointer(),
            vertex_shader_compiled_code->GetBufferSize(),
            NULL,
            &vertex_shader);
        PrintResultIfFailed(result);

        ID3D11PixelShader* pixel_shader = nullptr;
        result = device->CreatePixelShader(
            pixel_shader_compiled_code->GetBufferPointer(),
            pixel_shader_compiled_code->GetBufferSize(),
            NULL,
            &pixel_shader);
        PrintResultIfFailed(result);

        // DIRECT 3D - REMAINING INPUT/BUFFER SETUP.
        std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_shader_input_description =
        {
            D3D11_INPUT_ELEMENT_DESC
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = 0,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
            D3D11_INPUT_ELEMENT_DESC
            {
                .SemanticName = "COLOR",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
            D3D11_INPUT_ELEMENT_DESC
            {
                .SemanticName = "NORMAL",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
            D3D11_INPUT_ELEMENT_DESC
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0,
            },
        };
        ID3D11InputLayout* vertex_input_layout = nullptr;
        result = device->CreateInputLayout(
            vertex_shader_input_description.data(),
            (UINT)vertex_shader_input_description.size(),
            vertex_shader_compiled_code->GetBufferPointer(),
            vertex_shader_compiled_code->GetBufferSize(),
            &vertex_input_layout);
        PrintResultIfFailed(result);
        pixel_shader_compiled_code->Release();
        vertex_shader_compiled_code->Release();

        D3D11_SAMPLER_DESC sampler_description =
        {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .MipLODBias = 0.0f,
            .MaxAnisotropy = 1,
            .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
            .BorderColor = {},
            .MinLOD = 0,
            .MaxLOD = D3D11_FLOAT32_MAX
        };
        ID3D11SamplerState* sampler_state = nullptr;
        result = device->CreateSamplerState(&sampler_description, &sampler_state);
        PrintResultIfFailed(result);

        D3D11_BUFFER_DESC transformation_matrix_buffer_description =
        {
            .ByteWidth = sizeof(TransformationMatrixBuffer),
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            .MiscFlags = 0,
            .StructureByteStride = 0,
        };
        ID3D11Buffer* transformation_matrix_buffer = nullptr;
        result = device->CreateBuffer(&transformation_matrix_buffer_description, NULL, &transformation_matrix_buffer);
        PrintResultIfFailed(result);

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
        graphics_device->VertexShader = vertex_shader;
        graphics_device->PixelShader = pixel_shader;
        graphics_device->SamplerState = sampler_state;
        graphics_device->VertexInputLayout = vertex_input_layout;
        graphics_device->TransformMatrixBuffer = transformation_matrix_buffer;

        return graphics_device;
    }

    /// Shutdowns the graphics device, freeing up allocated resources.
    void Direct3DGraphicsDevice::Shutdown()
    {
        if (SamplerState)
        {
            SamplerState->Release();
            SamplerState = nullptr;
        }
        if (TransformMatrixBuffer)
        {
            TransformMatrixBuffer->Release();
            TransformMatrixBuffer = nullptr;
        }
        if (VertexInputLayout)
        {
            VertexInputLayout->Release();
            VertexInputLayout = nullptr;
        }
        if (PixelShader)
        {
            PixelShader->Release();
            PixelShader = nullptr;
        }
        if (VertexShader)
        {
            VertexShader->Release();
            VertexShader = nullptr;
        }
        if (RasterizerState)
        {
            RasterizerState->Release();
            RasterizerState = nullptr;
        }
#if DEPTH_BUFFER
        if (DepthStencilView)
        {
            DepthStencilView->Release();
            DepthStencilView = nullptr;
        }
        if (DepthStencilState)
        {
            DepthStencilState->Release();
            DepthStencilState = nullptr;
        }
        if (DepthStencilBuffer)
        {
            DepthStencilBuffer->Release();
            DepthStencilBuffer = nullptr;
        }
#endif
        if (RenderTargetView)
        {
            RenderTargetView->Release();
            RenderTargetView = nullptr;
        }
        if (SwapChain)
        {
            SwapChain->Release();
            SwapChain = nullptr;
        }
        if (DeviceContext)
        {
            DeviceContext->Release();
            DeviceContext = nullptr;
        }
        if (Device)
        {
            Device->Release();
            Device = nullptr;
        }
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

    /// Does nothing since no additional loading is needed for 3D objects on the CPU.
    /// @param[in,out]  object_3D - The object to load.  Nothing is done since the object is already loaded into CPU memory.
    void Direct3DGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // Reference the parameter to avoid compiler warnings.
        object_3D;
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    /// @param[in]  color - The background color to clear to.
    void Direct3DGraphicsDevice::ClearBackground(const GRAPHICS::Color& color)
    {
        float background_color[] =
        {
            color.Red,
            color.Green,
            color.Blue,
            color.Alpha,
        };
        DeviceContext->ClearRenderTargetView(RenderTargetView, background_color);
        DeviceContext->ClearDepthStencilView(
            DepthStencilView,
            D3D11_CLEAR_DEPTH,
            1.0f,
            0);
    }

    /// Renders the specified scene using the graphics device.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @todo   Use all of rendering settings.
    void Direct3DGraphicsDevice::Render(
        const GRAPHICS::Scene& scene,
        const GRAPHICS::RenderingSettings& rendering_settings)
    {
        unsigned int width_in_pixels = Window->GetWidthInPixels();
        unsigned int height_in_pixels = Window->GetHeightInPixels();
        float aspect_ratio = static_cast<float>(width_in_pixels) / static_cast<float>(height_in_pixels);
        DirectX::XMMATRIX perspective_matrix = DirectX::XMMatrixPerspectiveFovLH(
            MATH::Angle<float>::DegreesToRadians(rendering_settings.Camera.FieldOfView).Value,
            aspect_ratio,
            rendering_settings.Camera.NearClipPlaneViewDistance,
            rendering_settings.Camera.FarClipPlaneViewDistance);
        DirectX::XMMATRIX orthographic_matrix = DirectX::XMMatrixOrthographicLH(
            (float)width_in_pixels,
            (float)height_in_pixels,
            rendering_settings.Camera.NearClipPlaneViewDistance,
            rendering_settings.Camera.FarClipPlaneViewDistance);

#define TRANSPOSE 0
#if TRANSPOSE
        MATH::Matrix4x4f camera_view_transform = camera.ViewTransform();
        DirectX::XMMATRIX view_matrix = DirectX::XMMATRIX(camera_view_transform.ElementsInRowMajorOrder());

        MATH::Matrix4x4f projection_transform = camera.ProjectionTransform();
        DirectX::XMMATRIX projection_matrix = DirectX::XMMATRIX(projection_transform.ElementsInRowMajorOrder());
#else
        MATH::Matrix4x4f camera_view_transform = rendering_settings.Camera.ViewTransform();
        DirectX::XMMATRIX view_matrix = DirectX::XMMATRIX(camera_view_transform.Elements.ValuesInColumnMajorOrder().data());

        MATH::Matrix4x4f projection_transform = rendering_settings.Camera.ProjectionTransform();
        DirectX::XMMATRIX projection_matrix = DirectX::XMMATRIX(projection_transform.Elements.ValuesInColumnMajorOrder().data());
#endif

        std::optional< std::vector<GRAPHICS::LIGHTING::Light>> lights = std::vector<GRAPHICS::LIGHTING::Light>();
        lights->emplace_back(
            GRAPHICS::LIGHTING::Light
            {
                .Type = GRAPHICS::LIGHTING::LightType::POINT,
                .Color = GRAPHICS::Color(1.0f, 1.0f, 1.0f, 1.0f),
                .PointLightWorldPosition = MATH::Vector3f(0.0f, 0.0f, 5.0f)
            });
        bool is_lit = lights.has_value();

        // RENDER EACH OBJECT.
        for (const auto& object_3D : scene.Objects)
        {
            for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
            {
                for (const auto& triangle : mesh.Triangles)
                {
#if TRANSPOSE
                    MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
                    DirectX::XMMATRIX world_matrix = DirectX::XMMATRIX(world_transform.ElementsInRowMajorOrder());
#else
                    MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
                    DirectX::XMMATRIX world_matrix = DirectX::XMMATRIX(world_transform.Elements.ValuesInColumnMajorOrder().data());
#endif

                    D3D11_MAPPED_SUBRESOURCE mapped_matrix_buffer;
                    HRESULT result = DeviceContext->Map(TransformMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_matrix_buffer);
                    PrintResultIfFailed(result);
                    TransformationMatrixBuffer* matrix_buffer = (TransformationMatrixBuffer*)mapped_matrix_buffer.pData;

                    bool is_textured = static_cast<bool>(triangle.Material->Texture);
                    matrix_buffer->IsTexturedAndIsLit.x = is_textured;

                    matrix_buffer->IsTexturedAndIsLit.y = is_lit;
                    if (is_lit)
                    {
                        const GRAPHICS::LIGHTING::Light& first_light = lights->at(0);
                        matrix_buffer->LightPosition = DirectX::XMFLOAT4(
                            first_light.PointLightWorldPosition.X,
                            first_light.PointLightWorldPosition.Y,
                            first_light.PointLightWorldPosition.Z,
                            1.0f);
                        matrix_buffer->InputLightColor = DirectX::XMFLOAT4(
                            first_light.Color.Red,
                            first_light.Color.Green,
                            first_light.Color.Blue,
                            first_light.Color.Alpha);
                    }

#if TRANSPOSE
                    matrix_buffer->WorldMatrix = XMMatrixTranspose(world_matrix);
                    matrix_buffer->ViewMatrix = XMMatrixTranspose(view_matrix);
                    matrix_buffer->ProjectionMatrix = XMMatrixTranspose(projection_matrix);
#else
                    matrix_buffer->WorldMatrix = world_matrix;
                    matrix_buffer->ViewMatrix = view_matrix;
                    matrix_buffer->ProjectionMatrix = projection_matrix;
#endif
                    DeviceContext->Unmap(TransformMatrixBuffer, 0);
                    DeviceContext->VSSetConstantBuffers(0, 1, &TransformMatrixBuffer);

                    DeviceContext->IASetInputLayout(VertexInputLayout);
                    DeviceContext->VSSetShader(VertexShader, NULL, 0);
                    DeviceContext->PSSetShader(PixelShader, NULL, 0);

                    D3D11_BUFFER_DESC vertex_buffer_description
                    {
                        .ByteWidth = sizeof(VertexInputBuffer) * GRAPHICS::GEOMETRY::Triangle::VERTEX_COUNT,
                        .Usage = D3D11_USAGE_DEFAULT,
                        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                        .CPUAccessFlags = 0,
                        .MiscFlags = 0,
                        .StructureByteStride = 0,
                    };

                    ID3D11Texture2D* object_texture = nullptr;
                    ID3D11ShaderResourceView* texture_view = nullptr;
                    std::vector<MATH::Vector2f> texture_coordinates;
                    if (is_textured)
                    {
                        D3D11_TEXTURE2D_DESC texture_description =
                        {
                            .Width = triangle.Material->Texture->GetWidthInPixels(),
                            .Height = triangle.Material->Texture->GetHeightInPixels(),
                            .MipLevels = 0,
                            .ArraySize = 1,
                            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                            .SampleDesc = {.Count = 1, .Quality = 0 },
                            .Usage = D3D11_USAGE_DEFAULT,
                            .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
                            .CPUAccessFlags = 0,
                            .MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS
                        };
                        result = Device->CreateTexture2D(&texture_description, NULL, &object_texture);
                        PrintResultIfFailed(result);
                        UINT texture_row_pitch = (4 * texture_description.Width) * sizeof(uint8_t);
                        DeviceContext->UpdateSubresource(
                            object_texture,
                            0,
                            NULL,
                            triangle.Material->Texture->GetRawData(),
                            texture_row_pitch,
                            0);

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
                        result = Device->CreateShaderResourceView(object_texture, &texture_shader_resource_description, &texture_view);
                        PrintResultIfFailed(result);
                        DeviceContext->GenerateMips(texture_view);
                    }

                    GRAPHICS::GEOMETRY::Triangle world_space_triangle = triangle;
                    for (auto& vertex : world_space_triangle.Vertices)
                    {
                        MATH::Vector4f homogeneous_vertex = MATH::Vector4f::HomogeneousPositionVector(vertex.Position);
                        MATH::Vector4f world_homogeneous_vertex = world_transform * homogeneous_vertex;
                        vertex.Position = MATH::Vector3f(world_homogeneous_vertex.X, world_homogeneous_vertex.Y, world_homogeneous_vertex.Z);
                    }

                    MATH::Vector3f surface_normal = world_space_triangle.SurfaceNormal();
                    VertexInputBuffer vertices[] =
                    {
                        VertexInputBuffer
                        {
                            .Position = DirectX::XMFLOAT4(triangle.Vertices[0].Position.X, triangle.Vertices[0].Position.Y, triangle.Vertices[0].Position.Z, 1.0f),
                            .Color = DirectX::XMFLOAT4(
                                triangle.Vertices[0].Color.Red,
                                triangle.Vertices[0].Color.Green,
                                triangle.Vertices[0].Color.Blue,
                                triangle.Vertices[0].Color.Alpha),
                            .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, 1.0f),
                            .TextureCoordinates = DirectX::XMFLOAT2(
                                triangle.Vertices[0].TextureCoordinates.X,
                                triangle.Vertices[0].TextureCoordinates.Y),
                        },
                        VertexInputBuffer
                        {
                            .Position = DirectX::XMFLOAT4(triangle.Vertices[1].Position.X, triangle.Vertices[1].Position.Y, triangle.Vertices[1].Position.Z, 1.0f),
                            .Color = DirectX::XMFLOAT4(
                                triangle.Vertices[1].Color.Red,
                                triangle.Vertices[1].Color.Green,
                                triangle.Vertices[1].Color.Blue,
                                triangle.Vertices[1].Color.Alpha),
                            .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, 1.0f),
                            .TextureCoordinates = DirectX::XMFLOAT2(
                                triangle.Vertices[1].TextureCoordinates.X,
                                triangle.Vertices[1].TextureCoordinates.Y),
                        },
                        VertexInputBuffer
                        {
                            .Position = DirectX::XMFLOAT4(triangle.Vertices[2].Position.X, triangle.Vertices[2].Position.Y, triangle.Vertices[2].Position.Z, 1.0f),
                            .Color = DirectX::XMFLOAT4(
                                triangle.Vertices[2].Color.Red,
                                triangle.Vertices[2].Color.Green,
                                triangle.Vertices[2].Color.Blue,
                                triangle.Vertices[2].Color.Alpha),
                            .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, 1.0f),
                            .TextureCoordinates = DirectX::XMFLOAT2(
                                triangle.Vertices[2].TextureCoordinates.X,
                                triangle.Vertices[2].TextureCoordinates.Y),
                        },
                    };
                    D3D11_SUBRESOURCE_DATA vertex_data
                    {
                        .pSysMem = vertices,
                        .SysMemPitch = 0,
                        .SysMemSlicePitch = 0,
                    };
                    ID3D11Buffer* vertex_buffer = nullptr;
                    result = Device->CreateBuffer(&vertex_buffer_description, &vertex_data, &vertex_buffer);
                    PrintResultIfFailed(result);

                    unsigned int stride = sizeof(VertexInputBuffer);
                    unsigned int offset = 0;
                    DeviceContext->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
                    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                    if (is_textured)
                    {
                        DeviceContext->PSSetSamplers(0, 1, &SamplerState);
                        DeviceContext->PSSetShaderResources(0, 1, &texture_view);
                    }

                    UINT vertex_count = GRAPHICS::GEOMETRY::Triangle::VERTEX_COUNT;
                    constexpr UINT START_VERTEX_INDEX = 0;
                    DeviceContext->Draw(vertex_count, START_VERTEX_INDEX);

                    vertex_buffer->Release();

                    if (is_textured)
                    {
                        texture_view->Release();
                        object_texture->Release();
                    }
                }
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    void Direct3DGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
        SwapChain->Present(1, 0);
    }
}
