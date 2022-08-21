#include <sstream>
#include <string>
#include "Graphics/OpenGL/OpenGL.h"

namespace GRAPHICS::OPEN_GL
{
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    /// The callback function for OpenGL to call for debug messages.
    /// Prints the debug message for visibility.
    /// @param[in]  source - The source of the message.
    /// @param[in]  type - The type of the message.
    /// @param[in]  id - The ID of the message.
    /// @param[in]  severity - The severity of the message.
    /// @param[in]  length_in_characters - The length of the message in characters.
    /// @param[in]  message - The actual message.
    /// @param[in]  user_parameter - Any user parameter.
    void OpenGLDebugMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        void* user_parameter)
    {
        // PROVIDE MAXIMUM DEBUG VISIBILITY.
        std::stringstream debug_message;
        debug_message
            << "\nOpenGL debug:"
            << "\tsource = " << source
            << "\ttype = " << type
            << "\tid = " << id
            << "\tseverity = " << severity
            << "\tlength = " << length
            << "\tmessage = " << message
            << "\tuser_parameter = " << user_parameter
            << "\n";
        OutputDebugString(debug_message.str().c_str());
    }

    /// Attempts to load all necessary OpenGL functions.
    /// @return True if loading succeeds; false otherwise.
    bool LoadOpenGLFunctions()
    {
        // LOAD OPEN GL FUNCTIONS.
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        int regular_open_gl_loading_return_code = gl3wInit();

        // CHECK IF LOADING SUCCEEDED.
        bool loading_succeeded = (
            wglChoosePixelFormatARB &&
            wglCreateContextAttribsARB &&
            (0 == regular_open_gl_loading_return_code));
        return loading_succeeded;
    }

    /// Initializes OpenGL by loading the necessary functions.
    /// @param[in] device_context - The device context for which OpenGL should be initialized.
    /// @return True if initialization succeeds; false otherwise.
    bool Initialize(const HDC device_context)
    {
        // SET THE PIXEL FORMAT.
        PIXELFORMATDESCRIPTOR pixel_format = {};
        pixel_format.nSize = sizeof(pixel_format);
        const WORD REQUIRED_VERSION = 1;
        pixel_format.nVersion = REQUIRED_VERSION;
        pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixel_format.iPixelType = PFD_TYPE_RGBA;
        pixel_format.cColorBits = 32;
        pixel_format.cDepthBits = 24;
        pixel_format.cStencilBits = 8;

        const int FAILED_TO_FIND_PIXEL_FORMAT = 0;
        int temp_pixel_format_index = ChoosePixelFormat(device_context, &pixel_format);
        bool pixel_format_chosen = (FAILED_TO_FIND_PIXEL_FORMAT != temp_pixel_format_index);
        if (!pixel_format_chosen)
        {
            return false;
        }

        BOOL pixel_format_set = SetPixelFormat(device_context, temp_pixel_format_index, &pixel_format);
        if (!pixel_format_set)
        {
            return false;
        }

        // CREATE A TEMPORARY OPEN GL RENDERING CONTEXT.
        // This is necessary in order to load the OpenGL functions.
        HGLRC temp_open_gl_render_context = wglCreateContext(device_context);
        bool temp_open_gl_render_context_created = (NULL != temp_open_gl_render_context);
        if (!temp_open_gl_render_context_created)
        {
            return false;
        }

        BOOL open_gl_context_made_current = wglMakeCurrent(device_context, temp_open_gl_render_context);
        if (!open_gl_context_made_current)
        {
            return false;
        }

        // LOAD THE OPEN GL FUNCTIONS.
        bool open_gl_functions_loaded = LoadOpenGLFunctions();
        if (!open_gl_functions_loaded)
        {
            return false;
        }

        // CONFIGURE DEBUG LOGGING.
              // Only done in debug builds to avoid potential performance penalties during release builds.
#if _DEBUG
        constexpr void* NO_DEBUG_USER_DATA = nullptr;
        glDebugMessageCallback(OpenGLDebugMessageCallback, NO_DEBUG_USER_DATA);
        constexpr GLenum NO_SOURCE_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLenum NO_TYPE_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLenum NO_SEVERITY_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLsizei NO_ID_DEBUG_MESSAGES_TO_FILTER_COUNT = 0;
        constexpr GLuint* NO_ID_DEBUG_MESSAGE_FILTERING = nullptr;
        constexpr GLboolean ENABLE_DEBUG_MESSAGE = GL_TRUE;
        glDebugMessageControl(
            NO_SOURCE_DEBUG_MESSAGE_FILTERIING,
            NO_TYPE_DEBUG_MESSAGE_FILTERIING,
            NO_SEVERITY_DEBUG_MESSAGE_FILTERIING,
            NO_ID_DEBUG_MESSAGES_TO_FILTER_COUNT,
            NO_ID_DEBUG_MESSAGE_FILTERING,
            ENABLE_DEBUG_MESSAGE);
#endif

        // SET THE PIXEL FORMAT.
        const int pixel_format_attribute_list[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            ATTRIBUTE_LIST_TERMINATOR
        };
        const float* const NO_FLOATING_POINT_PIXEL_FORMAT_ATTRIBUTES = nullptr;
        const unsigned int MAX_PIXEL_FORMATS_TO_CHOOSE = 1;
        int pixel_format_index = 0;
        UINT pixel_format_count = 0;

        BOOL gl_pixel_format_chosen = wglChoosePixelFormatARB(
            device_context,
            pixel_format_attribute_list,
            NO_FLOATING_POINT_PIXEL_FORMAT_ATTRIBUTES,
            MAX_PIXEL_FORMATS_TO_CHOOSE,
            &pixel_format_index,
            &pixel_format_count);
        if (!gl_pixel_format_chosen)
        {
            return false;
        }
        SetPixelFormat(device_context, pixel_format_index, &pixel_format);

        // DELETE THE TEMPORARY OPEN GL RENDERING CONTEXT.
        wglDeleteContext(temp_open_gl_render_context);

        return true;
    }
}
