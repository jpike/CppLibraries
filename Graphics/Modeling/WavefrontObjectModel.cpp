#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "Graphics/Modeling/WavefrontObjectModel.h"
#include "Graphics/Modeling/WavefrontMaterial.h"

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
            MATH::Vector3f first_vertex = vertices.at(std::get<0>(face));
            MATH::Vector3f second_vertex = vertices.at(std::get<1>(face));
            MATH::Vector3f third_vertex = vertices.at(std::get<2>(face));

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

                current_mesh->Material = current_materials[material_name];

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

                GRAPHICS::Triangle triangle;
                triangle.Vertices =
                {
                    first_vertex_position,
                    second_vertex_position,
                    third_vertex_position
                };

                current_mesh->Material.VertexColors.push_back(GRAPHICS::Color::WHITE);
                current_mesh->Material.VertexColors.push_back(GRAPHICS::Color::WHITE);
                current_mesh->Material.VertexColors.push_back(GRAPHICS::Color::WHITE);

                current_mesh->Material.VertexTextureCoordinates.push_back(first_vertex_texture_coordinates);
                current_mesh->Material.VertexTextureCoordinates.push_back(second_vertex_texture_coordinates);
                current_mesh->Material.VertexTextureCoordinates.push_back(third_vertex_texture_coordinates);

                triangle.Material = std::make_shared<GRAPHICS::Material>(current_mesh->Material);

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
                        MATH::Vector3f& first_vertex_position = vertex_positions.at(first_vertex_index);
                        MATH::Vector2f& first_vertex_texture_coordinates = vertex_texture_coordinates.at(first_vertex_texture_coordinate_index);
                        MATH::Vector3f& first_vertex_normal = vertex_normals.at(first_vertex_normal_index);
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

                        MATH::Vector3f& third_vertex_position = vertex_positions.at(third_vertex_index);
                        MATH::Vector2f& third_vertex_texture_coordinates = vertex_texture_coordinates.at(third_vertex_texture_coordinate_index);
                        MATH::Vector3f& third_vertex_normal = vertex_normals.at(third_vertex_normal_index);
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

                        MATH::Vector3f& fourth_vertex_position = vertex_positions.at(fourth_vertex_index);
                        MATH::Vector2f& fourth_vertex_texture_coordinates = vertex_texture_coordinates.at(fourth_vertex_texture_coordinate_index);
                        MATH::Vector3f& fourth_vertex_normal = vertex_normals.at(fourth_vertex_normal_index);
                        GRAPHICS::VertexWithAttributes fourth_vertex =
                        {
                            .X = fourth_vertex_position.X,
                            .Y = fourth_vertex_position.Y,
                            .Z = fourth_vertex_position.Z,
                            .TextureU = fourth_vertex_texture_coordinates.X,
                            .TextureV = fourth_vertex_texture_coordinates.Y,
                            //.TextureU = 0.0f,
                            //.TextureV = 1.0f,
                            //// @todo
                            //.ColorAlphaRedGreenBlue = (uint32_t)rand(),
                            .ColorAlphaRedGreenBlue = 0xFFFFFFFF,
                        };
                        current_mesh->Vertices.push_back(fourth_vertex);
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
}
