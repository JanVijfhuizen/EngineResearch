#include "pch.h"
#include "Graphics/VkRenderGraph.h"
#include "Graphics/VkApp.h"
#include "Graphics/VkImage.h"
#include "Graphics/VkPipeline.h"
#include "Graphics/VkSwapChain.h"
#include "Jlb/JMath.h"
#include "Jlb/LinSort.h"

namespace je::vk
{
	bool RenderNode::Resource::operator==(const Resource& other) const
	{
		return resolution == other.resolution;
	}

	RenderGraph::RenderGraph(Arena& arena, Arena& tempArena, App& app, Allocator& allocator, SwapChain& swapChain, const Array<RenderNode>& nodes) :
		_arena(arena), _app(app), _allocator(allocator), _swapChain(swapChain)
	{
		const auto _ = tempArena.CreateScope();

		const size_t length = nodes.length;
		const size_t frameCount = swapChain.GetLength();
		const auto tempNodes = CreateArray<TempNode>(tempArena, length);

		for (size_t i = 0; i < length; ++i)
		{
			const auto& node = nodes[i];
			auto& tempNode = tempNodes[i];

			tempNode.index = i;
			tempNode.renderNode = &node;
			tempNode.children = CreateLinkedList<TempNode*>();
			tempNode.inputResources = CreateLinkedList<TempResource*>();
			tempNode.outputResources = CreateLinkedList<TempResource*>();
			tempNode.inputResourceVariations = CreateLinkedList<TempResource::Variation*>();
			tempNode.outputResourceVariations = CreateLinkedList<TempResource::Variation*>();

#ifdef _DEBUG
			if(node.outputs.data)
			{
				// Assert if the resolution is the same among outputs.
				glm::ivec2 resolution = node.outputs[0].resource.resolution;
				for (auto& output : node.outputs)
					assert(resolution == output.resource.resolution);
			}
#endif
		}
		
		// Find all different resource types.
		auto tempResources = CreateLinkedList<TempResource>();
		for (auto& tempNode : tempNodes)
		{
			for (auto& output : tempNode.renderNode->outputs)
			{
				bool contained = false;
				TempResource* resource = nullptr;

				for (auto& tempResource : tempResources)
					if (tempResource.resource == output.resource)
					{
						resource = &tempResource;
						contained = true;
						break;
					}

				if (!contained)
				{
					resource = &LinkedListAdd(tempResources, tempArena);
					resource->resource = output.resource;
					resource->variations = CreateLinkedList<TempResource::Variation>();
				}

				LinkedListAdd(tempNode.outputResources, tempArena, resource);
				TempResource::Variation* tempResourceVariation = nullptr;

				// Define lifetime and variations for resources.
				contained = false;
				for (auto& variation : resource->variations)
					if (variation.name == output.name)
					{
						contained = true;
						variation.lifeTimeStart = math::Min(variation.lifeTimeStart, tempNode.depth);
						tempResourceVariation = &variation;
						break;
					}

				if (!contained)
				{
					auto& variation = LinkedListAdd(resource->variations, tempArena);
					variation.name = output.name;
					tempResourceVariation = &variation;
				}

				LinkedListAdd(tempNode.outputResourceVariations, tempArena, tempResourceVariation);
			}

			// Define lifetime for resources.
			for (auto& input : tempNode.renderNode->inputs)
				for (auto& tempResource : tempResources)
					for (auto& variation : tempResource.variations)
						if (variation.name == input)
						{
							variation.lifeTimeEnd = math::Max(variation.lifeTimeEnd, tempNode.depth);
							tempResource.lifeTimeEnd = math::Max(tempResource.lifeTimeEnd, variation.lifeTimeEnd);
						}
		}

		// Link parents by comparing inputs and outputs.
		for (auto& tempNode : tempNodes)
		{
			const auto& inputs = tempNode.renderNode->inputs;
			if (!inputs.data)
				continue;

			for (auto& other : tempNodes)
			{
				if (&tempNode == &other)
					continue;

				const auto& outputs = other.renderNode->outputs;
				if (!outputs.data)
					continue;
				for (auto& input : inputs)
				{
					size_t index = 0;
					for (auto& output : outputs)
					{
						if (input == output.name)
						{
							auto& otherOutputResource = other.outputResources[index];

							tempNode.isRoot = false;
							LinkedListAdd(tempNode.inputResources, tempArena, otherOutputResource);
							LinkedListAdd(tempNode.inputResourceVariations, tempArena, other.outputResourceVariations[index]);
							LinkedListAdd(other.children, tempArena, &tempNode);
							goto NEXT;
						}
						++index;
					}
				}

			NEXT:
				continue;
			}
		}

		// Define depth of every node.
		for (auto& tempNode : tempNodes)
			if (tempNode.isRoot)
				DefineDepth(tempNode, 0);

		// Sort based on depth.
		const auto depthSorted = CreateArray<TempNode*>(tempArena, length);
		for (size_t i = 0; i < length; ++i)
		{
			auto& tempNode = tempNodes[i];
			depthSorted[i] = &tempNode;
		}
		LinSort(depthSorted.data, depthSorted.length, SortDepthNodes);

		// Define image indexes.
		size_t imageIndexesCount = 0;
		for (auto& tempNode : depthSorted)
		{
			auto& renderNode = *tempNode->renderNode;
			imageIndexesCount += renderNode.inputs.length + renderNode.outputs.length;
		}

		_attachmentIndexes = CreateArray<size_t>(arena, imageIndexesCount);

		// Find maximum parallel usage for the images. 
		for (auto& tempResource : tempResources)
		{
			size_t usage = 0;
			size_t maxUsage = 0;

			size_t deepestLayer = 0;
			for (auto& variation : tempResource.variations)
				deepestLayer = math::Max(variation.lifeTimeEnd, length);

			for (size_t i = 0; i < deepestLayer; ++i)
			{
				usage = 0;
				for (auto& variation : tempResource.variations)
					usage += variation.lifeTimeStart <= i && variation.lifeTimeEnd <= i;
				maxUsage = math::Max(maxUsage, usage);
			}

			tempResource.count = maxUsage;
		}

		// Find image count.
		size_t imageCount = 0;
		for (auto& tempResource : tempResources)
			imageCount += tempResource.count;
		_attachments = CreateArray<Attachment>(arena, imageCount * frameCount);

		// Define nodes and sync structs for individual frames.
		_nodes = CreateArray<Node>(arena, length);
		{
			size_t index = 0;
			for (auto& node : _nodes)
			{
				auto& tempNode = depthSorted[index];
				auto& renderNode = *tempNode->renderNode;
				node.renderFunc = renderNode.renderFunc;
				node.userPtr = renderNode.userPtr;
				node.inputCount = renderNode.inputs.length;
				node.outputCount = renderNode.outputs.length;
				++index;
			}
		}

		// Find the amount of batches.
		size_t batchCount = 0;
		for (auto& depthNode : depthSorted)
			batchCount = math::Max(batchCount, depthNode->depth);
		++batchCount;

		// Define batch indices.
		_layers = CreateArray<Layer>(arena, batchCount);
		{
			size_t idx = 0;
			for (auto& depthNode : depthSorted)
			{
				const size_t newBatch = depthNode->depth > idx;
				idx += newBatch;

				auto& layer = _layers[idx];
				++layer.index;
			}
		}
		
		// Create command buffers and semaphores for the individual layers.
		for (auto& layer : _layers)
		{
			layer.frames = CreateArray<Layer::Frame>(arena, frameCount);

			for (auto& frame : layer.frames)
			{
				VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
				cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmdBufferAllocInfo.commandPool = app.commandPool;
				cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				cmdBufferAllocInfo.commandBufferCount = 1;

				auto result = vkAllocateCommandBuffers(app.device, &cmdBufferAllocInfo, &frame.cmdBuffer);
				assert(!result);

				VkSemaphoreCreateInfo semaphoreCreateInfo{};
				semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				result = vkCreateSemaphore(_app.device, &semaphoreCreateInfo, nullptr, &frame.semaphore);
				assert(!result);
			}
		}

		// Create images.
		ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		for (size_t i = 0; i < frameCount; ++i)
		{
			const size_t index = imageCount * i;
			for (const auto& tempResource : tempResources)
			{
				auto& resource = tempResource.resource;
				imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
				imageCreateInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				
				const size_t resourceCount = tempResource.count;
				for (size_t j = 0; j < resourceCount; ++j)
				{
					auto& attachment = _attachments[index + j];
					attachment.image = CreateImage(app, allocator, imageCreateInfo, glm::ivec3(resource.resolution, 3));

					viewCreateInfo.subresourceRange.aspectMask = attachment.image.aspectFlags;
					viewCreateInfo.format = attachment.image.format;
					viewCreateInfo.image = attachment.image.image;
					const auto viewResult = vkCreateImageView(app.device, &viewCreateInfo, nullptr, &attachment.view);
					assert(!viewResult);
				}
			}
		}

		// Define image pools.
		{
			size_t index = 0;
			for (auto& tempResource : tempResources)
			{
				tempResource.imageQueue = CreateQueue<size_t>(tempArena, tempResource.count);
				for (size_t i = 0; i < tempResource.count; ++i)
					tempResource.imageQueue.Enqueue(i + index);
				index += tempResource.count;
			}
		}

		// Assign image indexes to resource variations.
		{
			size_t index = 0;

			for (const auto& layer : _layers)
			{
				// Define output images.
				size_t current = index;
				while (current < layer.index)
				{
					auto& tempNode = tempNodes[current];
					const size_t count = tempNode.outputResources.GetCount();
					for (size_t i = 0; i < count; ++i)
					{
						auto& variation = tempNode.outputResourceVariations[i];
						variation->imageIndex = tempNode.outputResources[i]->imageQueue.Dequeue();
					}
					
					++current;
				}

				// Pool input images which lifetime has come to an end.
				current = index;
				while (current < layer.index)
				{
					auto& tempNode = tempNodes[current];
					const size_t count = tempNode.inputResources.GetCount();
					for (size_t i = 0; i < count; ++i)
					{
						auto& variation = tempNode.inputResourceVariations[i];
						auto& inputResource = tempNode.inputResources[i];

						if (variation->lifeTimeEnd == layer.index)
							inputResource->imageQueue.Enqueue(variation->imageIndex);
					}
					++current;
				}

				index = current;
			}
		}

		// Create frame buffers & render passes & pipelines.
		{
			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.layers = 1;

			size_t index = 0;
			for (auto& node : _nodes)
			{
				const auto views = CreateArray<VkImageView>(tempArena, node.outputCount);
				glm::ivec3 resolution{};

				node.frameBuffers = CreateArray<VkFramebuffer>(arena, frameCount);
				
				const auto colorAttachments = CreateArray<VkAttachmentReference>(tempArena, node.outputCount);
				for (size_t i = 0; i < node.outputCount; ++i)
				{
					auto& attachment = colorAttachments[i];
					attachment.attachment = i;
					attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				VkSubpassDescription subpassDescription{};
				subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.length);
				subpassDescription.pColorAttachments = colorAttachments.data;

				VkSubpassDependency subpassDependency{};
				subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				subpassDependency.dstSubpass = 0;
				subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.srcAccessMask = 0;
				subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				VkAttachmentDescription attachmentInfo{};
				attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

				VkRenderPassCreateInfo renderPassCreateInfo{};
				renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassCreateInfo.attachmentCount = 1;
				renderPassCreateInfo.pAttachments = &attachmentInfo;
				renderPassCreateInfo.subpassCount = 1;
				renderPassCreateInfo.pSubpasses = &subpassDescription;
				renderPassCreateInfo.dependencyCount = 1;
				renderPassCreateInfo.pDependencies = &subpassDependency;

				const auto renderPassResult = vkCreateRenderPass(app.device, &renderPassCreateInfo, nullptr, &node.renderPass);
				assert(!renderPassResult);

				size_t frameIndex = 0;
				for (auto& frameBuffer : node.frameBuffers)
				{
					const size_t startIndex = index + node.inputCount;
					for (size_t i = 0; i < node.outputCount; ++i)
					{
						auto& attachmentIndex = _attachmentIndexes[startIndex + i];
						auto& attachment = _attachments[imageCount * frameIndex + attachmentIndex];
						views[i] = attachment.view;
						resolution = attachment.image.resolution;
					}

					frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(views.length);
					frameBufferCreateInfo.pAttachments = views.data;
					frameBufferCreateInfo.width = resolution.x;
					frameBufferCreateInfo.height = resolution.y;
					frameBufferCreateInfo.renderPass = node.renderPass;

					const auto result = vkCreateFramebuffer(app.device, &frameBufferCreateInfo, nullptr, &frameBuffer);
					assert(!result);
					
					++frameIndex;
				}

				auto& tempNode = depthSorted[index];
				auto& renderNode = *tempNode->renderNode;

				PipelineCreateInfo createInfo{};
				createInfo.layouts = renderNode.layouts;
				createInfo.renderPass = node.renderPass;
				createInfo.modules = renderNode.modules;
				createInfo.resolution = resolution;
				node.pipeline = CreatePipeline(createInfo, tempArena, app);

				node.resolution = resolution;
				index += node.inputCount + node.outputCount;
			}
		}

		// Update image views for user.
		for (auto& tempNode : depthSorted)
		{
			auto& renderNode = *tempNode->renderNode;
			auto& inputVariations = tempNode->inputResourceVariations;
			size_t index = 0;
			for (auto& inputVariation : inputVariations)
				for (size_t i = 0; i < frameCount; ++i)
				{
					renderNode.outImageViews[index] = _attachments[inputVariation->imageIndex + i * imageCount].view;
					++index;
				}
		}
	}

