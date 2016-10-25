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
		if (m_active)
		{
		}
	}

	void BasicMeshComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		meshID = eMODELS::MAX_MODELS;
		material = {};
	}

	bool operator==(const BasicMeshComponent & n1, const BasicMeshComponent & n2)
	{
		// if both pointing in the same address, then they are the same
		if (&n1 == &n2)
			return true;
		else
			return false;
	}
}
