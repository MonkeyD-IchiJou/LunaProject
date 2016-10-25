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
	}

	void FontComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		text = "text";
		material = {};
	}

	bool operator==(const FontComponent & n1, const FontComponent & n2)
	{
		// if both pointing in the same address, then they are the same
		if (&n1 == &n2)
			return true;
		else
			return false;
	}
}
