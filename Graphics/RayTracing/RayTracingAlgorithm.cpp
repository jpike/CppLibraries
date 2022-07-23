#include <algorithm>
#include <cmath>
#include <future>
#include <thread>
#include <vector>
#include "Graphics/Mesh.h"
#include "Graphics/RayTracing/RayTracingAlgorithm.h"
#include "Graphics/Shading/Lighting/Lighting.h"
#include "Graphics/TextureMappingAlgorithm.h"
#include "Math/Angle.h"

namespace GRAPHICS::RAY_TRACING
{
    /// Renders a scene to the specified render target.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @param[in,out]  render_target - The target to render to.
    void RayTracingAlgorithm::Render(const Scene& scene, const RenderingSettings& rendering_settings, GRAPHICS::IMAGES::Bitmap& render_target)
    {
        // TRANSFORM OBJECTS IN THE SCENE INTO WORLD SPACE.
        Scene scene_with_world_space_objects;
        scene_with_world_space_objects.BackgroundColor = scene.BackgroundColor;
        scene_with_world_space_objects.Lights = scene.Lights;
        for (const Object3D& untransformed_object : scene.Objects)
        {
            // INITIALIZE THE TRANSFORMED VERSION OF THE OBJECT.
            Object3D transformed_object;
            transformed_object.Spheres = untransformed_object.Spheres;
            transformed_object.Scale = untransformed_object.Scale;
            transformed_object.WorldPosition = untransformed_object.WorldPosition;
            transformed_object.RotationInRadians = untransformed_object.RotationInRadians;

            // TRANSFORM ALL MESHES IN THE OBJECT.
            MATH::Matrix4x4f world_transform = untransformed_object.WorldTransform();
            for (const auto& [mesh_name, untransformed_mesh] : untransformed_object.Model.MeshesByName)
            {
                // CREATE AN EMPTY MESH TO BE POPULATED WITH TRANSFORMED INFORMATION.
                Mesh transformed_mesh = { .Name = mesh_name };

                // TRANSFORM ALL TRIANGLES IN THE MESH.
                for (const GEOMETRY::Triangle& untransformed_triangle : untransformed_mesh.Triangles)
                {
                    // INITIALIZE THE TRANSFORMED VERSION OF THE TRIANGLE.
                    GEOMETRY::Triangle transformed_triangle;
                    transformed_triangle.Material = untransformed_triangle.Material;

                    // TRANSFORM EACH VERTEX OF THE TRIANGLE.
                    for (std::size_t vertex_index = 0; vertex_index < untransformed_triangle.Vertices.size(); ++vertex_index)
                    {
                        const VertexWithAttributes& untransformed_vertex = untransformed_triangle.Vertices[vertex_index];
                        MATH::Vector4f homogeneous_vertex = MATH::Vector4f::HomogeneousPositionVector(untransformed_vertex.Position);
                        MATH::Vector4f transformed_vertex = world_transform * homogeneous_vertex;
                        // Other non-positional attributes of the vertex need to be preserved at this stage.
                        transformed_triangle.Vertices[vertex_index] = untransformed_vertex;
                        transformed_triangle.Vertices[vertex_index].Position = MATH::Vector3f(transformed_vertex.X, transformed_vertex.Y, transformed_vertex.Z);
                    }

                    // STORE THE TRANSFORMED TRIANGLE.
                    transformed_mesh.Triangles.push_back(transformed_triangle);
                }

                // STORE THE TRANSFORMED MESH.
                transformed_object.Model.MeshesByName[mesh_name] = transformed_mesh;
            }

            // STORE THE TRANSFORMED OBJECT.
            scene_with_world_space_objects.Objects.push_back(transformed_object);
        }

        // COMPUTE HOW TO DIVIDE UP RENDERING OF PIXELS ACROSS MULTIPLE THREADS.
        unsigned int cpu_count = std::thread::hardware_concurrency();

        unsigned int render_target_height_in_pixels = render_target.GetHeightInPixels();
        unsigned int rows_per_thread = (render_target_height_in_pixels / cpu_count);
        unsigned int remaining_rows_per_last_thread = (render_target_height_in_pixels % cpu_count);

        // RENDER MULTIPLE ROWS OF PIXELS ACROSS MULTIPLE THREADS.
        std::vector<std::future<void>> ray_tracing_threads;
        for (unsigned int pixel_start_y = 0; pixel_start_y < render_target_height_in_pixels; pixel_start_y += rows_per_thread)
        {
            // DETERMINE THE ENDPOINT OF RENDERING FOR THE CURRENT THREAD.
            unsigned int pixel_end_y = pixel_start_y + rows_per_thread;
            std::size_t current_thread_count = ray_tracing_threads.size();
            bool last_ray_tracing_thread = (current_thread_count == (cpu_count - 1));
            if (last_ray_tracing_thread)
            {
                pixel_end_y += remaining_rows_per_last_thread;
            }

            // START RENDERING THE CURRENT BLOCK OF ROWS IN A NEW THREAD.
            std::future<void> ray_tracing_thread = std::async(
                std::launch::async,
                [&scene_with_world_space_objects, &rendering_settings, pixel_start_y, pixel_end_y, &render_target]()
                {
                    RayTracingAlgorithm::RenderRows(
                        scene_with_world_space_objects,
                        rendering_settings,
                        pixel_start_y,
                        pixel_end_y,
                        render_target);
                });
            ray_tracing_threads.emplace_back(std::move(ray_tracing_thread));
        }

        // WAIT FOR EACH RAY TRACING THREAD TO FINISH.
        for (auto& ray_tracing_thread : ray_tracing_threads)
        {
            ray_tracing_thread.wait();
        }
    }

