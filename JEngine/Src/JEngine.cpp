#include "pch.h"
/*
#include <iostream>

#include "Engine.h"

#include "Jlb/Archetype.h"
#include "JEngine/Modules/JobSystem.h"
#include "Jlb/Cecsar.h"
#include "JEngine/Graphics/ObjLoader.h"
#include "JEngine/Graphics/Texture.h"
#include "JEngine/Graphics/VkLayout.h"
#include "JEngine/Graphics/VkShader.h"
#include "Modules/RenderModule.h"
#include "JEngine/Modules/SceneModule.h"

struct SomeTask final
{
	size_t number = SIZE_MAX;
};

class SomeSystem final : public je::JobSystem<SomeTask>
{
public:
	SomeSystem(const size_t capacity, const size_t chunkCapacity)
		: JobSystem<SomeTask>(capacity, chunkCapacity)
	{
		
	}

	void OnBegin(je::engine::Info& info) override;

protected:
	void OnUpdate(je::engine::Info& info, const Jobs& jobs) override
	{
		for (const auto& batch : jobs)
			for (const auto& task : batch)
				std::cout << task.number << std::endl;
		std::cout << "end of frame" << std::endl;
	}
};

void SomeSystem::OnBegin(je::engine::Info& info)
{
	JobSystem<SomeTask>::OnBegin(info);

	SomeTask aTask{};
	aTask.number = 8;
	SomeTask bTask{};
	bTask.number = 14;
	SomeTask cTask{};
	cTask.number = 1;

	TryAdd(aTask);
	TryAdd(bTask);
	TryAdd(cTask);
}
*/

