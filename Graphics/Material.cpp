#include <fstream>
#include <optional>
#include <sstream>
#include "Graphics/Images/Bitmap.h"
#include "Graphics/Material.h"

namespace GRAPHICS
{
    std::unordered_map<std::string, Material> Material::Load(const std::filesystem::path& mtl_filepath)
    {
        // OPEN THE FILE.
        std::ifstream material_file(mtl_filepath);
        bool material_file_opened = material_file.is_open();
        if (!material_file_opened)
        {
            return {};
        }

        // READ IN THE DATA FROM THE .MTL FILE.
        // Note that this reading may not yet be fully robust.
        // It only handles the absolute minimum as currently needed for basic demos.
        std::unordered_map<std::string, Material> materials_by_name;
        // Information on a given material is spread across multiple lines, so we need
        // to keep track of the current material across multiple lines.
        std::optional<Material> current_material;
        std::string line;
        while (std::getline(material_file, line))
        {
            // SKIP OVER ANY BLANK LINES.
            bool is_blank_line = line.empty();
            if (is_blank_line)
            {
                continue;
            }

            // SKIP OVER ANY COMMENT LINES.
            constexpr char MTL_COMMENT_CHARACTER = '#';
            bool is_comment_line = line.starts_with(MTL_COMMENT_CHARACTER);
            if (is_comment_line)
            {
                continue;
            }

            // CHECK IF A NEW MATERIAL IS BEING DEFINED.
            const std::string NEW_MATERIAL_KEYWORD = "newmtl";
            bool is_new_material_line = line.starts_with(NEW_MATERIAL_KEYWORD);
            if (is_new_material_line)
            {
                // CHECK IF ANOTHER MATERIAL IS BEING PROCESSED.
                // Before processing a new material, we need to finish the old one.
                if (current_material)
                {
                    // STORE THE PRIOR MATERIAL NOW THAT IT IS COMPLETE.
                    materials_by_name[current_material->Name] = *current_material;
                }

                // POPULATE THE NEW MATERIAL'S NAME.
                current_material = Material();

                std::istringstream line_data(line);
                // Skip past the field indicator.
                std::string field_indicator;
                line_data >> field_indicator;
                line_data >> current_material->Name;

                current_material->Shading = ShadingType::MATERIAL;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY SPECULAR EXPONENT.
            const std::string SPECULAR_EXPONENT_INDICATOR = "Ns";
            bool is_specular_exponent_line = line.starts_with(SPECULAR_EXPONENT_INDICATOR);
            if (is_specular_exponent_line)
            {
                std::istringstream line_data(line);
                // Skip past the data type indicator.
                std::string data_type_indicator;
                line_data >> data_type_indicator;

                line_data >> current_material->SpecularPower;

                continue;
            }

            // READ IN ANY AMBIENT COLOR.
            const std::string AMBIENT_COLOR_INDICATOR = "Ka";
            bool is_ambient_color_line = line.starts_with(AMBIENT_COLOR_INDICATOR);
            if (is_ambient_color_line)
            {
                std::istringstream line_data(line);
                // Skip past the color type indicator.
                std::string color_type_indicator;
                line_data >> color_type_indicator;

                Color color = Color::BLACK;
                line_data >> color.Red;
                line_data >> color.Green;
                line_data >> color.Blue;

                current_material->AmbientColor = color;

                continue;
            }

            // READ IN ANY DIFFUSE COLOR.
            const std::string DIFFUSE_COLOR_INDICATOR = "Kd";
            bool is_diffuse_color_line = line.starts_with(DIFFUSE_COLOR_INDICATOR);
            if (is_diffuse_color_line)
            {
                std::istringstream line_data(line);
                // Skip past the color type indicator.
                std::string color_type_indicator;
                line_data >> color_type_indicator;

                Color color = Color::BLACK;
                line_data >> color.Red;
                line_data >> color.Green;
                line_data >> color.Blue;

                current_material->DiffuseColor = color;

                continue;
            }

            // READ IN ANY SPECULAR COLOR.
            const std::string SPECULAR_COLOR_INDICATOR = "Ks";
            bool is_specular_color_line = line.starts_with(SPECULAR_COLOR_INDICATOR);
            if (is_specular_color_line)
            {
                std::istringstream line_data(line);
                // Skip past the color type indicator.
                std::string color_type_indicator;
                line_data >> color_type_indicator;

                Color color = Color::BLACK;
                line_data >> color.Red;
                line_data >> color.Green;
                line_data >> color.Blue;

                current_material->SpecularColor = color;

                continue;
            }

            // READ IN ANY EMISSIVE COLOR.
            const std::string EMISSIVE_COLOR_INDICATOR = "Ke";
            bool is_emissive_color_line = line.starts_with(EMISSIVE_COLOR_INDICATOR);
            if (is_emissive_color_line)
            {
                std::istringstream line_data(line);
                // Skip past the color type indicator.
                std::string color_type_indicator;
                line_data >> color_type_indicator;

                Color color = Color::BLACK;
                line_data >> color.Red;
                line_data >> color.Green;
                line_data >> color.Blue;

                current_material->EmissiveColor = color;

                continue;
            }

            /// @todo   Not sure what "Ni" means.
            /// https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library
            /// says optical density (index of refraction).

            // READ IN ANY OPAQUENESS LEVEL.
            constexpr char DISSOLVED_LEVEL_INDICATOR = 'd';
            bool is_dissolved_level_line = line.starts_with(DISSOLVED_LEVEL_INDICATOR);
            if (is_dissolved_level_line)
            {
                std::istringstream line_data(line);
                // Skip past the data type indicator.
                std::string data_type_indicator;
                line_data >> data_type_indicator;

                // UPDATE THE ALPHA FOR ALL COLORS.
                float alpha = Color::MAX_FLOAT_COLOR_COMPONENT;
                line_data >> alpha;

                current_material->AmbientColor.Alpha = alpha;
                current_material->DiffuseColor.Alpha = alpha;
                current_material->SpecularColor.Alpha = alpha;
                current_material->EmissiveColor.Alpha = alpha;

                continue;
            }

            // READ IN THE ILLUMINATION MODEL.
            const std::string ILLUMINATION_MODEL_INDICATOR = "illum";
            bool is_illumination_model_line = line.starts_with(ILLUMINATION_MODEL_INDICATOR);
            if (is_illumination_model_line)
            {
                /// @todo
                continue;
            }

            // READ IN ANY TEXTURE.
            const std::string TEXTURE_MAP_INDICATOR = "map";
            /// @todo   Handle tabs!
            bool is_texture_map_line = line.starts_with(TEXTURE_MAP_INDICATOR) || line.starts_with("\t" + TEXTURE_MAP_INDICATOR);
            if (is_texture_map_line)
            {
                std::istringstream line_data(line);
                // Skip past the data type indicator.
                std::string data_type_indicator;
                line_data >> data_type_indicator;

                std::string texture_filename;
                line_data >> texture_filename;

                std::filesystem::path material_directory_path = mtl_filepath.parent_path();
                std::filesystem::path texture_filepath = material_directory_path / texture_filename;

                /// @todo   Error-handling...different file types.
                current_material->Texture = GRAPHICS::IMAGES::Bitmap::LoadPng(texture_filepath, ColorFormat::RGBA);
                // for bgfx - a, b, g, r
                current_material->Shading = ShadingType::TEXTURED;

                continue;
            }
        }

        // STORE ANY LEFTOVER MATERIAL.
        if (current_material)
        {
            materials_by_name[current_material->Name] = *current_material;
        }

        // RETURN THE MATERIALS.
        return materials_by_name;
    }
}
