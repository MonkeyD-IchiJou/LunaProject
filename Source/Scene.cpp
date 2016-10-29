#include "Scene.h"
#include "ComponentManager.h"
#include "Entity.h"
#include "Renderer.h"

namespace luna
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		if (m_componentmanager != nullptr)
		{
			delete m_componentmanager;
			m_componentmanager = nullptr;
		}
	}

	void Scene::GetInstanceData_(std::vector<InstanceData>& instancedatas, const std::vector<RenderingInfo>& renderinfos)
	{
		// store another vector just for the instance data
		instancedatas.resize(RenderingInfo::totalcounter);
		int count = 0;
		InstanceData instancedata = {};

		for (int i = 0; i < renderinfos.size(); ++i)
		{
			const auto& renderdata = renderinfos[i];

			for (int j = 0; j < renderdata.instancedatas.size(); ++j, ++count)
			{
				const BasicMeshComponent& r = *renderdata.instancedatas[j];
				const TransformationComponent& t = *r.GetOwner()->transformation;

				// update the instance datas
				instancedata.model = t.GetModel();
				instancedata.transpose_inverse_model = t.GetTransposeInverseModel();
				instancedata.material = r.material.color;

				instancedatas[count] = instancedata;
				instancedata = {};
			}
		}
	}
}