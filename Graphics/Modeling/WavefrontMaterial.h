#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include "Graphics/Material.h"
#include <Memory/Pointers.h>

namespace GRAPHICS::MODELING
{
    /// A material in the .mtl (Material Template Library) format.
    /// See https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library.
    class WavefrontMaterial
    {
    public:
        static std::unordered_map<std::string, MEMORY::NonNullSharedPointer<Material>> Load(const std::filesystem::path& mtl_filepath);
    };
}