    /// Renders rows of pixels for a scene using ray tracing.
    /// @param[in]  scene_with_world_space_objects - The scene with world space objects to render.
    /// @param[in]  rendering_settings - General rendering settings to use.
    /// @param[in]  pixel_start_y - The starting y coordinate of the rows to render.
    /// @param[in]  pixel_end_y - The ending y coordinate of the rows to render.
    /// @param[in,out]  render_target - The target to render to.
    void RayTracingAlgorithm::RenderRows(
        const Scene& scene_with_world_space_objects,
        const RenderingSettings& rendering_settings,
        const unsigned int pixel_start_y,
        const unsigned int pixel_end_y,
        GRAPHICS::IMAGES::Bitmap& render_target)
    {
        // RENDER EACH ROW OF PIXELS IN THE RANGE.
        for (unsigned int y = pixel_start_y; y <= pixel_end_y; ++y)
        {
            // RENDER EACH COLUMN IN THE CURRENT ROW.
            unsigned int render_target_width_in_pixels = render_target.GetWidthInPixels();
            for (unsigned int x = 0; x < render_target_width_in_pixels; ++x)
            {
                // COMPUTE THE VIEWING RAY.
                MATH::Vector2ui pixel_coordinates(x, y);
                Ray ray = rendering_settings.Camera.ViewingRay(pixel_coordinates, render_target);

                // FIND THE CLOSEST OBJECT IN THE SCENE THAT THE RAY INTERSECTS.
                std::optional<RayObjectIntersection> closest_intersection = ComputeClosestIntersection(scene_with_world_space_objects, ray);

                // COLOR THE CURRENT PIXEL.
                if (closest_intersection)
                {
                    // COMPUTE THE CURRENT PIXEL'S COLOR.
                    Color color = ComputeColor(scene_with_world_space_objects, *closest_intersection, rendering_settings, rendering_settings.MaxReflectionCount);
                    render_target.WritePixel(x, y, color);
                }
                else
                {
                    // FILL THE PIXEL WITH THE BACKGROUND COLOR.
                    render_target.WritePixel(x, y, scene_with_world_space_objects.BackgroundColor);
                }
            }
        }
    }

