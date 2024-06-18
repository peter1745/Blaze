#pragma once

#include "Node.hpp"

namespace Blaze::HTML {

	class Element : public Node
	{
	public:
		Element(const String& name)
			: Node(name, NodeType::Element) {}

		static NodeType static_type() { return NodeType::Element; }
	};

}
