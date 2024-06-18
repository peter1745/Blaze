#pragma once

#include "Node.hpp"

namespace Blaze::HTML {

	// https://dom.spec.whatwg.org/#characterdata
	class CharacterData final : public Node
	{
	protected:
		CharacterData(const String& name, const NodeType type, const String& data)
			: Node(name, type), m_data(data)
		{}

	protected:
		String m_data;
	};

}