    /// Computes the closest intersection in the scene of a specific ray.
    /// @param[in]  scene - The scene in which to search for intersections.
    /// @param[in]  ray - The ray to use for searching for intersections.
    /// @param[in]  ignored_object - An optional object to be ignored.  If provided,
    ///     this object will be ignored for intersections.  This provides an easy way
    ///     to calculate intersections from reflected rays without having the object
    ///     being reflected off of infinitely intersected with.
    /// @return The closest intersection, if one was found; unpopulated if no intersection
    ///     was found between the ray and an object in the scene.
    std::optional<RayObjectIntersection> RayTracingAlgorithm::ComputeClosestIntersection(
        const Scene& scene,
        const Ray& ray,
        const Surface& ignored_object)
    {
        // FIND THE CLOSEST OBJECT IN THE SCENE THAT THE RAY INTERSECTS.
        std::optional<RayObjectIntersection> closest_intersection = std::nullopt;
        for (const auto& current_object : scene.Objects)
        {
            // SEARCH FOR INTERSECTIONS IN SPHERES.
            for (const auto& current_sphere : current_object.Spheres)
            {
                // SKIP OVER THE CURRENT OBJECT IF IT SHOULD BE IGNORED.
                const GEOMETRY::Sphere* const* ignored_sphere = std::get_if<const GEOMETRY::Sphere*>(&ignored_object.Shape);
                if (ignored_sphere)
                {
                    bool ignore_current_object = ((*ignored_sphere) == &current_sphere);
                    if (ignore_current_object)
                    {
                        continue;
                    }
                }

                // CHECK IF THE RAY INTERSECTS THE CURRENT OBJECT.
                std::optional<RayObjectIntersection> intersection = current_sphere.Intersect(ray);
                bool ray_hit_object = (std::nullopt != intersection);
                if (!ray_hit_object)
                {
                    // CONTINUE SEEING IF OTHER OBJECTS ARE HIT.
                    continue;
                }

                // UPDATE THE CLOSEST INTERSECTION APPROPRIATELY.
                if (closest_intersection)
                {
                    // ONLY OVERWRITE THE CLOSEST INTERSECTION IF THE NEWEST ONE IS CLOSER.
                    bool new_intersection_closer = (intersection->DistanceFromRayToObject < closest_intersection->DistanceFromRayToObject);
                    if (new_intersection_closer)
                    {
                        closest_intersection = intersection;
                    }
                }
                else
                {
                    // SET THIS FIRST INTERSECTION AS THE CLOSEST.
                    closest_intersection = intersection;
                }
            }

            // SEARCH FOR INTERSECTIONS IN ALL MESHES OF THE OBJECT.
            for (const auto& [mesh_name, mesh] : current_object.Model.MeshesByName)
            {
                // SEARCH FOR INTERSECTIONS IN ALL TRIANGLES OF THE CURRENT MESH.
                /// @todo   Convert things to operate on triangles as opposed to "objects"?
                for (const auto& current_triangle : mesh.Triangles)
                {
                    // SKIP OVER THE CURRENT OBJECT IF IT SHOULD BE IGNORED.
                    const GEOMETRY::Triangle* const* ignored_triangle = std::get_if<const GEOMETRY::Triangle*>(&ignored_object.Shape);
                    if (ignored_triangle)
                    {
                        bool ignore_current_object = ((*ignored_triangle) == &current_triangle);
                        if (ignore_current_object)
                        {
                            continue;
                        }
                    }

                    // CHECK IF THE RAY INTERSECTS THE CURRENT OBJECT.
                    std::optional<RayObjectIntersection> intersection = current_triangle.Intersect(ray);
                    bool ray_hit_object = (std::nullopt != intersection);
                    if (!ray_hit_object)
                    {
                        // CONTINUE SEEING IF OTHER OBJECTS ARE HIT.
                        continue;
                    }

                    // UPDATE THE CLOSEST INTERSECTION APPROPRIATELY.
                    if (closest_intersection)
                    {
                        // ONLY OVERWRITE THE CLOSEST INTERSECTION IF THE NEWEST ONE IS CLOSER.
                        bool new_intersection_closer = (intersection->DistanceFromRayToObject < closest_intersection->DistanceFromRayToObject);
                        if (new_intersection_closer)
                        {
                            closest_intersection = intersection;
                        }
                    }
                    else
                    {
                        // SET THIS FIRST INTERSECTION AS THE CLOSEST.
                        closest_intersection = intersection;
                    }
                }
            }
        }

        return closest_intersection;
    }

