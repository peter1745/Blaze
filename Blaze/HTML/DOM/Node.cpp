#include "Node.hpp"

#include "Document.hpp"
#include "DocumentFragment.hpp"
#include "Element.hpp"
#include "CharacterData.hpp"

namespace Blaze::HTML {

	// https://dom.spec.whatwg.org/#concept-node-insert
	void insert_into(Node& parent, WeakRef<Node> node, WeakRef<Node> child, bool suppressObservers = false)
	{
		ensure_not_null(node);

		std::list nodeList{ SharedRef(node) };

		// 1. Let nodes be node’s children, if node is a DocumentFragment node; otherwise « node ».
		auto& nodes = node.is<DocumentFragment>() ? node->m_child_nodes : nodeList;

		// 2. Let count be nodes’s size.
		size_t count = nodes.size();

		// 3. If count is 0, then return.
		if (count == 0)
		{
			return;
		}

		// 4. If node is a DocumentFragment node, then:
		if (node.is<DocumentFragment>())
		{
			// 1. Remove its children with the suppress observers flag set.
			// 2. Queue a tree mutation record for node with « », nodes, null, and null.
			CSTM_Assert(false);
		}

		// 5. If child is non-null, then:
		if (child != null)
		{
			// 1. For each live range whose start node is parent and start offset is greater than child’s index, increase its start offset by count.
			// 2. For each live range whose end node is parent and end offset is greater than child’s index, increase its end offset by count.
			CSTM_Assert(false);
		}

		// 6. Let previousSibling be child’s previous sibling or parent’s last child if child is null.
		auto previousSibling = child != null ? child->m_previous_sibling : parent.last_child();

		// 7. For each node in nodes, in tree order:
		for (auto& n : nodes)
		{
			// 1. Adopt node into parent’s node document.
			parent.m_owner_document->adopt_node(node);

			// 2. If child is null, then append node to parent’s children.
			if (child.is_null())
			{
				parent.m_child_nodes.push_back(n);
			}

			// 3. Otherwise, insert node into parent’s children before child’s index.
			// 4. If parent is a shadow host whose shadow root’s slot assignment is "named" and node is a slottable, then assign a slot for node.
			// 5. If parent’s root is a shadow root, and parent is a slot whose assigned nodes is the empty list, then run signal a slot change for parent.
			// 6. Run assign slottables for a tree with node’s root.
			// 7. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order:
		}

		// 8. If suppress observers flag is unset, then queue a tree mutation record for parent with nodes, « », previousSibling, and child.
		// 9. Run the children changed steps for parent.
		// 10. Let staticNodeList be a list of nodes, initially « ».
		// 11. For each node of nodes, in tree order:
		// 11.1. For each shadow-including inclusive descendant inclusiveDescendant of node, in shadow-including tree order, append inclusiveDescendant to staticNodeList.
		// 12. For each node of staticNodeList, if node is connected, then run the post-connection steps with node.
	}

	WeakRef<Node> Node::insert_before(WeakRef<Node> node, WeakRef<Node> child)
	{
		ensure_not_null(node);

		// 1. Ensure pre-insertion validity of node into parent before child.
		// 1.1. If parent is not a Document, DocumentFragment, or Element node, then throw a "HierarchyRequestError" DOMException.
		if (!is<Document>() && !is<DocumentFragment>() && !is<Element>())
		{
			CSTM_Assert(false);
		}

		// 1.2. If node is a host-including inclusive ancestor of parent, then throw a "HierarchyRequestError" DOMException.
		// 1.3. If child is non-null and its parent is not parent, then throw a "NotFoundError" DOMException.
		if (!child.is_null() && child->m_parent != this)
		{
			CSTM_Assert(false);
		}

		// 1.4. If node is not a DocumentFragment, DocumentType, Element, or CharacterData node, then throw a "HierarchyRequestError" DOMException
		if (!node.is<DocumentFragment>() && !node.is<DocumentType>() && !node.is<Element>() && !node.is<CharacterData>())
		{
			CSTM_Assert(false);
		}

		// 2. Let referenceChild be child.
		auto referenceChild = child;

		// 3. If referenceChild is node, then set referenceChild to node’s next sibling.
		if (referenceChild == node)
		{
			referenceChild = node->m_next_sibling;
		}

		// 4. Insert node into parent before referenceChild.
		insert_into(*this, node, referenceChild);

		// 5. Return node.
		return node;
	}

	WeakRef<Node> Node::append_child(SharedRef<Node> node)
	{
		ensure_not_null(node);
		return insert_before(node, null);
	}

	std::vector<WeakRef<Node>> Node::descendants(bool includeSelf)
	{
		std::vector<WeakRef<Node>> nodes;

		if (includeSelf)
		{
			nodes.push_back(WeakRef{ this });
		}

		for (auto& child : m_child_nodes)
		{
			nodes.insert_range(nodes.end(), child->descendants(true));
		}

		return nodes;
	}

	SharedRef<Node> Node::create_impl(WeakRef<Document> document, SharedRef<Node> node)
	{
		ensure_not_null(node);

		if (node->is<Document>())
		{
			node->m_owner_document = node.as<Document>();
			return node;
		}

		if (document.is_null())
		{
			return node;
		}

		return document->adopt_node(node);
	}

}
