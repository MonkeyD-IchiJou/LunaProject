#include "Model.h"
#include "BasicMesh.h"
#include "DebugLog.h"

#include <fstream>

namespace luna
{
	Model::Model(ePRIMITIVE_MESH meshid)
	{
		m_totalmeshes = 1;

		switch (meshid)
		{
		case PRIMITIVE_QUAD:
			{
				m_meshes.resize(m_totalmeshes);

				// in vulkan space pls .. upside down de
				const std::vector<Vertex> vertices = {
					// front face
					{ { -1.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
					{ { -1.f, 1.f, 0.f },	{ 0.0f, 0.0f, 1.0f },  { 0.0f, 1.0f } },
					{ { 1.f, 1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
					{ { 1.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } }
				};

				const std::vector<uint32_t> indices = {
					0, 1, 2, 2, 3, 0
				};

				m_meshes[0] = new BasicMesh(vertices, indices);
			}

			break;

		case PRIMITIVE_CUBE:
			{
				m_meshes.resize(m_totalmeshes);

				const std::vector<Vertex> vertices = {

					// front face
					{ { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f },	{ 0.0f, 0.0f } },
					{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
					{ { 0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f },	{ 1.0f, 1.0f } },
					{ { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f },	{ 1.0f, 0.0f } },

					// right face
					{ { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f },   { 0.0f, 0.0f } },
					{ { 0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f },	{ 0.0f, 1.0f } },
					{ { 0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
					{ { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f },	{ 1.0f, 0.0f } },


					// back face
					{ { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f },   { 0.0f, 0.0f } },
					{ { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f },  { 0.0f, 1.0f } },
					{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
					{ { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f },  { 1.0f, 0.0f } },


					// left face
					{ { -0.5f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f } },
					{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
					{ { -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f } },
					{ { -0.5f, 0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f },   { 1.0f, 0.0f } },


					// upper face
					{ { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f },  { 0.0f, 0.0f } },
					{ { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, 	 { 0.0f, 1.0f } },
					{ { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, 	 { 1.0f, 1.0f } },
					{ { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, 	 { 1.0f, 0.0f } },

					// upper face
					{ { 0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f },  { 0.0f, 0.0f } },
					{ { 0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f },   { 0.0f, 1.0f } },
					{ { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f },  { 1.0f, 1.0f } },
					{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
				};

				const std::vector<uint32_t> indices = {
					0, 1, 2, 2, 3, 0, // front 
					4, 5, 6, 6, 7, 4, // right 
					8, 9, 10, 10, 11, 8, // back
					12, 13, 14, 14, 15, 12, // left
					16, 17, 18, 18, 19, 16, // up
					20, 21, 22, 22, 23, 20, // down
				};

				m_meshes[0] = new BasicMesh(vertices, indices);
			}
			break;

		default:
			DebugLog::throwEx("no such primitive mesh");
			break;
		}
	}

	int Model::getInteger_(const std::string& data, size_t& currentLine, const char& delim)
	{
		size_t offset = data.find(delim, currentLine);
		std::string ans = data.substr(currentLine, offset - currentLine);

		currentLine = offset + 1;

		return std::stoi(ans);
	}

	float Model::getFloat_(const std::string& data, size_t& currentLine, const char& delim)
	{
		size_t offset = data.find(delim, currentLine);
		std::string ans = data.substr(currentLine, offset - currentLine);

		currentLine = offset + 1;

		return std::stof(ans);
	}

	Model::Model(std::string path)
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR

		// load the lrl files
		// open my lrl file
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			DebugLog::throwEx("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// seek back to the beggining of the file and read all of the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		// begin to store the information
		std::string data = buffer.data();
		size_t currentLine = 0;

		while (currentLine < buffer.size())
		{
			int totalVertices = getInteger_(data, currentLine, ' ');
			int totalIndices = getInteger_(data, currentLine, '\n');

			std::vector<luna::Vertex> vertices(totalVertices);
			std::vector<uint32_t> indices(totalIndices);

			for (int i = 0; i < totalVertices; ++i)
			{
				glm::vec3& pos = vertices[i].pos;
				pos.x = getFloat_(data, currentLine, ' ');
				pos.y = getFloat_(data, currentLine, ' ');
				pos.z = getFloat_(data, currentLine, ' ');

				glm::vec3& normal = vertices[i].normal;
				normal.x = getFloat_(data, currentLine, ' ');
				normal.y = getFloat_(data, currentLine, ' ');
				normal.z = getFloat_(data, currentLine, ' ');

				glm::vec2& tex = vertices[i].texCoord;
				tex.x = getFloat_(data, currentLine, ' ');
				tex.y = getFloat_(data, currentLine, '\n');
			}

			for (int i = 0; i < totalIndices - 1; ++i)
			{
				indices[i] = ((uint32_t)getInteger_(data, currentLine, ' '));
			}
			indices[totalIndices - 1] = ((uint32_t)getInteger_(data, currentLine, '\n'));

			this->m_meshes.push_back(new BasicMesh(vertices, indices));
			m_totalmeshes++;
		}

#endif
	}

	void Model::Draw(const VkCommandBuffer & commandbuffer)
	{
		for (auto &mesh : m_meshes)
		{
			mesh->Draw(commandbuffer);
		}
	}

	void Model::DrawInstanced(const VkCommandBuffer & commandbuffer, const uint32_t& instancecount)
	{
		for (auto &mesh : m_meshes)
		{
			mesh->DrawInstanced(commandbuffer, instancecount);
		}
	}

	void Model::MapToDeviceMemory(const VkDevice & logicaldevice, const VkDeviceMemory & devicememory)
	{
		for (auto &mesh : m_meshes)
		{
			mesh->MapToDeviceMemory(logicaldevice, devicememory);
		}
	}

	void Model::setMainBuffer(const VkBuffer & buffer)
	{
		for (auto &mesh : m_meshes)
		{
			mesh->setMainBuffer(buffer);
		}
	}

	Model::~Model()
	{
		for (auto &i : m_meshes)
		{
			if (i != nullptr)
			{
				delete i;
			}
		}

		m_meshes.clear();
	}
}