	RenderGraph::~RenderGraph()
	{
		for (int32_t i = static_cast<int32_t>(_nodes.length) - 1; i >= 0; --i)
		{
			const auto& node = _nodes[i];
			for (const auto& frameBuffer : node.frameBuffers)
				vkDestroyFramebuffer(_app.device, frameBuffer, nullptr);
			vkDestroyRenderPass(_app.device, node.renderPass, nullptr);
			DestroyPipeline(node.pipeline, _app);
			DestroyArray(node.frameBuffers, _arena);
		}

		for (int32_t i = static_cast<int32_t>(_attachments.length) - 1; i >= 0; --i)
		{
			const auto& attachment = _attachments[i];
			vkDestroyImageView(_app.device, attachment.view, nullptr);
			DestroyImage(attachment.image, _app, _allocator);
		}

		for (int32_t i = static_cast<int32_t>(_layers.length) - 1; i >= 0; --i)
		{
			const auto& layer = _layers[i];
			for (const auto& frame : layer.frames)
				vkDestroySemaphore(_app.device, frame.semaphore, nullptr);
			DestroyArray(layer.frames, _arena);
		}

		DestroyArray(_layers, _arena);
		DestroyArray(_nodes, _arena);
		DestroyArray(_attachments, _arena);
		DestroyArray(_attachmentIndexes, _arena);
	}

