#include "Parser.hpp"

#include <CSTM/Scoped.hpp>

#include <deque>
#include <CSTM/StringView.hpp>
#include <CSTM/Unicode.hpp>

#include <stack>

namespace Blaze::HTML {

	using namespace CSTM;

	CSTM_TagType(Missing);

	struct DOCTYPEToken
	{
		std::variant<Missing, String> name{ Missing{} };
		std::variant<Missing, String> public_identifier{ Missing{} };
		std::variant<Missing, String> system_identifier{ Missing{} };
		bool force_quirks;
	};

	/*
	 struct Attribute
	 {
		String name;

		// Don't actually std::any here
		std::any value;
	 };
	 */

	struct StartTagToken
	{
		String name;
		bool self_closing;
		// std::vector<Attribute> attributes;
	};

	struct EndTagToken
	{
		String name;
		bool self_closing;
		// std::vector<Attribute> attributes;
	};

	struct CommentToken
	{
		String data;
	};

	struct CharacterToken
	{
		uint32_t data;
	};

	struct EOFToken {};

	using Token = std::variant<
		DOCTYPEToken,
		StartTagToken,
		EndTagToken,
		CommentToken,
		CharacterToken,
		EOFToken
	>;

	enum class TokenizerState
	{
		None,
		Data,
		CharacterReference,
		TagOpen,
		EndTagOpen,
		MarkupDeclarationOpen,
		TagName,
		BogusComment,
		CommentStartDash,
		Comment,
		CommentLessThanSign,
		CommentLessThanSignBang,
		CommentEndDash,
		DOCTYPE,
		BeforeDOCTYPEName,
		DOCTYPEName,

		Max,
	};

	class Tokenizer
	{
	public:
		Tokenizer() = default;

		explicit Tokenizer(const String& input)
			: m_input(input), m_position(0), m_state(TokenizerState::Data)
		{
			CodePointIterator{ input }.store(m_code_points);
		}

		Token next_token()
		{
			using enum TokenizerState;

#define ReconsumeCurrent()\
			m_position--;\
			break

			if (!m_queued_tokens.empty())
			{
				auto token = m_queued_tokens.back();
				m_queued_tokens.pop_back();
				return token;
			}

			auto consume_next = [&]() -> Result<uint32_t, NullType>
			{
				if (m_position + 1 >= m_code_points.size())
				{
					return Null;
				}

				return m_code_points[m_position++];
			};

			while (m_state != None)
			{
				auto c = consume_next();

				switch (m_state)
				{
				case Data:
				{
					if (!c.has_value())
					{
						// Emit an end-of-file token.
						CSTM_Assert(false);
					}

					if (c.value() == '&')
					{
						// Set the return state to the data state. Switch to the character reference state.
						CSTM_Assert(false);
					}
					else if (c.value() == '<')
					{
						// Switch to the tag open state.
						m_state = TagOpen;
					}
					else if (c.value() == '\0')
					{
						// This is an unexpected-null-character parse error. Emit the current input character as a character token.
						CSTM_Assert(false);
					}
					else
					{
						// Emit the current input character as a character token.
						return CharacterToken(c.value());
					}

					break;
				}
				case TagOpen:
				{
					if (!c.has_value())
					{
						// This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN SIGN character token and an end-of-file token.
						CSTM_Assert(false);
					}

					if (c.value() == '!')
					{
						// Switch to the markup declaration open state.
						m_state = MarkupDeclarationOpen;
					}
					else if (c.value() == '/')
					{
						// Switch to the end tag open state.
						CSTM_Assert(false);
					}
					else if (is_ascii_alpha_code_point(c.value()))
					{
						// Create a new start tag token, set its tag name to the empty string. Reconsume in the tag name state.
						CSTM_Assert(false);
					}
					else if (c.value() == '?')
					{
						// This is an unexpected-question-mark-instead-of-tag-name parse error. Create a comment token whose data is the empty string.
						// Reconsume in the bogus comment state.
						CSTM_Assert(false);
					}
					else
					{
						// This is an invalid-first-character-of-tag-name parse error. Emit a U+003C LESS-THAN SIGN character token. Reconsume in the data state.
						CSTM_Assert(false);
					}

					break;
				}
				case MarkupDeclarationOpen:
				{
					const auto chars = m_input.view(m_position - 1);
					CSTM_Assert(chars.has_value());

					// NOTE(Peter): Backtracking so we can more clearly consume several characters
					m_position--;

					if (chars.value().starts_with("--"))
					{
						// Consume those two characters, create a comment token whose data is the empty string, and switch to the comment start state.
						CSTM_Assert(false);
					}
					else if (chars.value().starts_with("DOCTYPE")) // TODO(Peter): Case-insensitive
					{
						// Consume those characters and switch to the DOCTYPE state.
						m_position += strlen("DOCTYPE");
						m_state = DOCTYPE;
					}
					else if (chars.value().starts_with("[CDATA["))
					{
						// Consume those characters.
						// If there is an adjusted current node and it is not an element in the HTML namespace,
						// then switch to the CDATA section state.
						// Otherwise, this is a cdata-in-html-content parse error.
						// Create a comment token whose data is the "[CDATA[" string.
						// Switch to the bogus comment state.
						CSTM_Assert(false);
					}
					else
					{
						// This is an incorrectly-opened-comment parse error.
						// Create a comment token whose data is the empty string.
						// Switch to the bogus comment state (don't consume anything in the current state).
						CSTM_Assert(false);
					}

					break;
				}
				case DOCTYPE:
				{
					if (!c.has_value())
					{
						// This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set its force-quirks flag to on. Emit the current token. Emit an end-of-file token.
						CSTM_Assert(false);
					}

					if (c.value() == '\t' || c.value() == '\n' || c.value() == '\f' || c.value() == ' ')
					{
						// Switch to the before DOCTYPE name state.
						m_state = BeforeDOCTYPEName;
					}
					else if (c.value() == '>')
					{
						// Reconsume in the before DOCTYPE name state.
						m_state = BeforeDOCTYPEName;
						ReconsumeCurrent();
					}
					else
					{
						// This is a missing-whitespace-before-doctype-name parse error. Reconsume in the before DOCTYPE name state.
						CSTM_Assert(false);
					}
				}
				case BeforeDOCTYPEName:
				{
					if (!c.has_value())
					{
						// This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set its force-quirks flag to on. Emit the current token. Emit an end-of-file token.
						CSTM_Assert(false);
					}

					if (c.value() == '\t' || c.value() == '\n' || c.value() == '\f' || c.value() == ' ')
					{
						// Ignore the character.
						break;
					}

					if (is_ascii_upper_alpha_code_point(c.value()))
					{
						// Create a new DOCTYPE token.
						DOCTYPEToken token{};

						// Set the token's name to the lowercase version of the current input character (add 0x0020 to the character's code point).
						token.name = String::create(Span{ c.value() + 0x0020 });
						m_current_token = token;

						// Switch to the DOCTYPE name state.
						m_state = DOCTYPEName;
					}
					else if (c.value() == '\0')
					{
						// This is an unexpected-null-character parse error. Create a new DOCTYPE token. Set the token's name to a U+FFFD REPLACEMENT CHARACTER character. Switch to the DOCTYPE name state.
						CSTM_Assert(false);
					}
					else if (c.value() == '>')
					{
						// This is a missing-doctype-name parse error. Create a new DOCTYPE token. Set its force-quirks flag to on. Switch to the data state. Emit the current token.
						CSTM_Assert(false);
					}
					else
					{
						// Create a new DOCTYPE token.
						DOCTYPEToken token{};

						// Set the token's name to the current input character.
						token.name = String::create(Span{ c.value() });
						m_current_token = token;

						// Switch to the DOCTYPE name state.
						m_state = DOCTYPEName;
					}

					break;
				}
				case DOCTYPEName:
				{
					if (!c.has_value())
					{
						// This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set its force-quirks flag to on. Emit the current token. Emit an end-of-file token.
						CSTM_Assert(false);
					}

					if (c.value() == '\t' || c.value() == '\n' || c.value() == '\f' || c.value() == ' ')
					{
						// Switch to the after DOCTYPE name state.
						break;
					}

					DOCTYPEToken& token = get_token_as<DOCTYPEToken>();

					if (c.value() == '>')
					{
						// Switch to the data state. Emit the current DOCTYPE token.
						m_state = Data;
						return token;
					}
					else if (is_ascii_upper_alpha_code_point(c.value()))
					{
						// Append the lowercase version of the current input character (add 0x0020 to the character's code point) to the current DOCTYPE token's name.
						CSTM_Assert(false);
					}
					else if (c.value() == '\0')
					{
						// This is an unexpected-null-character parse error. Append a U+FFFD REPLACEMENT CHARACTER character to the current DOCTYPE token's name.
						CSTM_Assert(false);
					}
					else
					{
						// Append the current input character to the current DOCTYPE token's name.
						token.name = std::get_if<String>(&token.name)->append_code_points(Span{ c.value() }).value();
					}

					break;
				}
				default:
					CSTM_Assert(false);
					return EOFToken{};
				}
			}

			return EOFToken{};
		}

