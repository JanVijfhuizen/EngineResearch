#include "pch.h"
#include "Graphics/VulkanInitializer.h"
#include <cstring>
#include <iostream>
#include "Graphics/VulkanApp.h"
#include "Jlb/Heap.h"
#include "Jlb/JMap.h"
#include "Jlb/JVector.h"

namespace je::vkinit
{
	SwapChainSupportDetails::operator bool() const
	{
		return formats && presentModes;
	}

	size_t SwapChainSupportDetails::GetRecommendedImageCount() const
	{
		size_t imageCount = capabilities.minImageCount + 1;
		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;
		return imageCount;
	}

	QueueFamilies::operator bool() const
	{
		return graphics != SIZE_MAX && present != SIZE_MAX && transfer != SIZE_MAX;
	}

	bool IsPhysicalDeviceValid(const PhysicalDeviceInfo& info)
	{
		return true;
	}

	size_t GetPhysicalDeviceRating(const PhysicalDeviceInfo& info)
	{
		size_t score = 0;
		const auto& properties = info.properties;
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;
		return score;
	}

	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures()
	{
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		return deviceFeatures;
	}

	void CheckValidationSupport(Arena& tempArena, const Array<StringView>& validationLayers)
	{
#ifdef NDEBUG
		return;
#endif

		const auto _ = tempArena.CreateScope();

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		const Array<VkLayerProperties> availableLayers{tempArena, layerCount};
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.GetData());

		const auto validationLayersView = validationLayers.GetView();
		const auto availableLayersView = availableLayers.GetView();

		// Iterate over all the layers to see if they are available.
		for (const auto& layer : validationLayersView)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayersView)
				if (strcmp(layer, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			assert(layerFound);
		}
	}

	VkApplicationInfo CreateApplicationInfo()
	{
		constexpr auto version = VK_MAKE_VERSION(1, 0, 0);
		
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Application";
		appInfo.applicationVersion = version;
		appInfo.pEngineName = "Vulkan Application";
		appInfo.engineVersion = version;
		appInfo.apiVersion = VK_API_VERSION_1_0;
		return appInfo;
	}

	VkBool32 DebugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT CreateDebugInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		return info;
	}

	VkInstance CreateInstance(const Array<StringView>& validationLayers, const Array<StringView>& instanceExtensions)
	{
		const auto appInfo = CreateApplicationInfo();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.GetLength());
		createInfo.ppEnabledExtensionNames = reinterpret_cast<const char**>(instanceExtensions.GetData());

		const auto validationCreateInfo = CreateDebugInfo();

		createInfo.enabledLayerCount = 0;
#ifdef _DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetLength());
		createInfo.ppEnabledLayerNames = reinterpret_cast<const char**>(validationLayers.GetData());
		createInfo.pNext = &validationCreateInfo;
