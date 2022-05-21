#pragma once

#if _WIN32

#include <optional>
#include <vector>
#include "Graphics/DepthBuffer.h"
#include "Graphics/Gui/Text.h"
#include "Graphics/Images/Bitmap.h"
#include "Graphics/Lighting/Light.h"
#include "Graphics/Scene.h"
#include "Graphics/ScreenSpaceTriangle.h"
#include "Graphics/Viewing/Camera.h"

namespace GRAPHICS::CPU_RENDERING
{
    /// A software (non-graphics hardware) rasterization algorithm
    /// (http://en.wikipedia.org/wiki/Rasterisation) for rendering.
    /// @todo   Alpha blending
    class CpuRasterizationAlgorithm
    {
    public:
        static void Render(const GUI::Text& text, IMAGES::Bitmap& render_target);

        static void Render(
            const Scene& scene, 
            const VIEWING::Camera& camera, 
            const bool cull_backfaces, 
            IMAGES::Bitmap& output_bitmap,
            DepthBuffer* depth_buffer);
        static void Render(
            const Object3D& object_3D, 
            const std::optional<std::vector<LIGHTING::Light>>& lights, 
            const VIEWING::Camera& camera,
            const bool cull_backfaces, 
            IMAGES::Bitmap& output_bitmap,
            DepthBuffer* depth_buffer);

        static Triangle TransformLocalToWorld(const Triangle& local_triangle, const MATH::Matrix4x4f& world_transform);

        static void Render(
            const ScreenSpaceTriangle& triangle, 
            IMAGES::Bitmap& render_target,
            DepthBuffer* depth_buffer);

        static void DrawLine(
            const MATH::Vector3f& start_vertex,
            const MATH::Vector3f& end_vertex,
            const Color& color,
            IMAGES::Bitmap& render_target,
            DepthBuffer* depth_buffer);
        static void DrawLineWithInterpolatedColor(
            const MATH::Vector3f& start_vertex,
            const MATH::Vector3f& end_vertex,
            const Color& start_color,
            const Color& end_color,
            IMAGES::Bitmap& render_target,
            DepthBuffer* depth_buffer);
    };
}

#endif
