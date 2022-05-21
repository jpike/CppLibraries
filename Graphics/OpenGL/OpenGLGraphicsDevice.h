#pragma once

#include <memory>
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/OpenGL/ShaderProgram.h"
#include "Windowing/IWindow.h"

/// Holds code related to the OpenGL graphics library.
namespace GRAPHICS::OPEN_GL
{
    /// A graphics device that can be interacted with using the OpenGL library.
    class OpenGLGraphicsDevice : public GRAPHICS::HARDWARE::IGraphicsDevice
    {
    public:
        // CREATION/SHUTDOWN.
        static std::unique_ptr<OpenGLGraphicsDevice> ConnectTo(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~OpenGLGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType Type() const override;
        void ChangeCapabilities(const GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType capabilities) override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(const GRAPHICS::Object3D& object_3D, const GRAPHICS::VIEWING::Camera& camera) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The capabilities of this device.
        GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType GraphicsDeviceCapabilities = static_cast<GRAPHICS::HARDWARE::IGraphicsDevice::GraphicsDeviceType>(
            GRAPHICS::HARDWARE::IGraphicsDevice::GPU | GRAPHICS::HARDWARE::IGraphicsDevice::OPEN_GL);
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The regular Windows device context.
        HDC WindowDeviceContext = nullptr;
        /// The OpenGL rendering context.
        HGLRC OpenGLRenderContext = nullptr;
        /// The default shader program.
        std::shared_ptr<ShaderProgram> ShaderProgram = nullptr;
    };
}
