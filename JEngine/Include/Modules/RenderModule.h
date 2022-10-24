#pragma once
#include "Module.h"
#include "Graphics/VkApp.h"

namespace je
{
	class Arena;

	namespace vk
	{
		class Pipeline;
		class Shader;
		class Mesh;
		class SwapChain;
		class Allocator;
	}

	namespace engine
	{
		class RenderModule final : public Module
		{
			vk::App _app{};
			vk::Allocator* _allocator = nullptr;
			vk::SwapChain* _swapChain = nullptr;

			vk::Mesh* _mesh = nullptr;
			vk::Shader* _shader = nullptr;
			vk::Pipeline* _pipeline = nullptr;

			void OnInitialize(Info& info) override;
			void OnExit(Info& info) override;

			void OnUpdate(Info& info) override;
			void OnPostUpdate(Info& info) override;
		};
	}
}
