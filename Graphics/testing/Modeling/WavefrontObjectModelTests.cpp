#include <filesystem>
#include <fstream>
#include <catch.hpp>
#include "Graphics/Modeling/WavefrontObjectModel.h"

TEST_CASE("No model is loaded from a file with only whitespace.", "[WavefrontObjectModelTests][Load]")
{
    // CREATE A FILE WITH ONLY WHITESPACE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    model_file << " \n\t\n";
    model_file.close();

    // VERIFY THAT NO MODEL CAN BE LOADED FROM THE FILE.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::LoadNew(MODEL_FILEPATH);
    REQUIRE_FALSE(model.has_value());

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}

TEST_CASE("No model is loaded from a file with only comments.", "[WavefrontObjectModelTests][Load]")
{
    // CREATE AN EMPTY FILE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    model_file 
        << "# Comment line 1\n"
        << "# Comment line 2\n";
    model_file.close();

    // VERIFY THAT NO MODEL CAN BE LOADED FROM THE FILE.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::LoadNew(MODEL_FILEPATH);
    REQUIRE_FALSE(model.has_value());

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}


