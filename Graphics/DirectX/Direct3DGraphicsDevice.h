#pragma once

#include <memory>
#include <d3d11.h>
#include "Graphics/DirectX/DirectXGpuResource.h"
#include "Graphics/DirectX/ShaderProgram.h"
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Windowing/IWindow.h"

/// Holds graphics code related to rendering with DirectX.
namespace GRAPHICS::DIRECT_X
{
    /// A graphics device that performs rendering using Direct3D.
    class Direct3DGraphicsDevice : public GRAPHICS::HARDWARE::IGraphicsDevice
    {
    public:
        // CREATION/SHUTDOWN.
        static std::unique_ptr<Direct3DGraphicsDevice> ConnectTo(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~Direct3DGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::GraphicsDeviceType Type() const override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const VIEWING::Camera& camera,
            const GRAPHICS::RenderingSettings& rendering_settings) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The graphics device.
        DirectXGpuResource<ID3D11Device> Device = nullptr;
        /// The device context.
        DirectXGpuResource<ID3D11DeviceContext> DeviceContext = nullptr;
        /// The swap chain.
        DirectXGpuResource<IDXGISwapChain> SwapChain = nullptr;
        /// The render target view.
        DirectXGpuResource<ID3D11RenderTargetView> RenderTargetView = nullptr;
        /// The depth-stencil buffer.
        DirectXGpuResource<ID3D11Texture2D> DepthStencilBuffer = nullptr;
        /// The depth-stencil state.
        DirectXGpuResource<ID3D11DepthStencilState> DepthStencilState = nullptr;
        /// The depth-stencil view.
        DirectXGpuResource<ID3D11DepthStencilView> DepthStencilView = nullptr;
        /// The rasterizer state.
        DirectXGpuResource<ID3D11RasterizerState> RasterizerState = nullptr;
        /// The shader program.
        std::unique_ptr<ShaderProgram> DefaultShaderProgram = nullptr;
    };
}
