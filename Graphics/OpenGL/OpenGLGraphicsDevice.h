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
        GRAPHICS::HARDWARE::GraphicsDeviceType Type() const override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const GRAPHICS::VIEWING::Camera& camera,
            const bool cull_backfaces,
            const bool depth_buffering) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
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
