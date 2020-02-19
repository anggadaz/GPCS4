#pragma once

#include "GveCommon.h"
#include "GveDevice.h"
#include "GveEnums.h"

namespace gve
{;

/**
 * \brief Command buffer flags
 * 
 * A set of flags used to specify which of
 * the command buffers need to be submitted.
 */
enum class GveCmdType
{
	ExecBuffer = 0,
	InitBuffer = 1,
	SdmaBuffer = 2,
};

using GveCmdTypeFlags = Flags<GveCmdType>;


/**
 * \brief Queue submission info
 *
 * Convenience struct that holds data for
 * actual command submissions. Internal use
 * only, array sizes are based on need.
 */
struct GveQueueSubmission
{
	uint32_t             waitCount;
	VkSemaphore          waitSync[2];
	VkPipelineStageFlags waitMask[2];
	uint32_t             wakeCount;
	VkSemaphore          wakeSync[2];
	uint32_t             cmdBufferCount;
	VkCommandBuffer      cmdBuffers[4];
};


class GveCmdList : public RcObject
{
public:
	GveCmdList(
		GveDevice*      device,
		GvePipelineType type);

	~GveCmdList();

	void beginRecording();

	void endRecording();

	VkResult submit(
		VkSemaphore waitSemaphore,
		VkSemaphore wakeSemaphore);

	VkResult synchronize();

	void reset();

	///

	void trackDescriptorPool(RcPtr<GveDescriptorPool>&& pool)
	{
		m_descriptorPoolTracker.trackDescriptorPool(std::move(pool));
	}

	///

	void updateDescriptorSets(
		uint32_t  descriptorWriteCount,
		const VkWriteDescriptorSet*  pDescriptorWrites) 
	{
		vkUpdateDescriptorSets(*m_device,
			descriptorWriteCount, pDescriptorWrites,
			0, nullptr);
	}


	void updateDescriptorSetWithTemplate(
		VkDescriptorSet               descriptorSet,
		VkDescriptorUpdateTemplateKHR descriptorTemplate,
		const void*                         data) 
	{
		vkUpdateDescriptorSetWithTemplateKHR(*m_device,
			descriptorSet, descriptorTemplate, data);
	}


	void cmdBeginConditionalRendering(
		const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) 
	{
		vkCmdBeginConditionalRenderingEXT(
			m_execBuffer, pConditionalRenderingBegin);
	}


	void cmdEndConditionalRendering() 
	{
		vkCmdEndConditionalRenderingEXT(m_execBuffer);
	}



	void cmdBeginQuery(
		VkQueryPool             queryPool,
		uint32_t                query,
		VkQueryControlFlags     flags) 
	{
		vkCmdBeginQuery(m_execBuffer,
			queryPool, query, flags);
	}


	void cmdBeginQueryIndexed(
		VkQueryPool             queryPool,
		uint32_t                query,
		VkQueryControlFlags     flags,
		uint32_t                index) 
	{
		vkCmdBeginQueryIndexedEXT(
			m_execBuffer, queryPool, query, flags, index);
	}


	void cmdBeginRenderPass(
		const VkRenderPassBeginInfo*  pRenderPassBegin,
		VkSubpassContents       contents) 
	{
		vkCmdBeginRenderPass(m_execBuffer,
			pRenderPassBegin, contents);
	}


	void cmdBeginTransformFeedback(
		uint32_t                  firstBuffer,
		uint32_t                  bufferCount,
		const VkBuffer*                 counterBuffers,
		const VkDeviceSize*             counterOffsets) 
	{
		vkCmdBeginTransformFeedbackEXT(m_execBuffer,
			firstBuffer, bufferCount, counterBuffers, counterOffsets);
	}


	void cmdBindDescriptorSet(
		VkPipelineBindPoint       pipeline,
		VkPipelineLayout          pipelineLayout,
		VkDescriptorSet           descriptorSet,
		uint32_t                  dynamicOffsetCount,
		const uint32_t*                 pDynamicOffsets) 
	{
		vkCmdBindDescriptorSets(m_execBuffer,
			pipeline, pipelineLayout, 0, 1,
			&descriptorSet, dynamicOffsetCount, pDynamicOffsets);
	}


