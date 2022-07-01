#pragma once

#include <memory>
#include "Graphics/Color.h"
#include "Graphics/Hardware/GraphicsDeviceType.h"
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

        // RESOURCE ALLOCATION.
        /// Loads the specified object into the graphics device, if needed.
        /// @param[in,out]  object_3D - The object to load into the graphics device.  It may be updated with device-specific information.
        virtual void Load(GRAPHICS::Object3D& object_3D) = 0;

        // RENDERING.
        /// Clears the background on the graphics device to the specified color.
        /// @param[in]  color - The color to clear the background to.
        virtual void ClearBackground(const GRAPHICS::Color& color) = 0;
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
