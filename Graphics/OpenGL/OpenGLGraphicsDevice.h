#pragma once

#include <optional>
#include <SDL/SDL.h>
#include "Windowing/SdlWindow.h"

/// Holds code related to the OpenGL graphics library.
namespace GRAPHICS::OPEN_GL
{
    /// A graphics device that can be interacted with using the OpenGL library.
    class OpenGLGraphicsDevice
    {
    public:
        static std::optional<OpenGLGraphicsDevice> Initialize(const WINDOWING::SdlWindow& window);

        void Clear();

        void Display(const WINDOWING::SdlWindow& window);

        void Shutdown();

        /// The OpenGL context associated with the graphics device.
        SDL_GLContext Context = nullptr;
    };
}
