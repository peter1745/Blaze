#pragma once

#include "Utility/Ref.hpp"

#include <CSTM/String.hpp>
#include <CSTM/StringView.hpp>
#include <CSTM/Nullable.hpp>

#include <list>
#include <vector>

namespace Blaze::HTML {

	using namespace CSTM;

	class Document;
	class CharacterData;

	enum class NodeType
	{
		Element = 1,
		Attribute = 2,
		Text = 3,
		CDATASection = 4,
		EntityReference = 5, // legacy
		Entity = 6, // legacy
		ProcessingInstruction = 7,
		Comment = 8,
		Document = 9,
		DocumentType = 10,
		DocumentFragment = 11,
		Notation = 12 // legacy
	};

	// https://dom.spec.whatwg.org/#interface-node
	class Node : public RefCounted//, EventTarget
	{
	public:
		~Node() override = default;

		[[nodiscard]]
		StringView name() const noexcept { return m_name.view().value(); }

		template<typename U>
		[[nodiscard]]
		bool is() const
		{
			static_assert(std::derived_from<U, Node>);
			return m_type == U::static_type();
		}

		template<>
		bool is<CharacterData>() const
		{
			return m_type == NodeType::Text || m_type == NodeType::ProcessingInstruction || m_type == NodeType::Comment;
		}

		[[nodiscard]]
		WeakRef<Document> document() const { return m_owner_document; }

		[[nodiscard]]
		WeakRef<Node> parent() const { return m_parent; }

		[[nodiscard]]
		const std::list<SharedRef<Node>>& children() const { return m_child_nodes; }

		// TODO(Peter): Sort out constness for first_child & last_child

		[[nodiscard]]
		WeakRef<Node> first_child() { return !m_child_nodes.empty() ? WeakRef(m_child_nodes.front()) : null; }

		[[nodiscard]]
		WeakRef<Node> last_child() { return !m_child_nodes.empty() ? WeakRef(m_child_nodes.back()) : null; }

		// TODO(Peter): [CEReactions]
		WeakRef<Node> insert_before(WeakRef<Node> node, WeakRef<Node> child);
		WeakRef<Node> append_child(SharedRef<Node> node);

		// https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-descendant
		[[nodiscard]]
		std::vector<WeakRef<Node>> descendants(bool includeSelf);

	public:
		template<typename T, typename... Args>
		static SharedRef<Node> create(WeakRef<Document> document, Args&&... args)
		{
			auto node = SharedRef<T>::create(std::forward<Args>(args)...);
			return create_impl(document, node.template as<Node>());
		}

	private:
		static SharedRef<Node> create_impl(WeakRef<Document> document, SharedRef<Node> node);

	protected:
		Node(const String& name, const NodeType type)
			: m_name(name), m_type(type)
		{}

	protected:
		const String m_name;
		const NodeType m_type;

		// readonly attribute USVString baseURI;

		WeakRef<Document> m_owner_document;

		WeakRef<Node> m_parent;

		std::list<SharedRef<Node>> m_child_nodes;
		WeakRef<Node> m_previous_sibling;
		WeakRef<Node> m_next_sibling;

	private:
		friend void insert_into(Node& parent, WeakRef<Node> node, WeakRef<Node> child, bool suppressObservers);
		friend void adopt_node_impl(SharedRef<Node> node, Document& document);
	};

}
