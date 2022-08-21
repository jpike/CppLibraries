#pragma once

#include <memory>
#include <string>
#include <d3d11.h>
#include "Graphics/DirectX/Direct3DGpuResource.h"

namespace GRAPHICS::DIRECT_X
{
    /// A "complete" shader program consisting of vertex and pixel shaders.
    class ShaderProgram
    {
    public:
        // STATIC CONSTANTS.
        /// Default vertex shader code.
        static const std::string DEFAULT_VERTEX_SHADER_CODE;
        /// Default pixel shader code.
        static const std::string DEFAULT_PIXEL_SHADER_CODE;

        // CONSTRUCTION/DESTRUCTION.
        static std::unique_ptr<ShaderProgram> CreateDefault(ID3D11Device& device);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The vertex shader.
        Direct3DGpuResource<ID3D11VertexShader> VertexShader = nullptr;
        /// The pixel shader.
        Direct3DGpuResource<ID3D11PixelShader> PixelShader = nullptr;
        /// The texture sampler state.
        Direct3DGpuResource<ID3D11SamplerState> SamplerState = nullptr;
        /// The vertex input layout.
        Direct3DGpuResource<ID3D11InputLayout> VertexInputLayout = nullptr;
        /// The transformation matrix buffer.
        Direct3DGpuResource<ID3D11Buffer> TransformMatrixBuffer = nullptr;
        /// The lighting buffer.
        Direct3DGpuResource<ID3D11Buffer> LightingBuffer = nullptr;
    };
}
