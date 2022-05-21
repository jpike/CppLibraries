#include "ErrorHandling/Asserts.h"
#include "Graphics/Bgfx/BgfxGraphicsDevice.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Windowing/SdlWindow.h"

namespace GRAPHICS::HARDWARE
{
    /// Creates a graphics device of the specified type designed to render to the specified window.
    /// @param[in]  device_type - The type of graphics device to create.
    /// @param[in,out]  window - The window to render to.  Non-const since non-const access is sometimes needed.
    /// @return The graphics device of the specified type, if successfully created; null if an error occurs.
    std::unique_ptr<IGraphicsDevice> IGraphicsDevice::Create(const IGraphicsDevice::GraphicsDeviceType device_type, WINDOWING::IWindow& window)
    {
        // CREATE A GRAPHICS DEVICE OF THE APPROPRIATE TYPE.
        if (IGraphicsDevice::CPU & device_type)
        {
            /// @todo   Support ray-tracing - ideally in the same class?
            std::unique_ptr<GRAPHICS::CPU_RENDERING::CpuGraphicsDevice> cpu_graphics_device = GRAPHICS::CPU_RENDERING::CpuGraphicsDevice::ConnectTo(window);
            return cpu_graphics_device;
        }
        else if (IGraphicsDevice::BGFX & device_type)
        {
            // MAKE SURE THE WINDOW IS OF THE CORRECT TYPE.
            WINDOWING::SdlWindow* sdl_window = dynamic_cast<WINDOWING::SdlWindow*>(&window);
            ASSERT_THEN_IF_NOT(sdl_window)
            {
                return nullptr;
            }

            // CREATE THE GRAPHICS DEVICE.
            std::unique_ptr<GRAPHICS::BGFX::BgfxGraphicsDevice> bgfx_graphics_device = GRAPHICS::BGFX::BgfxGraphicsDevice::Create(device_type, *sdl_window);
            return bgfx_graphics_device;
        }
        else
        {
            // INDICATE THAT NO GRAPHICS DEVICE OF THE SPECIFIED TYPE COULD BE CREATED.
            return nullptr;
        }
    }
}
