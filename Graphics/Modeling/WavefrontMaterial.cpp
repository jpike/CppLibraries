#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "ErrorHandling/Asserts.h"
#include "Graphics/Modeling/WavefrontMaterial.h"
#include "String/String.hpp"

namespace GRAPHICS::MODELING
{
    /// Attempts to load the materials from the specified .mtl file.
    /// @param[in]  mtl_filepath - The path of the .mtl file to load.
    /// @return The materials mapped by name, if successfull loaded; empty otherwise.
    std::unordered_map<std::string, MEMORY::NonNullSharedPointer<Material>> WavefrontMaterial::Load(const std::filesystem::path& mtl_filepath)
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
        std::unordered_map<std::string, MEMORY::NonNullSharedPointer<Material>> materials_by_name;
        std::shared_ptr<Material> current_material = nullptr;
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

            // SPLIT THE LINE INTO COMPONENTS BASED ON WHITESPACE.
            // The rest of the data in the file effectively has different components in each line separated by whitespace.
            // This splitting is not done for comment checking since comments could have large numbers of spaces.
            std::vector<std::string> current_line_components = STRING::String::SplitByWhitespace(line);
            // Most fields parsed below require at least 2 components.
            // Checking this now prevents having to deal with lower-level indices as much below.
            constexpr std::size_t MIN_REQUIRED_COMPONENT_COUNT = 2;
            std::size_t current_line_component_count = current_line_components.size();
            bool line_has_enough_components = (current_line_component_count >= MIN_REQUIRED_COMPONENT_COUNT);
            if (!line_has_enough_components)
            {
                assert("Line without enough components detected for WavefrontMaterial.");
                continue;
            }

