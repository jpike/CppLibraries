#include <vector>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/LightBuffer.h"
#include "Graphics/DirectX/Shader.h"
#include "Graphics/DirectX/ShaderProgram.h"
#include "Graphics/DirectX/TransformationMatrixBuffer.h"

namespace GRAPHICS::DIRECT_X
{
    const std::string ShaderProgram::DEFAULT_VERTEX_SHADER_CODE = R"HLSL( 
cbuffer TransformationMatrices
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

cbuffer Lighting
{
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
        float4 scaled_light_color = clamped_illumination * InputLightColor.rgba;
        pixel_input.LightColor = float4(scaled_light_color.rgb, 1.0);
    }
    else
    {
        pixel_input.LightColor = float4(1.0, 1.0, 1.0, 1.0);
    }
    

    return pixel_input;
}
)HLSL";

    const std::string ShaderProgram::DEFAULT_PIXEL_SHADER_CODE = R"HLSL( 
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
        /// @todo   Color components swapped for some reason.
        //return float4(lit_texture_color.rgb, 1.0);
        return float4(lit_texture_color.wzy, 1.0);
    }
    else
    {
        float4 lit_color = pixel_input.Color * pixel_input.LightColor;
        /// @todo   Color components swapped for some reason.
        //return float4(lit_color.rgb, 1.0);
        return float4(lit_color.wzy, 1.0);
    }
}
)HLSL";

    /// Creates the default shader program.
    /// @param[in,out]  device - The device on which to create the program.
    /// @return The default shader program, if successfully created.
    std::unique_ptr<ShaderProgram> ShaderProgram::CreateDefault(ID3D11Device& device)
    {
        // COMPILE THE DEFAULT VERTEX SHADER.
        std::optional<std::pair<ID3DBlob*, ID3D11VertexShader*>> vertex_shader_compiled_code_and_shader = Shader::CompileVertexShader(
            DEFAULT_VERTEX_SHADER_CODE,
            "VertexShaderEntryPoint",
            device);
        ASSERT_THEN_IF_NOT(vertex_shader_compiled_code_and_shader)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }
        ID3DBlob* vertex_shader_compiled_code = vertex_shader_compiled_code_and_shader->first;
        ID3D11VertexShader* vertex_shader = vertex_shader_compiled_code_and_shader->second;

        // COMPILE THE DEFAULT PIXEL SHADER.
        std::optional<std::pair<ID3DBlob*, ID3D11PixelShader*>> pixel_shader_compiled_code_and_shader = Shader::CompilePixelShader(
            DEFAULT_PIXEL_SHADER_CODE,
            "PixelShaderEntryPoint",
            device);
        ASSERT_THEN_IF_NOT(pixel_shader_compiled_code_and_shader)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }
        ID3DBlob* pixel_shader_compiled_code = pixel_shader_compiled_code_and_shader->first;
        ID3D11PixelShader* pixel_shader = pixel_shader_compiled_code_and_shader->second;

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
        HRESULT vertex_input_layout_creation_result = device.CreateInputLayout(
            vertex_shader_input_description.data(),
            (UINT)vertex_shader_input_description.size(),
            vertex_shader_compiled_code->GetBufferPointer(),
            vertex_shader_compiled_code->GetBufferSize(),
            &vertex_input_layout);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(vertex_input_layout_creation_result)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }
        // Compiled shader code blobs are no longer needed.
        pixel_shader_compiled_code->Release();
        vertex_shader_compiled_code->Release();

        // CREATE APPROPRIATE TEXTURE SAMPLING.
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
        HRESULT create_texture_sampler_result = device.CreateSamplerState(&sampler_description, &sampler_state);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_texture_sampler_result)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }

        // CREATE A BUFFER FOR TRANSFORMATION MATRICES.
        D3D11_BUFFER_DESC transformation_matrix_buffer_description =
        {
            .ByteWidth = sizeof(TransformationMatrixBuffer),
            // Transformation matrices will be updated by the CPU each frame but only read by the GPU.
            .Usage = D3D11_USAGE_DYNAMIC,
            // Transformation matrices will be constant across entire invocation of the shader.
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            // The CPU only needs to write to this buffer.
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            // No special resource flags are needed.
            .MiscFlags = 0,
            // No additional striding between elements in needed.
            .StructureByteStride = 0,
        };
        constexpr D3D11_SUBRESOURCE_DATA* NO_INITIAL_DATA = nullptr;
        ID3D11Buffer* transformation_matrix_buffer = nullptr;
        HRESULT transformaton_matrix_buffer_creation_result = device.CreateBuffer(&transformation_matrix_buffer_description, NO_INITIAL_DATA, &transformation_matrix_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(transformaton_matrix_buffer_creation_result)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }

        // CREATE A BUFFER FOR LIGHTING INFORMATION.
        D3D11_BUFFER_DESC lighting_buffer_description =
        {
            .ByteWidth = sizeof(LightBuffer),
            // Lighting information will be updated by the CPU each frame but only read by the GPU.
            .Usage = D3D11_USAGE_DYNAMIC,
            // Lighting information will be constant across entire invocation of the shader.
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            // The CPU only needs to write to this buffer.
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            // No special resource flags are needed.
            .MiscFlags = 0,
            // No additional striding between elements in needed.
            .StructureByteStride = 0,
        };
        ID3D11Buffer* lighting_buffer = nullptr;
        HRESULT lighting_buffer_creation_result = device.CreateBuffer(&lighting_buffer_description, NO_INITIAL_DATA, &lighting_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(lighting_buffer_creation_result)
        {
            // INDICATE THAT NO SHADER PROGRAM COULD BE CREATED.
            return nullptr;
        }

        // RETURN THE FINAL SHADER.
        std::unique_ptr<ShaderProgram> shader_program = std::make_unique<ShaderProgram>();

        shader_program->VertexShader = vertex_shader;
        shader_program->PixelShader = pixel_shader;
        shader_program->SamplerState = sampler_state;
        shader_program->VertexInputLayout = vertex_input_layout;
        shader_program->TransformMatrixBuffer = transformation_matrix_buffer;
        shader_program->LightingBuffer = lighting_buffer;

        return shader_program;
    }
}
