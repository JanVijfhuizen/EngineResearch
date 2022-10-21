#pragma once
#include "Jlb/View.h"

namespace je
{
	class Arena;

	namespace vk
	{
		class Shader;
		struct App;

		struct PipelineCreateInfo final
		{
			Arena* tempArena;
			App* app;
			Shader* shader;
			glm::ivec2 resolution;
			VkRenderPass renderPass;
			View<VkDescriptorSetLayout> layouts;

			bool shaderSamplingEnabled = false;
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			size_t pushConstantSize = 0;
			VkCompareOp depthBufferCompareOp = VK_COMPARE_OP_LESS;
			bool depthBufferEnabled = false;
			VkPipeline basePipeline = VK_NULL_HANDLE;
			int32_t basePipelineIndex = 0;
		};

		class Pipeline
		{
		public:
			Pipeline() = default;
			explicit Pipeline(const PipelineCreateInfo& info);
			Pipeline(Pipeline&& other) noexcept;
			Pipeline& operator=(Pipeline&& other) noexcept;
			~Pipeline();

		private:
			VkPipelineLayout _layout;
			VkPipeline _pipeline;
			App* _app = nullptr;
		};
	}
}
