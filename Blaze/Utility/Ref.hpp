#pragma once

#include <CSTM/Nullable.hpp>

#include <atomic>
#include <concepts>

namespace Blaze {

	using namespace CSTM;

	struct RefCountStorage
	{
		std::atomic_uint16_t shared_references = 0;
		std::atomic_uint16_t weak_references = 0;
	};

	class RefCounted
	{
	public:
		virtual ~RefCounted() = default;

	private:
		RefCountStorage* m_ref_count_storage = nullptr;

		template<typename T>
		friend class SharedRef;

		template<typename T>
		friend class WeakRef;
	};

	template<typename T, typename U>
	concept custom_rtti = requires(T t)
	{
		{ t.template is<U>() };
	};

	template<typename T>
	class WeakRef;

	template<typename T>
	class SharedRef
	{
	public:
		SharedRef() = default;

		SharedRef(const SharedRef& other) noexcept
			: m_ptr(other.m_ptr)
		{
			increase_ref_count();
		}

		SharedRef(SharedRef&& other) noexcept
			: m_ptr(std::exchange(other.m_ptr, nullptr)) {}

		SharedRef(const WeakRef<T>& other) noexcept
			: m_ptr(other.m_ptr)
		{
			increase_ref_count();
		}

		~SharedRef() noexcept
		{
			decrease_ref_count();
		}

		SharedRef& operator=(const SharedRef& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			decrease_ref_count();
			m_ptr = other.m_ptr;
			increase_ref_count();
			return *this;
		}

		SharedRef& operator=(SharedRef&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			decrease_ref_count();
			m_ptr = std::exchange(other.m_ptr, nullptr);
			return *this;
		}

		[[nodiscard]]
		bool operator==(null_t) const noexcept
		{
			return m_ptr == nullptr;
		}

		[[nodiscard]]
		bool operator==(const SharedRef& other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}

		[[nodiscard]]
		T* operator->() { return m_ptr; }

		[[nodiscard]]
		const T* operator->() const { return m_ptr; }

		[[nodiscard]]
		bool is_null() const { return m_ptr == nullptr; }

		template<typename U>
		requires(std::derived_from<U, T> || std::derived_from<T, U>)
		[[nodiscard]]
		bool is() const
		{
			if (m_ptr == nullptr)
			{
				return false;
			}

			if constexpr (custom_rtti<T, U>)
			{
				return m_ptr->template is<U>();
			}
			else
			{
				return dynamic_cast<U*>(m_ptr) != nullptr;
			}
		}

		template<typename U>
		requires(std::derived_from<U, T> || std::derived_from<T, U>)
		[[nodiscard]]
		SharedRef<U> as() const
		{
			SharedRef<U> result;
			result.m_ptr = dynamic_cast<U*>(m_ptr);
			result.increase_ref_count();
			return result;
		}

		template<typename... Args>
		requires(std::constructible_from<T, Args...>)
		static SharedRef create(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
		{
			SharedRef result;
			result.m_ptr = new T(std::forward<Args>(args)...);
			result.m_ptr->m_ref_count_storage = new RefCountStorage();
			result.m_ptr->m_ref_count_storage->shared_references = 1;
			return result;
		}

	private:
		void increase_ref_count() noexcept
		{
			if (m_ptr == nullptr)
			{
				return;
			}

			m_ptr->m_ref_count_storage->shared_references++;
		}

		void decrease_ref_count() noexcept
		{
			if (m_ptr == nullptr)
			{
				return;
			}

			if (--m_ptr->m_ref_count_storage->shared_references > 0)
			{
				return;
			}

			if (m_ptr->m_ref_count_storage->weak_references == 0)
			{
				delete m_ptr->m_ref_count_storage;
			}

			delete m_ptr;
			m_ptr = nullptr;
		}

	private:
		T* m_ptr = nullptr;

		template<typename U>
		friend class WeakRef;

		template<typename U>
		friend class SharedRef;
	};

	template<typename T>
	class WeakRef
	{
	public:
		WeakRef() noexcept = default;

