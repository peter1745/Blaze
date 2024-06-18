#pragma once

#include "Node.hpp"

namespace Blaze::HTML {

	// https://dom.spec.whatwg.org/#documenttype
	class DocumentType final : public Node
	{
	public:
		DocumentType(const String& name, const String& publicId, const String& systemId)
			: Node(name, NodeType::DocumentType), m_public_id(publicId), m_system_id(systemId) {}

		StringView public_id() const { return m_public_id.view().value_or({}); }
		StringView system_id() const { return m_system_id.view().value_or({}); }

		static NodeType static_type() { return NodeType::DocumentType; }

	private:
		String m_public_id;
		String m_system_id;
	};

	// https://html.spec.whatwg.org/multipage/dom.html#document
	class Document final : public Node
	{
	public:
		inline static String Name = String::create("#document");

	public:
		Document()
			: Node(Name, NodeType::Document) {}

		WeakRef<Node> adopt_node(SharedRef<Node> node);

		static NodeType static_type() { return NodeType::Document; }

	public:
		// TODO(Peter): [CEReactions]
		String title;

	};

}
