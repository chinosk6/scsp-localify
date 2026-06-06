#pragma once

#include <cstdint>
#include <vector>

struct Il2CppObject;

namespace material_texture_ids_sim
{
	// IL2CPP managed UnityEngine.Object layout:
	//   object + 0x10 = m_CachedPtr, the native Unity C++ object pointer.
	inline void* native_from_managed_material(Il2CppObject* managed_material)
	{
		if (!managed_material)
			return nullptr;

		return *reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(managed_material) + 0x10);
	}

	// Unity 6000.x reference build, Runtime/Shaders/Material.h native Material layout:
	//   Material + 0x48 = MSVC std::_Tree head/sentinel node pointer for texture property name IDs.
	//   Material + 0x50 = tree size.
	//
	// Node layout matches MSVC std::_Tree_node<int, void*>:
	//   +0x00 left
	//   +0x08 parent
	//   +0x10 right
	//   +0x18 color
	//   +0x19 is_nil/head flag
	//   +0x1C int value
	struct TreeNodeInt
	{
		TreeNodeInt* left;
		TreeNodeInt* parent;
		TreeNodeInt* right;
		std::uint8_t color;
		std::uint8_t is_nil;
		std::uint8_t padding[2];
		std::int32_t value;
	};

	static_assert(offsetof(TreeNodeInt, left) == 0x00);
	static_assert(offsetof(TreeNodeInt, parent) == 0x08);
	static_assert(offsetof(TreeNodeInt, right) == 0x10);
	static_assert(offsetof(TreeNodeInt, is_nil) == 0x19);
	static_assert(offsetof(TreeNodeInt, value) == 0x1C);

	struct NativeMaterialTextureIdTree
	{
		TreeNodeInt* head;
		std::uint64_t size;
	};

	inline NativeMaterialTextureIdTree texture_id_tree_from_native_material(void* native_material)
	{
		if (!native_material)
			return {};

		auto base = static_cast<std::uint8_t*>(native_material);
		return {
			*reinterpret_cast<TreeNodeInt**>(base + 0x48),
			*reinterpret_cast<std::uint64_t*>(base + 0x50),
		};
	}

	inline TreeNodeInt* tree_next(TreeNodeInt* node)
	{
		if (!node)
			return nullptr;

		auto next = node->right;
		if (next->is_nil == 0) {
			do {
				node = next;
				next = next->left;
			} while (next->is_nil == 0);
			return node;
		}

		next = node->parent;
		while (next->is_nil == 0 && node == next->right) {
			node = next;
			next = next->parent;
		}

		return next;
	}

	inline bool enumerate_texture_property_name_ids_from_native_material(
		void* native_material,
		std::vector<int>& out_ids,
		std::uint64_t max_reasonable_count = 4096)
	{
		out_ids.clear();

		auto tree = texture_id_tree_from_native_material(native_material);
		if (!tree.head)
			return false;

		if (tree.size > max_reasonable_count)
			return false;

		out_ids.reserve(static_cast<std::size_t>(tree.size));

		auto node = tree.head->left;
		for (std::uint64_t i = 0; node != tree.head; ++i) {
			if (!node || node->is_nil != 0 || i >= tree.size)
				return false;

			out_ids.push_back(node->value);
			node = tree_next(node);
		}

		return out_ids.size() == tree.size;
	}

	inline bool enumerate_texture_property_name_ids_from_managed_material(
		Il2CppObject* managed_material,
		std::vector<int>& out_ids,
		std::uint64_t max_reasonable_count = 4096)
	{
		return enumerate_texture_property_name_ids_from_native_material(
			native_from_managed_material(managed_material),
			out_ids,
			max_reasonable_count);
	}
}