            // CHECK IF A NEW MATERIAL IS BEING DEFINED.
            const std::string& first_line_component = current_line_components.front();
            const std::string NEW_MATERIAL_KEYWORD = "newmtl";
            bool is_new_material_line = (NEW_MATERIAL_KEYWORD == first_line_component);
            if (is_new_material_line)
            {
                // CHECK IF ANOTHER MATERIAL IS BEING PROCESSED.
                // Before processing a new material, we need to finish the old one.
                if (current_material)
                {
                    // STORE THE PRIOR MATERIAL NOW THAT IT IS COMPLETE.
                    materials_by_name.emplace(std::make_pair(current_material->Name, current_material));
                }

                // CREATE THE NEW MATERIAL.
                current_material = std::make_shared<Material>();
                current_material->Name = current_line_components.back();
                current_material->Shading = ShadingType::MATERIAL;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY AMBIENT COLOR.
            constexpr std::size_t INDICATOR_PLUS_COLOR_COMPONENT_COUNT = 4;
            constexpr std::size_t RED_INDEX = 1;
            constexpr std::size_t GREEN_INDEX = 2;
            constexpr std::size_t BLUE_INDEX = 3;
            const std::string AMBIENT_COLOR_INDICATOR = "Ka";
            bool is_ambient_color_line = (AMBIENT_COLOR_INDICATOR == first_line_component);
            if (is_ambient_color_line)
            {
                // ENSURE THERE ARE ENOUGH COLOR COMPONENTS.
                bool enough_color_components = (current_line_component_count >= INDICATOR_PLUS_COLOR_COMPONENT_COUNT);
                ASSERT_THEN_IF_NOT(enough_color_components)
                {
                    // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                    continue;
                }

                // PARSE THE AMBIENT COLOR.
                Color color = Color::BLACK;
                const std::string& red_string = current_line_components.at(RED_INDEX);
                color.Red = std::stof(red_string);
                const std::string& green_string = current_line_components.at(GREEN_INDEX);
                color.Green = std::stof(green_string);
                const std::string& blue_string = current_line_components.at(BLUE_INDEX);
                color.Blue = std::stof(blue_string);

                current_material->AmbientColor = color;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY DIFFUSE COLOR.
            const std::string DIFFUSE_COLOR_INDICATOR = "Kd";
            bool is_diffuse_color_line = (DIFFUSE_COLOR_INDICATOR == first_line_component);
            if (is_diffuse_color_line)
            {
                // ENSURE THERE ARE ENOUGH COLOR COMPONENTS.
                bool enough_color_components = (current_line_component_count >= INDICATOR_PLUS_COLOR_COMPONENT_COUNT);
                ASSERT_THEN_IF_NOT(enough_color_components)
                {
                    // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                    continue;
                }

                // PARSE THE DIFFUSE COLOR.
                Color color = Color::BLACK;
                const std::string& red_string = current_line_components.at(RED_INDEX);
                color.Red = std::stof(red_string);
                const std::string& green_string = current_line_components.at(GREEN_INDEX);
                color.Green = std::stof(green_string);
                const std::string& blue_string = current_line_components.at(BLUE_INDEX);
                color.Blue = std::stof(blue_string);

                current_material->DiffuseColor = color;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY SPECULAR COLOR.
            const std::string SPECULAR_COLOR_INDICATOR = "Ks";
            bool is_specular_color_line = (SPECULAR_COLOR_INDICATOR == first_line_component);
            if (is_specular_color_line)
            {
                // ENSURE THERE ARE ENOUGH COLOR COMPONENTS.
                bool enough_color_components = (current_line_component_count >= INDICATOR_PLUS_COLOR_COMPONENT_COUNT);
                ASSERT_THEN_IF_NOT(enough_color_components)
                {
                    // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                    continue;
                }

                // PARSE THE SPECULAR COLOR.
                Color color = Color::BLACK;
                const std::string& red_string = current_line_components.at(RED_INDEX);
                color.Red = std::stof(red_string);
                const std::string& green_string = current_line_components.at(GREEN_INDEX);
                color.Green = std::stof(green_string);
                const std::string& blue_string = current_line_components.at(BLUE_INDEX);
                color.Blue = std::stof(blue_string);

                current_material->SpecularColor = color;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY SPECULAR EXPONENT.
            const std::string SPECULAR_EXPONENT_INDICATOR = "Ns";
            bool is_specular_exponent_line = (SPECULAR_EXPONENT_INDICATOR == first_line_component);
            if (is_specular_exponent_line)
            {
                // PARSE THE SPECULAR EXPONENT.
                const std::string& specular_exponent_string = current_line_components.back();
                current_material->SpecularPower = std::stof(specular_exponent_string);

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY EMISSIVE COLOR.
            const std::string EMISSIVE_COLOR_INDICATOR = "Ke";
            bool is_emissive_color_line = (EMISSIVE_COLOR_INDICATOR == first_line_component);
            if (is_emissive_color_line)
            {
                // ENSURE THERE ARE ENOUGH COLOR COMPONENTS.
                bool enough_color_components = (current_line_component_count >= INDICATOR_PLUS_COLOR_COMPONENT_COUNT);
                ASSERT_THEN_IF_NOT(enough_color_components)
                {
                    // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                    continue;
                }

                // PARSE THE EMISSIVE COLOR.
                Color color = Color::BLACK;
                const std::string& red_string = current_line_components.at(RED_INDEX);
                color.Red = std::stof(red_string);
                const std::string& green_string = current_line_components.at(GREEN_INDEX);
                color.Green = std::stof(green_string);
                const std::string& blue_string = current_line_components.at(BLUE_INDEX);
                color.Blue = std::stof(blue_string);

                current_material->EmissiveColor = color;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY OPAQUENESS LEVEL.
            const std::string DISSOLVED_LEVEL_INDICATOR = "d";
            bool is_dissolved_level_line = (DISSOLVED_LEVEL_INDICATOR == first_line_component);
            if (is_dissolved_level_line)
            {
                // UPDATE THE ALPHA FOR TRANSPARENCY FOR ALL COLORS.
                const std::string& alpha_string = current_line_components.back();
                float alpha = std::stof(alpha_string);

                current_material->AmbientColor.Alpha = alpha;
                current_material->DiffuseColor.Alpha = alpha;
                current_material->SpecularColor.Alpha = alpha;
                current_material->EmissiveColor.Alpha = alpha;
                
                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY TRANSPARENCY LEVEL.
            const std::string TRANSPARENCY_LEVEL_INDICATOR = "Tr";
            bool is_transparency_level_line = (TRANSPARENCY_LEVEL_INDICATOR == first_line_component);
            if (is_transparency_level_line)
            {
                // UPDATE THE ALPHA FOR TRANSPARENCY FOR ALL COLORS.
                // This value is the opposite of the dissolved opaqueness level above.
                const std::string& transparency_string = current_line_components.back();
                float alpha = GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT - std::stof(transparency_string);

                current_material->AmbientColor.Alpha = alpha;
                current_material->DiffuseColor.Alpha = alpha;
                current_material->SpecularColor.Alpha = alpha;
                current_material->EmissiveColor.Alpha = alpha;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY TRANSMISSION FILTER COLOR.
            const std::string TRANSMISSION_FILTER_COLOR_INDICATOR = "Tf";
            bool is_transmission_filter_color = (TRANSMISSION_FILTER_COLOR_INDICATOR == first_line_component);
            if (is_transmission_filter_color)
            {
                /// @todo   Not sure how to handle this.
                assert("WavefrontMaterial - Tf not yet supported.");

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY OPTICAL DENSITY/INDEX OF REFRACTION.
            const std::string REFRACTION_INDEX_INDICATOR = "Ni";
            bool is_refraction_index = (REFRACTION_INDEX_INDICATOR == first_line_component);
            if (is_refraction_index)
            {
                /// @todo   Not sure how to handle this.
                assert("WavefrontMaterial - Ni not yet supported.");

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN THE ILLUMINATION MODEL.
            const std::string ILLUMINATION_MODEL_INDICATOR = "illum";
            bool is_illumination_model_line = (ILLUMINATION_MODEL_INDICATOR == first_line_component);
            if (is_illumination_model_line)
            {
                /// @todo   How important is this to support?
                assert("WavefrontMaterial - illum not yet supported.");

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }

            // READ IN ANY TEXTURE.
            const std::string TEXTURE_MAP_INDICATOR = "map";
            bool is_texture_map_line = first_line_component.starts_with(TEXTURE_MAP_INDICATOR);
            if (is_texture_map_line)
            {
                /// @todo Handle multiple texture maps? - including bump, disp, decal, refl

                const std::string& texture_filename = current_line_components.back();
                std::filesystem::path material_directory_path = mtl_filepath.parent_path();
                std::filesystem::path texture_filepath = material_directory_path / texture_filename;

                /// @todo   Error-handling...different file types and texture formats!
                current_material->Texture = GRAPHICS::IMAGES::Bitmap::LoadPng(texture_filepath, ColorFormat::RGBA);
                current_material->Shading = ShadingType::TEXTURED;

                // CONTINUE PROCESSING OTHER LINES IN THE FILE.
                continue;
            }
        }

        // STORE ANY FINAL MATERIAL.
        // It may not already be added.
        if (current_material)
        {
            // STORE THE PRIOR MATERIAL NOW THAT IT IS COMPLETE.
            materials_by_name.emplace(std::make_pair(current_material->Name, current_material));
        }

        // RETURN THE MATERIALS.
        return materials_by_name;
    }
}
