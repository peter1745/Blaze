#pragma once

#include <CSTM/String.hpp>

#include <variant>

namespace Blaze::URL {

	using namespace CSTM;

	using IPv4Address = uint32_t;
	using IPv6Address = std::array<uint16_t, 8>;

	using Host = std::variant<IPv4Address, IPv6Address, String>;
	using URLPath = std::variant<String, std::vector<String>>;

	class URL
	{
	public:
		[[nodiscard]]
		bool has_special_scheme() const noexcept;

		[[nodiscard]]
		bool has_opaque_path() const noexcept;

	private:
		String m_scheme;
		std::optional<Host> m_host { std::nullopt };
		std::optional<uint16_t> m_port { std::nullopt };
		URLPath m_path { std::vector<String>{} };

		friend class Parser;
	};

}
