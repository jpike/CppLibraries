#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "ErrorHandling/Asserts.h"
#include "Graphics/Modeling/WavefrontFaceVertexAttributeIndices.h"
#include "Graphics/Modeling/WavefrontObjectModel.h"
#include "Graphics/Modeling/WavefrontMaterial.h"
#include "String/String.hpp"

namespace GRAPHICS::MODELING
{
    /// Attempts to load the model from the specified .obj file.
    /// Any additional referenced files are automatically loaded to ensure a complete model is loaded.
    /// @param[in]  obj_filepath - The path of the .obj file to load.
    /// @return The 3D model, if successfull loaded; null otherwise.
    std::optional<GRAPHICS::MODELING::Model> WavefrontObjectModel::Load(const std::filesystem::path& filepath)
    {
        // OPEN THE FILE.
        std::ifstream obj_file(filepath);
        bool obj_file_opened = obj_file.is_open();
        if (!obj_file_opened)
        {
            return std::nullopt;
        }

        // READ IN THE DATA FROM THE .OBJ FILE.
        // Note that this reading may not yet be fully robust.
        // It only handles the absolute minimum as currently needed for basic demos.
        std::optional<GRAPHICS::MODELING::Model> model = std::nullopt;
        // The file format operates on the notion of various data applying to the current mesh/material.
        std::optional<GRAPHICS::Mesh> current_mesh = std::nullopt;
#if TODO_DEFAULT_MATERIAL
        // A default material is used for rendering for cases where a file may not yet have specified a material.
        // The exact defaults need to be determined, but for now, a basic white material is chosen.
        std::shared_ptr<GRAPHICS::Material> current_material = std::make_shared<GRAPHICS::Material>();
        current_material->Shading = GRAPHICS::ShadingType::FLAT;
        current_material->AmbientColor = GRAPHICS::Color::WHITE;
        current_material->DiffuseColor = GRAPHICS::Color::WHITE;
#else
        std::shared_ptr<GRAPHICS::Material> current_material = nullptr;
#endif
        // Multiple material library files may be included in the file.
        std::unordered_map<std::string, MEMORY::NonNullSharedPointer<GRAPHICS::Material>> current_materials;
        std::vector<MATH::Vector3f> vertex_positions;
        std::vector<GRAPHICS::Color> vertex_colors;
        std::vector<MATH::Vector2f> vertex_texture_coordinates;
        std::vector<MATH::Vector3f> vertex_normals;
        std::string line;
        while (std::getline(obj_file, line))
        {
            // SKIP OVER ANY BLANK LINES.
            bool is_blank_line = line.empty();
            if (is_blank_line)
            {
                continue;
            }

            // SKIP OVER ANY COMMENT LINES.
            constexpr char OBJ_COMMENT_CHARACTER = '#';
            bool is_comment_line = line.starts_with(OBJ_COMMENT_CHARACTER);
            if (is_comment_line)
            {
                continue;
            }

            // SPLIT THE LINE INTO COMPONENTS BASED ON WHITESPACE.
            // The rest of the data in the file effectively has different components in each line separated by whitespace.
            // This splitting is not done for comment checking since comments could have large numbers of spaces.
            std::vector<std::string> current_line_components = STRING::String::SplitByWhitespace(line);
            bool line_has_components = !current_line_components.empty();
            if (!line_has_components)
            {
                assert("Line without components detected for WavefrontObjectModel.");
                continue;
            }

            // PARSE ANY VERTEX DATA.
            constexpr char VERTEX_INDICATOR = 'v';
            bool is_vertex_line = line.starts_with(VERTEX_INDICATOR);
            if (is_vertex_line)
            {
                // CHECK THE TYPE OF VERTEX DATA BEING DEFINED.
                std::size_t line_actual_component_count = current_line_components.size();
                const std::string VERTEX_TEXTURE_COORDINATE_INDICATOR = "vt";
                const std::string VERTEX_NORMAL_INDICATOR = "vn";
                bool is_vertex_texture_coordinates = line.starts_with(VERTEX_TEXTURE_COORDINATE_INDICATOR);
                bool is_vertex_normal_data = line.starts_with(VERTEX_NORMAL_INDICATOR);
                if (is_vertex_texture_coordinates)
                {
                    // MAKE SURE THERE ARE ENOUGH COMPONENTS FOR TEXTURE COORDINATES.
                    // 2 components (traditionally UV) are required.
                    constexpr std::size_t INDICATOR_PLUS_VERTEX_TEXTURE_COORDINATE_COMPONENT_COUNT = 3;
                    bool enough_components_for_texture_coordinates = (line_actual_component_count >= INDICATOR_PLUS_VERTEX_TEXTURE_COORDINATE_COMPONENT_COUNT);
                    ASSERT_THEN_IF_NOT(enough_components_for_texture_coordinates)
                    {
                        // INDICATE THAT NO MODEL COULD BE SUCCESSFULLY PARSED.
                        return std::nullopt;
                    }

                    // PARSE OUT VERTEX COORDINATES.
                    // V coodinates are optional according to the spec but required by this graphics library.
                    // W coordinates are optional and not supported by this parser due to not being needed yet.
                    constexpr std::size_t U_INDEX = 1;
                    const std::string& u_string = current_line_components.at(U_INDEX);
                    float u = std::stof(u_string);

                    constexpr std::size_t V_INDEX = 2;
                    const std::string& v_string = current_line_components.at(V_INDEX);
                    // Texture coordinates are stored upside down in the file...or maybe switched (u <-> v)?
                    float v = 1.0f - std::stof(v_string);
                    /// @todo float v = std::stof(v_string);

                    vertex_texture_coordinates.emplace_back(u, v);
                }
                else if (is_vertex_normal_data)
                {
                    // MAKE SURE THERE ARE ENOUGH COMPONENTS FOR VERTEX NORMALS.
                    // XYZ coordinates are required.
                    constexpr std::size_t INDICATOR_PLUS_VERTEX_NORMAL_COMPONENT_COUNT = 4;
                    bool enough_components_for_vertex_normal = (line_actual_component_count >= INDICATOR_PLUS_VERTEX_NORMAL_COMPONENT_COUNT);
                    ASSERT_THEN_IF_NOT(enough_components_for_vertex_normal)
                    {
                        // INDICATE THAT NO MODEL COULD BE SUCCESSFULLY PARSED.
                        return std::nullopt;
                    }

                    // PARSE OUT VERTEX NORMALS.
                    constexpr std::size_t X_INDEX = 1;
                    const std::string& x_string = current_line_components.at(X_INDEX);
                    float x = std::stof(x_string);

                    constexpr std::size_t Y_INDEX = X_INDEX + 1;
                    const std::string& y_string = current_line_components.at(Y_INDEX);
                    float y = std::stof(y_string);

                    constexpr std::size_t Z_INDEX = Y_INDEX + 1;
                    const std::string& z_string = current_line_components.at(Z_INDEX);
                    float z = std::stof(z_string);

                    // Normals may not be unit vectors and thus need to be normalized first.
                    MATH::Vector3f vertex_normal(x, y, z);
                    vertex_normal = MATH::Vector3f::Normalize(vertex_normal);
                    vertex_normals.emplace_back(vertex_normal);
                }
                else
                {
                    // MAKE SURE THERE ARE ENOUGH COMPONENTS FOR A VERTEX POSITION.
                    // Vertex positions are assumed to be for a triangle/quad.
                    // Parameter-space coordinates (vp) are not currently supported.
                    // At a minimum, xyz coordinates are required after the indicator.
                    constexpr std::size_t INDICATOR_PLUS_VERTEX_POSITION_COMPONENT_COUNT = 4;
                    bool enough_components_for_vertex_position = (line_actual_component_count >= INDICATOR_PLUS_VERTEX_POSITION_COMPONENT_COUNT);
                    ASSERT_THEN_IF_NOT(enough_components_for_vertex_position)
                    {
                        // INDICATE THAT NO MODEL COULD BE SUCCESSFULLY PARSED.
                        return std::nullopt;
                    }

                    // EXTRACT THE VERTEX POSITION.
                    constexpr std::size_t X_POSITION_INDEX = 1;
                    const std::string& x_position_string = current_line_components.at(X_POSITION_INDEX);
                    float x = std::stof(x_position_string);

                    constexpr std::size_t Y_POSITION_INDEX = X_POSITION_INDEX + 1;
                    const std::string& y_position_string = current_line_components.at(Y_POSITION_INDEX);
                    float y = std::stof(y_position_string);

                    constexpr std::size_t Z_POSITION_INDEX = Y_POSITION_INDEX + 1;
                    const std::string& z_position_string = current_line_components.at(Z_POSITION_INDEX);
                    float z = std::stof(z_position_string);

                    // W coordinates are optional and not supported by this parser due to not being needed yet.
                    vertex_positions.emplace_back(x, y, z);

                    /// @todo   These are in a right-handed coordinate system, so do we need to flip z coordinates?

                    // PARSE ANY OPTIONAL COLOR COMPONENTS.
                    // [0, 1] color values may optionally be added after xyz coordinates and precludes having w coordinates.
                    // As of now, it is assumed that no alpha values exist in these files and that the alpha is always 1.
                    constexpr std::size_t COLOR_COMPONENT_COUNT = 3;
                    constexpr std::size_t COMPONENT_COUNT_FOR_VERTEX_COLORS = INDICATOR_PLUS_VERTEX_POSITION_COMPONENT_COUNT + COLOR_COMPONENT_COUNT;
                    bool enough_components_for_vertex_colors = (line_actual_component_count >= COMPONENT_COUNT_FOR_VERTEX_COLORS);
                    if (enough_components_for_vertex_colors)
                    {
                        // EXTRACT THE VERTEX COLOR.
                        constexpr std::size_t RED_INDEX = Z_POSITION_INDEX + 1;
                        const std::string& red_string = current_line_components.at(RED_INDEX);
                        float red = std::stof(red_string);

                        constexpr std::size_t GREEN_INDEX = RED_INDEX + 1;
                        const std::string& green_string = current_line_components.at(GREEN_INDEX);
                        float green = std::stof(green_string);

                        constexpr std::size_t BLUE_INDEX = GREEN_INDEX + 1;
                        const std::string& blue_string = current_line_components.at(BLUE_INDEX);
                        float blue = std::stof(blue_string);

                        vertex_colors.emplace_back(red, green, blue, GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT);
                    }
                }

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }

            // PARSE ANY FACE DATA.
            const std::string& line_first_component = current_line_components.front();
            const std::string FACE_INDICATOR = "f";
            bool is_face_line = (FACE_INDICATOR == line_first_component);
            if (is_face_line)
            {
                // PARSE VERTEX ATTRIBUTE INDICES FOR THE FACE.
                std::vector<WavefrontFaceVertexAttributeIndices> face_vertex_attribute_indices;
                constexpr std::size_t FIRST_VERTEX_INDEX = 1;
                std::size_t line_actual_component_count = current_line_components.size();
                for (std::size_t vertex_index = FIRST_VERTEX_INDEX; vertex_index < line_actual_component_count; ++vertex_index)
                {
                    // DETERMINE HOW MANY VERTEX ATTRIBUTE INDICES ARE IN THE FACE.
                    // Vertex position, texture, and normal indices are separated by slashes, and not all are required.
                    constexpr char VERTEX_ATTRIBUTE_INDEX_DELIMITER = '/';
                    const std::string& current_vertex_indices = current_line_components.at(vertex_index);
                    std::vector<std::string> vertex_attribute_indices = STRING::String::Split(current_vertex_indices, VERTEX_ATTRIBUTE_INDEX_DELIMITER);
                    bool vertex_attribute_indices_exist = !vertex_attribute_indices.empty();
                    ASSERT_THEN_IF_NOT(vertex_attribute_indices_exist)
                    {
                        // INDICATE NO MODEL COULD BE SUCCESSFULLY PARSED.
                        return std::nullopt;
                    }

                    // PARSE THE APPROPRIATE VERTEX ATTRIBUTES INDICES FROM THE LINE.
                    WavefrontFaceVertexAttributeIndices current_face_vertex_attribute_indices;
                    constexpr std::size_t ONLY_VERTEX_POSITION_INDICES_COMPONENT_COUNT = 1;
                    /// @todo   Does this properly handle missing texture coordinates and only positions + normals?
                    constexpr std::size_t VERTEX_POSITION_AND_TEXTURE_COORDINATE_INDICES_COMPONENT_COUNT = 2;
                    constexpr std::size_t VERTEX_POSITION_TEXTURE_COORDINATE_AND_NORMAL_INDICES_COMPONENT_COUNT = 3;
                    std::size_t vertex_attribute_index_count = vertex_attribute_indices.size();
                    if (vertex_attribute_index_count >= ONLY_VERTEX_POSITION_INDICES_COMPONENT_COUNT)
                    {
                        // PARSE THE VERTEX POSITION INDEX.
                        constexpr std::size_t VERTEX_POSITION_INDEX = 0;
                        const std::string& vertex_position_index_string = vertex_attribute_indices.at(VERTEX_POSITION_INDEX);
                        current_face_vertex_attribute_indices.VertexPositionIndex = std::stoul(vertex_position_index_string);
                    }
                    if (vertex_attribute_index_count >= VERTEX_POSITION_AND_TEXTURE_COORDINATE_INDICES_COMPONENT_COUNT)
                    {
                        // PARSE OUT THE TEXTURE COORDINATE INDEX.
                        const std::size_t TEXTURE_COORDINATE_INDEX = 1;
                        const std::string& texture_coordinate_index_string = vertex_attribute_indices.at(TEXTURE_COORDINATE_INDEX);
                        // Vertex texture coordinates are optional.  If vertex texture coordinates are missing, this string will be empty.
                        bool vertex_texture_coordinates_exist = !texture_coordinate_index_string.empty();
                        if (vertex_texture_coordinates_exist)
                        {
                            current_face_vertex_attribute_indices.VertexTextureCoordinateIndex = std::stoul(texture_coordinate_index_string);
                        }
                    }
                    if (vertex_attribute_index_count >= VERTEX_POSITION_TEXTURE_COORDINATE_AND_NORMAL_INDICES_COMPONENT_COUNT)
                    {
                        // PARSE OUT THE VERTEX NORMAL INDEX.
                        const std::size_t VERTEX_NORMAL_INDEX = 2;
                        const std::string& vertex_normal_index_string = vertex_attribute_indices.at(VERTEX_NORMAL_INDEX);
                        current_face_vertex_attribute_indices.VertexNormalIndex = std::stoul(vertex_normal_index_string);
                    }
                    else
                    {
                        assert("WavefrontObjectModel - Unexpected vertex attribute index count.");
                    }

                    // STORE THE FACE INFORMATION.
                    face_vertex_attribute_indices.emplace_back(current_face_vertex_attribute_indices);
                }

                // MAKE SURE THERE ARE ENOUGH VERTICES FOR A FACE.
                // Only triangles and quads supported - not more advanced geometry.
                std::size_t face_vertex_attribute_index_count = face_vertex_attribute_indices.size();
                bool enough_vertices_for_triangle = (face_vertex_attribute_index_count >= GRAPHICS::GEOMETRY::Triangle::VERTEX_COUNT);
                ASSERT_THEN_IF_NOT(enough_vertices_for_triangle)
                {
                    // INDICATE THAT NO MODEL COULD BE PARSED.
                    return std::nullopt;
                }

                // CREATE A TRIANGLE FOR THE FACE.
                constexpr std::size_t FACE_FIRST_VERTEX_INDEX = 0;
                const WavefrontFaceVertexAttributeIndices& first_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_FIRST_VERTEX_INDEX);
                std::size_t first_vertex_position_index = first_vertex_attribute_indices.VertexPositionIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& first_vertex_position = vertex_positions.at(first_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color first_vertex_color = GRAPHICS::Color::WHITE;
                bool first_vertex_color_exists = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != first_vertex_attribute_indices.VertexColorIndex);
                if (first_vertex_color_exists)
                {
                    first_vertex_color = vertex_colors.at(first_vertex_position_index);
                }

                MATH::Vector2f first_vertex_texture_coordinates;
                bool first_vertex_texture_coordinates_exist = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != first_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (first_vertex_texture_coordinates_exist)
                {
                    std::size_t first_vertex_texture_coordinate_index = first_vertex_attribute_indices.VertexTextureCoordinateIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    first_vertex_texture_coordinates = vertex_texture_coordinates.at(first_vertex_texture_coordinate_index);
                }

                /// @todo   Vertex normals?
                VertexWithAttributes first_vertex = 
                { 
                    .Position = first_vertex_position, 
                    .Color = first_vertex_color,
                    .TextureCoordinates = first_vertex_texture_coordinates,
                };

                constexpr std::size_t FACE_SECOND_VERTEX_INDEX = FACE_FIRST_VERTEX_INDEX + 1;
                const WavefrontFaceVertexAttributeIndices& second_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_SECOND_VERTEX_INDEX);
                std::size_t second_vertex_position_index = second_vertex_attribute_indices.VertexPositionIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& second_vertex_position = vertex_positions.at(second_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color second_vertex_color = GRAPHICS::Color::WHITE;
                bool second_vertex_color_exists = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != second_vertex_attribute_indices.VertexColorIndex);
                if (second_vertex_color_exists)
                {
                    second_vertex_color = vertex_colors.at(second_vertex_position_index);
                }

                MATH::Vector2f second_vertex_texture_coordinates;
                bool second_vertex_texture_coordinates_exist = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != second_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (second_vertex_texture_coordinates_exist)
                {
                    std::size_t second_vertex_texture_coordinate_index = second_vertex_attribute_indices.VertexTextureCoordinateIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    second_vertex_texture_coordinates = vertex_texture_coordinates.at(second_vertex_texture_coordinate_index);
                }

                /// @todo   Vertex normals?
                VertexWithAttributes second_vertex =
                {
                    .Position = second_vertex_position,
                    .Color = second_vertex_color,
                    .TextureCoordinates = second_vertex_texture_coordinates,
                };

                constexpr std::size_t FACE_THIRD_VERTEX_INDEX = FACE_SECOND_VERTEX_INDEX + 1;
                const WavefrontFaceVertexAttributeIndices& third_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_THIRD_VERTEX_INDEX);
                std::size_t third_vertex_position_index = third_vertex_attribute_indices.VertexPositionIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& third_vertex_position = vertex_positions.at(third_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color third_vertex_color = GRAPHICS::Color::WHITE;
                bool third_vertex_color_exists = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != third_vertex_attribute_indices.VertexColorIndex);
                if (third_vertex_color_exists)
                {
                    third_vertex_color = vertex_colors.at(third_vertex_position_index);
                }