		WeakRef(const SharedRef<T>& shared) noexcept
			: m_ptr(shared.m_ptr), m_ref_count_storage(m_ptr->m_ref_count_storage)
		{
			increase_ref_count();
		}

		WeakRef(const WeakRef& other) noexcept
			: m_ptr(other.m_ptr), m_ref_count_storage(other.m_ref_count_storage)
		{
			increase_ref_count();
		}

		WeakRef(WeakRef&& other) noexcept
			: m_ptr(std::exchange(other.m_ptr, nullptr)), m_ref_count_storage(std::exchange(other.m_ref_count_storage, nullptr)) {}

		WeakRef(null_t) noexcept
			: m_ptr(nullptr), m_ref_count_storage(nullptr) {}

		WeakRef(T* value) noexcept
			: m_ptr(value), m_ref_count_storage(value->m_ref_count_storage)
		{
			increase_ref_count();
		}

		~WeakRef() noexcept
		{
			decrease_ref_count();
		}

		WeakRef& operator=(const WeakRef& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			decrease_ref_count();
			m_ptr = other.m_ptr;
			m_ref_count_storage = other.m_ref_count_storage;
			increase_ref_count();

			return *this;
		}

		WeakRef& operator=(WeakRef&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}

			decrease_ref_count();
			m_ptr = std::exchange(other.m_ptr, nullptr);
			m_ref_count_storage = std::exchange(other.m_ref_count_storage, nullptr);

			return *this;
		}

		WeakRef& operator=(T* other) noexcept
		{
			if (m_ptr == other)
			{
				return *this;
			}

			m_ptr = other;
			m_ref_count_storage = other->m_ref_count_storage;
			increase_ref_count();

			return *this;
		}

		[[nodiscard]]
		bool operator==(null_t) const noexcept
		{
			return m_ref_count_storage == nullptr;
		}

		[[nodiscard]]
		bool operator==(const WeakRef& other) const noexcept
		{
			return m_ref_count_storage == other.m_ref_count_storage;
		}

		[[nodiscard]]
		bool operator==(const SharedRef<T>& other) const noexcept
		{
			return m_ptr == other.m_ptr;
		}

		[[nodiscard]]
		bool operator==(const T* other) const noexcept
		{
			return m_ptr == other;
		}

		[[nodiscard]]
		bool is_null() const { return m_ref_count_storage == nullptr; }

		[[nodiscard]]
		T* operator->() { return m_ptr; }

		[[nodiscard]]
		const T* operator->() const { return m_ptr; }

		template<typename U>
		requires(std::derived_from<U, T> || std::derived_from<T, U>)
		[[nodiscard]]
		bool is() const
		{
			if (m_ptr == nullptr)
			{
				return false;
			}

			if constexpr (custom_rtti<T, U>)
			{
				return m_ptr->template is<U>();
			}
			else
			{
				return dynamic_cast<U*>(m_ptr) != nullptr;
			}
		}

		template<typename U>
		requires(std::derived_from<U, T> || std::derived_from<T, U>)
		[[nodiscard]]
		WeakRef<U> as() const
		{
			WeakRef<U> result;
			result.m_ptr = dynamic_cast<U*>(m_ptr);
			result.m_ref_count_storage = m_ref_count_storage;
			return result;
		}

	private:
		void increase_ref_count() noexcept
		{
			if (m_ref_count_storage == nullptr)
			{
				return;
			}

			m_ref_count_storage->weak_references++;
		}

		void decrease_ref_count() noexcept
		{
			if (m_ref_count_storage == nullptr)
			{
				return;
			}

			if (--m_ref_count_storage->weak_references > 0)
			{
				return;
			}

			if (m_ref_count_storage->shared_references == 0)
			{
				delete m_ref_count_storage;
				m_ref_count_storage = nullptr;
			}
		}

	private:
		T* m_ptr = nullptr;
		RefCountStorage* m_ref_count_storage = nullptr;

		template<typename U>
		friend class SharedRef;

		template<typename U>
		friend class WeakRef;

	};

}
