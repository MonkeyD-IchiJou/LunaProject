#include "Model.h"
#include "BasicMesh.h"
#include "DebugLog.h"

#include <fstream>

namespace luna
{
	Model::Model(ePRIMITIVE_MESH meshid)
	{
		switch (meshid)
		{
		case PRIMITIVE_QUAD:
			{
				m_meshes.resize(1);

				const std::vector<Vertex> vertices = {
					{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
					{ { 1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
					{ { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
					{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
				};

				const std::vector<uint32_t> indices = {
					0, 1, 2, 2, 3, 0
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
		}
	}

	void Model::Draw(const VkCommandBuffer & commandbuffer)
	{
		for (auto &mesh : m_meshes)
		{
			mesh->Draw(commandbuffer);
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