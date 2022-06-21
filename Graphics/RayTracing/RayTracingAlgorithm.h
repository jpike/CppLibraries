#pragma once

#include <optional>
#include "Graphics/Color.h"
#include "Graphics/Geometry/Sphere.h"
#include "Graphics/Images/Bitmap.h"
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Graphics/RayTracing/RayTraceableShape.h"
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
        void Render(const Scene& scene, const VIEWING::Camera& camera, GRAPHICS::IMAGES::Bitmap& render_target);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// True if ambient lighting should be calculated; false otherwise.
        bool Ambient = true;
        /// True if shadows should be calculated; false otherwise.
        bool Shadows = true;
        /// True if diffuse shading should be calculated; false otherwise.
        bool Diffuse = true;
        /// True if specular shading should be calculated; false otherwise.
        bool Specular = true;
        /// True if reflections should be calculated; false otherwise.
        bool Reflections = true;
        /// The maximum number of reflections to computer (if reflections are enabled).
        /// More reflections will take longer to render an image.
        unsigned int ReflectionCount = 5;

    private:
        // PRIVATE HELPER METHODS.
        void RenderRows(
            const Scene& scene_with_world_space_objects,
            const VIEWING::Camera& camera,
            const unsigned int pixel_start_y,
            const unsigned int pixel_end_y,
            GRAPHICS::IMAGES::Bitmap& render_target) const;
        GRAPHICS::Color ComputeColor(
            const Scene& scene,
            const RayObjectIntersection& intersection,
            const unsigned int remaining_reflection_count) const;
        std::optional<RayObjectIntersection> ComputeClosestIntersection(
            const Scene& scene,
            const Ray& ray,
            const RayTraceableShape& ignored_object = {}) const;
    };
}