#endif

		VkInstance instance;
		const auto result = vkCreateInstance(&createInfo, nullptr, &instance);
		assert(!result);

		return instance;
	}

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")))
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")))
			func(instance, debugMessenger, pAllocator);
		}

	VkDebugUtilsMessengerEXT CreateDebugger(const VkInstance instance)
	{
#ifdef NDEBUG
		return;
#endif

		const auto createInfo = CreateDebugInfo();
		VkDebugUtilsMessengerEXT debugger;
		const auto result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugger);
		assert(!result);
		return debugger;
	}

	QueueFamilies GetQueueFamilies(Arena& arena, const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
	{
		const auto _ = arena.CreateScope();

		QueueFamilies families{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		const Array<VkQueueFamilyProperties> queueFamilies{arena, queueFamilyCount};
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.GetData());

		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies.GetView())
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				families.graphics = i;

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
				families.transfer = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
				families.present = i;

			if (families)
				break;
			++i;
		}

		return families;
	}

	bool CheckDeviceExtensionSupport(Arena& arena, const VkPhysicalDevice physicalDevice, const View<StringView>&extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		const Array<VkExtensionProperties> availableExtensions{arena, extensionCount};
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.GetData());

		const auto extensionsView = availableExtensions.GetView();

		bool found = true;
		for (const auto& extension : extensions)
		{
			found = false;
			for (const auto& availableExtension : extensionsView)
				if (strcmp(extension.GetData(), availableExtension.extensionName) == 0)
				{
					found = true;
					break;
				}

			if (!found)
				break;
		}

		return found;
	}

	SwapChainSupportDetails QuerySwapChainSupport(Arena& arena, const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			auto& formats = details.formats;
			formats = Array<VkSurfaceFormatKHR>{ arena, formatCount }.GetView();
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.GetData());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			auto& presentModes = details.presentModes;
			presentModes = Array<VkPresentModeKHR>{ arena, presentModeCount }.GetView();

			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
				&presentModeCount, presentModes.GetData());
		}

		return details;
	}

	VkPhysicalDevice SelectPhysicalDevice(const Info& info, const VkInstance instance, const VkSurfaceKHR surface)
	{
		assert(info.isPhysicalDeviceValid);
		assert(info.getPhysicalDeviceRating);

		auto& arena = *info.tempArena;
		const auto _ = arena.CreateScope();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(deviceCount);

		const Array<VkPhysicalDevice> devices{arena, deviceCount};
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.GetData());

		Heap<VkPhysicalDevice> candidates{arena, deviceCount};
		
		for (const auto& device : devices.GetView())
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			const auto families = GetQueueFamilies(arena, device, surface);
			if (!families)
				continue;

			if (!CheckDeviceExtensionSupport(arena, device, info.deviceExtensions))
				continue;

			{
				const auto swapChainSupportScope = arena.CreateScope();
				auto swapChainSupport = QuerySwapChainSupport(arena, device, surface);
				if (!swapChainSupport)
					continue;
			}

			PhysicalDeviceInfo physicalDeviceInfo{};
			physicalDeviceInfo.device = device;
			physicalDeviceInfo.features = deviceFeatures;
			physicalDeviceInfo.properties = deviceProperties;

			if (!info.isPhysicalDeviceValid(physicalDeviceInfo))
				continue;

			candidates.Insert(device, info.getPhysicalDeviceRating(physicalDeviceInfo));
		}

		assert(candidates.GetCount() > 0);
		return candidates.Peek();
	}
	
	void CreateLogicalDevice(VulkanApp& app, const Info& info, Arena& arena, const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
	{
		const auto _ = arena.CreateScope();

		const auto queueFamilies = GetQueueFamilies(arena, physicalDevice, surface);

		constexpr size_t queueFamiliesCount = sizeof(size_t) * 3;
		Vector<VkDeviceQueueCreateInfo> queueCreateInfos{arena, queueFamiliesCount};
		Map<size_t> familyIndexes{arena, queueFamiliesCount};

		const size_t queueFamiliesIndexes[3]
		{
			queueFamilies.graphics,
			queueFamilies.present,
			queueFamilies.transfer
		};

		constexpr float queuePriority = 1;
		for (auto family : queueFamiliesIndexes)
		{
			if (familyIndexes.Contains(family))
				continue;
			familyIndexes.Insert(family, family);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(family);
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.Add(queueCreateInfo);
		}

		assert(info.getPhysicalDeviceFeatures);
		const auto features = info.getPhysicalDeviceFeatures();

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.GetCount());
		createInfo.pQueueCreateInfos = queueCreateInfos.GetData();
		createInfo.pEnabledFeatures = &features;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(info.deviceExtensions.GetLength());
		createInfo.ppEnabledExtensionNames = reinterpret_cast<const char**>(info.deviceExtensions.GetData());

		createInfo.enabledLayerCount = 0;
#ifdef _DEBUG
		const auto& validationLayers = info.validationLayers;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetLength());
		createInfo.ppEnabledLayerNames = reinterpret_cast<const char**>(validationLayers.GetData());
