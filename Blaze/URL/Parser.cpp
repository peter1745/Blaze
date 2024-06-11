#include "Parser.hpp"

#include <CSTM/Assert.hpp>
#include <CSTM/Unicode.hpp>

namespace Blaze::URL {

	Result<URL, NullType> Parser::parse(String input, std::optional<URL> base) noexcept
	{
		return basic_parse(std::move(input), std::move(base))
			.and_then([](auto&& url)
			{
				if (url.m_scheme == "blob")
				{
					// NOTE(Peter): Set urls blob entry (See https://url.spec.whatwg.org/#url-parsing)
					CSTM_ToDo();
				}

				return url;
			});
	}

	static const std::vector<uint32_t> C0Controls = []
	{
		std::vector<uint32_t> result;
		for (uint32_t i = 0x0000; i <= 0x001F; i++)
			result.emplace_back(i);
		return result;
	}();

	static const std::vector<uint32_t> C0ControlsAndSpace = []
	{
		auto result = C0Controls;
		result.emplace_back(0x0020);
		return result;
	}();

	void on_input_string_error(StringError error)
	{

	}

	void on_buffer_string_error(StringError error)
	{

	}

	Result<URL, NullType> Parser::basic_parse(String input, std::optional<URL> base) noexcept
	{
		// https://url.spec.whatwg.org/#concept-basic-url-parser

		// 1. If url is not given:
		// 1.1. Set url to a new URL.
		URL url;

		// 1.2. If input contains any leading or trailing C0 control or space, invalid-URL-unit validation error.
		if (input.starts_with_any_code_point(C0ControlsAndSpace) || input.ends_with_any_code_point(C0ControlsAndSpace))
		{
			// report_validation_error(URLValidationError::InvalidURLUnit);
		}

		// 1.3. Remove any leading and trailing C0 control or space from input.
		input = input
			.remove_leading_code_points(C0ControlsAndSpace)
			.or_else(on_input_string_error)
			.value_or(input);

		input = input
			.remove_trailing_code_points(C0ControlsAndSpace)
			.or_else(on_input_string_error)
			.value_or(input);

		// 2. If input contains any ASCII tab or newline, invalid-URL-unit validation error.
		if (input.contains_any("\t\n\r"))
		{
			// report_validation_error(URLValidationError::InvalidURLUnit);
		}

		// 3. Remove all ASCII tab or newline from input.
		input = input
			.remove_any("\t\n\r")
			.or_else(on_input_string_error)
			.value_or(input);

		// 4. Let state be state override if given, or scheme start state otherwise.
		State state = State::SchemeStart;

		// 5. Set encoding to the result of getting an output encoding from encoding.

		// 6. Let buffer be the empty string.
		String buffer;

		// 7. Let atSignSeen, insideBrackets, and passwordTokenSeen be false.
		bool atSignSeen = false,
			insideBrackets = false,
			passwordTokenSeen = false;

		// 8. Let pointer be a pointer for input.
		constexpr int32_t EOFCodePoint = -1;
		int32_t pointer = 0;

		// 9. Keep running the following state machine by switching on state.
		// If after a run pointer points to the EOF code point, go to the next step.
		// Otherwise, increase pointer by 1 and continue with the state machine.

		while (pointer != EOFCodePoint)
		{
			const auto c = CodePointIterator{ input }
						.code_point_at(pointer)
						.or_else([]{ CSTM_Assert(false); })
						.value();

			std::vector<uint32_t> remaining;
			CodePointIterator{ input }.store(remaining, pointer);

			switch (state)
			{
			case State::SchemeStart:
			{
				// 1. If c is an ASCII alpha, append c, lowercased, to buffer, and set state to scheme state.
				if (is_ascii_alpha_code_point(c))
				{
					buffer = buffer
						.append_code_points(std::array{ ascii_to_lower_code_point(c) })
						.or_else(on_buffer_string_error)
						.value_or(buffer);
					state = State::Scheme;
					break;
				}

				// 2. Otherwise, if state override is not given, set state to no scheme state and decrease pointer by 1.
				state = State::NoScheme;
				pointer--;
				break;

				// 3. Otherwise, return failure. NOTE: Never happens since we don't have state override
			}
			case State::Scheme:
			{
				// 1. If c is an ASCII alphanumeric, U+002B (+), U+002D (-), or U+002E (.), append c, lowercased, to buffer.
				if (is_ascii_alphanumeric_code_point(c) || c == '+' || c == '-' || c == '.')
				{
					buffer = buffer
						.append_code_points(std::array{ ascii_to_lower_code_point(c) })
						.or_else(on_buffer_string_error)
						.value_or(buffer);
					break;
				}

				// 2. Otherwise, if c is U+003A (:), then:
				if (c == ':')
				{
					// 1. If state override is given, then:
					// 1.1 If url’s scheme is a special scheme and buffer is not a special scheme, then return.
					// 1.2 If url’s scheme is not a special scheme and buffer is a special scheme, then return.
					// 1.3 If url includes credentials or has a non-null port, and buffer is "file", then return.
					// 1.4 If url’s scheme is "file" and its host is an empty host, then return.

					// 2. Set url’s scheme to buffer.
					url.m_scheme = buffer;

					// 3. If state override is given, then:
					// 3.1 If url’s port is url’s scheme’s default port, then set url’s port to null.
					// 3.2 Return.

					// 4. Set buffer to the empty string.
					buffer = {};

					// 5. If url’s scheme is "file", then:
					if (url.m_scheme == "file")
					{
						// 1. If remaining does not start with "//", special-scheme-missing-following-solidus validation error.
						// 2. Set state to file state.
						CSTM_ToDo();
						break;
					}

					// 6. Otherwise, if url is special, base is non-null, and base’s scheme is url’s scheme:
					if (url.has_special_scheme() && base.has_value() && base->m_scheme == url.m_scheme)
					{
						// 1. Assert: base is special (and therefore does not have an opaque path).
						// 2. Set state to special relative or authority state.
						CSTM_ToDo();
						break;
					}

					// 7. Otherwise, if url is special, set state to special authority slashes state.
					if (url.has_special_scheme())
					{
						state = State::SpecialAuthoritySlashes;
						break;
					}

					// 8. Otherwise, if remaining starts with an U+002F (/), set state to path or authority state and increase pointer by 1.
					if (remaining[0] == '/')
					{
						CSTM_ToDo();
					}

					// 9. Otherwise, set url’s path to the empty string and set state to opaque path state.
					url.m_path = {};
					state = State::Opaque;
					break;
				}

				// 3. Otherwise, if state override is not given, set buffer to the empty string, state to no scheme state, and start over (from the first code point in input).
				buffer = {};
				state = State::NoScheme;
				pointer = -1;
				break;

				// 4. Otherwise, return failure.
			}
			case State::SpecialAuthoritySlashes:
			{
				// 1. If c is U+002F (/) and remaining starts with U+002F (/),
				// then set state to special authority ignore slashes state and increase pointer by 1.
				if (c == '/' && remaining[0] == '/')
				{
					state = State::SpecialAuthorityIgnoreSlashes;
					pointer++;
					break;
				}

				// 2. Otherwise, special-scheme-missing-following-solidus validation error,
				// set state to special authority ignore slashes state and decrease pointer by 1.
				// report_validation_error(URLValidationError::SpecialSchemeMissingFollowingSolidus);
				state = State::SpecialAuthorityIgnoreSlashes;
				pointer--;
				break;
			}
			case State::SpecialAuthorityIgnoreSlashes:
			{
				// 1. If c is neither U+002F (/) nor U+005C (\), then set state to authority state and decrease pointer by 1.
				if (c != '/' && c != '\\')
				{
					state = State::Authority;
					pointer--;
					break;
				}

				// 2. Otherwise, special-scheme-missing-following-solidus validation error.
				// report_validation_error(URLValidationError::SpecialSchemeMissingFollowingSolidus);
				break;
			}
			case State::Authority:
			{
				// 1. If c is U+0040 (@), then:
				if (c == '@')
				{
					// 1. Invalid-credentials validation error.
					// 2. If atSignSeen is true, then prepend "%40" to buffer.
					// 3. Set atSignSeen to true.
					// 4. For each codePoint in buffer:
					// 4.1 If codePoint is U+003A (:) and passwordTokenSeen is false, then set passwordTokenSeen to true and continue.
					// 4.2 Let encodedCodePoints be the result of running UTF-8 percent-encode codePoint using the userinfo percent-encode set.
					// 4.3 If passwordTokenSeen is true, then append encodedCodePoints to url’s password.
					// 4.4 Otherwise, append encodedCodePoints to url’s username.
					// 5. Set buffer to the empty string.
					CSTM_ToDo();
					break;
				}

				// 2. Otherwise, if one of the following is true:
				if ((c == EOFCodePoint || c == '/' || c == '?' || c == '#') || (url.has_special_scheme() && c == '\\'))
				{
					// 1. If atSignSeen is true and buffer is the empty string, host-missing validation error, return failure.
					if (atSignSeen && buffer.is_empty())
					{
						// report_validation_error(URLValidationError::HostMissing);
						return Null;
					}

					// 2. Decrease pointer by buffer’s code point length + 1, set buffer to the empty string, and set state to host state.
					pointer -= static_cast<int32_t>(CodePointIterator{ buffer }.count() + 1);
					buffer = {};
					state = State::Host;
					break;
				}

				// 3. Otherwise, append c to buffer.
				buffer = buffer
						.append_code_points(std::array{ c })
						.or_else(on_buffer_string_error)
						.value_or(buffer);
				break;
			}
			case State::Host:
			case State::Hostname:
			{
				// 1. If state override is given and url’s scheme is "file", then decrease pointer by 1 and set state to file host state.
				// 2. Otherwise, if c is U+003A (:) and insideBrackets is false, then:
				if (c == ':' && !insideBrackets)
				{
					// 1. If buffer is the empty string, host-missing validation error, return failure.
					// 2. If state override is given and state override is hostname state, then return.
					// 3. Let host be the result of host parsing buffer with url is not special.
					// 4. If host is failure, then return failure.
					// 5. Set url’s host to host, buffer to the empty string, and state to port state.
					CSTM_ToDo();
				}

				// 3. Otherwise, if one of the following is true:
				if ((c == EOFCodePoint || c == '/' || c == '?' || c == '#') || (url.has_special_scheme() && c == '\\'))
				{
					pointer--;

					// 1. If url is special and buffer is the empty string, host-missing validation error, return failure.
					if (url.has_special_scheme() && buffer.is_empty())
					{
						// report_validation_error(URLValidationError::HostMissing);
						return Null;
					}

					// 2. Otherwise, if state override is given, buffer is the empty string, and either url includes credentials or url’s port is non-null, return.
					// 3. Let host be the result of host parsing buffer with url is not special.
					auto host = parse_host(buffer, !url.has_special_scheme());

					// 4. If host is failure, then return failure.
					if (host.has_error())
					{
						return Null;
					}

					// 5. Set url’s host to host, buffer to the empty string, and state to path start state.
					url.m_host = host.value();
					buffer = {};
					state = State::PathStart;
					break;

					// 6. If state override is given, then return.
				}

				// 4. Otherwise:
				// 4.1 If c is U+005B ([), then set insideBrackets to true.
				if (c == '[')
				{
					insideBrackets = true;
				}

				// 4.2 If c is U+005D (]), then set insideBrackets to false.
				if (c == ']')
				{
					insideBrackets = false;
				}

				// 4.3 Append c to buffer.
				buffer = buffer
						.append_code_points(std::array{ c })
						.or_else(on_buffer_string_error)
						.value_or(buffer);
				break;
			}
			default:
				CSTM_ToDo();
			}

			pointer++;
		}

		return Null;
	}

