#pragma once

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
        static std::unique_ptr<CpuGraphicsDevice> Create(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~CpuGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType Type() const override;
        /// Updates the capabilities of the graphics device.
        /// @param[in]  capabilities - The new capabilities to change to.
        ///     These will overwrite any old capabilities.
        ///     If not valid for this graphics device, then no changes will occur.
        void ChangeCapabilities(const GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType capabilities) override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(const GRAPHICS::Object3D& object_3D, const GRAPHICS::Camera& camera) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The capabilities of this device.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType DeviceCapabilities = GRAPHICS::HARDWARE::IGraphicsDevice::CPU;
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The buffer holds colors for the rendered image.
        GRAPHICS::IMAGES::Bitmap ColorBuffer = GRAPHICS::IMAGES::Bitmap(0, 0, GRAPHICS::ColorFormat::RGBA);
    };
}