int main()
{
	/*
	struct RenderResources final
	{
		VkDescriptorSetLayout _layout{};
		VkShaderModule _modules[2]{};
		je::vk::Mesh _mesh{};
		je::vk::Image _image{};
		VkImageView _view{};
		VkDescriptorPool _descriptorPool{};
		je::Array<VkDescriptorSet_T*> _descriptorSets{};
		VkSampler _sampler{};

		static void DefineResources(je::Arena& arena, je::Arena& tempArena, const je::vk::App& app, const je::vk::Allocator& allocator,
			const size_t swapChainLength, const glm::ivec2 swapChainResolution, void* userPtr)
		{
			const auto ptr = static_cast<RenderResources*>(userPtr);

			je::vk::Binding binding;
			binding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
			binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			je::Array<je::vk::Binding> bindings{};
			bindings.data = &binding;
			bindings.length = 1;
			ptr->_layout = CreateLayout(tempArena, app, bindings);

			ptr->_modules[0] = CreateShaderModule(tempArena, app, "Shaders/vert2.spv");
			ptr->_modules[1] = CreateShaderModule(tempArena, app, "Shaders/frag2.spv");

			{
				const auto _ = tempArena.CreateScope();

				je::Array<je::vk::Vertex> verts{};
				je::Array<je::vk::Vertex::Index> inds{};
				constexpr float scale = .5f;
				je::obj::Load(tempArena, "Meshes/cube.obj", verts, inds, scale);
				ptr->_mesh = CreateMesh(app, allocator, verts, inds);
			}

#ifdef _DEBUG
			const auto textures = je::CreateArray<const char*>(tempArena, 4);
			textures.data[0] = "Textures/humanoid.png";
			textures.data[1] = "Textures/moveArrow.png";
			textures.data[2] = "Textures/bash-card.png";
			textures.data[3] = "Textures/tile.png";
			je::texture::GenerateAtlas(arena, tempArena, textures, "Textures/atlas.png", "atlas.txt");
#endif

			ptr->_image = je::texture::LoadAtlas(app, allocator, "Textures/atlas.png", "atlas.txt");

			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.subresourceRange.aspectMask = ptr->_image.aspectFlags;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;
			viewCreateInfo.image = ptr->_image.image;
			viewCreateInfo.format = ptr->_image.format;

			auto result = vkCreateImageView(app.device, &viewCreateInfo, nullptr, &ptr->_view);
			assert(!result);

			// Create descriptor pool.
			VkDescriptorPoolSize poolSize;
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = static_cast<uint32_t>(swapChainLength);
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			poolInfo.maxSets = static_cast<uint32_t>(swapChainLength);

			result = vkCreateDescriptorPool(app.device, &poolInfo, nullptr, &ptr->_descriptorPool);
			assert(!result);

			auto layouts = je::CreateArray<VkDescriptorSetLayout>(tempArena, swapChainLength);
			for (auto& layout : layouts)
				layout = ptr->_layout;

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = ptr->_descriptorPool;
			allocInfo.descriptorSetCount = layouts.length;
			allocInfo.pSetLayouts = layouts.data;

			ptr->_descriptorSets = je::CreateArray<VkDescriptorSet>(arena, swapChainLength);
			result = vkAllocateDescriptorSets(app.device, &allocInfo, ptr->_descriptorSets.data);
			assert(!result);

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(app.physicalDevice, &properties);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			// Since I'm going for pixelart, this will be the default.
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0;
			samplerInfo.maxLod = 0;

			result = vkCreateSampler(app.device, &samplerInfo, nullptr, &ptr->_sampler);
		}
		static void DestroyResources(je::Arena& arena, const je::vk::App& app, const je::vk::Allocator& allocator, void* userPtr)
		{
			const auto ptr = static_cast<RenderResources*>(userPtr);
			vkDestroySampler(app.device, ptr->_sampler, nullptr);
			vkDestroyDescriptorSetLayout(app.device, ptr->_layout, nullptr);
			DestroyArray(ptr->_descriptorSets, arena);
			vkDestroyDescriptorPool(app.device, ptr->_descriptorPool, nullptr);
			vkDestroyImageView(app.device, ptr->_view, nullptr);
			DestroyImage(ptr->_image, app, allocator);
			DestroyMesh(ptr->_mesh, app, allocator);
			for (const auto& mod : ptr->_modules)
				vkDestroyShaderModule(app.device, mod, nullptr);
		}
		static je::Array<je::vk::RenderNode> DefineRenderGraph(je::Arena& tempArena, const size_t swapChainLength, const glm::ivec2 swapChainResolution, void* userPtr)
		{
			const auto ptr = static_cast<RenderResources*>(userPtr);
			const auto views = je::CreateArray<VkImageView>(tempArena, swapChainLength * 0);

			je::vk::PipelineCreateInfo::Module shaderModules[2];
			shaderModules[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderModules[0].module = ptr->_modules[0];
			shaderModules[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderModules[1].module = ptr->_modules[1];
			je::Array<je::vk::PipelineCreateInfo::Module> shader{};
			shader.data = shaderModules;
			shader.length = 2;

			je::vk::RenderNode::Output output{};
			output.name = "Result";
			output.resource.resolution = swapChainResolution;
			je::Array<je::vk::RenderNode::Output> outputs{};
			outputs.data = &output;
			outputs.length = 1;

			const auto nodes = je::CreateArray<je::vk::RenderNode>(tempArena, 1);
			auto& node = nodes[0];
			node.outputs = outputs;
			node.renderFunc = Render;
			node.userPtr = ptr;
			node.modules = shader;
			node.outImageViews = views;
			node.layouts.length = 1;
			node.layouts.data = &ptr->_layout;
			return nodes;
		}
		static void BindRenderGraphResources(const je::Array<je::vk::RenderNode>& nodes, const je::vk::App& app, const size_t swapChainLength, void* userPtr)
		{
			const auto ptr = static_cast<RenderResources*>(userPtr);

			// Bind descriptor sets for the render node.
			for (size_t i = 0; i < swapChainLength; ++i)
			{
				VkWriteDescriptorSet write{};
			
				// Bind instance buffer.
				VkDescriptorBufferInfo instanceInfo{};
				instanceInfo.buffer = _instanceBuffers[i].buffer;
				instanceInfo.offset = 0;
				instanceInfo.range = sizeof(Job) * JobSystem<Job>::GetLength();

				auto& instanceWrite = writes[0];
				instanceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				instanceWrite.dstBinding = 0;
				instanceWrite.dstSet = _descriptorSets[i];
				instanceWrite.descriptorCount = 1;
				instanceWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				instanceWrite.pBufferInfo = &instanceInfo;
				instanceWrite.dstArrayElement = 0;
				
				// Bind texture atlas.
				VkDescriptorImageInfo  atlasInfo{};
				atlasInfo.imageLayout = ptr->_image.layout;
				atlasInfo.imageView = ptr->_view;
				atlasInfo.sampler = ptr->_sampler;

				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstBinding = 0;
				write.dstSet = ptr->_descriptorSets[i];
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo = &atlasInfo;
				write.dstArrayElement = 0;

				vkUpdateDescriptorSets(app.device, 1, &write, 0, nullptr);
			}
		}
		static void Render(const VkCommandBuffer cmd, const VkPipelineLayout layout, void* userPtr, const size_t frameIndex)
		{
			const auto ptr = static_cast<RenderResources*>(userPtr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
				0, 1, &ptr->_descriptorSets[frameIndex], 0, nullptr);
			ptr->_mesh.Draw(cmd, 1);
		}
	} renderResources;

	je::engine::RenderModuleCreateInfo renderModuleCreateInfo{};
	renderModuleCreateInfo.defineResources = RenderResources::DefineResources;
	renderModuleCreateInfo.destroyResources = RenderResources::DestroyResources;
	renderModuleCreateInfo.defineRenderGraph = RenderResources::DefineRenderGraph;
	renderModuleCreateInfo.bindRenderGraphResources = RenderResources::BindRenderGraphResources;
	renderModuleCreateInfo.userPtr = &renderResources;

	je::EngineRunInfo runInfo{};
	runInfo.renderModuleCreateInfo = &renderModuleCreateInfo;
	runInfo.defineAdditionalModules = [](je::Arena& dumpArena, je::Finder<je::Module>::Initializer& initializer)
	{
		size_t capacity = 2;
		size_t chunkCapacity = 4;
		initializer.Add<SomeSystem>(capacity, chunkCapacity);

		const auto scenes = je::CreateArray<je::SceneInfo>(dumpArena, 1);

		auto& scene = scenes[0];
		scene.onBegin = [](const je::Finder<je::Module>& finder, je::ecs::Cecsar& cecsar, void* userPtr)
		{
			const size_t testArchetype = cecsar.DefineArchetype<int, float, bool>();
			je::Tuple<je::ecs::Entity, int, float, bool> prototype{ {}, 0, 14, true };

			for (int i = 0; i < 17; ++i)
			{
				++je::Get<1>(prototype);
				cecsar.Add(testArchetype, prototype);
			}

			return true;
		};
		scene.onUpdate = [](const je::Finder<je::Module>& finder, je::ecs::Cecsar& cecsar, void* userPtr)
		{
			struct Info final
			{
				size_t i = 0;
			} in;

			auto scope = cecsar.CreateScope<int>();
			scope.Iterate([&in](int& i)
				{
					std::cout << i << " " << in.i++ << std::endl;
				});

			finder.Get<je::SceneModule>()->Unload(0);
			return true;
		};

		initializer.Add<je::SceneModule>(scenes);
	};

	je::Engine engine{};
	return static_cast<int>(engine.Run(runInfo));
	*/

	return 0;
}