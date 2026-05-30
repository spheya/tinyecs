#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>

#include "meta.hpp"

namespace ecs {

	template<typename T, size_type N>
	class small_vector {
	public:
		using value_type = T;
		using size_type = ecs::size_type;
		using iterator = T*;
		using const_iterator = const T*;
		constexpr static size_type local_storage_size = N;

	public:
		small_vector() noexcept;
		small_vector(std::initializer_list<T> values);
		small_vector(const small_vector<T, N>& other);
		small_vector<T, N>& operator=(const small_vector<T, N>& other);
		small_vector(small_vector<T, N>&& other) noexcept;
		small_vector<T, N>& operator=(small_vector<T, N>&& other) noexcept;
		~small_vector();

		[[nodiscard]] size_type size() const noexcept;
		[[nodiscard]] size_type capacity() const noexcept;
		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] T* data() noexcept;
		[[nodiscard]] const T* data() const noexcept;

		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator end() const noexcept;

		[[nodiscard]] T& front() noexcept;
		[[nodiscard]] const T& front() const noexcept;
		[[nodiscard]] T& back() noexcept;
		[[nodiscard]] const T& back() const noexcept;

		template<typename... Args>
		void emplace_back(Args&&... args);
		void push_back(const T& value);
		void pop_back();

		void reserve(size_type capacity);
		void resize(size_type size);
		void clear();

		[[nodiscard]] T& operator[](size_type idx) noexcept;
		[[nodiscard]] const T& operator[](size_type idx) const noexcept;

		[[nodiscard]] bool operator==(const small_vector<T, N>& other) const noexcept;
		[[nodiscard]] bool operator!=(const small_vector<T, N>& other) const noexcept;

	private:
		void destroy();
		T* local_storage() noexcept;
		const T* local_storage() const noexcept;