	void cmdBindIndexBuffer(
		VkBuffer                buffer,
		VkDeviceSize            offset,
		VkIndexType             indexType) 
	{
		vkCmdBindIndexBuffer(m_execBuffer,
			buffer, offset, indexType);
	}


	void cmdBindPipeline(
		VkPipelineBindPoint     pipelineBindPoint,
		VkPipeline              pipeline) 
	{
		vkCmdBindPipeline(m_execBuffer,
			pipelineBindPoint, pipeline);
	}


	void cmdBindTransformFeedbackBuffers(
		uint32_t                firstBinding,
		uint32_t                bindingCount,
		const VkBuffer*               pBuffers,
		const VkDeviceSize*           pOffsets,
		const VkDeviceSize*           pSizes) 
	{
		vkCmdBindTransformFeedbackBuffersEXT(m_execBuffer,
			firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
	}


	void cmdBindVertexBuffers(
		uint32_t                firstBinding,
		uint32_t                bindingCount,
		const VkBuffer*               pBuffers,
		const VkDeviceSize*           pOffsets) 
	{
		vkCmdBindVertexBuffers(m_execBuffer,
			firstBinding, bindingCount, pBuffers, pOffsets);
	}


	void cmdBlitImage(
		VkImage                 srcImage,
		VkImageLayout           srcImageLayout,
		VkImage                 dstImage,
		VkImageLayout           dstImageLayout,
		uint32_t                regionCount,
		const VkImageBlit*            pRegions,
		VkFilter                filter) 
	{
		vkCmdBlitImage(m_execBuffer,
			srcImage, srcImageLayout,
			dstImage, dstImageLayout,
			regionCount, pRegions, filter);
	}


	void cmdClearAttachments(
		uint32_t                attachmentCount,
		const VkClearAttachment*      pAttachments,
		uint32_t                rectCount,
		const VkClearRect*            pRects) 
	{
		vkCmdClearAttachments(m_execBuffer,
			attachmentCount, pAttachments,
			rectCount, pRects);
	}


	void cmdClearColorImage(
		VkImage                 image,
		VkImageLayout           imageLayout,
		const VkClearColorValue*      pColor,
		uint32_t                rangeCount,
		const VkImageSubresourceRange* pRanges) 
	{
		vkCmdClearColorImage(m_execBuffer,
			image, imageLayout, pColor,
			rangeCount, pRanges);
	}


	void cmdClearDepthStencilImage(
		VkImage                 image,
		VkImageLayout           imageLayout,
		const VkClearDepthStencilValue* pDepthStencil,
		uint32_t                rangeCount,
		const VkImageSubresourceRange* pRanges) 
	{
		vkCmdClearDepthStencilImage(m_execBuffer,
			image, imageLayout, pDepthStencil,
			rangeCount, pRanges);
	}


	// TODO:
	// Currently I use single time command buffer to submit
	// copy commands, which is not efficient, we should batch
	// these operations and submit them once.
	VkCommandBuffer cmdBeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool                 = m_execPool;
		allocInfo.commandBufferCount          = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(*m_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void cmdEndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo       = {};
		submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(*m_device, m_execPool, 1, &commandBuffer);
	}

	void cmdCopyBuffer(
		VkCommandBuffer         cmdBuffer,
		VkBuffer                srcBuffer,
		VkBuffer                dstBuffer,
		uint32_t                regionCount,
		const VkBufferCopy*     pRegions) 
	{
		//m_cmdBuffersUsed.set(cmdBuffer);

		vkCmdCopyBuffer(cmdBuffer,
			srcBuffer, dstBuffer,
			regionCount, pRegions);
	}


	void cmdCopyBufferToImage(
		VkCommandBuffer         cmdBuffer,
		VkBuffer                srcBuffer,
		VkImage                 dstImage,
		VkImageLayout           dstImageLayout,
		uint32_t                regionCount,
		const VkBufferImageCopy*      pRegions) 
	{
		//m_cmdBuffersUsed.set(cmdBuffer);

		vkCmdCopyBufferToImage(cmdBuffer,
			srcBuffer, dstImage, dstImageLayout,
			regionCount, pRegions);
	}


	//void cmdCopyImage(
	//	DxvkCmdBuffer           cmdBuffer,
	//	VkImage                 srcImage,
	//	VkImageLayout           srcImageLayout,
	//	VkImage                 dstImage,
	//	VkImageLayout           dstImageLayout,
	//	uint32_t                regionCount,
	//	const VkImageCopy*            pRegions) {
	//	m_cmdBuffersUsed.set(cmdBuffer);

	//	vkCmdCopyImage(getCmdBuffer(cmdBuffer),
	//		srcImage, srcImageLayout,
	//		dstImage, dstImageLayout,
	//		regionCount, pRegions);
	//}


	//void cmdCopyImageToBuffer(
	//	DxvkCmdBuffer           cmdBuffer,
	//	VkImage                 srcImage,
	//	VkImageLayout           srcImageLayout,
	//	VkBuffer                dstBuffer,
	//	uint32_t                regionCount,
	//	const VkBufferImageCopy*      pRegions) {
	//	m_cmdBuffersUsed.set(cmdBuffer);

	//	vkCmdCopyImageToBuffer(getCmdBuffer(cmdBuffer),
	//		srcImage, srcImageLayout, dstBuffer,
	//		regionCount, pRegions);
	//}


	void cmdCopyQueryPoolResults(
		VkQueryPool             queryPool,
		uint32_t                firstQuery,
		uint32_t                queryCount,
		VkBuffer                dstBuffer,
		VkDeviceSize            dstOffset,
		VkDeviceSize            stride,
		VkQueryResultFlags      flags) 
	{
		vkCmdCopyQueryPoolResults(m_execBuffer,
			queryPool, firstQuery, queryCount,
			dstBuffer, dstOffset, stride, flags);
	}


	void cmdDispatch(
		uint32_t                x,
		uint32_t                y,
		uint32_t                z) 
	{
		vkCmdDispatch(m_execBuffer, x, y, z);
	}


	void cmdDispatchIndirect(
		VkBuffer                buffer,
		VkDeviceSize            offset) 
	{
		vkCmdDispatchIndirect(
			m_execBuffer, buffer, offset);
	}


	void cmdDraw(
		uint32_t                vertexCount,
		uint32_t                instanceCount,
		uint32_t                firstVertex,
		uint32_t                firstInstance) 
	{
		vkCmdDraw(m_execBuffer,
			vertexCount, instanceCount,
			firstVertex, firstInstance);
	}


	void cmdDrawIndirect(
		VkBuffer                buffer,
		VkDeviceSize            offset,
		uint32_t                drawCount,
		uint32_t                stride) 
	{
		vkCmdDrawIndirect(m_execBuffer,
			buffer, offset, drawCount, stride);
	}


	void cmdDrawIndirectCount(
		VkBuffer                buffer,
		VkDeviceSize            offset,
		VkBuffer                countBuffer,
		VkDeviceSize            countOffset,
		uint32_t                maxDrawCount,
		uint32_t                stride) 
	{
		vkCmdDrawIndirectCountKHR(m_execBuffer,
			buffer, offset, countBuffer, countOffset, maxDrawCount, stride);
	}


	void cmdDrawIndexed(
		uint32_t                indexCount,
		uint32_t                instanceCount,
		uint32_t                firstIndex,
		uint32_t                vertexOffset,
		uint32_t                firstInstance) 
	{
		vkCmdDrawIndexed(m_execBuffer,
			indexCount, instanceCount,
			firstIndex, vertexOffset,
			firstInstance);
	}


	void cmdDrawIndexedIndirect(
		VkBuffer                buffer,
		VkDeviceSize            offset,
		uint32_t                drawCount,
		uint32_t                stride) 
	{
		vkCmdDrawIndexedIndirect(m_execBuffer,
			buffer, offset, drawCount, stride);
	}


	void cmdDrawIndexedIndirectCount(
		VkBuffer                buffer,
		VkDeviceSize            offset,
		VkBuffer                countBuffer,
		VkDeviceSize            countOffset,
		uint32_t                maxDrawCount,
		uint32_t                stride) 
	{
		vkCmdDrawIndexedIndirectCountKHR(m_execBuffer,
			buffer, offset, countBuffer, countOffset, maxDrawCount, stride);
	}


	void cmdDrawIndirectVertexCount(
		uint32_t                instanceCount,
		uint32_t                firstInstance,
		VkBuffer                counterBuffer,
		VkDeviceSize            counterBufferOffset,
		uint32_t                counterOffset,
		uint32_t                vertexStride) 
	{
		vkCmdDrawIndirectByteCountEXT(m_execBuffer,
			instanceCount, firstInstance, counterBuffer,
			counterBufferOffset, counterOffset, vertexStride);
	}


	void cmdEndQuery(
		VkQueryPool             queryPool,
		uint32_t                query) 
	{
		vkCmdEndQuery(m_execBuffer, queryPool, query);
	}


	void cmdEndQueryIndexed(
		VkQueryPool             queryPool,
		uint32_t                query,
		uint32_t                index) 
	{
		vkCmdEndQueryIndexedEXT(
			m_execBuffer, queryPool, query, index);
	}


	void cmdEndRenderPass() 
	{
		vkCmdEndRenderPass(m_execBuffer);
	}


	void cmdEndTransformFeedback(
		uint32_t                  firstBuffer,
		uint32_t                  bufferCount,
		const VkBuffer*                 counterBuffers,
		const VkDeviceSize*             counterOffsets) 
	{
		vkCmdEndTransformFeedbackEXT(m_execBuffer,
			firstBuffer, bufferCount, counterBuffers, counterOffsets);
	}


	void cmdFillBuffer(
		VkBuffer                dstBuffer,
		VkDeviceSize            dstOffset,
		VkDeviceSize            size,
		uint32_t                data) 
	{
		vkCmdFillBuffer(m_execBuffer,
			dstBuffer, dstOffset, size, data);
	}


	//void cmdPipelineBarrier(
	//	DxvkCmdBuffer           cmdBuffer,
	//	VkPipelineStageFlags    srcStageMask,
	//	VkPipelineStageFlags    dstStageMask,
	//	VkDependencyFlags       dependencyFlags,
	//	uint32_t                memoryBarrierCount,
	//	const VkMemoryBarrier*        pMemoryBarriers,
	//	uint32_t                bufferMemoryBarrierCount,
	//	const VkBufferMemoryBarrier*  pBufferMemoryBarriers,
	//	uint32_t                imageMemoryBarrierCount,
	//	const VkImageMemoryBarrier*   pImageMemoryBarriers) 
	//{
	//	m_cmdBuffersUsed.set(cmdBuffer);

	//	vkCmdPipelineBarrier(getCmdBuffer(cmdBuffer),
	//		srcStageMask, dstStageMask, dependencyFlags,
	//		memoryBarrierCount, pMemoryBarriers,
	//		bufferMemoryBarrierCount, pBufferMemoryBarriers,
	//		imageMemoryBarrierCount, pImageMemoryBarriers);
	//}


	void cmdPushConstants(
		VkPipelineLayout        layout,
		VkShaderStageFlags      stageFlags,
		uint32_t                offset,
		uint32_t                size,
		const void*                   pValues) 
	{
		vkCmdPushConstants(m_execBuffer,
			layout, stageFlags, offset, size, pValues);
	}


	//void cmdResetQuery(
	//	VkQueryPool             queryPool,
	//	uint32_t                queryId,
	//	VkEvent                 event) 
	//{
	//	if (event == VK_NULL_HANDLE) 
	//	{
	//		vkResetQueryPoolEXT(*m_device, queryPool, queryId, 1);
	//	}
	//	else 
	//	{
	//		//m_cmdBuffersUsed.set(DxvkCmdBuffer::InitBuffer);

	//		vkResetEvent(*m_device, event);
	//		vkCmdResetQueryPool(m_initBuffer, queryPool, queryId, 1);
	//		vkCmdSetEvent(m_initBuffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	//	}
	//}


	//void cmdResetQueryPool(
	//	VkQueryPool             queryPool,
	//	uint32_t                firstQuery,
	//	uint32_t                queryCount) 
	//{
	//	//m_cmdBuffersUsed.set(DxvkCmdBuffer::InitBuffer);

	//	vkCmdResetQueryPool(m_initBuffer,
	//		queryPool, firstQuery, queryCount);
	//}


	void cmdResolveImage(
		VkImage                 srcImage,
		VkImageLayout           srcImageLayout,
		VkImage                 dstImage,
		VkImageLayout           dstImageLayout,
		uint32_t                regionCount,
		const VkImageResolve*         pRegions) 
	{
		vkCmdResolveImage(m_execBuffer,
			srcImage, srcImageLayout,
			dstImage, dstImageLayout,
			regionCount, pRegions);
	}


	//void cmdUpdateBuffer(
	//	DxvkCmdBuffer           cmdBuffer,
	//	VkBuffer                dstBuffer,
	//	VkDeviceSize            dstOffset,
	//	VkDeviceSize            dataSize,
	//	const void*                   pData) 
	//{
	//	m_cmdBuffersUsed.set(cmdBuffer);

	//	vkCmdUpdateBuffer(getCmdBuffer(cmdBuffer),
	//		dstBuffer, dstOffset, dataSize, pData);
	//}


	void cmdSetBlendConstants(const float blendConstants[4]) 
	{
		vkCmdSetBlendConstants(m_execBuffer, blendConstants);
	}


	void cmdSetDepthBias(
		float                   depthBiasConstantFactor,
		float                   depthBiasClamp,
		float                   depthBiasSlopeFactor) 
	{
		vkCmdSetDepthBias(m_execBuffer,
			depthBiasConstantFactor,
			depthBiasClamp,
			depthBiasSlopeFactor);
	}


	void cmdSetDepthBounds(
		float                   minDepthBounds,
		float                   maxDepthBounds) 
	{
		vkCmdSetDepthBounds(m_execBuffer,
			minDepthBounds,
			maxDepthBounds);
	}


	void cmdSetEvent(
		VkEvent                 event,
		VkPipelineStageFlags    stages) 
	{
		vkCmdSetEvent(m_execBuffer, event, stages);
	}


	void cmdSetScissor(
		uint32_t                firstScissor,
		uint32_t                scissorCount,
		const VkRect2D*               scissors) 
	{
		vkCmdSetScissor(m_execBuffer,
			firstScissor, scissorCount, scissors);
	}


	void cmdSetStencilReference(
		VkStencilFaceFlags      faceMask,
		uint32_t                reference) 
	{
		vkCmdSetStencilReference(m_execBuffer,
			faceMask, reference);
	}


	void cmdSetViewport(
		uint32_t                firstViewport,
		uint32_t                viewportCount,
		const VkViewport*             viewports) 
	{
		vkCmdSetViewport(m_execBuffer,
			firstViewport, viewportCount, viewports);
	}


	void cmdWriteTimestamp(
		VkPipelineStageFlagBits pipelineStage,
		VkQueryPool             queryPool,
		uint32_t                query) 
	{
		vkCmdWriteTimestamp(m_execBuffer,
			pipelineStage, queryPool, query);
	}


private:
	bool createCommandBuffer(GvePipelineType type);

	void destroyCommandBuffers();

	VkResult submitToQueue(
		VkQueue                   queue,
		VkFence                   fence,
		const GveQueueSubmission& info);

	VkCommandBuffer selectCmdBuffer(GveCmdType cmdType) const;

private:
	GveDevice* m_device;

	VkFence m_fence = VK_NULL_HANDLE;

	VkCommandPool m_execPool     = VK_NULL_HANDLE;
	VkCommandPool m_transferPool = VK_NULL_HANDLE;

	VkCommandBuffer m_execBuffer = VK_NULL_HANDLE;
	VkCommandBuffer m_initBuffer = VK_NULL_HANDLE;
	VkCommandBuffer m_sdmaBuffer = VK_NULL_HANDLE;

	VkSemaphore m_sdmaSemaphore = VK_NULL_HANDLE;

	GveCmdTypeFlags m_cmdTypeUsed = 0;

	GveDescriptorPoolTracker m_descriptorPoolTracker;
};


}  // namespace gve
