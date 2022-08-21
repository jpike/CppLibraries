#include <vector>
#include "ErrorHandling/Asserts.h"
#include "Graphics/DirectX/VertexInputBuffer.h"

namespace GRAPHICS::DIRECT_X
{
    /// Fills a vertex buffer with the data in the model.
    /// @param[in]  model - The model whose vertices to place in the buffer.
    /// @param[in,out]  device - The device in which to allocate a vertex buffer.
    /// @return The vertex buffer, if successfully filled; null if an error occurs.
    ID3D11Buffer* VertexInputBuffer::Fill(const MODELING::Model& model, ID3D11Device& device)
    {
        // GET VERTEX DATA FOR ALL MESHES.
        std::vector<VertexInputBuffer> vertices;
        for (const auto& [mesh_name, mesh] : model.MeshesByName)
        {
            // GET VERTEX DATA FOR EACH TRIANGLE.
            for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
            {
                // CALCULATE THE TRIANGLE'S SURFACE NORMAL.
                MATH::Vector3f surface_normal = triangle.SurfaceNormal();

                // POPULATE THE VERTEX INPUT BUFFER WITH ALL OF THE TRIANGLE'S VERTICES.
                constexpr float HOMOGENOUS_POSITION_W_COORDINATE = 1.0f;
                vertices.push_back(
                    VertexInputBuffer
                    {
                        .Position = DirectX::XMFLOAT4(triangle.Vertices[0].Position.X, triangle.Vertices[0].Position.Y, triangle.Vertices[0].Position.Z, HOMOGENOUS_POSITION_W_COORDINATE),
                        .Color = DirectX::XMFLOAT4(
                            triangle.Vertices[0].Color.Red,
                            triangle.Vertices[0].Color.Green,
                            triangle.Vertices[0].Color.Blue,
                            triangle.Vertices[0].Color.Alpha),
                        .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, 1.0f),
                        .TextureCoordinates = DirectX::XMFLOAT2(
                            triangle.Vertices[0].TextureCoordinates.X,
                            triangle.Vertices[0].TextureCoordinates.Y),
                    });

                vertices.push_back(
                    VertexInputBuffer
                    {
                        .Position = DirectX::XMFLOAT4(triangle.Vertices[1].Position.X, triangle.Vertices[1].Position.Y, triangle.Vertices[1].Position.Z, HOMOGENOUS_POSITION_W_COORDINATE),
                        .Color = DirectX::XMFLOAT4(
                            triangle.Vertices[1].Color.Red,
                            triangle.Vertices[1].Color.Green,
                            triangle.Vertices[1].Color.Blue,
                            triangle.Vertices[1].Color.Alpha),
                        .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, 1.0f),
                        .TextureCoordinates = DirectX::XMFLOAT2(
                            triangle.Vertices[1].TextureCoordinates.X,
                            triangle.Vertices[1].TextureCoordinates.Y),
                    });

                vertices.push_back(
                    VertexInputBuffer
                    {
                        .Position = DirectX::XMFLOAT4(triangle.Vertices[2].Position.X, triangle.Vertices[2].Position.Y, triangle.Vertices[2].Position.Z, HOMOGENOUS_POSITION_W_COORDINATE),
                        .Color = DirectX::XMFLOAT4(
                            triangle.Vertices[2].Color.Red,
                            triangle.Vertices[2].Color.Green,
                            triangle.Vertices[2].Color.Blue,
                            triangle.Vertices[2].Color.Alpha),
                        .Normal = DirectX::XMFLOAT4(surface_normal.X, surface_normal.Y, surface_normal.Z, HOMOGENOUS_POSITION_W_COORDINATE),
                        .TextureCoordinates = DirectX::XMFLOAT2(
                            triangle.Vertices[2].TextureCoordinates.X,
                            triangle.Vertices[2].TextureCoordinates.Y),
                    });
            }
        }

        // DESCRIBE THE VERTEX BUFFER.
        UINT vertex_count = static_cast<UINT>(vertices.size());
        D3D11_BUFFER_DESC vertex_buffer_description
        {
            .ByteWidth = sizeof(VertexInputBuffer) * vertex_count,
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
            // No other special flags are needed.
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0,
        };

        D3D11_SUBRESOURCE_DATA vertex_data
        {
            .pSysMem = vertices.data(),
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0,
        };
        ID3D11Buffer* vertex_buffer = nullptr;
        HRESULT create_vertex_buffer_result = device.CreateBuffer(&vertex_buffer_description, &vertex_data, &vertex_buffer);
        ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(create_vertex_buffer_result)
        {
            // INDICATE THAT THE VERTEX BUFFER COULD NOT BE POPULATED.
            return nullptr;
        }

        // RETURN THE VERTEX BUFFER.
        return vertex_buffer;
    }
}
