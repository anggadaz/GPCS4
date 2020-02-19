#include "ScePresenter.h"
#include "UtilMath.h"
#include "UtilString.h"

#include "../Gve/GveDevice.h"
#include "../Gve/GvePhysicalDevice.h"

LOG_CHANNEL(Graphic.Sce.ScePresenter);

namespace sce
{;


ScePresenter::ScePresenter(
	const RcPtr<gve::GveDevice>& device,
	const PresenterDesc&         desc):
	m_presentQueue(desc.presentQueue)
{
}

ScePresenter::~ScePresenter()
{
	destroySwapchain();
}

PresenterInfo ScePresenter::info() const
{
	return m_info;
}


PresenterSync ScePresenter::getSyncObjects() const
{
	return m_syncObjects[m_frameIndex];
}

PresenterImage ScePresenter::getImage(uint32_t index) const
{
	return m_images[index];
}

VkResult ScePresenter::acquireNextImage(VkSemaphore signal, VkFence fence, uint32_t& index)
{
	VkResult status;
	do
	{
		if (fence != VK_NULL_HANDLE)
		{
			status = vkResetFences(*m_device, 1, &fence);
			if (status != VK_SUCCESS)
			{
				break;
			}
		}

		status = vkAcquireNextImageKHR(
		    *m_device,
			m_swapchain,
			std::numeric_limits<uint64_t>::max(),
			signal,
			fence,
			&m_imageIndex);

		if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
		{
			break;
		}

		m_frameIndex += 1;
		m_frameIndex %= m_info.imageCount;

		index = m_imageIndex;

	} while (false);
	return status;
}

VkResult ScePresenter::waitForFence(VkFence fence)
{
	vkWaitForFences(*m_device, 1, &fence, VK_FALSE, UINT64_MAX);
}

VkResult ScePresenter::presentImage(VkSemaphore wait)
{
	VkPresentInfoKHR info;
	info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext              = nullptr;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores    = &wait;
	info.swapchainCount     = 1;
	info.pSwapchains        = &m_swapchain;
	info.pImageIndices      = &m_imageIndex;
	info.pResults           = nullptr;

	return vkQueuePresentKHR(m_presentQueue, &info);
}

VkResult ScePresenter::recreateSwapChain(const PresenterDesc& desc)
{
	VkResult result = VK_RESULT_MAX_ENUM;
	do 
	{
		LOG_ASSERT(desc.fullScreen == false, "Fullscrenn is not supported yet.");
		if (m_swapchain)
		{
			destroySwapchain();
		}

		SwapChainSupportDetails details = querySwapChainSupport(*m_device->physicalDevice(), desc);
		// Select actual swap chain properties and create swap chain
		m_info.format      = pickFormat(details);
		m_info.presentMode = pickPresentMode(details);
		m_info.imageExtent = pickImageExtent(details, desc.imageExtent);
		m_info.imageCount  = pickImageCount(details, desc.imageCount);

		if (!m_info.imageExtent.width || !m_info.imageExtent.height)
		{
			break;
		}

		VkSwapchainCreateInfoKHR swapInfo;
		swapInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapInfo.pNext                 = nullptr;
		swapInfo.flags                 = 0;
		swapInfo.surface               = desc.windowSurface;
		swapInfo.minImageCount         = m_info.imageCount;
		swapInfo.imageFormat           = m_info.format.format;
		swapInfo.imageColorSpace       = m_info.format.colorSpace;
		swapInfo.imageExtent           = m_info.imageExtent;
		swapInfo.imageArrayLayers      = 1;
		swapInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		swapInfo.queueFamilyIndexCount = 0;
		swapInfo.pQueueFamilyIndices   = nullptr;
		swapInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapInfo.presentMode           = m_info.presentMode;
		swapInfo.clipped               = VK_TRUE;
		swapInfo.oldSwapchain          = VK_NULL_HANDLE;

		LOG_DEBUG(
			"Presenter: Actual swap chain properties:"
			"\n  Format:       %d"
			"\n  Present mode: %d"
			"\n  Buffer size:  %dx%d"
			"\n  Image count:  %d",
			m_info.format.format,
			m_info.presentMode, 
			m_info.imageExtent.width, m_info.imageExtent.height, 
			m_info.imageCount);

		result = vkCreateSwapchainKHR(*m_device, &swapInfo, nullptr, &m_swapchain);
		if (result != VK_SUCCESS)
		{
			break;
		}
		
		initImages();
		initSyncObjects();

		// Invalidate indices
		m_imageIndex = 0;
		m_frameIndex = 0;

		// Update present queue
		m_presentQueue = desc.presentQueue;

		result = VK_SUCCESS;
	} while (false);
	return result;
}

bool ScePresenter::initImages()
{
	bool ret = false;
	do 
	{
		uint32_t imageCount = 0;

		std::vector<VkImage> images;
		vkGetSwapchainImagesKHR(*m_device, m_swapchain, &imageCount, nullptr);
		if (!imageCount)
		{
			break;
		}

		images.resize(imageCount);
		vkGetSwapchainImagesKHR(*m_device, m_swapchain, &imageCount, images.data());

		bool hasError = false;
		for (uint32_t i = 0; i != imageCount; ++i)
		{
			m_images[i].image = images[i];

			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.pNext                 = nullptr;
			viewInfo.flags                 = 0;
			viewInfo.image                 = images[i];
			viewInfo.viewType              = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format                = m_info.format.format;
			viewInfo.components            = VkComponentMapping{
                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
			};
			viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel   = 0;
			viewInfo.subresourceRange.levelCount     = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount     = 1;

			VkResult result = vkCreateImageView(*m_device, &viewInfo, nullptr, &m_images[i].view);
			if (result != VK_SUCCESS)
			{
				hasError = true;
				break;
			}
		}

		if (hasError)
		{
			break;
		}

		ret = true;
	} while (false);
	return ret;
}

bool ScePresenter::initSyncObjects()
{
	bool ret = false;
	do
	{
		// Create one set of semaphores per swap image
		m_syncObjects.resize(m_info.imageCount);

		bool hasError = false;
		for (uint32_t i = 0; i < m_syncObjects.size(); i++)
		{
			VkFenceCreateInfo fenceInfo;
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = 0;

			VkResult status = vkCreateFence(*m_device, &fenceInfo, nullptr, &m_syncObjects[i].fence);
			if (status != VK_SUCCESS)
			{
				hasError = true;
				break;
			}

			VkSemaphoreCreateInfo semInfo;
			semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semInfo.pNext = nullptr;
			semInfo.flags = 0;

			status = vkCreateSemaphore(*m_device, &semInfo, nullptr, &m_syncObjects[i].acquire);
			if (status != VK_SUCCESS)
			{
				hasError = true;
				break;
			}

			status = vkCreateSemaphore(*m_device, &semInfo, nullptr, &m_syncObjects[i].present);
			if (status != VK_SUCCESS)
			{
				hasError = true;
				break;
			}
		}

		if (hasError)
		{
			break;
		}
		ret  = true;
	}while(false);
	return ret;
}

SwapChainSupportDetails ScePresenter::querySwapChainSupport(VkPhysicalDevice device, const PresenterDesc& desc)
{
	SwapChainSupportDetails details;
	do 
	{
		VkSurfaceKHR surface = desc.windowSurface;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount == 0)
		{
			break;
		}

		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount == 0)
		{
			break;
		}

		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	} while (false);
	return details;
}

