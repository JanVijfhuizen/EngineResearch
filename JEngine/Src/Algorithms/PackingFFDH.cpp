#include "pch.h"
#include "Algorithms/PackingFFDH.h"

#include "Jlb/JMove.h"
#include "Jlb/JVector.h"
#include "Jlb/LinSort.h"

namespace je::packing
{
	struct Ref final
	{
		glm::ivec2 shape;
		size_t length;
		size_t index;
	};

	bool RefSorter(Ref& a, Ref& b)
	{
		return a.length < b.length;
	}

	Array<glm::ivec2> Pack(Arena& arena, Arena& tempArena, const View<glm::ivec2>& shapes, const glm::ivec2 area)
	{
		struct Node final
		{
			glm::ivec2 position{};
			glm::ivec2 shape{};
		};

		// Collect metadata from the shapes.
		const size_t length = shapes.GetLength();
		Vector<Node> nodes{tempArena, length };

		{
			auto& root = nodes.Add();
			root.shape = area;
			assert(area.x > 0 && area.y > 0);
		}

		const Array<Ref> refs{tempArena, length };
		for (size_t i = 0; i < length; ++i)
		{
			auto& ref = refs[i];
			ref.shape = shapes[i];
			ref.length = glm::length(ref.shape);
			ref.index = i;
			assert(ref.shape.x > 0 && ref.shape.y > 0);
		}

		// Sort from largest to smallest.
		LinSort(refs.GetData(), length, RefSorter);

		Vector<glm::ivec2> ret{arena, length};

		// Iterate over all the references and put them in nodes.
		for (const auto& ref : refs.GetView())
		{
			const size_t nodeCount = nodes.GetCount();
			const auto& shape = ref.shape;

			for (size_t i = 0; i < nodeCount; ++i)
			{
				const auto& node = nodes[i];
				const auto& nodeShape = node.shape;

				const auto diff = glm::ivec2(nodeShape.x - shape.x, nodeShape.y - shape.y);

				// If it does not fit within the node.
				if (diff.x < 0 || diff.y < 0)
					continue;

				ret.Add(node.position);
				
				if(diff.y > 0)
				{
					auto& newNode = nodes.Add();
					newNode.position = node.position + glm::ivec2(0, shape.y);
					newNode.shape = node.shape - glm::ivec2(0, shape.y);
				}
				if(diff.x > 0)
				{
					auto& newNode = nodes.Add();
					newNode.position = node.position + glm::ivec2(shape.x, 0);
					newNode.shape = node.shape - glm::ivec2(shape.x, diff.y);
				}

				nodes.Remove(i);
				break;
			}
		}

		return static_cast<Array<glm::ivec2>>(Move(ret));
	}
}