	private:
		T* m_data;
		size_type m_size = 0;
		size_type m_capacity = N;
		alignas(alignof(T)) char m_local_storage[sizeof(T) * N];
	};

	// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init)
	template<typename T, size_type N>
	small_vector<T, N>::small_vector() noexcept : m_data(local_storage()) {}

	template<typename T, size_type N>
	small_vector<T, N>::small_vector(std::initializer_list<T> values) : m_size(values.size()), m_capacity(values.size()) {
		if(values.size() <= N) {
			m_data = local_storage();
		} else {
			m_data = static_cast<T*>(malloc(values.size() * sizeof(T)));
			if(m_data == nullptr) throw std::bad_alloc();
		}

		if constexpr(std::is_trivially_copyable_v<T>) {
			memcpy(m_data, values.begin(), values.size() * sizeof(T));
		} else {
			for(size_type i = 0; i < values.size(); ++i) std::construct_at(m_data + i, values[i]);
		}
	}

	template<typename T, size_type N>
	small_vector<T, N>::small_vector(const small_vector<T, N>& other) : m_size(other.m_size), m_capacity(other.m_capacity) {
		if(other.m_data == other.local_storage()) {
			m_data = local_storage();
		} else {
			m_data = static_cast<T*>(malloc(other.m_capacity * sizeof(T)));
			if(m_data == nullptr) throw std::bad_alloc();
		}

		if constexpr(std::is_trivially_copyable_v<T>) {
			memcpy(m_data, other.m_data, other.m_size * sizeof(T));
		} else {
			for(size_type i = 0; i < other.m_size; ++i) std::construct_at(m_data + i, other.m_data[i]);
		}
	}

	template<typename T, size_type N>
	small_vector<T, N>& small_vector<T, N>::operator=(const small_vector<T, N>& other) {
		if(this == &other) return *this;
		destroy();

		m_size = other.m_size;
		m_capacity = other.m_capacity;

		if(other.m_data == other.local_storage()) {
			m_data = local_storage();
		} else {
			m_data = static_cast<T*>(malloc(other.m_capacity * sizeof(T)));
			if(m_data == nullptr) throw std::bad_alloc();
		}

		if constexpr(std::is_trivially_copyable_v<T>) {
			memcpy(m_data, other.m_data, other.m_size * sizeof(T));
		} else {
			for(size_type i = 0; i < other.m_size; ++i) std::construct_at(m_data + i, other.m_data[i]);
		}
		return *this;
	}

	template<typename T, size_type N>
	small_vector<T, N>::small_vector(small_vector<T, N>&& other) noexcept : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
		if(other.m_data == other.local_storage()) {
			m_data = local_storage();
			if constexpr(std::is_trivially_copyable_v<T>) {
				memcpy(m_data, other.m_data, other.m_size * sizeof(T));
			} else {
				for(size_type i = 0; i < other.m_size; ++i) {
					std::construct_at(m_data + i, std::move(other.m_data[i]));
					std::destroy_at(other.m_data + i);
				}
			}
		}

		other.m_data = other.local_storage();
		other.m_size = 0;
		other.m_capacity = N;
	}

	template<typename T, size_type N>
	small_vector<T, N>& small_vector<T, N>::operator=(small_vector<T, N>&& other) noexcept {
		if(this == &other) return *this;
		destroy();

		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;

		if(other.m_data == other.local_storage()) {
			m_data = local_storage();
			if constexpr(std::is_trivially_copyable_v<T>) {
				memcpy(m_data, other.m_data, other.m_size * sizeof(T));
			} else {
				for(size_type i = 0; i < other.m_size; ++i) {
					std::construct_at(m_data + i, std::move(other.m_data[i]));
					std::destroy_at(other.m_data + i);
				}
			}
		}

		other.m_data = other.local_storage();
		other.m_size = 0;
		other.m_capacity = N;

		return *this;
	}

	template<typename T, size_type N>
	small_vector<T, N>::~small_vector() {
		destroy();
	}
	// NOLINTEND(cppcoreguidelines-pro-type-member-init)

	template<typename T, size_type N>
	void small_vector<T, N>::destroy() {
		if constexpr(!std::is_trivially_destructible_v<T>)
			for(T* it = m_data; it != m_data + m_size; ++it) std::destroy_at(it);
		if(m_data != local_storage()) free(m_data);
	}

	template<typename T, size_type N>
	T* small_vector<T, N>::local_storage() noexcept {
		return reinterpret_cast<T*>(m_local_storage);
	}

	template<typename T, size_type N>
	const T* small_vector<T, N>::local_storage() const noexcept {
		return reinterpret_cast<const T*>(m_local_storage);
	}

	template<typename T, size_type N>
	size_type small_vector<T, N>::size() const noexcept {
		return m_size;
	}

	template<typename T, size_type N>
	size_type small_vector<T, N>::capacity() const noexcept {
		return m_capacity;
	}

	template<typename T, size_type N>
	bool small_vector<T, N>::empty() const noexcept {
		return m_size == 0;
	}

	template<typename T, size_type N>
	T* small_vector<T, N>::data() noexcept {
		return m_data;
	}

	template<typename T, size_type N>
	const T* small_vector<T, N>::data() const noexcept {
		return m_data;
	}

	template<typename T, size_type N>
	small_vector<T, N>::iterator small_vector<T, N>::begin() noexcept {
		return m_data;
	}

	template<typename T, size_type N>
	small_vector<T, N>::const_iterator small_vector<T, N>::begin() const noexcept {
		return m_data;
	}

	template<typename T, size_type N>
	small_vector<T, N>::iterator small_vector<T, N>::end() noexcept {
		return m_data + m_size;
	}

	template<typename T, size_type N>
	small_vector<T, N>::const_iterator small_vector<T, N>::end() const noexcept {
		return m_data + m_size;
	}

	template<typename T, size_type N>
	T& small_vector<T, N>::front() noexcept {
		return *m_data;
	}

	template<typename T, size_type N>
	const T& small_vector<T, N>::front() const noexcept {
		return *m_data;
	}

	template<typename T, size_type N>
	T& small_vector<T, N>::back() noexcept {
		return *(m_data + m_size);
	}

	template<typename T, size_type N>
	const T& small_vector<T, N>::back() const noexcept {
		return *(m_data + m_size);
	}

	template<typename T, size_type N>
	template<typename... Args>
	void small_vector<T, N>::emplace_back(Args&&... args) {
		if(m_size == m_capacity) reserve(2 * m_capacity);
		std::construct_at(m_data + m_size, std::forward<Args>(args)...);
		++m_size;
	}

	template<typename T, size_type N>
	void small_vector<T, N>::push_back(const T& value) {
		if(m_size == m_capacity) reserve(2 * m_capacity);
		std::construct_at(m_data + m_size, value);
		++m_size;
	}

	template<typename T, size_type N>
	void small_vector<T, N>::pop_back() {
		std::destroy_at(m_data + m_size);
		--m_size;
	}

	template<typename T, size_type N>
	void small_vector<T, N>::reserve(size_type capacity) {
		if(capacity < m_capacity) return;

		if constexpr(std::is_trivially_copyable_v<T>) {
			if(m_data == local_storage()) {
				m_data = static_cast<T*>(malloc(capacity * sizeof(T)));
				if(m_data == nullptr) throw std::bad_alloc();
				memcpy(m_data, local_storage(), m_size * sizeof(T));
			} else {
				// realloc is not great if the size is not close to the capacity, but I don't think that case happens a lot
				void* newData = realloc(m_data, capacity * sizeof(T));
				if(newData == nullptr) throw std::bad_alloc();
				m_data = static_cast<T*>(newData);
			}
		} else {
			T* newData = static_cast<T*>(malloc(capacity * sizeof(T)));
			if(newData == nullptr) throw std::bad_alloc();
			for(size_type i = 0; i < m_size; ++i) {
				std::construct_at(newData + i, std::move(m_data[i]));
				std::destroy_at(m_data + i);
			}
			if(m_data != local_storage()) free(m_data);
			m_data = newData;
		}
		m_capacity = capacity;
	}

	template<typename T, size_type N>
	void small_vector<T, N>::resize(size_type size) {
		if(size > m_capacity) reserve(size);

		for(T* it = m_data + m_size; it < m_data + size; ++it) std::construct_at(it);

		if constexpr(!std::is_trivially_destructible_v<T>)
			for(T* it = m_data + size; it < m_data + m_size; ++it) std::destroy_at(it);

		m_size = size;
	}

	template<typename T, size_type N>
	void small_vector<T, N>::clear() {
		if constexpr(!std::is_trivially_destructible_v<T>)
			for(T* it = m_data; it != m_data + m_size; ++it) std::destroy_at(it);
		m_size = 0;
	}

	template<typename T, size_type N>
	T& small_vector<T, N>::operator[](size_type idx) noexcept {
		return m_data[idx];
	}

	template<typename T, size_type N>
	const T& small_vector<T, N>::operator[](size_type idx) const noexcept {
		return m_data[idx];
	}

	template<typename T, size_type N>
	bool small_vector<T, N>::operator==(const small_vector<T, N>& other) const noexcept {
		if(m_size != other.m_size) return false;
		for(size_type i = 0; i < m_size; ++i)
			if(m_data[i] != other.m_data[i]) return false;
	}

	template<typename T, size_type N>
	bool small_vector<T, N>::operator!=(const small_vector<T, N>& other) const noexcept {
		return !(*this == other);
	}

} // namespace ecs
