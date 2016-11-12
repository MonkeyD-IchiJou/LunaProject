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
		auto fontc = entity->findComponentT<FontComponent>();
		if (fontc)
		{
			fontc->text = std::to_string(global::DeltaTime) + "s";
			fontc->GetOwner()->transformation->position.y = static_cast<float>(WinNative::getInstance()->getWinSurfaceSizeY());
		}
	}
}
