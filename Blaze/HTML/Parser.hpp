#pragma once

#include "DOM/Document.hpp"

#include <CSTM/String.hpp>

namespace Blaze::HTML {

	using namespace CSTM;

	class Parser
	{
	public:
		explicit Parser(WeakRef<Document> document)
			: m_document(document)
		{
			ensure_not_null(m_document);
		}

		void parse(const String& input);

	private:
		WeakRef<Document> m_document;
	};

}
