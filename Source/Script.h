#ifndef SCRIPT_H
#define SCRIPT_H

namespace luna
{
	class Entity;

	class Script
	{
	public:
		Script();
		virtual ~Script();

		virtual void Update(Entity* entity) = 0;
	};
}

#endif

