#include "URL.hpp"

namespace Blaze::URL {

	bool URL::has_special_scheme() const noexcept
	{
		if (m_scheme == "ftp") return true;
		if (m_scheme == "file") return true;
		if (m_scheme == "http") return true;
		if (m_scheme == "https") return true;
		if (m_scheme == "ws") return true;
		if (m_scheme == "wss") return true;

		return false;
	}

	bool URL::has_opaque_path() const noexcept
	{
		return std::holds_alternative<String>(m_path);
	}

}
