#ifndef COMMANDBUFFER_PACKET_H
#define COMMANDBUFFER_PACKET_H

#include <vector>
#include "platform.h"

namespace luna
{
	class CommandBufferPacket
	{
	public:
		CommandBufferPacket(const uint32_t& queueFamilyIndex, const VkDevice logicaldevice);
		~CommandBufferPacket();

		void Destroy(const VkDevice logicaldevice);

		/* rendering static recording purpose */
		VkCommandPool primary_commandpool = VK_NULL_HANDLE;
		VkCommandBuffer offscreen_cmdbuffer = VK_NULL_HANDLE;

		/* rendering dynamic recording purpose */
		VkCommandPool secondary_commandpool_thrd0 = VK_NULL_HANDLE;
		VkCommandBuffer lightingsubpass_secondary_cmdbuff = VK_NULL_HANDLE;

		/* rendering dynamic recording purpose */
		VkCommandPool secondary_commandpool_thrd1 = VK_NULL_HANDLE;
		VkCommandBuffer gbuffer_secondary_cmdbuff = VK_NULL_HANDLE;
	};
}

#endif

