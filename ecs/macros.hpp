#pragma once

#if defined(__clang__)
	#define RESTRICT __restrict__
#elif defined(__GNUC__) || defined(__GNUG__)
	#define RESTRICT __restrict__
#elif defined(_MSC_VER)
	#define RESTRICT __restrict
#else
	#define RESTRICT
#endif
