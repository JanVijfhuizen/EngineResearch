#include "pch.h"
#include "Graphics/ObjLoader.h"
#include "Jlb/FileLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// Source: https://vkguide.dev/docs/chapter-3/obj_loading/
// I was too lazy to write my own obj loader.

namespace je::obj
{
	void Load(Arena& arena, const StringView& path, const Arena::Scope& scope, Array<vk::Vertex>& outVertices, Array<vk::Vertex::Index>& outIndices, const float scale)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		
		std::string warn;
		std::string err;

		//load the OBJ file
		LoadObj(&attrib, &shapes, &materials, &warn, &err, path, nullptr);
		
		assert(err.empty());

		std::vector<vk::Vertex> vertices{};
		
		for (const auto& shape : shapes)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			auto& faceVertices = shape.mesh.num_face_vertices;
			for (const auto& faceVertex : faceVertices)
			{
				assert(faceVertex == 3);

				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++) 
				{
					// access to vertex
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

					// Vertex position.
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					// Vertex normal.
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					// Vertex texture coordinates.
					tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

					// Copy it into our vertex
					vk::Vertex vert;
					vert.position.x = vx;
					vert.position.y = vy;
					vert.position.z = vz;
					vert.position *= scale;

					vert.normal.x = nx;
					vert.normal.y = ny;
					vert.normal.z = nz;

					vert.textureCoordinates.x = tx;
					vert.textureCoordinates.y = ty;

					vertices.push_back(vert);
				}
				index_offset += 3;
			}
		}

		outVertices = {arena, vertices.size()};
		outIndices = { arena, outVertices.GetLength() };

		for (size_t i = 0; i < outVertices.GetLength(); ++i)
		{
			outVertices[i] = vertices[i];
			outIndices[i] = static_cast<uint16_t>(i);
		}
	}
}