VkSurfaceFormatKHR ScePresenter::pickFormat(const SwapChainSupportDetails& details)
{
	VkSurfaceFormatKHR format = {};
	do 
	{
		bool foundBest = false;
		for (const auto& availableFormat : details.formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				format = availableFormat;
				foundBest = true;
				break;
			}
		}

		if (!foundBest)
		{
			format = details.formats[0];
		}
	} while (false);
	return format;
}

VkPresentModeKHR ScePresenter::pickPresentMode(const SwapChainSupportDetails& details)
{
	VkPresentModeKHR mode = {};
	do 
	{
		bool foundBest = false;
		for (const auto& availablePresentMode : details.presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				mode = availablePresentMode;
				foundBest = true;
				break;
			}
		}

		if (!foundBest)
		{
			mode = VK_PRESENT_MODE_FIFO_KHR;
		}
	} while (false);
	return mode;
}

VkExtent2D ScePresenter::pickImageExtent(const SwapChainSupportDetails& details, VkExtent2D desired)
{
	VkExtent2D extent = {};
	do
	{
		const VkSurfaceCapabilitiesKHR& caps = details.capabilities;

		if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			extent = caps.currentExtent;
			break;
		}

		extent.width  = util::clamp(desired.width, caps.minImageExtent.width, caps.maxImageExtent.width);
		extent.height = util::clamp(desired.height, caps.minImageExtent.height, caps.maxImageExtent.height);

	} while (false);
	return extent;
}

uint32_t ScePresenter::pickImageCount(const SwapChainSupportDetails& details, uint32_t desired)
{
	const VkSurfaceCapabilitiesKHR& caps  = details.capabilities;
	uint32_t count = util::clamp(desired, caps.minImageCount, caps.maxImageCount);
	return count;
}

void ScePresenter::destroySwapchain()
{
	for (const auto& img : m_images)
	{
		vkDestroyImageView(*m_device, img.view, nullptr);
	}
	
	for (const auto& sem : m_syncObjects)
	{
		vkDestroyFence(*m_device, sem.fence, nullptr);
		vkDestroySemaphore(*m_device, sem.acquire, nullptr);
		vkDestroySemaphore(*m_device, sem.present, nullptr);
	}

	vkDestroySwapchainKHR(*m_device, m_swapchain, nullptr);

	m_images.clear();
	m_syncObjects.clear();

	m_swapchain = VK_NULL_HANDLE;
}

}  // namespace sce