    /// Computes color based on the specified intersection in the scene.
    /// @param[in]  scene - The scene in which the color is being computed.
    /// @param[in]  intersection - The intersection for which to compute the color.
    /// @param[in]  rendering_settings - Settings to use for rendering.
    /// @param[in]  remaining_reflection_count - The remaining reflection depth for color computation.
    ///     To compute more accurate light, rays need to be reflected, but we don't want this to go on forever.
    ///     Furthermore, more rays can be computationally expensive for little more gain, which is why 
    ///     the amount of reflection is capped.
    /// @return The computed color.
    GRAPHICS::Color RayTracingAlgorithm::ComputeColor(
        const Scene& scene, 
        const RayObjectIntersection& intersection,
        const RenderingSettings& rendering_settings,
        const unsigned int remaining_reflection_count)
    {
        // INITIALIZE THE COLOR TO HAVE NO CONTRIBUTION FROM ANY SOURCES.
        Color final_color = Color::BLACK;

        // ADD IN TEXTURE COLORS IF ENABLED.
        std::shared_ptr<Material> intersected_material = intersection.Object.GetMaterial();
        MATH::Vector3f intersection_point = intersection.IntersectionPoint();
        if (rendering_settings.TextureMapping)
        {
            /// @todo   How to we know whether texture is for diffuse or not?
            /// @todo   Color base_diffuse_color = intersected_material->DiffuseColor;
            /// @todo   Handle spheres versus triangles.
            const GEOMETRY::Triangle* const* intersected_triangle = std::get_if<const GEOMETRY::Triangle*>(&intersection.Object.Shape);
            if (intersected_material->DiffuseProperties.Texture && intersected_triangle)
            {
                MATH::Vector2f current_point(intersection_point.X, intersection_point.Y);
                Color texture_color = TextureMappingAlgorithm::LookupTexel(
                    **intersected_triangle,
                    current_point,
                    *intersected_material->DiffuseProperties.Texture);
                final_color += texture_color;
            }
        }

        // ADD IN ANY LIGHTING IF ENABLED.
        if (rendering_settings.LightingSettings.Enabled)
        {
            // COMPUTE SHADOWING FACTORS IF ENABLED.
            std::vector<float> shadow_factors_by_light_index;
            if (rendering_settings.LightingSettings.ShadowsEnabled)
            {
                for (const SHADING::LIGHTING::Light& light : scene.Lights)
                {
                    // CAST A RAY OUT TO COMPUTE SHADOWS IF ENABLED.
                    // To simplify later parts of the algorithm, a shadow factor of 1 (no shadowing) should always be computed.
                    constexpr float NO_SHADOWING = 1.0f;
                    float shadow_factor = NO_SHADOWING;

                    // The direction may need to be computed differently based on the type of light.
                    MATH::Vector3f direction_from_point_to_light;
                    if (SHADING::LIGHTING::LightType::DIRECTIONAL == light.Type)
                    {
                        // The computations are based on the opposite direction.
                        direction_from_point_to_light = MATH::Vector3f::Scale(-1.0f, light.DirectionalLightDirection);
                    }
                    else if (SHADING::LIGHTING::LightType::POINT == light.Type)
                    {
                        direction_from_point_to_light = light.PointLightWorldPosition - intersection_point;
                    }

                    // SHOOT A SHADOW RAY OUT FROM THE INTERSECTION POINT TO THE LIGHT.
                    Ray shadow_ray(intersection_point, direction_from_point_to_light);
                    std::optional<RayObjectIntersection> shadow_intersection = ComputeClosestIntersection(scene, shadow_ray, intersection.Object);
                    if (shadow_intersection)
                    {
                        // DETERMINE THE SHADOW FACTOR BASED ON THE INTERSECTION.
                        // For a shadow to occur, the intersection with another object must occur in front of the shadow ray.
                        // Similarly, the intersection must occur before the ray hits the light (hence why the shadow ray
                        // is computed with a direction that is not unit length but the full length from the intersection
                        // point to the light - it makes checking for the distance to the light easier).
                        constexpr float NO_DISTANCE_IN_FRONT_OF_SHADOW_RAY = 0.0f;
                        constexpr float DISTANCE_AT_LIGHT = 1.0f;
                        bool shadow_intersection_in_range = (
                            (NO_DISTANCE_IN_FRONT_OF_SHADOW_RAY < shadow_intersection->DistanceFromRayToObject) &&
                            (shadow_intersection->DistanceFromRayToObject < DISTANCE_AT_LIGHT));
                        if (shadow_intersection_in_range)
                        {
                            constexpr float FULL_SHADOWING = 0.0f;
                            shadow_factor = FULL_SHADOWING;
                        }
                        else
                        {
                            shadow_factor = NO_SHADOWING;
                        }
                    }

                    // STORE THE SHADOW FACTOR FOR THE LIGHT.
                    shadow_factors_by_light_index.push_back(shadow_factor);
                }
            }

            // ADD IN LIGHTING.
            Color light_color = GRAPHICS::SHADING::LIGHTING::Lighting::Compute(
                intersection.Ray->Origin,
                scene.Lights,
                intersection.Object,
                intersection_point,
                rendering_settings.LightingSettings,
                shadow_factors_by_light_index);
            final_color += light_color;
        }

        // COMPUTE REFLECTED LIGHT IF POSSIBLE.
        if (rendering_settings.Reflections)
        {
            // CHECK IF THE RAY CAN BE REFLECTED.
            // In addition to the remaining reflections, there's no need to compute
            // color from reflected light in the material isn't reflective.
            bool ray_can_be_reflected = (remaining_reflection_count > 0) && (intersected_material->ReflectivityProportion > 0.0f);
            if (!ray_can_be_reflected)
            {
                // RETURN THE COLOR AS-IS.
                return final_color;
            }

            // COMPUTE THE REFLECTED RAY.
            MATH::Vector3f direction_from_ray_origin_to_intersection = intersection_point - intersection.Ray->Origin;
            MATH::Vector3f normalized_direction_from_ray_origin_to_intersection = MATH::Vector3f::Normalize(direction_from_ray_origin_to_intersection);
            MATH::Vector3f unit_surface_normal = intersection.Object.GetNormal(intersection_point);
            float length_of_ray_along_surface_normal = MATH::Vector3f::DotProduct(normalized_direction_from_ray_origin_to_intersection, unit_surface_normal);
            float twice_length_of_ray_along_surface_normal = 2.0f * length_of_ray_along_surface_normal;
            MATH::Vector3f twice_reflected_ray_along_surface_normal = MATH::Vector3f::Scale(twice_length_of_ray_along_surface_normal, unit_surface_normal);
            MATH::Vector3f reflected_ray_direction = normalized_direction_from_ray_origin_to_intersection - twice_reflected_ray_along_surface_normal;
            MATH::Vector3f normalized_reflected_ray_direction = MATH::Vector3f::Normalize(reflected_ray_direction);
            Ray reflected_ray(intersection_point, normalized_reflected_ray_direction);

            // CHECK FOR ANY INTERSECTIONS FROM THE REFLECTED RAY.
            std::optional<RayObjectIntersection> reflected_intersection = ComputeClosestIntersection(scene, reflected_ray, intersection.Object);
            if (reflected_intersection)
            {
                // COMPUTE THE REFLECTED COLOR.
                const unsigned int child_reflection_count = remaining_reflection_count - 1;
                Color raw_reflected_color = ComputeColor(scene, *reflected_intersection, rendering_settings, child_reflection_count);
                Color reflected_color = Color::ScaleRedGreenBlue(intersected_material->ReflectivityProportion, raw_reflected_color);
                final_color += reflected_color;
            }
            else
            {
                // ADD REFLECTED LIGHT CONTRIBUTED FROM THE BACKGROUND.
                Color reflected_color = Color::ScaleRedGreenBlue(intersected_material->ReflectivityProportion, scene.BackgroundColor);
                final_color += reflected_color;
            }
        }

        return final_color;
    }
}
