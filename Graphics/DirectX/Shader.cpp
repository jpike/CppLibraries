#include <Windows.h>
#include <d3dcompiler.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/Shader.h"

namespace GRAPHICS::DIRECT_X
{
    /// Compiles a vertex shader.
    /// @param[in]  shader_code - The shader code to compile.
    /// @param[in]  entry_point_function_name - The name of the entry point function in the shader code.
    /// @param[in,out]  device - The device for which to create the shader.
    /// @return The compiled code and the vertex shader, if successfully compiled; null if an error occurs.
    std::optional<std::pair<ID3DBlob*, ID3D11VertexShader*>> Shader::CompileVertexShader(
        const std::string& shader_code,
        const std::string& entry_point_function_name,
        ID3D11Device& device)
    {
        // COMPILE THE VERTEX SHADER CODE.
        ID3DBlob* vertex_shader_compiled_code = nullptr;
        ID3DBlob* vertex_shader_error_messages = nullptr;
        constexpr LPCSTR UNUSED_SOURCE_NAME = nullptr;
        constexpr D3D_SHADER_MACRO* NO_DEFINES = nullptr;
        constexpr ID3DInclude* NO_INCLUDES = nullptr;
        constexpr UINT NO_EFFECT_FLAGS = 0;
        HRESULT verter_shader_compilation_result = D3DCompile(
            shader_code.data(),
            shader_code.size(),
            UNUSED_SOURCE_NAME,
            NO_DEFINES,
            NO_INCLUDES,
            entry_point_function_name.c_str(),
            "vs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
            NO_EFFECT_FLAGS,
            &vertex_shader_compiled_code,
            &vertex_shader_error_messages);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(verter_shader_compilation_result)
        {
            // PROVIDE DEBUG VISIBILITY INTO VERTEX SHADER COMPILATION ISSUES.
            if (vertex_shader_error_messages)
            {
                OutputDebugString((char*)vertex_shader_error_messages->GetBufferPointer());
            }

            // INDICATE THAT NO SHADER COULD BE CREATED.
            return std::nullopt;
        }

        // CREATE THE ACTUAL VERTEX SHADER.
        ID3D11VertexShader* vertex_shader = nullptr;
        constexpr ID3D11ClassLinkage* NO_CLASS_LINKAGE = nullptr;
        HRESULT vertex_shader_creation_result = device.CreateVertexShader(
            vertex_shader_compiled_code->GetBufferPointer(),
            vertex_shader_compiled_code->GetBufferSize(),
            NO_CLASS_LINKAGE,
            &vertex_shader);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(vertex_shader_creation_result)
        {
            // INDICATE THAT NO SHADER COULD BE CREATED.
            return std::nullopt;
        }

        // RETURN THE COMPILED CODE AND THE VERTEX SHADER.
        return std::make_pair(vertex_shader_compiled_code, vertex_shader);
    }

    /// Compiles a pixel shader.
    /// @param[in]  shader_code - The shader code to compile.
    /// @param[in]  entry_point_function_name - The name of the entry point function in the shader code.
    /// @param[in,out]  device - The device for which to create the shader.
    /// @return The compiled code and the pixel shader, if successfully compiled; null if an error occurs.
    std::optional<std::pair<ID3DBlob*, ID3D11PixelShader*>> Shader::CompilePixelShader(
        const std::string& shader_code,
        const std::string& entry_point_function_name,
        ID3D11Device& device)
    {
        // COMPILE THE PIXEL SHADER CODE.
        ID3DBlob* pixel_shader_compiled_code = nullptr;
        ID3DBlob* pixel_shader_error_messages = nullptr;
        constexpr LPCSTR UNUSED_SOURCE_NAME = nullptr;
        constexpr D3D_SHADER_MACRO* NO_DEFINES = nullptr;
        constexpr ID3DInclude* NO_INCLUDES = nullptr;
        constexpr UINT NO_EFFECT_FLAGS = 0;
        HRESULT pixel_shader_compilation_result = D3DCompile(
            shader_code.data(),
            shader_code.size(),
            UNUSED_SOURCE_NAME,
            NO_DEFINES,
            NO_INCLUDES,
            entry_point_function_name.c_str(),
            "ps_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
            NO_EFFECT_FLAGS,
            &pixel_shader_compiled_code,
            &pixel_shader_error_messages);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(pixel_shader_compilation_result)
        {
            // PROVIDE DEBUG VISIBILITY INTO PIXEL SHADER COMPILATION ISSUES.
            if (pixel_shader_error_messages)
            {
                OutputDebugString((char*)pixel_shader_error_messages->GetBufferPointer());
            }

            // INDICATE THAT NO SHADER COULD BE CREATED.
            return std::nullopt;
        }

        // CREATE THE ACTUAL PIXEL SHADER.
        ID3D11PixelShader* pixel_shader = nullptr;
        constexpr ID3D11ClassLinkage* NO_CLASS_LINKAGE = nullptr;
        HRESULT pixel_shader_creation_result = device.CreatePixelShader(
            pixel_shader_compiled_code->GetBufferPointer(),
            pixel_shader_compiled_code->GetBufferSize(),
            NO_CLASS_LINKAGE,
            &pixel_shader);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(pixel_shader_creation_result)
        {
            // INDICATE THAT NO SHADER COULD BE CREATED.
            return std::nullopt;
        }

        // RETURN THE COMPILED CODE AND THE PIXEL SHADER.
        return std::make_pair(pixel_shader_compiled_code, pixel_shader);
    }
}
