#pragma once

#include "Graphics/DepthBuffer.h"
#include "Graphics/Hardware/GraphicsDeviceType.h"
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
        static std::unique_ptr<CpuGraphicsDevice> ConnectTo(const GRAPHICS::HARDWARE::GraphicsDeviceType device_type, WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~CpuGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::GraphicsDeviceType Type() const override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const GRAPHICS::RenderingSettings& rendering_settings) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The type of this device.
        GRAPHICS::HARDWARE::GraphicsDeviceType DeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER;
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The buffer holds colors for the rendered image.
        GRAPHICS::IMAGES::Bitmap ColorBuffer = GRAPHICS::IMAGES::Bitmap(0, 0, GRAPHICS::ColorFormat::RGBA);
        /// The buffer holding depth values for depth/z-buffering.
        GRAPHICS::DepthBuffer DepthBuffer = GRAPHICS::DepthBuffer(0, 0);
    };
}
