#include "ResourceManager.h"
#include "Model.h"
#include "BasicMesh.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "BasicUBO.h"

#include "Texture2D.h"
#include "BasicImage.h"
#include "BasicImageAttachment.h"

namespace luna
{
	std::once_flag ResourceManager::m_sflag{};
	ResourceManager* ResourceManager::m_instance = nullptr;

	ResourceManager::ResourceManager()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		// init all the textures and meshes
		Init_();
	}

	void ResourceManager::Init_()
	{
		// image samplers create first 
		CreateAllSamplers_();

		// Main Device Memory and Main Buffer (Graphic layout) 
		/*********************************************************************************************************************************************/
		/*-------------------------------------------------------------------------------------------------------------------------------------------*/
		/* [                                                        Allocated DEVICE MEMORY                                                        ] */
		/*-------------------------------------------------------------------------------------------------------------------------------------------*/
		/* [                                  DATA BUFFER                                  ] [      IMAGE BUFFER       ] [      IMAGE BUFFER       ] */
		/*-------------------------------------------------------------------------------------------------------------------------------------------*/
		/* { | UBO datas | Vertex Datas | Index Datas | Vertex Datas | Index Datas | PAD | } { | Image Datas | | PAD | } { | Image Datas | | PAD | } */
		/*-------------------------------------------------------------------------------------------------------------------------------------------*/
		/*********************************************************************************************************************************************/

		// init ubos
		UBO = new BasicUBO();

		// init all the mesh
		Models.resize(MAX_MODELS);
		
		Models[eMODELS::QUAD_MODEL] = new Model(ePRIMITIVE_MESH::PRIMITIVE_QUAD);
		Models[eMODELS::BOXES_MODEL] = new Model("./../Assets/Models/boxes.lrl");
		Models[eMODELS::BUNNY_MODEL] = new Model("./../Assets/Models/bunny.lrl");
		Models[eMODELS::TYRA_MODEL] = new Model("./../Assets/Models/tyra.lrl");
		Models[eMODELS::CHALET_MODEL] = new Model("./../Assets/Models/boxes.lrl");

		// init all images and its image buffers too
		Textures.resize(MAX_TEXTURE_ATT);
		Textures[eTEXTURES::CHALET_TEXTURE] = new Texture2D("./../Assets/Textures/chalet.jpg", ImageSamplers[eIMAGESAMPLING::BASIC_SAMPLER]);
		Textures[eTEXTURES::DEFAULT_TEXTURE] = new Texture2D("./../Assets/Textures/default.tga", ImageSamplers[eIMAGESAMPLING::BASIC_SAMPLER]);
		Textures[eTEXTURES::BASIC_TEXTURE] = new Texture2D("./../Assets/Textures/texture.jpg", ImageSamplers[eIMAGESAMPLING::BASIC_SAMPLER]);
		Textures[eTEXTURES::HILL_TEXTURE] = new Texture2D("./../Assets/Textures/hills.tga", ImageSamplers[eIMAGESAMPLING::BASIC_SAMPLER]);

		// init all image attachments and its image buffers too
		Textures[eTEXTURES::DEPTH32_TEXTURE_ATT] = new Texture2D(eATTACHMENT_CREATE_TYPE::DEPTH_32_ATTACHMENT, 1080, 720, VK_IMAGE_ASPECT_DEPTH_BIT);

		// after retrieved all the models datas 
		LoadToDevice_();
	}

	void ResourceManager::LoadToDevice_()
	{
		/* staged buffer creation based on all the resources sizes (except image), then allocate the device memory to it */
		StagingBufferInit_();
		StagingBufferBinding_();

		/* map all the data to the staged device memory */
		UBO->MapToDeviceMemory(m_devicememory_staged);
		for (auto &model : Models)
			model->MapToDeviceMemory(m_logicaldevice, m_devicememory_staged);
		for (int i = 0; i < MAX_TEXTURE; ++i)
		{
			auto image = Textures[i]->getImage<BasicImage*>();
			image->MapToDeviceMemory(m_devicememory_staged);
			image->TransitionStagedAndMainImageLayout(); // transition the image layout before copying
		}
		
		/* main buffer creation based on all the resources sizes (except image), then allocate the device memory to it */
		DeviceBufferInit_();
		DeviceBufferBinding_();

		/* copy the buffer datas to the real main buffer */
		CopyBufferToDevice_();

		/* last setup */
		UBO->setMainBuffer(m_mainBuffer.buffer);
		for (auto &model : Models)
			model->setMainBuffer(m_mainBuffer.buffer);
		for (int i = 0; i < MAX_TEXTURE; ++i) // create the image view for the texture to be used later in the shader
			Textures[i]->getImage<VulkanImageBufferObject*>()->CreateImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT); 
	}

	void ResourceManager::StagingBufferInit_()
	{
		/* After all the meshes have inited, create a big buffer to contain all of them */
		// create my big buffer
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // later will transfer into gpu device memory
			buffinfo.size = VulkanBufferObject::CurrentBufferTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_stagingBuffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_stagingBuffer.buffer, &memoryrequirement);

			m_stagingBuffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // find the best heap in the GPU to store at
			m_stagingBuffer.BufferTotalSize = VulkanBufferObject::CurrentBufferTotalSize;
			m_stagingBuffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_stagingBuffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}
	}

	void ResourceManager::StagingBufferBinding_()
	{
		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_stagingBuffer.RequirementSizeInDeviceMem;
			for (int i = 0; i < MAX_TEXTURE; ++i)
				allocInfo.allocationSize += Textures[i]->getImage<BasicImage*>()->GetStageBufferData().RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_stagingBuffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_devicememory_staged));
		}

		// Buffers binding to the memory
		{
			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_stagingBuffer.buffer, m_devicememory_staged, 0);
			
			// bind all image buffers here
			for (int i = 0; i < MAX_TEXTURE; ++i)
			{
				BasicImage* image = Textures[i]->getImage<BasicImage*>();
				const VulkanImageBufferData & imagebuffer = image->GetStageBufferData();

				if (i != 0)
				{
					const BasicImage* previmage = Textures[i - 1]->getImage<BasicImage*>();
					image->SetStagedBufferOffset(previmage->GetStagedBufferOffset() + previmage->GetStageBufferData().RequirementSizeInDeviceMem);
				}
				else
					image->SetStagedBufferOffset( m_stagingBuffer.RequirementSizeInDeviceMem );

				// bind the image buffer with the offsets in device memory
				vkBindImageMemory(m_logicaldevice, imagebuffer.buffer, m_devicememory_staged, image->GetStagedBufferOffset());
			}
		}
	}

	void ResourceManager::DeviceBufferInit_()
	{
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffinfo.size = VulkanBufferObject::CurrentBufferTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_mainBuffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_mainBuffer.buffer, &memoryrequirement);

			m_mainBuffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // find the best heap in the GPU to store at
			m_mainBuffer.BufferTotalSize = VulkanBufferObject::CurrentBufferTotalSize;
			m_mainBuffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_mainBuffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}
	}

	void ResourceManager::DeviceBufferBinding_()
	{
		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_mainBuffer.RequirementSizeInDeviceMem;
			for (int i = 0; i < MAX_TEXTURE; ++i)
				allocInfo.allocationSize += Textures[i]->getImage<BasicImage*>()->GetMainBufferData().RequirementSizeInDeviceMem;
			for(int i = MAX_TEXTURE; i < MAX_TEXTURE_ATT; ++i)
				allocInfo.allocationSize += Textures[i]->getImage<BasicImageAttachment*>()->GetMainBufferData().RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_mainBuffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_devicememory_main));
		}

		// Buffers binding to the memory
		{
			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_mainBuffer.buffer, m_devicememory_main, 0);

			// bind all image buffers here
			for (int i = 0; i < MAX_TEXTURE; ++i)
			{
				BasicImage* image = Textures[i]->getImage<BasicImage*>();
				const VulkanImageBufferData & imagebuffer = image->GetMainBufferData();

				if (i != 0)
				{
					const BasicImage* previmage = Textures[i - 1]->getImage<BasicImage*>();
					image->SetImageBufferOffset(previmage->GetBufferOffset() + previmage->GetMainBufferData().RequirementSizeInDeviceMem);
				}
				else
					image->SetImageBufferOffset( m_mainBuffer.RequirementSizeInDeviceMem );

				

				// bind the image buffer with the offsets in device memory
				vkBindImageMemory(m_logicaldevice, imagebuffer.buffer, m_devicememory_main, image->GetBufferOffset());
			}

			// attachment images buffer binding
			for (int i = MAX_TEXTURE; i < MAX_TEXTURE_ATT; ++i)
			{
				BasicImageAttachment* image = Textures[i]->getImage<BasicImageAttachment*>();
				const VulkanImageBufferData & imagebuffer = image->GetMainBufferData();

				if (i != MAX_TEXTURE)
				{
					const BasicImageAttachment* previmage = Textures[i - 1]->getImage<BasicImageAttachment*>();
					image->SetImageBufferOffset(previmage->GetBufferOffset() + previmage->GetMainBufferData().RequirementSizeInDeviceMem);
				}
				else
				{
					const BasicImage* previmage = Textures[MAX_TEXTURE - 1]->getImage<BasicImage*>();
					image->SetImageBufferOffset(previmage->GetBufferOffset() + previmage->GetMainBufferData().RequirementSizeInDeviceMem);
				}

				vkBindImageMemory(m_logicaldevice, imagebuffer.buffer, m_devicememory_main, image->GetBufferOffset());
				image->CreateImageView(); // at the same time, create a image view for it
			}
		}
	}

	void ResourceManager::CopyBufferToDevice_()
	{
		// create the temp command pool
		VkCommandPool commandpool = VK_NULL_HANDLE;
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		createinfo.queueFamilyIndex = Renderer::getInstance()->GetQueueFamilyIndices().graphicsFamily;
		vkCreateCommandPool(m_logicaldevice, &createinfo, nullptr, &commandpool);

		// create the command buffer and start recording it
		VkCommandBuffer commandbuffer = VK_NULL_HANDLE;

		// allocate the temporary command buffer
		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = commandpool;
		allocinfo.commandBufferCount = 1;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(m_logicaldevice, &allocinfo, &commandbuffer);

		// immediately start recording this command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandbuffer, &beginInfo);

		// start the copy cmd
		// copy the damn buffer
		VkBufferCopy copyregion{};
		copyregion.srcOffset = 0;
		copyregion.dstOffset = 0;
		copyregion.size = VulkanBufferObject::CurrentBufferTotalSize;
		vkCmdCopyBuffer(commandbuffer, m_stagingBuffer.buffer, m_mainBuffer.buffer, 1, &copyregion);

		// image copy as well
		for (int i = 0; i < MAX_TEXTURE; ++i)
		{
			Textures[i]->getImage<BasicImage*>()->CopyImage(commandbuffer);
		}

		// end the command buffer recording
		vkEndCommandBuffer(commandbuffer);

		// then submit this to the graphics queue to execute it
		// now execute the command buffer
		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandbuffer;

		vkQueueSubmit(Renderer::getInstance()->GetGraphicQueue(), 1, &submitinfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(Renderer::getInstance()->GetGraphicQueue());

		// clean up
		vkDestroyCommandPool(m_logicaldevice, commandpool, nullptr);
		if (m_stagingBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_stagingBuffer.buffer, nullptr);
			m_stagingBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_staged != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_staged, nullptr);
			m_devicememory_staged = VK_NULL_HANDLE;
		}
	}

	void ResourceManager::CreateAllSamplers_()
	{
		ImageSamplers.resize(MAX_SAMPLERS);

		// image sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE; // mainly used for percentage-closer filtering on shadow maps
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.f;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = 0.f;

		DebugLog::EC(vkCreateSampler(m_logicaldevice, &samplerInfo, nullptr, &ImageSamplers[eIMAGESAMPLING::BASIC_SAMPLER]));
	}

	void ResourceManager::Destroy()
	{
		// delete all the meshes in the models
		for (auto &model : Models)
		{
			if (model != nullptr)
			{
				delete model;
			}
		}
		Models.clear();

		// delete the ubo
		if (UBO != nullptr)
		{
			delete UBO;
			UBO = nullptr;
		}

		// delete all the textures
		for (auto &texture : Textures)
		{
			if (texture != nullptr)
			{
				delete texture;
				texture = nullptr;
			}
		}
		Textures.clear();

		// delete all the image samplers
		for (auto &sampler : ImageSamplers)
		{
			if (sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(m_logicaldevice, sampler, nullptr);
			}
		}
		ImageSamplers.clear();

		if (m_stagingBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_stagingBuffer.buffer, nullptr);
			m_stagingBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_staged != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_staged, nullptr);
			m_devicememory_staged = VK_NULL_HANDLE;
		}

		if (m_mainBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_mainBuffer.buffer, nullptr);
			m_mainBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_main != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_main, nullptr);
			m_devicememory_main = VK_NULL_HANDLE;
		}
	}
}