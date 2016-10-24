#include "FontComponent.h"

namespace luna
{
	FontComponent::FontComponent() : Component(FONT_CTYPE)
	{
	}

	FontComponent::~FontComponent()
	{
	}

	void FontComponent::Update()
	{
		if (m_active)
		{
		}
	}

	void FontComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		text = "text";
		material = {};
	}
}
