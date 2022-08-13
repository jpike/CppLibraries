#pragma once

#include <optional>
#include <string>
#include <utility>
#include <d3d11.h>

namespace GRAPHICS::DIRECT_X
{
    /// A Direct3D shader.
    class Shader
    {
    public:
        static std::optional<std::pair<ID3DBlob*, ID3D11VertexShader*>> CompileVertexShader(
            const std::string& shader_code,
            const std::string& entry_point_function_name,
            ID3D11Device& device);

        static std::optional<std::pair<ID3DBlob*, ID3D11PixelShader*>> CompilePixelShader(
            const std::string& shader_code,
            const std::string& entry_point_function_name,
            ID3D11Device& device);
    };
}
