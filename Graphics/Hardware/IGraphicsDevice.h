#pragma once

#include <memory>
#include "Graphics/Color.h"
#include "Graphics/Object3D.h"
#include "Graphics/Scene.h"
#include "Graphics/Viewing/Camera.h"
#include "Windowing/IWindow.h"

/// Holds code related to graphics hardware.
namespace GRAPHICS::HARDWARE
{
    /// A base interface to support rendering via different kinds of graphics devices.
    class IGraphicsDevice
    {
    public:
        // ENUMS.
        /// Different types of graphics devices.
        /// Types of graphic devices are determined by combining different enum values
        /// as bitflags.  Not all combinations are valid.
        /// 
        /// An regular enum wrapped in this class to provide:
        /// - Closely associating this type with graphics devices - no need to have a separate file/class.
        /// - Named scoping for enum values.
        /// - Easier integer manipulation of values.
        enum GraphicsDeviceType
        {
            /// Invalid value to effectively indicate "no" rendering and provide a reasonable default 0 value.
            NONE = 0,

            /// A graphics device using the CPU for rendering (as opposed to a GPU).
            CPU = 1 << 0,
            /// A graphics device using rasterization for rendering.
            RASTERIZER = 1 << 1,
            /// A graphics device using ray tracing for rendering.
            RAY_TRACER = 1 << 2,

            /// A graphics device using the GPU for rendering.
            GPU = 1 << 3,
            /// A graphics device using OpenGL for rendering.
            OPEN_GL = 1 << 4,
            /// A graphics device using Direct3D for rendering.
            DIRECT_3D = 1 << 5,

            /// A graphics device using the BGFX library (https://bkaradzic.github.io/bgfx/).
            BGFX = 1 << 6,
        };
        // SUPPORTED DEVICE TYPE COMBINATIONS.
        /// Rendering via CPU rasterization.
        static constexpr GraphicsDeviceType CPU_RASTERIZER = static_cast<GraphicsDeviceType>(CPU | RASTERIZER);
        /// Rendering via CPU ray tracing.
        static constexpr GraphicsDeviceType CPU_RAY_TRACER = static_cast<GraphicsDeviceType>(CPU | RAY_TRACER);

        // CREATION/SHUTDOWN.
        static std::unique_ptr<IGraphicsDevice> Create(const GraphicsDeviceType device_type, WINDOWING::IWindow& window);
        /// Shuts down operation of the graphics device.  Should free any resources.
        virtual void Shutdown() = 0;
        /// Default virtual destructor to support inheritance.
        virtual ~IGraphicsDevice() = default;

        // INFORMATION RETRIEVAL.
        /// Gets the type of the graphics device.
        /// @return The type of the graphics device.
        virtual GraphicsDeviceType Type() const = 0;
        /// Updates the capabilities of the graphics device.
        /// @param[in]  capabilities - The new capabilities to change to.
        ///     These will overwrite any old capabilities.
        ///     If not valid for this graphics device, then no changes will occur.
        virtual void ChangeCapabilities(const GraphicsDeviceType capabilities) = 0;

        // RESOURCE ALLOCATION.
        /// Loads the specified object into the graphics device, if needed.
        /// @param[in,out]  object_3D - The object to load into the graphics device.  It may be updated with device-specific information.
        virtual void Load(GRAPHICS::Object3D& object_3D) = 0;

        // RENDERING.
        /// Clears the background on the graphics device to the specified color.
        /// @param[in]  color - The color to clear the background to.
        virtual void ClearBackground(const GRAPHICS::Color& color) = 0;
        /// Renders the specified object using the graphics device.
        /// @param[in]  object_3D - The object to render.
        /// @param[in]  camera - The camera to use for viewing.
        /// @param[in]  cull_backfaces - True if backface culling should occur; false if not.
        /// @param[in]  depth_buffering - True if depth buffering should be used; false if not.
        virtual void Render(
            const GRAPHICS::Object3D& object_3D, 
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) = 0;
        /// Renders the specified scene using the graphics device.
        /// @param[in]  scene - The scene to render.
        /// @param[in]  camera - The camera to use for viewing.
        /// @param[in]  cull_backfaces - True if backface culling should occur; false if not.
        /// @param[in]  depth_buffering - True if depth buffering should be used; false if not.
        virtual void Render(
            const GRAPHICS::Scene& scene,
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) = 0;
        /// Displays the rendered image from the graphics device.
        /// @param[in,out]  window - The window in which to display the image.
        ///     Note that the image may not necessarily be displayed in the provided window.  Due to the way many graphics APIs
        ///     and operating systems typically work, the device is often connected to a window upon creation, and then the
        ///     operating system ultimately is responsible for putting the particular image in the window.
        ///     However, this parameter is provided to support graphics APIs that do not have this restriction and to provide
        ///     clarity of intent.
        virtual void DisplayRenderedImage(WINDOWING::IWindow& window) = 0;
    };
}