	VkSemaphore RenderGraph::Update() const
	{
		const size_t frameIndex = _swapChain.GetIndex();

		VkSemaphore semaphore = VK_NULL_HANDLE;
		size_t index = 0;
		size_t attachmentIndex = 0;

		for (const auto& layer : _layers)
		{
			const auto& frame = layer.frames.data[frameIndex];

			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkResetCommandBuffer(frame.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			vkBeginCommandBuffer(frame.cmdBuffer, &cmdBufferBeginInfo);
			
			while(index < layer.index)
			{
				const auto& node = _nodes[index];

				const VkClearValue clearColor = { 1.f, 1.f, 1.f, 1.f };

				// Transition layouts to attachments.
				attachmentIndex += node.inputCount;
				for (size_t i = 0; i < node.outputCount; ++i)
				{
					auto& attachment = _attachments[_attachmentIndexes[attachmentIndex + i]];
					ImageTransitionLayout(attachment.image, frame.cmdBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
				}

				VkExtent2D extent{};
				extent.width = node.resolution.x;
				extent.height = node.resolution.y;

				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderArea.offset = { 0, 0 };
				renderPassBeginInfo.renderPass = node.renderPass;
				renderPassBeginInfo.framebuffer = node.frameBuffers.data[frameIndex];
				renderPassBeginInfo.renderArea.extent = extent;
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(frame.cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				node.pipeline.Bind(frame.cmdBuffer);
				node.renderFunc(frame.cmdBuffer, node.pipeline.layout, node.userPtr, frameIndex);
				vkCmdEndRenderPass(frame.cmdBuffer);

				// transition layouts to shader read only.
				for (size_t i = 0; i < node.outputCount; ++i)
				{
					auto& attachment = _attachments[_attachmentIndexes[attachmentIndex + i]];
					ImageTransitionLayout(attachment.image, frame.cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
				}

				attachmentIndex += node.outputCount;
				++index;
			}

			auto result = vkEndCommandBuffer(frame.cmdBuffer);
			assert(!result);
			
			constexpr VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &frame.cmdBuffer;
			submitInfo.waitSemaphoreCount = semaphore != VK_NULL_HANDLE;
			submitInfo.pWaitSemaphores = &semaphore;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &frame.semaphore;
			submitInfo.pWaitDstStageMask = &waitStage;

			result = vkQueueSubmit(_app.queues[App::renderQueue], 1, &submitInfo, nullptr);
			assert(!result);

			semaphore = frame.semaphore;
		}

		return semaphore;
	}

	VkImageView RenderGraph::GetResult(const size_t frameIndex) const
	{
		const size_t index = _attachmentIndexes.length * (frameIndex + 1) - 1;
		const size_t finalAttachmentIndex = _attachmentIndexes[index];

		const auto& attachment = _attachments[finalAttachmentIndex];
		return attachment.view;
	}

	void RenderGraph::DefineDepth(TempNode& node, const size_t depth)
	{
		if (node.depth >= depth)
			return;

		node.depth = depth;
		for (const auto& child : node.children)
			DefineDepth(*child, depth + 1);
	}

	bool RenderGraph::SortDepthNodes(TempNode*& a, TempNode*& b)
	{
		return true;
	}
}
