#include "TextChangeScript2.h"
#include "Entity.h"
#include "FontComponent.h"
#include "Global.h"
#include "TransformationComponent.h"
#include "WinNative.h"

namespace luna
{
	TextChangeScript2::TextChangeScript2()
	{
	}

	TextChangeScript2::~TextChangeScript2()
	{
	}

	void TextChangeScript2::Update(Entity * entity)
	{
		auto fontc = entity->findComponentT<FontComponent>(FONT_CTYPE);
		if (fontc)
		{
			fontc->GetOwner()->transformation->position.x = static_cast<float>(WinNative::getInstance()->getWinSurfaceSizeX() - 540.f);
		}
	}
}
