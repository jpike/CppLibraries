#include "ErrorHandling/Asserts.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/DirectX/Direct3DGraphicsDevice.h"
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/OpenGL/OpenGLGraphicsDevice.h"
#include "Windowing/SdlWindow.h"

namespace GRAPHICS::HARDWARE
{
    /// Creates a graphics device of the specified type designed to render to the specified window.
    /// @param[in]  device_type - The type of graphics device to create.
    /// @param[in,out]  window - The window to render to.  Non-const since non-const access is sometimes needed.
    /// @return The graphics device of the specified type, if successfully created; null if an error occurs.
    std::unique_ptr<IGraphicsDevice> IGraphicsDevice::Create(const GraphicsDeviceType device_type, WINDOWING::IWindow& window)
    {
        // CREATE A GRAPHICS DEVICE OF THE APPROPRIATE TYPE.
        switch (device_type)
        {
            case GraphicsDeviceType::CPU_RASTERIZER:
            case GraphicsDeviceType::CPU_RAY_TRACER:
            {
                std::unique_ptr<GRAPHICS::CPU_RENDERING::CpuGraphicsDevice> cpu_graphics_device = GRAPHICS::CPU_RENDERING::CpuGraphicsDevice::ConnectTo(device_type, window);
                return cpu_graphics_device;
            }
            case GraphicsDeviceType::OPEN_GL:
            {
                std::unique_ptr<GRAPHICS::OPEN_GL::OpenGLGraphicsDevice> open_gl_graphics_device = GRAPHICS::OPEN_GL::OpenGLGraphicsDevice::ConnectTo(window);
                return open_gl_graphics_device;
            }
            case GraphicsDeviceType::DIRECT_3D:
            {
                std::unique_ptr<GRAPHICS::DIRECT_X::Direct3DGraphicsDevice> direct_3d_graphics_device = GRAPHICS::DIRECT_X::Direct3DGraphicsDevice::ConnectTo(window);
                return direct_3d_graphics_device;
            }
            default:
            {
                // INDICATE THAT NO GRAPHICS DEVICE OF THE SPECIFIED TYPE COULD BE CREATED.
                return nullptr;
            }
        }
    }
}
