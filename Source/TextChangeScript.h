#ifndef TEXT_CHANGE_SCRIPT_H
#define TEXT_CHANGE_SCRIPT_H

#include "Script.h"

namespace luna
{
	class TextChangeScript :
		public Script
	{
	public:
		TextChangeScript();
		virtual ~TextChangeScript();
		void Update(Entity* entity) override;
	};
}

#endif