	Result<Host, NullType> Parser::parse_host(String input, bool isOpaque) noexcept
	{
		// 1. If input starts with U+005B ([), then:
		if (input.starts_with("["))
		{
			// 1. If input does not end with U+005D (]), IPv6-unclosed validation error, return failure.
			if (!input.ends_with("]"))
			{
				// report_validation_error(URLValidationError::IPv6Unclosed);
				return Null;
			}

			// 2. Return the result of IPv6 parsing input with its leading U+005B ([) and trailing U+005D (]) removed.
			CSTM_ToDo();
		}

		// 2. If isOpaque is true, then return the result of opaque-host parsing input.
		if (isOpaque)
		{
			return parse_opaque_host(input);
		}

		// 3. Assert: input is not the empty string.
		CSTM_Assert(!input.is_empty());

		// 4. Let domain be the result of running UTF-8 decode without BOM on the percent-decoding of input.
		auto domain = percent_decode_bytes(input);

		// 5. Let asciiDomain be the result of running domain to ASCII with domain and false.


		// 6. If asciiDomain is failure, then return failure.
		// 7. If asciiDomain contains a forbidden domain code point, domain-invalid-code-point validation error, return failure.
		// 8. If asciiDomain ends in a number, then return the result of IPv4 parsing asciiDomain.
		// 9. Return asciiDomain.
		return Null;
	}

