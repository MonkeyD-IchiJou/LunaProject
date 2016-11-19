#include "ComponentManager.h"
#include "Entity.h"
#include "WinNative.h"
#include "TextureResources.h"
#include "Font.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace luna
{
	int RenderingInfo::totalcounter = 0;

	ComponentManager::ComponentManager(const int MAX_COMPONENTS)
	{
		/* Init and resize all the components */
		m_transformationContainer.m_components.resize(MAX_COMPONENTS);
		m_basicmeshContainer.m_components.resize(MAX_COMPONENTS);
		m_fontContainer.m_components.resize(MAX_COMPONENTS);
		m_cameraContainer.m_components.resize(MAX_COMPONENTS);
		m_dirlightContainer.m_components.resize(MAX_COMPONENTS);
		m_pointlightContainer.m_components.resize(MAX_COMPONENTS);
		m_scriptContainer.m_components.resize(MAX_COMPONENTS);
	}

	ComponentManager::~ComponentManager()
	{
		/* delete all the components */
	}

	void ComponentManager::Update()
	{
		m_basicmeshContainer.Update();
		m_fontContainer.Update();
		m_dirlightContainer.Update();
		m_pointlightContainer.Update();
		m_scriptContainer.Update();
		m_cameraContainer.Update();
		m_transformationContainer.Update(); // transformation always the last one to update
	}

	void ComponentManager::GetRenderingData(std::vector<RenderingInfo>& renderinfos)
	{
		auto totalbasicmeshsize = m_basicmeshContainer.m_components.size();

		// make sure there are components for it
		if (totalbasicmeshsize > 0)
		{
			// rmb to set it back to 0 if restarting
			RenderingInfo::totalcounter = 0;

			if(renderinfos.size() > 0)
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
		glm::mat4 proj = glm::ortho(0.f, static_cast<float>(win->getWinSurfaceSizeX()), static_cast<float>(win->getWinSurfaceSizeY()), 0.f);
		glm::vec2 cursor = {};
		glm::vec2 toplefthandcorner = {};
		glm::vec2 position = {};

		for (auto & fontcomponent : m_fontContainer.m_components)
		{
			const auto owner = fontcomponent.GetOwner();
			if (owner != nullptr && fontcomponent.isActive() == true)
			{
				const auto &str = fontcomponent.text;
				const auto &material = fontcomponent.material;
				cursor = {};

				for (int i = 0; i < str.size(); ++i)
				{
					fontinstancedatas.push_back(FontInstanceData());
					auto& fid = fontinstancedatas.back();
					toplefthandcorner = {};
					position = {};

					const vulkanchar& vc = font->vulkanChars[str[i]];

					// calc the top left hand corner position
					toplefthandcorner.x = cursor.x + vc.xoffset;
					toplefthandcorner.y = cursor.y + vc.yoffset;

					// then find the correct position relative with the left hand corner
					position = { toplefthandcorner.x + vc.halfsize.x, toplefthandcorner.y - vc.halfsize.y };

					fid.transformation = proj * 
						owner->transformation->GetModel() * 
						glm::translate(glm::mat4(), glm::vec3(position, 0.f)) * 
						glm::scale(glm::mat4(), glm::vec3(vc.size, 1.f));

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

	void ComponentManager::GetMainDirLightData(MainDirLightData & maindirlightdata)
	{
		for (auto& dirlight : m_dirlightContainer.m_components)
		{
			if (dirlight.isActive())
			{
				maindirlightdata.diffusespec = glm::vec4(dirlight.diffuse, dirlight.specular);
				maindirlightdata.ambientlight = glm::vec4(dirlight.ambient, 0.f);
				maindirlightdata.dirlightdir = glm::vec4(dirlight.direction, 1.f);
				break;
			}
		}
	}

	void ComponentManager::GetPointLightsData(std::vector<PointLightData>& pointlightdata)
	{
		if (pointlightdata.size() > 0)
			pointlightdata.clear();

		for (auto& pointlight : m_pointlightContainer.m_components)
		{
			if (pointlight.isActive())
			{
				PointLightData p{};
				p.color =  glm::vec4(pointlight.color, 1.0f);
				p.position = glm::vec4(pointlight.GetOwner()->transformation->position, 1.0f);
				pointlightdata.emplace_back(p);
			}
		}
	}

	void ComponentManager::GetMainCamData(UBOData & maincamdata, glm::vec3& maincampos)
	{
		for (auto& cam : m_cameraContainer.m_components)
		{
			if (cam.isActive() && cam.maincam)
			{
				maincamdata.view = cam.GetView();
				maincamdata.proj = cam.GetProjection();
				maincampos = cam.GetOwner()->transformation->position;
				break;
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
