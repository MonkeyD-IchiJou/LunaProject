#ifndef ROTATE_SCRIPT_H
#define ROTATE_SCRIPT_H

#include "Script.h"

namespace luna
{
	class RotateScript :
		public Script
	{
	public:
		RotateScript();
		virtual ~RotateScript();

		void Update(Entity* entity) override;
	};
}

#endif