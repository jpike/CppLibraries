#pragma once

#include <optional>
#include "Graphics/Color.h"
#include "Graphics/Images/Bitmap.h"
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Scene.h"
#include "Graphics/Surface.h"
#include "Graphics/Viewing/Camera.h"

/// Holds code related to ray tracing.
namespace GRAPHICS::RAY_TRACING
{
    /// A basic ray tracing algorithm.
    class RayTracingAlgorithm
    {
    public:
        // MAIN RENDERING METHOD.
        static void Render(const Scene& scene, const RenderingSettings& rendering_settings, GRAPHICS::IMAGES::Bitmap& render_target);

        // RENDERING PARALLELIZATION HELPER METHOD.
        static void RenderRows(
            const Scene& scene_with_world_space_objects,
            const RenderingSettings& rendering_settings,
            const unsigned int pixel_start_y,
            const unsigned int pixel_end_y,
            GRAPHICS::IMAGES::Bitmap& render_target);

        // OBJECT INTERSECTION.
        static std::optional<RayObjectIntersection> ComputeClosestIntersection(
            const Scene& scene,
            const Ray& ray,
            const Surface& ignored_object = {});

        // SHADOWING.
        static std::vector<float> ComputeShadowFactors(
            const Scene& scene,
            const RayObjectIntersection& intersection);

        // COLOR COMPUTATION.
        static GRAPHICS::Color ComputeColor(
            const Scene& scene,
            const RayObjectIntersection& intersection,
            const RenderingSettings& rendering_settings,
            const unsigned int remaining_reflection_count);
    };
}
