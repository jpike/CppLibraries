// Older OpenGL functions must be undefined to avoid missing symbols.
#undef GL_VERSION_1_1
#include <gl3w/GL/gl3w.h>
#include "Graphics/OpenGL/VertexBuffer.h"

namespace GRAPHICS::OPEN_GL
{
    /// Frees vertex buffer resources upon destruction.
    VertexBuffer::~VertexBuffer()
    {
        constexpr GLsizei ONE_VERTEX_BUFFER = 1;
        glDeleteBuffers(ONE_VERTEX_BUFFER, &BufferId);

        constexpr GLsizei ONE_VERTEX_ARRAY = 1;
        glDeleteVertexArrays(ONE_VERTEX_ARRAY, &ArrayId);
    }

    /// Fills this vertex buffer with the data in the model.
    /// @param[in]  model - The model whose vertices to place in the buffer.
    void VertexBuffer::Fill(const MODELING::Model& model)
    {
        // ALLOCATE A VERTEX ARRAY/BUFFER.
        constexpr GLsizei ONE_VERTEX_ARRAY = 1;
        glGenVertexArrays(ONE_VERTEX_ARRAY, &ArrayId);
        glBindVertexArray(ArrayId);

        constexpr GLsizei ONE_VERTEX_BUFFER = 1;
        glGenBuffers(ONE_VERTEX_BUFFER, &BufferId);

        // FILL THE BUFFER WITH VERTEX DATA FROM ALL MESHES.
        std::vector<float> vertex_attribute_values;
        for (const auto& [mesh_name, mesh] : model.MeshesByName)
        {
            // FILL THE BUFFER WITH VERTEX DATA FROM ALL TRIANGLES.
            for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
            {
                // GET ALL VERTEX DATA FOR THE CURRENT TRIANGLE.
                MATH::Vector3f surface_normal = triangle.SurfaceNormal();
                for (const VertexWithAttributes& vertex : triangle.Vertices)
                {
                    // SET THE VERTEX POSITION.
                    vertex_attribute_values.emplace_back(vertex.Position.X);
                    vertex_attribute_values.emplace_back(vertex.Position.Y);
                    vertex_attribute_values.emplace_back(vertex.Position.Z);
                    constexpr float HOMOGENEOUS_VERTEX_W = 1.0f;
                    vertex_attribute_values.emplace_back(HOMOGENEOUS_VERTEX_W);

                    // SET THE VERTEX COLOR.
                    vertex_attribute_values.emplace_back(vertex.Color.Red);
                    vertex_attribute_values.emplace_back(vertex.Color.Green);
                    vertex_attribute_values.emplace_back(vertex.Color.Blue);
                    vertex_attribute_values.emplace_back(vertex.Color.Alpha);

                    // SET VERTEX TEXTURE COORDINATES.
                    vertex_attribute_values.emplace_back(vertex.TextureCoordinates.X);
                    vertex_attribute_values.emplace_back(vertex.TextureCoordinates.Y);

                    // SET THE VERTEX NORMAL.
                    vertex_attribute_values.emplace_back(surface_normal.X);
                    vertex_attribute_values.emplace_back(surface_normal.Y);
                    vertex_attribute_values.emplace_back(surface_normal.Z);
                }
            }
        }

        // FILL THE BUFFER WITH THE VERTEX ATTRIBUTE DATA.
        GLsizeiptr vertex_data_size_in_bytes = sizeof(float) * vertex_attribute_values.size();
        glBindBuffer(GL_ARRAY_BUFFER, BufferId);
        glBufferData(GL_ARRAY_BUFFER, vertex_data_size_in_bytes, vertex_attribute_values.data(), GL_STATIC_DRAW);
    }
}