	private:
		template<typename T>
		auto& get_token_as()
		{
			auto* token = std::get_if<T>(&m_current_token.value());
			CSTM_Assert(token != nullptr);
			return *token;
		}

	private:
		String m_input;
		std::vector<uint32_t> m_code_points;
		size_t m_position;

		TokenizerState m_state;
		std::deque<Token> m_queued_tokens;

		std::optional<Token> m_current_token;
	};

	void Parser::parse(const String& input)
	{
		Tokenizer tokenizer(input);

		std::stack<WeakRef<Node>> openElements;

		/*
			A Document object has an associated parser cannot change the mode flag (a boolean). It is initially false.
			When the user agent is to apply the rules for the "initial" insertion mode, the user agent must handle the token as follows:
		*/

#define token_is(Type) auto* token = std::get_if<Type>(&next); token

		while (true)
		{
			auto next = tokenizer.next_token();

			// https://html.spec.whatwg.org/multipage/parsing.html#tree-construction

			if (token_is(DOCTYPEToken))
			{
				const auto* name = std::get_if<String>(&token->name);
				const auto* publicId = std::get_if<String>(&token->public_identifier);
				const auto* systemId = std::get_if<String>(&token->system_identifier);

				// If the DOCTYPE token's name is not "html", or the token's public identifier is not missing, or the token's system identifier is neither missing nor "about:legacy-compat", then there is a parse error.
				if ((name != nullptr && *name != "html") || publicId != nullptr || (systemId != nullptr && *systemId != "about:legacy-compat"))
				{
					CSTM_Assert(false);
				}

				/*
					Append a DocumentType node to the Document node, with its
					name set to the name given in the DOCTYPE token, or the empty string
					if the name was missing; its public ID set to the public identifier given
					in the DOCTYPE token, or the empty string if the public identifier
					was missing; and its system ID set to the system identifier given
					in the DOCTYPE token, or the empty string if the
					system identifier was missing.
				*/

				String nameOrEmpty = name != nullptr ? *name : String{};
				String publicIdOrEmpty = publicId != nullptr ? *publicId : String{};
				String systemIdOrEmpty = systemId != nullptr ? *systemId : String{};

				auto documentType = Node::create<DocumentType>(m_document, nameOrEmpty, publicIdOrEmpty, systemIdOrEmpty);
			}
		}
	}

}
