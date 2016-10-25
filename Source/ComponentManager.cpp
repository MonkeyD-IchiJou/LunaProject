#include "ComponentManager.h"
#include "Entity.h"
#include "WinNative.h"
#include "TextureResources.h"
#include "Font.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

namespace luna
{
	int RenderingInfo::totalcounter = 0;

	ComponentManager::ComponentManager(const int MAX_COMPONENTS)
	{
		/* Init and resize all the components */
		m_transformationContainer.m_components.resize(MAX_COMPONENTS);
		m_basicmeshContainer.m_components.resize(MAX_COMPONENTS);
		m_fontContainer.m_components.resize(MAX_COMPONENTS);
	}

	ComponentManager::~ComponentManager()
	{
		/* delete all the components */
	}

	void ComponentManager::Update()
	{
		m_transformationContainer.Update();
		m_basicmeshContainer.Update();
		m_fontContainer.Update();
	}

	void ComponentManager::GetRenderingData(std::vector<RenderingInfo>& renderinfos)
	{
		auto totalbasicmeshsize = m_basicmeshContainer.m_components.size();

		// make sure there are components for it
		if (totalbasicmeshsize > 0)
		{
			// rmb to set it back to 0 if restarting
			RenderingInfo::totalcounter = 0;

			auto totalrenderdatasize = renderinfos.size();

			if(totalrenderdatasize > 0)
				renderinfos.clear();

			// reserve the datas
			renderinfos.reserve(m_transformationContainer.m_components.size());
			RenderingInfo rd{};

			// store the first rendering data
			StoreNewRenderingData_(&m_basicmeshContainer.m_components[0], renderinfos, rd);

			// basic mesh components checking for rendering info
			// compare with the rest of them
			for (int meshCount = 1; meshCount < totalbasicmeshsize; ++meshCount)
			{
				auto &mesh = m_basicmeshContainer.m_components[meshCount];

				if (mesh.GetOwner() != nullptr && mesh.isActive() == true)
				{
					// if the component has owner and is active
					// group those who have similar texture and models id
					// and then store the instance data

					bool found = false;

					// check whether got similar texture and model id or not
					for (int i = 0; i < renderinfos.size(); ++i)
					{
						auto &renderdata = renderinfos[i];

						if (mesh.meshID == renderdata.modelID &&
							mesh.material.textureID == renderdata.textureID)
						{
							// if same mesh and same texture, only increase the total render
							StoreRenderingData_(&mesh, renderdata);
							found = true;
							break;
						}
					}

					if (!found)
					{
						// if it has a new set of mesh and texture, then store a new one for it
						StoreNewRenderingData_(&mesh, renderinfos, rd);
					}
				}
			}
		}
	}

	void ComponentManager::GetFontInstanceData(std::vector<FontInstanceData>& fontinstancedatas)
	{
		auto totalsize = fontinstancedatas.size();

		if (totalsize > 0)
			fontinstancedatas.clear();

		fontinstancedatas.reserve(256);

		// handles
		WinNative* win = WinNative::getInstance();
		auto texrsc = TextureResources::getInstance();

		// choose the font info 
		auto font = texrsc->Fonts[FONT_EVA];

		// scale with screen size
		glm::mat4 proj = glm::ortho(0.f, (float)win->getWinSizeX(), (float)win->getWinSizeY(), 0.f);
		glm::vec2 cursor = {};

		for (auto & fontcomponent : m_fontContainer.m_components)
		{
			if (fontcomponent.GetOwner() != nullptr && fontcomponent.isActive() == true)
			{
				const auto &str = fontcomponent.text;
				const auto &material = fontcomponent.material;
				cursor = {};

				for (int i = 0; i < str.size(); ++i)
				{
					fontinstancedatas.push_back(FontInstanceData());
					auto& fid = fontinstancedatas[i];

					const vulkanchar& vc = font->vulkanChars[str[i]];

					// calc the top left hand corner position
					glm::vec2 toplefthandcorner = glm::vec2(cursor.x + vc.xoffset, cursor.y + vc.yoffset);

					// then find the correct position relative with the left hand corner
					glm::vec2 position = { toplefthandcorner.x + vc.halfsize.x, toplefthandcorner.y - vc.halfsize.y };

					glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(position, 0.f));
					glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(vc.size, 1.f));

					fid.transformation = proj * fontcomponent.GetOwner()->transformation->GetModel() * t * s;
					fid.uv[0] = vc.uv[0];
					fid.uv[1] = vc.uv[1];
					fid.uv[2] = vc.uv[2];
					fid.uv[3] = vc.uv[3];

					// next cursor pointing at
					cursor.x += vc.xadvance;

					// materials set up 
					fid.fontMaterials[0] = material.color; // color
					fid.fontMaterials[1] = glm::vec4(material.width, material.edge, material.borderwidth, material.borderedge); // properties
					fid.fontMaterials[2] = material.outlinecolor; // outline color
					fid.fontMaterials[3] = glm::vec4(material.borderOffset, 0.f, 0.f); // border offset
				}
			}
		}
	}

	void ComponentManager::StoreNewRenderingData_(BasicMeshComponent* mesh, std::vector<RenderingInfo>& renderdatas, RenderingInfo& rd)
	{
		// store the renderdata
		rd.modelID = mesh->meshID;
		rd.textureID = mesh->material.textureID;
		StoreRenderingData_(mesh, rd);

		renderdatas.push_back(rd);
		rd = {};
	}

	void ComponentManager::StoreRenderingData_(BasicMeshComponent * mesh, RenderingInfo& rd)
	{
		rd.instancedatas.push_back(mesh);
		RenderingInfo::totalcounter++;
	}
}
