#pragma once

#include "URL.hpp"

#include <CSTM/StringView.hpp>
#include <CSTM/Result.hpp>

namespace Blaze::URL {

	using namespace CSTM;

	class Parser
	{
		enum class State
		{
			SchemeStart,
			Scheme,
			NoScheme,
			Opaque,
			SpecialAuthoritySlashes,
			SpecialAuthorityIgnoreSlashes,
			Authority,
			Host,
			Hostname,
			PathStart,
		};
	public:
		[[nodiscard]]
		Result<URL, NullType> parse(String input, std::optional<URL> base = std::nullopt) noexcept;

	private:
		[[nodiscard]]
		Result<URL, NullType> basic_parse(String input, std::optional<URL> base = std::nullopt) noexcept;

		[[nodiscard]]
		Result<Host, NullType> parse_host(String input, bool isOpaque = false) noexcept;

		[[nodiscard]]
		Result<Host, NullType> parse_opaque_host(String input) noexcept;

		[[nodiscard]]
		Result<String, NullType> percent_decode_bytes(String input) noexcept;

		[[nodiscard]]
		Result<String, NullType> domain_to_ascii(String domain, bool beStrict) noexcept;
	};

}
