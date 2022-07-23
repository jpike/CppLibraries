#include "ErrorHandling/Asserts.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/CpuRendering/CpuRasterizationAlgorithm.h"
#include "Graphics/RayTracing/RayTracingAlgorithm.h"

namespace GRAPHICS::CPU_RENDERING
{
    /// Attempts to connect a graphics device to the specified window for rendering via the CPU.
    /// @param[in]  device_type - The type of graphics device to connect to the window.
    /// @param[in,out]  window - The window in which to do CPU rendering.  Non-const since non-const access is sometimes needed.
    /// return  The CPU graphics device, if successfully connected to the window; null if an error occurs.
    std::unique_ptr<CpuGraphicsDevice> CpuGraphicsDevice::ConnectTo(const GRAPHICS::HARDWARE::GraphicsDeviceType device_type, WINDOWING::IWindow& window)
    {
        auto graphics_device = std::make_unique<CpuGraphicsDevice>();

        graphics_device->DeviceType = device_type;
        graphics_device->Window = &window;

        // CREATE A CLEARED COLOR BUFFER FOR THE GRAPHICS DEVICE.
        unsigned int width_in_pixels = window.GetWidthInPixels();
        unsigned int height_in_pixels = window.GetHeightInPixels();
        /// @todo   Figure out color format.
        graphics_device->ColorBuffer = GRAPHICS::IMAGES::Bitmap(width_in_pixels, height_in_pixels, GRAPHICS::ColorFormat::ARGB);
        // Clearing to black helps ensure a known, common initial state for the color buffer.
        graphics_device->ColorBuffer.FillPixels(GRAPHICS::Color::BLACK);

        graphics_device->DepthBuffer = GRAPHICS::DepthBuffer(width_in_pixels, height_in_pixels);

        return graphics_device;
    }

    /// Shutdowns the graphics device, freeing up allocated resources.
    void CpuGraphicsDevice::Shutdown()
    {
        // @todo
    }

    /// Shuts down the graphics device to ensure all resources are freed.
    CpuGraphicsDevice::~CpuGraphicsDevice()
    {
        Shutdown();
    }

    /// Gets the type of the graphics device.
    /// @return The type of the graphics device.
    GRAPHICS::HARDWARE::GraphicsDeviceType CpuGraphicsDevice::Type() const
    {
        return DeviceType;
    }

    /// Does nothing since no additional loading is needed for 3D objects on the CPU.
    /// @param[in,out]  object_3D - The object to load.  Nothing is done since the object is already loaded into CPU memory.
    void CpuGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // Reference the parameter to avoid compiler warnings.
        object_3D;
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    /// @param[in]  color - The background color to clear to.
    void CpuGraphicsDevice::ClearBackground(const GRAPHICS::Color& color)
    {
        ColorBuffer.FillPixels(color);

        DepthBuffer.ClearToDepth(GRAPHICS::DepthBuffer::MAX_DEPTH);
    }

    /// Renders the specified scene using the graphics device.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  camera - The camera to use for viewing.
    /// @param[in]  cull_backfaces - True if backface culling should occur; false if not.
    /// @param[in]  depth_buffering - True if depth buffering should be used; false if not.
    void CpuGraphicsDevice::Render(
        const GRAPHICS::Scene& scene,
        const GRAPHICS::RenderingSettings& rendering_settings)
    {
        switch (DeviceType)
        {
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER:
            {
                GRAPHICS::DepthBuffer* depth_buffer = rendering_settings.DepthBuffering ? &DepthBuffer : nullptr;
                CpuRasterizationAlgorithm::Render(
                    scene,
                    rendering_settings,
                    ColorBuffer,
                    depth_buffer);
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER:
            {
                GRAPHICS::RAY_TRACING::RayTracingAlgorithm::Render(
                    scene,
                    rendering_settings,
                    ColorBuffer);
                break;
            }
            default:
            {
                // No rendering can occur for an invalid device type.
                break;
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    void CpuGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
        // DISPLAY THE COLORED IMAGE IN THE WINDOW. 
        window.Display(ColorBuffer);
    }
}
