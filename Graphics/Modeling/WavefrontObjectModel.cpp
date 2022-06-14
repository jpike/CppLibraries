#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "ErrorHandling/Asserts.h"
#include "Graphics/Modeling/WavefrontObjectModel.h"
#include "Graphics/Modeling/WavefrontMaterial.h"
#include "String/String.hpp"

namespace GRAPHICS::MODELING
{
    /// Attempts to load the model from the specified .obj file.
    /// Any additional referenced files are automatically loaded to ensure a complete model is loaded.
    /// @param[in]  obj_filepath - The path of the .obj file to load.
    /// @return The 3D model, if successfull loaded; null otherwise.
    std::optional<Object3D> WavefrontObjectModel::Load(const std::filesystem::path& obj_filepath)
    {
        // OPEN THE FILE.
        std::ifstream obj_file(obj_filepath);
        bool obj_file_opened = obj_file.is_open();
        if (!obj_file_opened)
        {
            return std::nullopt;
        }

        // READ IN THE DATA FROM THE .OBJ FILE.
        // Note that this reading may not yet be fully robust.
        // It only handles the absolute minimum as currently needed for basic demos.
        constexpr char SPACE_SEPARATOR = ' ';
        std::vector<std::filesystem::path> material_filenames;
        std::vector<MATH::Vector3f> vertices;
        std::vector< std::tuple<std::size_t, std::size_t, std::size_t> > face_vertex_indices;
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

            // TRACK ANY MATERIALS THAT NEED TO BE LOADED SEPARATELY.
            const std::string MATERIAL_LIBRARY_KEYWORD = "mtllib";
            bool is_material_file_line = line.starts_with(MATERIAL_LIBRARY_KEYWORD);
            if (is_material_file_line)
            {
                // FIND WHERE THE MATERIAL FILENAME STARTS IN THE LINE.
                std::size_t space_separator_index = line.find(SPACE_SEPARATOR);
                bool space_separator_found = (std::string::npos != space_separator_index);
                if (!space_separator_found)
                {
                    return std::nullopt;
                }

                // The material filename starts right after the space separator.
                std::size_t material_filename_start_index = space_separator_index + 1;

                // READ THE MATERIAL FILENAME.
                std::string material_filename = line.substr(material_filename_start_index);
                material_filenames.push_back(material_filename);

                // CONTINUE PROCESSING OTHER LINES.
                continue;
            }

            /// @todo   What to do with usemtl?
            const std::string USE_MATERIAL_KEYWORD = "usemtl";
            bool is_use_material_line = line.starts_with(USE_MATERIAL_KEYWORD);
            if (is_use_material_line)
            {
                continue;
            }

            // CHECK IF AN OBJECT IS BEING DEFINED.
            /// @todo   Should this code handle multiple objects per file?
            constexpr char OBJECT_INDICATOR = 'o';
            bool is_named_object_line = line.starts_with(OBJECT_INDICATOR);
            if (is_named_object_line)
            {
                /// @todo   Track object names?
                continue;
            }

            // CHECK IF VERTEX DATA IS BEING DEFINED.
            constexpr char VERTEX_INDICATOR = 'v';
            bool is_vertex_line = line.starts_with(VERTEX_INDICATOR);
            if (is_vertex_line)
            {
                // CHECK THE TYPE OF VERTEX DATA BEING DEFINED.
                const std::string VERTEX_TEXTURE_COORDINATE_INDICATOR = "vt";
                const std::string VERTEX_NORMAL_INDICATOR = "vn";
                bool is_vertex_texture_coordinates = line.starts_with(VERTEX_TEXTURE_COORDINATE_INDICATOR);
                bool is_vertex_normal_data = line.starts_with(VERTEX_NORMAL_INDICATOR);
                if (is_vertex_texture_coordinates)
                {
                    /// @todo   Handle.  Not important yet.
                }
                else if (is_vertex_normal_data)
                {
                    /// @todo   Handle.  Not important yet.
                }
                else
                {
                    // The line just consists of regular vertex postion data.

                    /// @todo   Make this more efficient.
                    std::istringstream line_data(line);
                    // Skip past the vertex data type indicator.
                    std::string vertex_data_type_indicator;
                    line_data >> vertex_data_type_indicator;

                    MATH::Vector3f vertex_position;
                    line_data >> vertex_position.X;
                    line_data >> vertex_position.Y;
                    line_data >> vertex_position.Z;
                    vertices.push_back(vertex_position);
                }
            }

            /// @todo   What to do with shading line?
            constexpr char SHADING_INDICATOR = 's';
            bool is_shading_line = line.starts_with(SHADING_INDICATOR);
            if (is_shading_line)
            {
                continue;
            }

            constexpr char FACE_INDICATOR = 'f';
            bool is_face_line = line.starts_with(FACE_INDICATOR);
            if (is_face_line)
            {
                // The line has the following format:
                // f v1_index/vt1_index/vn1_index v2_index/vt2_index/vn2_index v3_index/vt3_index/vn3_index

                /// @todo   Make this more efficient.
                std::istringstream line_data(line);
                // Skip past the face data type indicator.
                std::string face_data_type_indicator;
                line_data >> face_data_type_indicator;

                /// @todo   Make the below more robust and read in all indices.

                std::string first_vertex_indices;
                line_data >> first_vertex_indices;

                constexpr char VERTEX_INDEX_DELIMITER = '/';
                std::size_t first_vertex_delimiter_position = first_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string first_vertex_index_string = first_vertex_indices.substr(0, first_vertex_delimiter_position);
                std::size_t first_vertex_index = std::stoul(first_vertex_index_string);

                // The vertex indices in the file start at 1, rather than 0.
                constexpr std::size_t VERTEX_INDEX_OFFSET = 1;
                first_vertex_index -= VERTEX_INDEX_OFFSET;

                std::string second_vertex_indices;
                line_data >> second_vertex_indices;

                std::size_t second_vertex_delimiter_position = second_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string second_vertex_index_string = second_vertex_indices.substr(0, second_vertex_delimiter_position);
                std::size_t second_vertex_index = std::stoul(second_vertex_index_string);

                second_vertex_index -= VERTEX_INDEX_OFFSET;

                std::string third_vertex_indices;
                line_data >> third_vertex_indices;

                std::size_t third_vertex_delimiter_position = third_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string third_vertex_index_string = third_vertex_indices.substr(0, third_vertex_delimiter_position);
                std::size_t third_vertex_index = std::stoul(third_vertex_index_string);

                third_vertex_index -= VERTEX_INDEX_OFFSET;

                auto face_indices = std::make_tuple(first_vertex_index, second_vertex_index, third_vertex_index);
                face_vertex_indices.push_back(face_indices);
            }
        }

