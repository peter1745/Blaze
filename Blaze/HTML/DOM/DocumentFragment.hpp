#pragma once

#include "Node.hpp"

namespace Blaze::HTML {

	class DocumentFragment : public Node
	{
	public:
		inline static String Name = String::create("#document-fragment");

	public:
		DocumentFragment()
			: Node(Name, NodeType::DocumentFragment) {}

		static NodeType static_type() { return NodeType::DocumentFragment; }

	};

}
