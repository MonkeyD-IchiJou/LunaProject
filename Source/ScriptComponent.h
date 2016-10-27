#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "Component.h"

namespace luna
{
	class Script;

	class ScriptComponent :
		public Component
	{
	public:
		ScriptComponent();
		virtual ~ScriptComponent();

		void Update() override;
		void Reset() override;

		Script* script = nullptr;
	};
}

#endif

