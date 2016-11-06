#include "TextChangeScript.h"
#include "Entity.h"
#include "FontComponent.h"
#include "Global.h"
#include "TransformationComponent.h"
#include "WinNative.h"

namespace luna
{
	TextChangeScript::TextChangeScript()
	{
	}

	TextChangeScript::~TextChangeScript()
	{
	}

	void TextChangeScript::Update(Entity * entity)
	{
		auto fontc = entity->findComponentT<FontComponent>(FONT_CTYPE);
		if (fontc)
		{
			fontc->text = std::to_string(global::DeltaTime);
			fontc->GetOwner()->transformation->position.y = WinNative::getInstance()->getWinSurfaceSizeY();
		}
	}
}
