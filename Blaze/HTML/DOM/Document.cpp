#include "Document.hpp"
#include "DocumentFragment.hpp"
#include "Element.hpp"

namespace Blaze::HTML {

	// https://dom.spec.whatwg.org/#concept-node-adopt
	void adopt_node_impl(SharedRef<Node> node, Document& document)
	{
		ensure_not_null(node);

		// 1. Let oldDocument be node’s node document.
		auto oldDocument = node->document();

		// 2. If node’s parent is non-null, then remove node.
		if (!node->parent().is_null())
		{
			CSTM_Assert(false);
		}

		// 3. If document is not oldDocument, then:
		if (&document != oldDocument)
		{
			auto descendants = node->descendants(true);

			// 1. For each inclusiveDescendant in node’s shadow-including inclusive descendants:
			for (auto& inclusiveDescendant : descendants)
			{
				// 1. Set inclusiveDescendant’s node document to document.
				inclusiveDescendant->m_owner_document = &document;

				// 2. If inclusiveDescendant is an element, then set the node document of each attribute in inclusiveDescendant’s attribute list to document.
				if (inclusiveDescendant.is<Element>())
				{
					CSTM_Assert(false);
				}
			}

			// 2. For each inclusiveDescendant in node’s shadow-including inclusive descendants that is custom, enqueue a custom element callback reaction with inclusiveDescendant, callback name "adoptedCallback", and an argument list containing oldDocument and document.
			// 3. For each inclusiveDescendant in node’s shadow-including inclusive descendants, in shadow-including tree order, run the adopting steps with inclusiveDescendant and oldDocument.
		}
	}

	WeakRef<Node> Document::adopt_node(SharedRef<Node> node)
	{
		ensure_not_null(node);

		// 1. If node is a document, then throw a "NotSupportedError" DOMException.
		if (node.is<Document>())
		{
			CSTM_Assert(false);
		}

		// 2. If node is a shadow root, then throw a "HierarchyRequestError" DOMException.
		// 3. If node is a DocumentFragment node whose host is non-null, then return.
		if (node.is<DocumentFragment>())
		{
			CSTM_Assert(false);
		}

		// 4. Adopt node into this.
		adopt_node_impl(node, *this);

		// 5. Return node
		return node;
	}

}
