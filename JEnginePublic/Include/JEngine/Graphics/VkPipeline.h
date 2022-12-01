#pragma once
#include "Jlb/Array.h"

namespace je
{
	class Arena;

	namespace vk
	{
		struct App;

		struct Pipeline final
		{
			VkPipelineLayout layout;
			VkPipeline pipeline;

			void Bind(VkCommandBuffer cmd) const;
		};

		struct PipelineCreateInfo final
		{
			struct Module final
			{
				VkShaderModule module = VK_NULL_HANDLE;
				VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL;
			};

			Array<Module> modules;
			glm::ivec2 resolution;
			VkRenderPass renderPass;

			Array<VkDescriptorSetLayout> layouts{};
			bool shaderSamplingEnabled = false;
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			size_t pushConstantSize = 0;
			VkCompareOp depthBufferCompareOp = VK_COMPARE_OP_LESS;
			bool depthBufferEnabled = false;
			VkPipeline basePipeline = VK_NULL_HANDLE;
			int32_t basePipelineIndex = 0;
		};

		[[nodiscard]] Pipeline CreatePipeline(const PipelineCreateInfo& info, Arena& tempArena, const App& app);
		void DestroyPipeline(const Pipeline& pipeline, const App& app);
	}
}