        // LOAD ANY MATERIALS.
        // Materials are expected to all be within the same folder as the .obj file.
        std::filesystem::path model_folder_path = obj_filepath.parent_path();
        std::vector<std::shared_ptr<Material>> materials;
        for (const auto& material_filename : material_filenames)
        {
            std::filesystem::path material_filepath = model_folder_path / material_filename;
            std::shared_ptr<Material> material = WavefrontMaterial::Load(material_filepath);
            if (material)
            {
                materials.push_back(material);
            }
        }

        // FORM THE FINAL OBJECT.
        Object3D object_3d;
        for (const std::tuple<std::size_t, std::size_t, std::size_t>& face : face_vertex_indices)
        {
            // GET THE VERTICES.
            MATH::Vector3f first_vertex_position = vertices.at(std::get<0>(face));
            MATH::Vector3f second_vertex_position = vertices.at(std::get<1>(face));
            MATH::Vector3f third_vertex_position = vertices.at(std::get<2>(face));

            /// @todo   Other vertex attributes?
            VertexWithAttributes first_vertex = { .Position = first_vertex_position };
            VertexWithAttributes second_vertex = { .Position = second_vertex_position };
            VertexWithAttributes third_vertex = { .Position = third_vertex_position };

            // ADD THE CURRENT TRIANGLE.
            /// @todo   How to handle materials?  Need some kind of permanent storage.
            Triangle triangle(materials[0], { first_vertex, second_vertex, third_vertex });
            object_3d.Model.MeshesByName["DefaultMesh"].Triangles.push_back(triangle);
        }

