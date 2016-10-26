#include "BasicMeshComponent.h"

namespace luna
{
	BasicMeshComponent::BasicMeshComponent() : Component(BASICMESH_CTYPE)
	{
	}

	BasicMeshComponent::~BasicMeshComponent()
	{
	}

	void BasicMeshComponent::Update()
	{
	}

	void BasicMeshComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		meshID = eMODELS::MAX_MODELS;
		material = {};
	}
}
