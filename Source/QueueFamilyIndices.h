#ifndef QUEUE_FAMILY_INDICES
#define QUEUE_FAMILY_INDICES

namespace luna
{
	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;

		inline bool isComplete() const
		{
			return graphicsFamily >= 0;
		}
	};
}

#endif
