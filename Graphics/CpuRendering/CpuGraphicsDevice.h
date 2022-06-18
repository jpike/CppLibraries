#pragma once

#include "Graphics/DepthBuffer.h"
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/Images/Bitmap.h"
#include "Windowing/IWindow.h"

/// Holds graphics code related to rendering on a CPU (rather than a GPU).
namespace GRAPHICS::CPU_RENDERING
{
    /// A graphics device that performs rendering using a CPU.
    class CpuGraphicsDevice : public GRAPHICS::HARDWARE::IGraphicsDevice
    {
    public:
        // CREATION/SHUTDOWN.
        static std::unique_ptr<CpuGraphicsDevice> ConnectTo(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~CpuGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType Type() const override;
        void ChangeCapabilities(const GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType capabilities) override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Object3D& object_3D,
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The capabilities of this device.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType GraphicsDeviceCapabilities = GRAPHICS::HARDWARE::IGraphicsDevice::CPU;
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The buffer holds colors for the rendered image.
        GRAPHICS::IMAGES::Bitmap ColorBuffer = GRAPHICS::IMAGES::Bitmap(0, 0, GRAPHICS::ColorFormat::RGBA);
        /// The buffer holding depth values for depth/z-buffering.
        GRAPHICS::DepthBuffer DepthBuffer = GRAPHICS::DepthBuffer(0, 0);
    };
}
