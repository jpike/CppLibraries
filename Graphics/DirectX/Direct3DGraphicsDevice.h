#pragma once

#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Windowing/IWindow.h"

/// Holds graphics code related to rendering with DirectX.
namespace GRAPHICS::DIRECT_X
{
    #define DEPTH_BUFFER 1

    struct TransformationMatrixBuffer
    {
        DirectX::XMMATRIX WorldMatrix;
        DirectX::XMMATRIX ViewMatrix;
        DirectX::XMMATRIX ProjectionMatrix;
        /// @todo   Remove hack for texturing/light here.
        DirectX::XMFLOAT4 LightPosition;
        DirectX::XMFLOAT4 InputLightColor;
        DirectX::XMINT2 IsTexturedAndIsLit;
    };

    struct VertexInputBuffer
    {
        DirectX::XMFLOAT4 Position;
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT4 Normal;
        DirectX::XMFLOAT2 TextureCoordinates;
    };

    /// A graphics device that performs rendering using Direct3D.
    class Direct3DGraphicsDevice : public GRAPHICS::HARDWARE::IGraphicsDevice
    {
    public:
        // CREATION/SHUTDOWN.
        static std::unique_ptr<Direct3DGraphicsDevice> ConnectTo(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~Direct3DGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType Type() const override;
        void ChangeCapabilities(const GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType capabilities) override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The capabilities of this device.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType GraphicsDeviceCapabilities = static_cast<GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType>(
            GRAPHICS::HARDWARE::IGraphicsDevice::GPU | GRAPHICS::HARDWARE::IGraphicsDevice::DIRECT_3D);
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;

        /// @todo   Document members.
        ID3D11Device* Device = nullptr;
        ID3D11DeviceContext* DeviceContext = nullptr;
        IDXGISwapChain* SwapChain = nullptr;
        ID3D11RenderTargetView* RenderTargetView = nullptr;
#if DEPTH_BUFFER
        ID3D11Texture2D* DepthStencilBuffer = nullptr;
        ID3D11DepthStencilState* DepthStencilState = nullptr;
        ID3D11DepthStencilView* DepthStencilView = nullptr;
#endif
        ID3D11RasterizerState* RasterizerState = nullptr;
        ID3D11VertexShader* VertexShader = nullptr;
        ID3D11PixelShader* PixelShader = nullptr;
        ID3D11SamplerState* SamplerState = nullptr;
        ID3D11InputLayout* VertexInputLayout = nullptr;
        ID3D11Buffer* TransformMatrixBuffer = nullptr;
    };
}