                MATH::Vector2f third_vertex_texture_coordinates;
                bool third_vertex_texture_coordinates_exist = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != third_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (third_vertex_texture_coordinates_exist)
                {
                    std::size_t third_vertex_texture_coordinate_index = third_vertex_attribute_indices.VertexTextureCoordinateIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    third_vertex_texture_coordinates = vertex_texture_coordinates.at(third_vertex_texture_coordinate_index);
                }

                /// @todo   Vertex normals?
                VertexWithAttributes third_vertex =
                {
                    .Position = third_vertex_position,
                    .Color = third_vertex_color,
                    .TextureCoordinates = third_vertex_texture_coordinates,
                };
                
                GRAPHICS::GEOMETRY::Triangle triangle;
                triangle.Material = current_material;
                triangle.Vertices =
                {
                    first_vertex,
                    second_vertex,
                    third_vertex
                };

                // A model and mesh must exist first before adding a triangle.
                if (!model)
                {
                    model = GRAPHICS::MODELING::Model();
                }
                if (!current_mesh)
                {
                    current_mesh = GRAPHICS::Mesh { .Name = "Default" };
                }

                current_mesh->Triangles.emplace_back(triangle);

                // CREATE AN ADDITIONAL TRIANLGE IF A QUAD EXISTS.
                // Quads are common enough to implement support here.
                constexpr std::size_t QUAD_VERTEX_COUNT = 4;
                bool is_quad = (QUAD_VERTEX_COUNT == face_vertex_attribute_index_count);
                if (is_quad)
                {
                    // EXTRACT THE FOURTH VERTEX.
                    constexpr std::size_t FACE_FOURTH_VERTEX_INDEX = FACE_THIRD_VERTEX_INDEX + 1;
                    const WavefrontFaceVertexAttributeIndices& fourth_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_FOURTH_VERTEX_INDEX);
                    std::size_t fourth_vertex_position_index = fourth_vertex_attribute_indices.VertexPositionIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    const MATH::Vector3f& fourth_vertex_position = vertex_positions.at(fourth_vertex_position_index);