#endif
		
		const auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &app.device);
		assert(!result);

		uint32_t i = 0;
		for (const auto& family : queueFamiliesIndexes)
		{
			vkGetDeviceQueue(app.device, static_cast<uint32_t>(family), 0, &app.queues[i]);
			i++;
		}
	}

	VkCommandPool CreateCommandPool(Arena& arena, const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface, const VkDevice device)
	{
		const auto families = GetQueueFamilies(arena, physicalDevice, surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = static_cast<uint32_t>(families.graphics);
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandPool commandPool;
		const auto result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
		assert(!result);

		return commandPool;
	}

	VulkanApp CreateApp(const Info& info)
	{
		const auto _ = info.tempArena->CreateScope();

		VulkanApp app{};

		bool debugExtensionPresent = true;
#ifdef _DEBUG
		debugExtensionPresent = false;
		for (auto& instanceExtension : info.instanceExtensions)
			if (instanceExtension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			{
				debugExtensionPresent = true;
				break;
			}
#endif
		const Array<StringView> instanceExtensions
		{
			*info.tempArena,
			 info.instanceExtensions.GetLength() + !debugExtensionPresent
		};
		if (!debugExtensionPresent)
			instanceExtensions[instanceExtensions.GetLength() - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

		// Add swap chain extension if not present.
		bool swapChainExtensionPresent = false;
		for (auto& deviceExtension : info.deviceExtensions)
			if(deviceExtension == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				swapChainExtensionPresent = true;
				break;
			}

		const Array<StringView> deviceExtensions
		{
			*info.tempArena,
			info.deviceExtensions.GetLength() + !swapChainExtensionPresent
		};
		if (!swapChainExtensionPresent)
			deviceExtensions[deviceExtensions.GetLength() - 1] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		// Add khronos validation if not present.
		bool khronosValidationPresent = false;
		for (auto& validationLayer : info.validationLayers)
			if(validationLayer == "VK_LAYER_KHRONOS_validation")
			{
				khronosValidationPresent = true;
				break;
			}

		const Array<StringView> validationLayers
		{
			*info.tempArena,
			info.validationLayers.GetLength() + !khronosValidationPresent
		};
		if (!khronosValidationPresent)
			validationLayers[validationLayers.GetLength() - 1] = "VK_LAYER_KHRONOS_validation";

		memcpy(validationLayers.GetData(), info.validationLayers.GetData(), sizeof(StringView) * info.validationLayers.GetLength());
		memcpy(instanceExtensions.GetData(), info.instanceExtensions.GetData(), sizeof(StringView) * info.instanceExtensions.GetLength());
		memcpy(deviceExtensions.GetData(), info.deviceExtensions.GetData(), sizeof(StringView) * info.deviceExtensions.GetLength());

		Info updatedInfo = info;
		updatedInfo.validationLayers = validationLayers;
		updatedInfo.instanceExtensions = instanceExtensions;
		updatedInfo.deviceExtensions = deviceExtensions;

		CheckValidationSupport(*updatedInfo.tempArena, validationLayers);
		app.instance = CreateInstance(validationLayers, instanceExtensions);
		app.debugger = CreateDebugger(app.instance);
		app.surface = updatedInfo.createSurface(app.instance);
		app.physicalDevice = SelectPhysicalDevice(updatedInfo, app.instance, app.surface);
		CreateLogicalDevice(app, updatedInfo, *updatedInfo.tempArena, app.physicalDevice, app.surface);
		app.commandPool = CreateCommandPool(*updatedInfo.tempArena, app.physicalDevice, app.surface, app.device);

		return app;
	}

	void DestroyApp(const VulkanApp& app)
	{
		vkDestroyCommandPool(app.device, app.commandPool, nullptr);
		vkDestroyDevice(app.device, nullptr);
		vkDestroySurfaceKHR(app.instance, app.surface, nullptr);
#ifdef _DEBUG
		DestroyDebugUtilsMessengerEXT(app.instance, app.debugger, nullptr);
#endif
		vkDestroyInstance(app.instance, nullptr);
	}
}
