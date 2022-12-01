#include "pch.h"
#include "JEngine/Graphics/VkPipeline.h"
#include "JEngine/Graphics/Vertex.h"
#include "JEngine/Graphics/VkApp.h"
#include "Jlb/Array.h"

namespace je::vk
{
	void Pipeline::Bind(const VkCommandBuffer cmd) const
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	Pipeline CreatePipeline(const PipelineCreateInfo& info, Arena& tempArena, const App& app)
	{
		const auto _ = tempArena.CreateScope();

		Pipeline pipeline{};

		const auto modules = CreateArray<VkPipelineShaderStageCreateInfo>(tempArena, info.modules.length);
		for (size_t i = 0; i < modules.length; ++i)
		{
			auto& mod = modules[i];
			auto& infoMod = info.modules[i];
			mod.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			mod.pName = "main";
			mod.stage = infoMod.stage;
			mod.module = infoMod.module;
		}

		const auto bindingDescription = Vertex::GetBindingDescriptions(tempArena);
		const auto attributeDescription = Vertex::GetAttributeDescriptions(tempArena);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.length);
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.length);
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(info.resolution.x);
		viewport.height = static_cast<float>(info.resolution.y);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent =
		{
			static_cast<uint32_t>(info.resolution.x),
			static_cast<uint32_t>(info.resolution.y)
		};

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multiSampling{};
		multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampling.sampleShadingEnable = info.shaderSamplingEnabled;
		multiSampling.minSampleShading = .2f;
		multiSampling.rasterizationSamples = info.samples;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = static_cast<uint32_t>(info.pushConstantSize);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = info.depthBufferCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(info.layouts.length);
		pipelineLayoutInfo.pSetLayouts = info.layouts.data;
		pipelineLayoutInfo.pushConstantRangeCount = info.pushConstantSize > 0;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

		auto result = vkCreatePipelineLayout(app.device, &pipelineLayoutInfo, nullptr, &pipeline.layout);
		assert(!result);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = modules.data;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multiSampling;
		pipelineInfo.pDepthStencilState = info.depthBufferEnabled ? &depthStencil : nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = pipeline.layout;
		pipelineInfo.renderPass = info.renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = info.basePipeline;
		pipelineInfo.basePipelineIndex = info.basePipelineIndex;

		result = vkCreateGraphicsPipelines(app.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline);
		assert(!result);

		return pipeline;
	}

	void DestroyPipeline(const Pipeline& pipeline, const App& app)
	{
		vkDestroyPipeline(app.device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(app.device, pipeline.layout, nullptr);
	}
}
