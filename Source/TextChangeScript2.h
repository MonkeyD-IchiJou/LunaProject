#ifndef TEXT_CHANGE_SCRIPT2_H
#define TEXT_CHANGE_SCRIPT2_H

#include "Script.h"

namespace luna
{
	class TextChangeScript2 :
		public Script
	{
	public:
		TextChangeScript2();
		virtual ~TextChangeScript2();
		void Update(Entity* entity) override;
	};
}

#endif