	Result<Host, NullType> Parser::parse_opaque_host(String input) noexcept
	{
		return Null;
	}

	Result<String, NullType> Parser::percent_decode_bytes(String input) noexcept
	{
		// 1. Let output be an empty byte sequence.
		std::vector<byte> output;
		output.reserve(input.byte_count());

		// 2. For each byte byte in input:
		for (size_t i = 0; i < input.byte_count(); i++)
		{
			const auto b = input.byte_at(i);

			// 1. If byte is not 0x25 (%), or if we can't safely read 2 more bytes from i, then append byte to output.
			if (b != '%' || i + 2 >= input.byte_count())
			{
				output.push_back(b);
				continue;
			}

			const auto b1 = input.byte_at(i + 1);
			const auto b2 = input.byte_at(i + 2);

			// 2. Otherwise, if byte is 0x25 (%) and the next two bytes after byte in input are not in the ranges 0x30 (0) to 0x39 (9),
			//		0x41 (A) to 0x46 (F), and 0x61 (a) to 0x66 (f), all inclusive, append byte to output.
			if (!is_hexadecimal(b1) || !is_hexadecimal(b2))
			{
				output.push_back(b);
				continue;
			}

			// 3. Let bytePoint be the two bytes after byte in input, decoded, and then interpreted as hexadecimal number.
			const auto bytePoint = from_hex2(b1, b2);

			// 4. Append a byte whose value is bytePoint to output.
			output.push_back(bytePoint);

			// 5. Skip the next two bytes in input.
			i += 2;
		}

		// 3. Return output.
		return String::create(Span<byte>{ output });
	}

	Result<String, NullType> Parser::domain_to_ascii(String domain, bool beStrict) noexcept
	{
		return Null;
	}

}
