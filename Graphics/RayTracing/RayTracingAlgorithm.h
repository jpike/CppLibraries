#pragma once

#include <optional>
#include "Graphics/Color.h"
#include "Graphics/Geometry/Sphere.h"
#include "Graphics/Images/Bitmap.h"
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Graphics/RayTracing/Surface.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Scene.h"
#include "Graphics/Viewing/Camera.h"

/// Holds code related to ray tracing.
namespace GRAPHICS::RAY_TRACING
{
    /// A basic ray tracing algorithm.
    class RayTracingAlgorithm
    {
    public:
        // PUBLIC METHODS.
        void Render(const Scene& scene, const RenderingSettings& rendering_settings, GRAPHICS::IMAGES::Bitmap& render_target);

    private:
        // PRIVATE HELPER METHODS.
        void RenderRows(
            const Scene& scene_with_world_space_objects,
            const RenderingSettings& rendering_settings,
            const unsigned int pixel_start_y,
            const unsigned int pixel_end_y,
            GRAPHICS::IMAGES::Bitmap& render_target) const;
        GRAPHICS::Color ComputeColor(
            const Scene& scene,
            const RayObjectIntersection& intersection,
            const RenderingSettings& rendering_settings,
            const unsigned int remaining_reflection_count) const;
        std::optional<RayObjectIntersection> ComputeClosestIntersection(
            const Scene& scene,
            const Ray& ray,
            const Surface& ignored_object = {}) const;
    };
}