        return object_3d;
    }

    std::optional<GRAPHICS::MODELING::Model> WavefrontObjectModel::LoadAsSingle3DModel(const std::filesystem::path& filepath)
    {
        // OPEN THE FILE.
        std::ifstream obj_file(filepath);
        bool obj_file_opened = obj_file.is_open();
        if (!obj_file_opened)
        {
            return {};
        }

        std::filesystem::path model_folder_path = filepath.parent_path();

        // READ IN THE DATA FROM THE .OBJ FILE.
        std::optional<GRAPHICS::MODELING::Model> current_model = GRAPHICS::MODELING::Model();
        std::optional<GRAPHICS::MODELING::Mesh> current_mesh;

        std::unordered_map<std::string, GRAPHICS::Material> current_materials;

        std::shared_ptr<GRAPHICS::Material> current_material = nullptr;

        // Note that this reading may not yet be fully robust.
        // It only handles the absolute minimum as currently needed for basic demos.
        constexpr char SPACE_SEPARATOR = ' ';
        std::vector<std::filesystem::path> material_filenames;
        std::vector<MATH::Vector3f> vertex_positions;
        std::vector<MATH::Vector2f> vertex_texture_coordinates;
        std::vector<MATH::Vector3f> vertex_normals;
        std::vector< std::tuple<std::size_t, std::size_t, std::size_t> > face_vertex_indices;
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

            // TRACK ANY MATERIALS THAT NEED TO BE LOADED SEPARATELY.
            const std::string MATERIAL_LIBRARY_KEYWORD = "mtllib";
            bool is_material_file_line = line.starts_with(MATERIAL_LIBRARY_KEYWORD);
            if (is_material_file_line)
            {
                // FIND WHERE THE MATERIAL FILENAME STARTS IN THE LINE.
                std::size_t space_separator_index = line.find(SPACE_SEPARATOR);
                bool space_separator_found = (std::string::npos != space_separator_index);
                if (!space_separator_found)
                {
                    return current_model;
                }

                // The material filename starts right after the space separator.
                std::size_t material_filename_start_index = space_separator_index + 1;

                // READ THE MATERIAL FILENAME.
                std::string material_filename = line.substr(material_filename_start_index);
                //material_filenames.push_back(material_filename);
                std::filesystem::path material_filepath = model_folder_path / material_filename;
                /// @todo   Map materials by name?
                /// @todo   Handle multiple material libraries?
                current_materials = GRAPHICS::Material::Load(material_filepath);

                // CONTINUE PROCESSING OTHER LINES.
                continue;
            }

            /// @todo   What to do with usemtl?
            const std::string USE_MATERIAL_KEYWORD = "usemtl";
            bool is_use_material_line = line.starts_with(USE_MATERIAL_KEYWORD);
            if (is_use_material_line)
            {
                /// @todo   Lookup materials (load mtlib first) or store string names for a while?
                std::istringstream line_data(line);
                std::string use_material_keyword;
                line_data >> use_material_keyword;

                std::string material_name;
                line_data >> material_name;

                if (!current_mesh)
                {
                    current_mesh = GRAPHICS::MODELING::Mesh();
                }

                current_material = std::make_shared<GRAPHICS::Material>(current_materials[material_name]);

                continue;
            }

            // CHECK IF AN OBJECT IS BEING DEFINED.
            /// @todo   Should this code handle multiple objects per file?
            constexpr char OBJECT_INDICATOR = 'o';
            bool is_named_object_line = line.starts_with(OBJECT_INDICATOR);
            if (is_named_object_line)
            {
                if (current_mesh)
                {
                    // Store the mesh if already has a name.
                    if (!current_mesh->Name.empty())
                    {
                        current_model->MeshesByName[current_mesh->Name] = *current_mesh;
                    }
                }
                else
                {
                    current_mesh = GRAPHICS::MODELING::Mesh();
                }

                std::istringstream line_data(line);
                std::string object_indicator;
                line_data >> object_indicator;
                line_data >> current_mesh->Name;

                // SKIP SINCE THIS SHOULD BE CONSIDERED THE SAME MODEL.
                continue;
            }

            // CHECK IF VERTEX DATA IS BEING DEFINED.
            constexpr char VERTEX_INDICATOR = 'v';
            bool is_vertex_line = line.starts_with(VERTEX_INDICATOR);
            if (is_vertex_line)
            {
                // CHECK THE TYPE OF VERTEX DATA BEING DEFINED.
                const std::string VERTEX_TEXTURE_COORDINATE_INDICATOR = "vt";
                const std::string VERTEX_NORMAL_INDICATOR = "vn";
                bool is_vertex_texture_coordinates = line.starts_with(VERTEX_TEXTURE_COORDINATE_INDICATOR);
                bool is_vertex_normal_data = line.starts_with(VERTEX_NORMAL_INDICATOR);
                if (is_vertex_texture_coordinates)
                {
                    /// @todo   Handle.  Not important yet.

                    /// @todo   Make this more efficient.
                    std::istringstream line_data(line);
                    // Skip past the vertex data type indicator.
                    std::string vertex_data_type_indicator;
                    line_data >> vertex_data_type_indicator;

                    MATH::Vector2f vertex_texture_coordinate;
                    line_data >> vertex_texture_coordinate.X;
                    line_data >> vertex_texture_coordinate.Y;

                    //vertex_texture_coordinate.X = 1.0f - vertex_texture_coordinate.X;
                    vertex_texture_coordinate.Y = 1.0f - vertex_texture_coordinate.Y;

                    vertex_texture_coordinates.push_back(vertex_texture_coordinate);
                }
                else if (is_vertex_normal_data)
                {
                    /// @todo   Handle.  Not important yet.

                    /// @todo   Make this more efficient.
                    std::istringstream line_data(line);
                    // Skip past the vertex data type indicator.
                    std::string vertex_data_type_indicator;
                    line_data >> vertex_data_type_indicator;

                    MATH::Vector3f vertex_normal;
                    line_data >> vertex_normal.X;
                    line_data >> vertex_normal.Y;
                    line_data >> vertex_normal.Z;
                    vertex_normals.push_back(vertex_normal);
                }
                else
                {
                    // The line just consists of regular vertex postion data.

                    /// @todo   Make this more efficient.
                    std::istringstream line_data(line);
                    // Skip past the vertex data type indicator.
                    std::string vertex_data_type_indicator;
                    line_data >> vertex_data_type_indicator;

                    MATH::Vector3f vertex_position;
                    line_data >> vertex_position.X;
                    line_data >> vertex_position.Y;
                    line_data >> vertex_position.Z;
                    vertex_positions.push_back(vertex_position);
                }
            }

            /// @todo   What to do with shading line?
            constexpr char SHADING_INDICATOR = 's';
            bool is_shading_line = line.starts_with(SHADING_INDICATOR);
            if (is_shading_line)
            {
                continue;
            }

            constexpr char FACE_INDICATOR = 'f';
            bool is_face_line = line.starts_with(FACE_INDICATOR);
            if (is_face_line)
            {
                // The line has the following format:
                // f v1_index/vt1_index/vn1_index v2_index/vt2_index/vn2_index v3_index/vt3_index/vn3_index

                /// @todo   Make this more efficient.
                std::istringstream line_data(line);
                // Skip past the face data type indicator.
                std::string face_data_type_indicator;
                line_data >> face_data_type_indicator;

                /// @todo   Make the below more robust and read in all indices.

                std::string first_vertex_indices;
                line_data >> first_vertex_indices;

                constexpr char VERTEX_INDEX_DELIMITER = '/';
                std::size_t first_vertex_delimiter_position = first_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string first_vertex_index_string = first_vertex_indices.substr(0, first_vertex_delimiter_position);
                std::size_t first_vertex_index = std::stoul(first_vertex_index_string);

                // The vertex indices in the file start at 1, rather than 0.
                constexpr std::size_t VERTEX_INDEX_OFFSET = 1;
                first_vertex_index -= VERTEX_INDEX_OFFSET;

                std::size_t first_vertex_last_delimiter_position = first_vertex_indices.rfind(VERTEX_INDEX_DELIMITER);
                std::string first_vertex_texture_coordinate_index_string = first_vertex_indices.substr(
                    first_vertex_delimiter_position + 1,
                    first_vertex_last_delimiter_position - first_vertex_delimiter_position - 1);
                std::size_t first_vertex_texture_coordinate_index = std::stoul(first_vertex_texture_coordinate_index_string);
                first_vertex_texture_coordinate_index -= VERTEX_INDEX_OFFSET;

                std::string first_vertex_normal_index_string = first_vertex_indices.substr(first_vertex_last_delimiter_position + 1);
                std::size_t first_vertex_normal_index = std::stoul(first_vertex_normal_index_string);
                first_vertex_normal_index -= VERTEX_INDEX_OFFSET;

                std::string second_vertex_indices;
                line_data >> second_vertex_indices;

                std::size_t second_vertex_delimiter_position = second_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string second_vertex_index_string = second_vertex_indices.substr(0, second_vertex_delimiter_position);
                std::size_t second_vertex_index = std::stoul(second_vertex_index_string);

                second_vertex_index -= VERTEX_INDEX_OFFSET;

                std::size_t second_vertex_last_delimiter_position = second_vertex_indices.rfind(VERTEX_INDEX_DELIMITER);
                std::string second_vertex_texture_coordinate_index_string = second_vertex_indices.substr(
                    second_vertex_delimiter_position + 1,
                    second_vertex_last_delimiter_position - second_vertex_delimiter_position - 1);
                std::size_t second_vertex_texture_coordinate_index = std::stoul(second_vertex_texture_coordinate_index_string);
                second_vertex_texture_coordinate_index -= VERTEX_INDEX_OFFSET;

                std::string second_vertex_normal_index_string = second_vertex_indices.substr(second_vertex_last_delimiter_position + 1);
                std::size_t second_vertex_normal_index = std::stoul(second_vertex_normal_index_string);
                second_vertex_normal_index -= VERTEX_INDEX_OFFSET;

                std::string third_vertex_indices;
                line_data >> third_vertex_indices;

                std::size_t third_vertex_delimiter_position = third_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                std::string third_vertex_index_string = third_vertex_indices.substr(0, third_vertex_delimiter_position);
                std::size_t third_vertex_index = std::stoul(third_vertex_index_string);

                third_vertex_index -= VERTEX_INDEX_OFFSET;

                std::size_t third_vertex_last_delimiter_position = third_vertex_indices.rfind(VERTEX_INDEX_DELIMITER);
                std::string third_vertex_texture_coordinate_index_string = third_vertex_indices.substr(
                    third_vertex_delimiter_position + 1,
                    third_vertex_last_delimiter_position - third_vertex_delimiter_position - 1);
                std::size_t third_vertex_texture_coordinate_index = std::stoul(third_vertex_texture_coordinate_index_string);
                third_vertex_texture_coordinate_index -= VERTEX_INDEX_OFFSET;

                std::string third_vertex_normal_index_string = third_vertex_indices.substr(third_vertex_last_delimiter_position + 1);
                std::size_t third_vertex_normal_index = std::stoul(third_vertex_normal_index_string);
                third_vertex_normal_index -= VERTEX_INDEX_OFFSET;

                // After positions comes texture coordinates.

                // After texture coordinates come normals.

                //auto face_indices = std::make_tuple(first_vertex_index, second_vertex_index, third_vertex_index);
                //face_vertex_indices.push_back(face_indices);

                MATH::Vector3f& first_vertex_position = vertex_positions.at(first_vertex_index);
                MATH::Vector2f& first_vertex_texture_coordinates = vertex_texture_coordinates.at(first_vertex_texture_coordinate_index);
                MATH::Vector3f& first_vertex_normal = vertex_normals.at(first_vertex_normal_index);

                MATH::Vector3f& second_vertex_position = vertex_positions.at(second_vertex_index);
                MATH::Vector2f& second_vertex_texture_coordinates = vertex_texture_coordinates.at(second_vertex_texture_coordinate_index);
                MATH::Vector3f& second_vertex_normal = vertex_normals.at(second_vertex_normal_index);

                MATH::Vector3f& third_vertex_position = vertex_positions.at(third_vertex_index);
                MATH::Vector2f& third_vertex_texture_coordinates = vertex_texture_coordinates.at(third_vertex_texture_coordinate_index);
                MATH::Vector3f& third_vertex_normal = vertex_normals.at(third_vertex_normal_index);

                VertexWithAttributes first_vertex = { .Position = first_vertex_position, .TextureCoordinates = first_vertex_texture_coordinates };
                VertexWithAttributes second_vertex = { .Position = second_vertex_position, .TextureCoordinates = second_vertex_texture_coordinates };
                VertexWithAttributes third_vertex = { .Position = third_vertex_position, .TextureCoordinates = third_vertex_texture_coordinates };

                GRAPHICS::Triangle triangle;
                triangle.Vertices =
                {
                    first_vertex,
                    second_vertex,
                    third_vertex
                };

                triangle.Material = current_material;

                current_mesh->Triangles.push_back(triangle);

#if VERTEX_WITH_ATTRIBUTES
                GRAPHICS::VertexWithAttributes first_vertex =
                {
                    .X = first_vertex_position.X,
                    .Y = first_vertex_position.Y,
                    .Z = first_vertex_position.Z,
                    .TextureU = first_vertex_texture_coordinates.X,
                    .TextureV = first_vertex_texture_coordinates.Y,
                    //.TextureU = 0.0f,
                    //.TextureV = 0.0f,
                    //// @todo
                    //.ColorAlphaRedGreenBlue = (uint32_t)rand(),
                    .ColorAlphaRedGreenBlue = 0xFFFFFFFF,
                };
                current_mesh->Vertices.push_back(first_vertex);

                GRAPHICS::VertexWithAttributes second_vertex =
                {
                    .X = second_vertex_position.X,
                    .Y = second_vertex_position.Y,
                    .Z = second_vertex_position.Z,
                    .TextureU = second_vertex_texture_coordinates.X,
                    .TextureV = second_vertex_texture_coordinates.Y,
                    //.TextureU = 1.0f,
                    //.TextureV = 0.0f,
                    //// @todo
                    //.ColorAlphaRedGreenBlue = (uint32_t)rand(),
                    .ColorAlphaRedGreenBlue = 0xFFFFFFFF,
                };
                current_mesh->Vertices.push_back(second_vertex);

                GRAPHICS::VertexWithAttributes third_vertex =
                {
                    .X = third_vertex_position.X,
                    .Y = third_vertex_position.Y,
                    .Z = third_vertex_position.Z,
                    .TextureU = third_vertex_texture_coordinates.X,
                    .TextureV = third_vertex_texture_coordinates.Y,
                    //.TextureU = 1.0f,
                    //.TextureV = 1.0f,
                    //// @todo
                    //.ColorAlphaRedGreenBlue = (uint32_t)rand(),
                    .ColorAlphaRedGreenBlue = 0xFFFFFFFF,
                };
                current_mesh->Vertices.push_back(third_vertex);
#endif

#define QUADS 1
#if QUADS
                /// @todo Handling quads!
                if ((!line_data.eof()) && (!line_data.bad()))
                {
                    std::string fourth_vertex_indices;
                    line_data >> fourth_vertex_indices;

                    //if ((!line_data.eof()) && (!line_data.bad()))
                    {
                        std::size_t fourth_vertex_delimiter_position = fourth_vertex_indices.find(VERTEX_INDEX_DELIMITER);
                        std::string fourth_vertex_index_string = fourth_vertex_indices.substr(0, fourth_vertex_delimiter_position);
                        std::size_t fourth_vertex_index = std::stoul(fourth_vertex_index_string);

                        fourth_vertex_index -= VERTEX_INDEX_OFFSET;

                        std::size_t fourth_vertex_last_delimiter_position = fourth_vertex_indices.rfind(VERTEX_INDEX_DELIMITER);
                        std::string fourth_vertex_texture_coordinate_index_string = fourth_vertex_indices.substr(
                            fourth_vertex_delimiter_position + 1,
                            fourth_vertex_last_delimiter_position - fourth_vertex_delimiter_position - 1);
                        std::size_t fourth_vertex_texture_coordinate_index = std::stoul(fourth_vertex_texture_coordinate_index_string);
                        fourth_vertex_texture_coordinate_index -= VERTEX_INDEX_OFFSET;

                        std::string fourth_vertex_normal_index_string = fourth_vertex_indices.substr(fourth_vertex_last_delimiter_position + 1);
                        std::size_t fourth_vertex_normal_index = std::stoul(fourth_vertex_normal_index_string);
                        fourth_vertex_normal_index -= VERTEX_INDEX_OFFSET;

                        // https://stackoverflow.com/questions/23723993/converting-quadriladerals-in-an-obj-file-into-triangles
                        // first, third, fourth

                        MATH::Vector3f& fourth_vertex_position = vertex_positions.at(fourth_vertex_index);
                        MATH::Vector2f& fourth_vertex_texture_coordinates = vertex_texture_coordinates.at(fourth_vertex_texture_coordinate_index);
                        MATH::Vector3f& fourth_vertex_normal = vertex_normals.at(fourth_vertex_normal_index);
                        GRAPHICS::VertexWithAttributes fourth_vertex =
                        {
                            .Position = fourth_vertex_position,
                            .TextureCoordinates = fourth_vertex_texture_coordinates,
                        };

                        GRAPHICS::Triangle extra_triangle_for_quad;
                        extra_triangle_for_quad.Vertices =
                        {
                            first_vertex,
                            third_vertex,
                            fourth_vertex,
                        };

                        extra_triangle_for_quad.Material = current_material;

                        current_mesh->Triangles.push_back(extra_triangle_for_quad);
                    }
                }
#endif
            }
        }

        // Last mesh after the above.
        if (current_mesh)
        {
            current_model->MeshesByName[current_mesh->Name] = *current_mesh;
        }

        return current_model;
    }

    /// Attempts to load the model from the specified .obj file.
    /// Any additional referenced files are automatically loaded to ensure a complete model is loaded.
    /// @param[in]  obj_filepath - The path of the .obj file to load.
    /// @return The 3D model, if successfull loaded; null otherwise.
    std::optional<GRAPHICS::MODELING::Model> WavefrontObjectModel::LoadNew(const std::filesystem::path& filepath)
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
        std::optional<GRAPHICS::MODELING::Mesh> current_mesh = std::nullopt;
        std::shared_ptr<GRAPHICS::Material> current_material = nullptr;
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
                std::vector<FaceVertexAttributeIndices> face_vertex_attribute_indices;
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
                    FaceVertexAttributeIndices current_face_vertex_attribute_indices;
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
                        current_face_vertex_attribute_indices.VertexTextureCoordinateIndex = std::stoul(texture_coordinate_index_string);
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
                bool enough_vertices_for_triangle = (face_vertex_attribute_index_count >= GRAPHICS::Triangle::VERTEX_COUNT);
                ASSERT_THEN_IF_NOT(enough_vertices_for_triangle)
                {
                    // INDICATE THAT NO MODEL COULD BE PARSED.
                    return std::nullopt;
                }

                // CREATE A TRIANGLE FOR THE FACE.
                constexpr std::size_t FACE_FIRST_VERTEX_INDEX = 0;
                const FaceVertexAttributeIndices& first_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_FIRST_VERTEX_INDEX);
                std::size_t first_vertex_position_index = first_vertex_attribute_indices.VertexPositionIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& first_vertex_position = vertex_positions.at(first_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color first_vertex_color = GRAPHICS::Color::WHITE;
                bool first_vertex_color_exists = (FaceVertexAttributeIndices::UNSET_INDEX != first_vertex_attribute_indices.VertexColorIndex);
                if (first_vertex_color_exists)
                {
                    first_vertex_color = vertex_colors.at(first_vertex_position_index);
                }

                MATH::Vector2f first_vertex_texture_coordinates;
                bool first_vertex_texture_coordinates_exist = (FaceVertexAttributeIndices::UNSET_INDEX != first_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (first_vertex_texture_coordinates_exist)
                {
                    std::size_t first_vertex_texture_coordinate_index = first_vertex_attribute_indices.VertexTextureCoordinateIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
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
                const FaceVertexAttributeIndices& second_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_SECOND_VERTEX_INDEX);
                std::size_t second_vertex_position_index = second_vertex_attribute_indices.VertexPositionIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& second_vertex_position = vertex_positions.at(second_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color second_vertex_color = GRAPHICS::Color::WHITE;
                bool second_vertex_color_exists = (FaceVertexAttributeIndices::UNSET_INDEX != second_vertex_attribute_indices.VertexColorIndex);
                if (second_vertex_color_exists)
                {
                    second_vertex_color = vertex_colors.at(second_vertex_position_index);
                }

                MATH::Vector2f second_vertex_texture_coordinates;
                bool second_vertex_texture_coordinates_exist = (FaceVertexAttributeIndices::UNSET_INDEX != second_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (second_vertex_texture_coordinates_exist)
                {
                    std::size_t second_vertex_texture_coordinate_index = second_vertex_attribute_indices.VertexTextureCoordinateIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
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
                const FaceVertexAttributeIndices& third_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_THIRD_VERTEX_INDEX);
                std::size_t third_vertex_position_index = third_vertex_attribute_indices.VertexPositionIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                const MATH::Vector3f& third_vertex_position = vertex_positions.at(third_vertex_position_index);

                /// @todo   Assume that the color is at the same as the position index?
                GRAPHICS::Color third_vertex_color = GRAPHICS::Color::WHITE;
                bool third_vertex_color_exists = (FaceVertexAttributeIndices::UNSET_INDEX != third_vertex_attribute_indices.VertexColorIndex);
                if (third_vertex_color_exists)
                {
                    third_vertex_color = vertex_colors.at(third_vertex_position_index);
                }

                MATH::Vector2f third_vertex_texture_coordinates;
                bool third_vertex_texture_coordinates_exist = (FaceVertexAttributeIndices::UNSET_INDEX != third_vertex_attribute_indices.VertexTextureCoordinateIndex);
                if (third_vertex_texture_coordinates_exist)
                {
                    std::size_t third_vertex_texture_coordinate_index = third_vertex_attribute_indices.VertexTextureCoordinateIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    third_vertex_texture_coordinates = vertex_texture_coordinates.at(third_vertex_texture_coordinate_index);
                }

                /// @todo   Vertex normals?
                VertexWithAttributes third_vertex =
                {
                    .Position = third_vertex_position,
                    .Color = third_vertex_color,
                    .TextureCoordinates = third_vertex_texture_coordinates,
                };
                
                GRAPHICS::Triangle triangle;
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
                    current_mesh = GRAPHICS::MODELING::Mesh { .Name = "Default" };
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
                    const FaceVertexAttributeIndices& fourth_vertex_attribute_indices = face_vertex_attribute_indices.at(FACE_FOURTH_VERTEX_INDEX);
                    std::size_t fourth_vertex_position_index = fourth_vertex_attribute_indices.VertexPositionIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
                    const MATH::Vector3f& fourth_vertex_position = vertex_positions.at(fourth_vertex_position_index);

                    /// @todo   Assume that the color is at the same as the position index?
                    GRAPHICS::Color fourth_vertex_color = GRAPHICS::Color::WHITE;
                    bool fourth_vertex_color_exists = (FaceVertexAttributeIndices::UNSET_INDEX != fourth_vertex_attribute_indices.VertexColorIndex);
                    if (fourth_vertex_color_exists)
                    {
                        fourth_vertex_color = vertex_colors.at(fourth_vertex_position_index);
                    }

                    MATH::Vector2f fourth_vertex_texture_coordinates;
                    bool fourth_vertex_texture_coordinates_exist = (FaceVertexAttributeIndices::UNSET_INDEX != fourth_vertex_attribute_indices.VertexTextureCoordinateIndex);
                    if (fourth_vertex_texture_coordinates_exist)
                    {
                        std::size_t fourth_vertex_texture_coordinate_index = fourth_vertex_attribute_indices.VertexTextureCoordinateIndex - FaceVertexAttributeIndices::OFFSET_FROM_ZERO_BASED_INDEX;
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
                    GRAPHICS::Triangle extra_triangle_for_quad;
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
                current_materials = WavefrontMaterial::LoadNew(material_filepath);

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
                current_mesh = GRAPHICS::MODELING::Mesh { .Name = mesh_name };

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