                    /// @todo   Assume that the color is at the same as the position index?
                    GRAPHICS::Color fourth_vertex_color = GRAPHICS::Color::WHITE;
                    bool fourth_vertex_color_exists = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != fourth_vertex_attribute_indices.VertexColorIndex);
                    if (fourth_vertex_color_exists)
                    {
                        fourth_vertex_color = vertex_colors.at(fourth_vertex_position_index);
                    }

                    MATH::Vector2f fourth_vertex_texture_coordinates;
                    bool fourth_vertex_texture_coordinates_exist = (WavefrontFaceVertexAttributeIndices::UNSET_INDEX != fourth_vertex_attribute_indices.VertexTextureCoordinateIndex);
                    if (fourth_vertex_texture_coordinates_exist)
                    {
                        std::size_t fourth_vertex_texture_coordinate_index = fourth_vertex_attribute_indices.VertexTextureCoordinateIndex - WavefrontFaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                        fourth_vertex_texture_coordinates = vertex_texture_coordinates.at(fourth_vertex_texture_coordinate_index);
                    }

                    /// @todo   Vertex normals?
                    VertexWithAttributes fourth_vertex =
                    {
                        .Position = fourth_vertex_position,
                        .Color = fourth_vertex_color,
                        .TextureCoordinates = fourth_vertex_texture_coordinates,
                    };
                    
                    // CREATE THE ADDITIONAL TRIANGLE.
                    // Vertices should be in first, third, then fourth order
                    // (see https://stackoverflow.com/questions/23723993/converting-quadriladerals-in-an-obj-file-into-triangles).
                    GRAPHICS::GEOMETRY::Triangle extra_triangle_for_quad;
                    extra_triangle_for_quad.Material = current_material;
                    extra_triangle_for_quad.Vertices =
                    {
                        first_vertex,
                        third_vertex,
                        fourth_vertex
                    };
                    current_mesh->Triangles.emplace_back(extra_triangle_for_quad);
                }

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }

            // SKIP OVER ANY LINE DATA.
            const std::string LINE_INDICATOR = "l";
            bool is_polyline_line = (LINE_INDICATOR == line_first_component);
            if (is_polyline_line)
            {
                assert("Polylines not yet supported for WavefrontObjectModel.");
                continue;
            }

            // PARSE ANY MATERIAL FILES.
            const std::string MATERIAL_LIBRARY_KEYWORD = "mtllib";
            bool is_material_file_line = (MATERIAL_LIBRARY_KEYWORD == line_first_component);
            if (is_material_file_line)
            {
                // PARSE THE MATERIALS FROM THE FILE.
                std::filesystem::path material_filepath = current_line_components.back();
                current_materials = WavefrontMaterial::Load(material_filepath);

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }

            // ENSURE THE CORRECT MATERIAL IS BEING USED.
            const std::string USE_MATERIAL_KEYWORD = "usemtl";
            bool is_use_material_line = (USE_MATERIAL_KEYWORD == line_first_component);
            if (is_use_material_line)
            {
                // SWITCH TO USING THE NEWLY SPECIFIED MATERIAL GOING FORWARD.
                const std::string& material_name = current_line_components.back();
                current_material = current_materials.at(material_name);

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }

            // CREATE ANY NEW MESH OBJECTS.
            const std::string OBJECT_INDICATOR = "o";
            bool is_named_object_line = (OBJECT_INDICATOR == line_first_component);
            if (is_named_object_line)
            {
                // ENSURE A MODEL EXISTS.
                if (!model)
                {
                    model = GRAPHICS::MODELING::Model();
                }

                // ENSURE ANY PREVIOUS MESH IS STORED WITH THE MODEL.
                if (current_mesh)
                {
                    model->MeshesByName[current_mesh->Name] = *current_mesh;
                }

                // CREATE A NEW MESH WITH THE APPROPRIATE NAME.
                const std::string& mesh_name = current_line_components.back();
                current_mesh = GRAPHICS::Mesh { .Name = mesh_name };

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }
            
            // SKIP OVER ANY GROUP.
            const std::string GROUP_INDICATOR = "g";
            bool is_group_line = (GROUP_INDICATOR == line_first_component);
            if (is_group_line)
            {
                assert("Groups not yet supported for WavefrontObjectModel.");
                continue;
            }

            // PARSE ANY SHADING INFORMATION.
            const std::string SHADING_INDICATOR = "s";
            bool is_shading_line = (SHADING_INDICATOR == line_first_component);
            if (is_shading_line)
            {
                /// @todo   Not sure how to handle this.

                // CONTINUE PARSING ADDITIONAL FIELDS.
                continue;
            }
        }

        // ENSURE ANY FINAL MESH IS STORED WITH THE MODEL.
        if (model)
        {
            if (current_mesh)
            {
                model->MeshesByName[current_mesh->Name] = *current_mesh;
            }
        }

        return model;
    }
}
