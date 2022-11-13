#include "pch.h"
#include "Graphics/VkInitializer.h"
#include <cstring>
#include <iostream>
#include "Graphics/VkApp.h"
#include "Jlb/Array.h"
#include "Jlb/Heap.h"
#include "Jlb/JMap.h"
#include "Jlb/JVector.h"

namespace je::vk::init
{
	SwapChainSupportDetails::operator bool() const
	{
		return formats.data && presentModes.data;
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

	void CheckValidationSupport(Arena& tempArena, const Array<const char*>& validationLayers)
	{
#ifdef NDEBUG
		return;
#endif

		const auto _ = tempArena.CreateScope();

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		const auto availableLayers = CreateArray<VkLayerProperties>(tempArena, layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data);
		
		// Iterate over all the layers to see if they are available.
		for (const auto& layer : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
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

	VkInstance CreateInstance(const Array<const char*>& validationLayers, const Array<const char*>& instanceExtensions)
	{
		const auto appInfo = CreateApplicationInfo();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.length);
		createInfo.ppEnabledExtensionNames = instanceExtensions.data;

		const auto validationCreateInfo = CreateDebugInfo();

		createInfo.enabledLayerCount = 0;
#ifdef _DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.length);
		createInfo.ppEnabledLayerNames = reinterpret_cast<const char**>(validationLayers.data);
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

		const auto queueFamilies = CreateArray<VkQueueFamilyProperties>(arena, queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data);

		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
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

	bool CheckDeviceExtensionSupport(Arena& arena, const VkPhysicalDevice physicalDevice, const View<const char*>& extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		const auto availableExtensions = CreateArray<VkExtensionProperties>(arena, extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data);
		
		bool found = true;
		for (const auto& extension : extensions)
		{
			found = false;
			for (const auto& availableExtension : availableExtensions)
				if (strcmp(extension, availableExtension.extensionName) == 0)
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
			formats = CreateArray<VkSurfaceFormatKHR>(arena, formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data);
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			auto& presentModes = details.presentModes;
			presentModes = CreateArray<VkPresentModeKHR>(arena, presentModeCount);

			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
				&presentModeCount, presentModes.data);
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

		const auto devices = CreateArray<VkPhysicalDevice>(arena, deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data);

		auto candidates = CreateHeap<VkPhysicalDevice>(arena, deviceCount);
		
		for (const auto& device : devices)
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

		assert(candidates.count > 0);
		return candidates.Peek();
	}
	
	void CreateLogicalDevice(App& app, const Info& info, Arena& arena, const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
	{
		const auto _ = arena.CreateScope();

		const auto queueFamilies = GetQueueFamilies(arena, physicalDevice, surface);

		constexpr size_t queueFamiliesCount = sizeof(size_t) * 3;
		auto queueCreateInfos = CreateVector<VkDeviceQueueCreateInfo>(arena, queueFamiliesCount);
		auto familyIndexes = CreateMap<size_t>(arena, queueFamiliesCount);

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
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.count);
		createInfo.pQueueCreateInfos = queueCreateInfos.data;
		createInfo.pEnabledFeatures = &features;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(info.deviceExtensions.length);
		createInfo.ppEnabledExtensionNames = reinterpret_cast<const char**>(info.deviceExtensions.data);

		createInfo.enabledLayerCount = 0;
#ifdef _DEBUG
		const auto& validationLayers = info.validationLayers;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.length);
		createInfo.ppEnabledLayerNames = reinterpret_cast<const char**>(validationLayers.data);
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

	App CreateApp(const Info& info)
	{
		const auto _ = info.tempArena->CreateScope();

		App app{};

		bool debugExtensionPresent = true;
#ifdef _DEBUG
		debugExtensionPresent = false;
		for (const auto& instanceExtension : info.instanceExtensions)
			if (instanceExtension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			{
				debugExtensionPresent = true;
				break;
			}
#endif
		const auto instanceExtensions = CreateArray<const char*>(*info.tempArena, info.instanceExtensions.length + !debugExtensionPresent);
		if (!debugExtensionPresent)
			instanceExtensions.data[instanceExtensions.length - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

		// Add swap chain extension if not present.
		bool swapChainExtensionPresent = false;
		for (const auto& deviceExtension : info.deviceExtensions)
			if(strcmp(deviceExtension, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				swapChainExtensionPresent = true;
				break;
			}

		const auto deviceExtensions = CreateArray<const char*>(*info.tempArena, info.deviceExtensions.length + !swapChainExtensionPresent);
		if (!swapChainExtensionPresent)
			deviceExtensions.data[deviceExtensions.length - 1] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		// Add khronos validation if not present.
		bool khronosValidationPresent = false;
		for (const auto& validationLayer : info.validationLayers)
			if(strcmp(validationLayer, "VK_LAYER_KHRONOS_validation"))
			{
				khronosValidationPresent = true;
				break;
			}

		const auto validationLayers = CreateArray<const char*>(*info.tempArena, info.validationLayers.length + !khronosValidationPresent);
		if (!khronosValidationPresent)
			validationLayers.data[validationLayers.length - 1] = "VK_LAYER_KHRONOS_validation";

		memcpy(validationLayers.data, info.validationLayers.data, sizeof(const char*) * info.validationLayers.length);
		memcpy(instanceExtensions.data, info.instanceExtensions.data, sizeof(const char*) * info.instanceExtensions.length);
		memcpy(deviceExtensions.data, info.deviceExtensions.data, sizeof(const char*) * info.deviceExtensions.length);

		Info updatedInfo = info;
		updatedInfo.validationLayers = static_cast<View<const char*>>(validationLayers);
		updatedInfo.instanceExtensions = static_cast<View<const char*>>(instanceExtensions);
		updatedInfo.deviceExtensions = static_cast<View<const char*>>(deviceExtensions);

		CheckValidationSupport(*updatedInfo.tempArena, validationLayers);
		app.instance = CreateInstance(validationLayers, instanceExtensions);
		app.debugger = CreateDebugger(app.instance);
		app.surface = updatedInfo.createSurface(app.instance);
		app.physicalDevice = SelectPhysicalDevice(updatedInfo, app.instance, app.surface);
		CreateLogicalDevice(app, updatedInfo, *updatedInfo.tempArena, app.physicalDevice, app.surface);
		app.commandPool = CreateCommandPool(*updatedInfo.tempArena, app.physicalDevice, app.surface, app.device);

		return app;
	}

	void DestroyApp(const App& app)
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
