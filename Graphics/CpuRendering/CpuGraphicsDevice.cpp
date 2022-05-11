#include "ErrorHandling/Asserts.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/RayTracing/RayTracingAlgorithm.h"
#include "Graphics/SoftwareRasterizationAlgorithm.h"

namespace GRAPHICS::CPU_RENDERING
{
    /// Attempts to create a graphics device to perform CPU rasterization to the specified window.
    /// @param[in,out]  window - The window in which to do CPU rendering.  Non-const since non-const access is sometimes needed.
    /// return  The CPU graphics device, if successfully created; null if an error occurs.
    std::unique_ptr<CpuGraphicsDevice> CpuGraphicsDevice::Create(WINDOWING::IWindow& window)
    {
        auto graphics_device = std::make_unique<CpuGraphicsDevice>();

        graphics_device->DeviceCapabilities = static_cast<GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType>(
            GRAPHICS::HARDWARE::IGraphicsDevice::CPU | GRAPHICS::HARDWARE::IGraphicsDevice::RASTERIZER);
        graphics_device->Window = &window;

        // CREATE A CLEARED COLOR BUFFER FOR THE GRAPHICS DEVICE.
        unsigned int width_in_pixels = window.GetWidthInPixels();
        unsigned int height_in_pixels = window.GetHeightInPixels();
        /// @todo   Figure out color format.
        graphics_device->ColorBuffer = GRAPHICS::IMAGES::Bitmap(width_in_pixels, height_in_pixels, GRAPHICS::ColorFormat::ARGB);
        // Clearing to black helps ensure a known, common initial state for the color buffer.
        graphics_device->ColorBuffer.FillPixels(GRAPHICS::Color::BLACK);

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
    GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType CpuGraphicsDevice::Type() const
    {
        return DeviceCapabilities;
    }

    /// Updates the capabilities of the graphics device.
    /// @param[in]  capabilities - The new capabilities to change to.
    ///     These will overwrite any old capabilities.
    ///     If not valid for this graphics device (must be CPU capabilities), then no changes will occur.
    void CpuGraphicsDevice::ChangeCapabilities(const GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType capabilities)
    {
        ASSERT_THEN_IF(capabilities & GRAPHICS::HARDWARE::IGraphicsDevice::CPU)
        {
            DeviceCapabilities = capabilities;
        }
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
    }

    /// Renders the specified on using the graphics device.
    /// @param[in]  object_3D - The object to render.
    /// @param[in]  camera - The camera to use for viewing.
    void CpuGraphicsDevice::Render(const GRAPHICS::Object3D& object_3D, const GRAPHICS::Camera& camera)
    {
        bool rasterization_enabled = (DeviceCapabilities & GRAPHICS::HARDWARE::IGraphicsDevice::RASTERIZER);
        if (rasterization_enabled)
        {
            GRAPHICS::SoftwareRasterizationAlgorithm::Render(
                object_3D,
                std::nullopt,
                camera,
                false,
                ColorBuffer,
                nullptr);
        }

        bool ray_tracing_enabled = (DeviceCapabilities & GRAPHICS::HARDWARE::IGraphicsDevice::RAY_TRACER);
        if (ray_tracing_enabled)
        {
            Scene scene;
            scene.BackgroundColor = GRAPHICS::Color::BLUE;
            scene.Objects.emplace_back(object_3D);
            scene.PointLights = std::vector<GRAPHICS::Light>();
            scene.PointLights->emplace_back(
                GRAPHICS::Light
                {
                    .Type = GRAPHICS::LightType::POINT,
                    .Color = GRAPHICS::Color(1.0f, 1.0f, 1.0f, 1.0f),
                    .PointLightWorldPosition = MATH::Vector3f(0.0f, 0.0f, 0.0f)
                });
            GRAPHICS::RAY_TRACING::RayTracingAlgorithm ray_tracing_algorithm;
            ray_tracing_algorithm.Render(
                scene, 
                camera,
                ColorBuffer);
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
