#pragma once

#include <cstdlib>

inline float rand_float() {
	return float(rand()) / float(RAND_MAX);
}

template<typename T>
struct init_component {
	T operator()() { return {}; }
};

template<size_t I>
struct small_component {
	float f[2];
};

template<size_t I>
struct medium_component {
	float f[16];
};

template<size_t I>
struct big_component {
	float f[32];
};

template<size_t I>
struct init_component<small_component<I>> {
	small_component<I> operator()() {
		return small_component<I>{
			.f = { rand_float(), rand_float() }
		};
	}
};

template<size_t I>
struct init_component<medium_component<I>> {
	medium_component<I> operator()() {
		medium_component<I> result;
		for(int i = 0; i < 16; ++i) result.f[i] = rand_float();
		return result;
	}
};

template<size_t I>
struct init_component<big_component<I>> {
	big_component<I> operator()() {
		big_component<I> result;
		for(int i = 0; i < 32; ++i) result.f[i] = rand_float();
		return result;
	}
};

template<size_t I1, size_t I2>
inline small_component<I1> operator+(small_component<I1> a, small_component<I2> b) {
	small_component<I1> result;
	for(int i = 0; i < 2; ++i) result.f[i] = a.f[i] + b.f[i];
	return result;
}

template<size_t I1, size_t I2>
inline medium_component<I1> operator+(medium_component<I1> a, medium_component<I2> b) {
	medium_component<I1> result;
	for(int i = 0; i < 16; ++i) result.f[i] = a.f[i] + b.f[i];
	return result;
}

template<size_t I1, size_t I2>
inline big_component<I1> operator+(big_component<I1> a, big_component<I2> b) {
	big_component<I1> result;
	for(int i = 0; i < 32; ++i) result.f[i] = a.f[i] + b.f[i];
	return result;
}
