#include <filesystem>
#include <fstream>
#include <catch.hpp>
#include "Graphics/Modeling/WavefrontObjectModel.h"

TEST_CASE("No model is loaded from a file with only whitespace.", "[WavefrontObjectModel][Load]")
{
    // CREATE A FILE WITH ONLY WHITESPACE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    model_file << " \n\t\n";
    model_file.close();

    // VERIFY THAT NO MODEL CAN BE LOADED FROM THE FILE.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::Load(MODEL_FILEPATH);
    REQUIRE_FALSE(model.has_value());

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}

TEST_CASE("No model is loaded from a file with only comments.", "[WavefrontObjectModel][Load]")
{
    // CREATE A FILE WITH ONLY COMMENTS.
    const std::filesystem::path MODEL_FILEPATH = "TestModel.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    model_file 
        << "# Comment line 1\n"
        << "# Comment line 2\n";
    model_file.close();

    // VERIFY THAT NO MODEL CAN BE LOADED FROM THE FILE.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::Load(MODEL_FILEPATH);
    REQUIRE_FALSE(model.has_value());

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}

TEST_CASE("A basic square can be loaded.", "[WavefrontObjectModel][Load]")
{
    // CREATE A WAVEFRONT OBJECT FILE FOR A SQUARE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel_Square.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    // This is based on an example from https://fegemo.github.io/cefet-cg/attachments/obj-spec.pdf.
    model_file
        << R"OBJ(
v 0.000000 2.000000 0.000000
v 0.000000 0.000000 0.000000
v 2.000000 0.000000 0.000000
v 2.000000 2.000000 0.000000
f 1 2 3 4
        )OBJ";
    model_file.close();

    // LOAD THE MODEL.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::Load(MODEL_FILEPATH);

    // VERIFY THE MODEL WAS PROPERLY LOADED.
    REQUIRE(model);
    
    // Only a single default mesh should exist.
    REQUIRE(1 == model->MeshesByName.size());
    const std::string EXPECTED_MESH_NAME = "Default";
    const GRAPHICS::Mesh& mesh = model->MeshesByName[EXPECTED_MESH_NAME];
    REQUIRE(EXPECTED_MESH_NAME == mesh.Name);

    // The mesh should be visible.
    REQUIRE(mesh.Visible);

    // The mesh should have the proper triangles.
    GRAPHICS::GEOMETRY::Triangle first_triangle;
    first_triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 0.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 0.0f),
        },
    };
    GRAPHICS::GEOMETRY::Triangle second_triangle;
    second_triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 2.0f, 0.0f),
        },
    };
    std::vector<GRAPHICS::GEOMETRY::Triangle> expected_triangles =
    {
        first_triangle,
        second_triangle
    };
    REQUIRE(expected_triangles == mesh.Triangles);

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}

TEST_CASE("A basic cube can be loaded.", "[WavefrontObjectModel][Load]")
{
    // CREATE A WAVEFRONT OBJECT FILE FOR A CUBE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel_Cube.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    // This is based on an example from https://fegemo.github.io/cefet-cg/attachments/obj-spec.pdf.
    model_file
        << R"OBJ(
v 0.000000 2.000000 2.000000
v 0.000000 0.000000 2.000000
v 2.000000 0.000000 2.000000
v 2.000000 2.000000 2.000000
v 0.000000 2.000000 0.000000
v 0.000000 0.000000 0.000000
v 2.000000 0.000000 0.000000
v 2.000000 2.000000 0.000000
f 1 2 3 4
f 8 7 6 5
f 4 3 7 8
f 5 1 4 8
f 5 6 2 1
f 2 6 7 3
        )OBJ";
    model_file.close();

    // LOAD THE MODEL.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::Load(MODEL_FILEPATH);

    // VERIFY THE MODEL WAS PROPERLY LOADED.
    REQUIRE(model);

    // Only a single default mesh should exist.
    REQUIRE(1 == model->MeshesByName.size());
    const std::string EXPECTED_MESH_NAME = "Default";
    const GRAPHICS::Mesh& mesh = model->MeshesByName[EXPECTED_MESH_NAME];
    REQUIRE(EXPECTED_MESH_NAME == mesh.Name);

    // The mesh should be visible.
    REQUIRE(mesh.Visible);

    // The mesh should have the proper triangles.
    std::vector<GRAPHICS::VertexWithAttributes> vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 2.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 0.0f, 2.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 2.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 2.0f, 2.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 0.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 2.0f, 0.0f),
        },
    };

    // 1st quad.
    GRAPHICS::GEOMETRY::Triangle triangle_1;
    triangle_1.Vertices = { vertices[0], vertices[1], vertices[2] };
    GRAPHICS::GEOMETRY::Triangle triangle_2;
    triangle_2.Vertices = { vertices[0], vertices[2], vertices[3] };
    
    // 2nd quad.
    GRAPHICS::GEOMETRY::Triangle triangle_3;
    triangle_3.Vertices = { vertices[7], vertices[6], vertices[5] };
    GRAPHICS::GEOMETRY::Triangle triangle_4;
    triangle_4.Vertices = { vertices[7], vertices[5], vertices[4] };

    // 3rd quad.
    GRAPHICS::GEOMETRY::Triangle triangle_5;
    triangle_5.Vertices = { vertices[3], vertices[2], vertices[6] };
    GRAPHICS::GEOMETRY::Triangle triangle_6;
    triangle_6.Vertices = { vertices[3], vertices[6], vertices[7] };

    // 4th quad.
    GRAPHICS::GEOMETRY::Triangle triangle_7;
    triangle_7.Vertices = { vertices[4], vertices[0], vertices[3] };
    GRAPHICS::GEOMETRY::Triangle triangle_8;
    triangle_8.Vertices = { vertices[4], vertices[3], vertices[7] };

    // 5th quad.
    GRAPHICS::GEOMETRY::Triangle triangle_9;
    triangle_9.Vertices = { vertices[4], vertices[5], vertices[1] };
    GRAPHICS::GEOMETRY::Triangle triangle_10;
    triangle_10.Vertices = { vertices[4], vertices[1], vertices[0] };

    // 6th quad.
    GRAPHICS::GEOMETRY::Triangle triangle_11;
    triangle_11.Vertices = { vertices[1], vertices[5], vertices[6] };
    GRAPHICS::GEOMETRY::Triangle triangle_12;
    triangle_12.Vertices = { vertices[1], vertices[6], vertices[2] };

    std::vector<GRAPHICS::GEOMETRY::Triangle> expected_triangles =
    {
        triangle_1,
        triangle_2,
        triangle_3,
        triangle_4,
        triangle_5,
        triangle_6,
        triangle_7,
        triangle_8,
        triangle_9,
        triangle_10,
        triangle_11,
        triangle_12,
    };
    REQUIRE(expected_triangles == mesh.Triangles);

    // DELETE THE MODEL FILE.
    std::filesystem::remove(MODEL_FILEPATH);
}

