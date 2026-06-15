#pragma once

#include <cstdlib>
#include <memory>

#include <tinyecs/tinyecs.hpp>

template<size_t I>
struct small_component {
	float f[2];
	auto operator<=>(const small_component<I>&) const = default;

	template<size_t J>
	small_component<I> operator+(small_component<J> b) {
		small_component<I> result;
		for(int i = 0; i < 2; ++i) result.f[i] = f[i] + b.f[i];
		return result;
	}
};

template<size_t I>
struct medium_component {
	float f[16];
	auto operator<=>(const medium_component<I>&) const = default;

	template<size_t J>
	medium_component<I> operator+(medium_component<J> b) {
		medium_component<I> result;
		for(int i = 0; i < 16; ++i) result.f[i] = f[i] + b.f[i];
		return result;
	}
};

template<size_t I>
struct big_component {
	float f[32];
	auto operator<=>(const big_component<I>&) const = default;

	template<size_t J>
	big_component<I> operator+(big_component<J> b) {
		big_component<I> result;
		for(int i = 0; i < 32; ++i) result.f[i] = f[i] + b.f[i];
		return result;
	}
};

struct unique_component {
	std::unique_ptr<int> ptr;
	bool operator==(const unique_component& other) const { return *ptr == *other.ptr; }
	bool operator!=(const unique_component& other) const { return *ptr != *other.ptr; }
};

struct tag {};

template<typename T>
struct component_generator;

template<size_t N>
struct component_generator<small_component<N>> {
	small_component<N> operator()(unsigned entity) {
		srand(entity + unsigned(N) * 2654418637u);
		return {
			.f = { float(rand()) / RAND_MAX, float(rand()) / RAND_MAX }
		};
	}
};

template<size_t N>
struct component_generator<medium_component<N>> {
	medium_component<N> operator()(unsigned entity) {
		srand(entity + unsigned(N) * 2654418637u);
		return {
			.f = { float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX }
		};
	}
};

template<size_t N>
struct component_generator<big_component<N>> {
	big_component<N> operator()(unsigned entity) {
		srand(entity + unsigned(N) * 2654418637u);
		return {
			.f = { float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX,
                  float(rand()) / RAND_MAX, float(rand()) / RAND_MAX }
		};
	}
};

template<>
struct component_generator<unique_component> {
	unique_component operator()(unsigned entity) {
		srand(entity);
		return { .ptr = std::make_unique<int>(rand()) };
	}
};

template<>
struct component_generator<tag> {
	tag operator()(unsigned /* entity */) { return {}; }
};