TEST_CASE("A texture-mapped square can be loaded.", "[WavefrontObjectModel][Load]")
{
    // CREATE A MATERIAL LIBRARY FILE.
    const std::filesystem::path MATERIAL_LIBRARY_FILEPATH = "TestMaterialLibrary_TestTexture.mtl";
    std::ofstream material_library_file(MATERIAL_LIBRARY_FILEPATH);
    material_library_file
        << R"MTL(
newmtl test_texture
map_Kd ../../Graphics/testing/data/test_texture.png
)MTL";
    material_library_file.close();

    // CREATE A WAVEFRONT OBJECT FILE FOR A SQUARE.
    const std::filesystem::path MODEL_FILEPATH = "TestModel_TexturedSquare.obj";
    std::ofstream model_file(MODEL_FILEPATH);
    // This is based on an example from https://fegemo.github.io/cefet-cg/attachments/obj-spec.pdf.
    model_file
        << R"OBJ(
mtllib TestMaterialLibrary_TestTexture.mtl

v 0.000000 2.000000 0.000000
v 0.000000 0.000000 0.000000
v 2.000000 0.000000 0.000000
v 2.000000 2.000000 0.000000
vt 0.000000 1.000000
vt 0.000000 0.000000
vt 1.000000 0.000000
vt 1.000000 1.000000

usemtl test_texture
f 1/1 2/2 3/3 4/4
        )OBJ";
    model_file.close();

    // LOAD THE MODEL.
    std::optional<GRAPHICS::MODELING::Model> model = GRAPHICS::MODELING::WavefrontObjectModel::Load(MODEL_FILEPATH);

    // VERIFY THE MODEL WAS PROPERLY LOADED.
    REQUIRE(model);

    // Only a single default mesh should exist.
    REQUIRE(1 == model->MeshesByName.size());
    const std::string EXPECTED_MESH_NAME = "Default";
    const GRAPHICS::Mesh& mesh = model->MeshesByName[EXPECTED_MESH_NAME];
    REQUIRE(EXPECTED_MESH_NAME == mesh.Name);

    // The mesh should be visible.
    REQUIRE(mesh.Visible);

    // The mesh should have the proper triangles.
    // For exact equality comparison, the texture must be copied over.
    // For simplicity, verification of the contents of the texture is excluded from this test but would be good to test later.
    const std::shared_ptr<GRAPHICS::Material>& material = mesh.Triangles.front().Material;
    REQUIRE(material);

    GRAPHICS::GEOMETRY::Triangle first_triangle;
    first_triangle.Material = material;

    // Texture coordinates in the file have 1 at the top, but the texture pixels have 1 at the bottom.
    constexpr float FLIP_VERTICAL_TEXTURE_COORDINATES = 1.0f;
    first_triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(0.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 1.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 0.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(0.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(1.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 0.0f),
        },
    };
    GRAPHICS::GEOMETRY::Triangle second_triangle;
    second_triangle.Material = material;
    second_triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 2.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(0.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 1.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 0.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(1.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(2.0f, 2.0f, 0.0f),
            .TextureCoordinates = MATH::Vector2f(1.0f, FLIP_VERTICAL_TEXTURE_COORDINATES - 1.0f),
        },
    };
    std::vector<GRAPHICS::GEOMETRY::Triangle> expected_triangles =
    {
        first_triangle,
        second_triangle
    };
    REQUIRE(expected_triangles == mesh.Triangles);

    // DELETE THE MODEL FILES.
    std::filesystem::remove(MODEL_FILEPATH);
    std::filesystem::remove(MATERIAL_LIBRARY_FILEPATH);
}
