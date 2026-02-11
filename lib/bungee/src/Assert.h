// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cfenv>
#include <complex>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <vector>

#ifndef BUNGEE_SELF_TEST
#	define BUNGEE_SELF_TEST 0 // no checks
// #define BUNGEE_SELF_TEST 1 // fast checks only
// #define BUNGEE_SELF_TEST 2 // all checks
#endif

namespace Bungee::Assert {

static constexpr int level = BUNGEE_SELF_TEST;

void fail(int level, const char *m2, const char *file, int line);

#define BUNGEE_ASSERT(l, condition) \
	do \
	{ \
		if constexpr (l <= Bungee::Assert::level) \
			if (!(condition)) \
				Bungee::Assert::fail(l, #condition, __FILE__, __LINE__); \
	} while (false)

// Checks with cost O(N) (for tests applied a small number of times per grain)
#define BUNGEE_ASSERT1(...) BUNGEE_ASSERT(1, (__VA_ARGS__))

// Checks with cost O(N*N) (for tests applied a small number of times per bin or per sample)
#define BUNGEE_ASSERT2(...) BUNGEE_ASSERT(2, (__VA_ARGS__))

#ifndef FE_INEXACT
#	define FE_INEXACT 0
#	define FE_UNDERFLOW 0
#	define FE_INVALID 0
#	define FE_OVERFLOW 0
#	define FE_DIVBYZERO 0
#endif
#ifndef FE_DENORMALOPERAND
#	define FE_DENORMALOPERAND 0
#endif

struct FloatingPointExceptions
{
#if BUNGEE_SELF_TEST
	int allowed;
	std::fenv_t original;

	FloatingPointExceptions(int allowed);
	~FloatingPointExceptions();

	void check() const;
#else
	inline FloatingPointExceptions(int) {}
	inline void check() const {}
#endif
};

struct Log
{
	int maxLevel = 0;
	int expected = 0;

	void log(int level, const char *format, ...);

	void checkCallSequence(int called);
};

static constexpr auto active = BUNGEE_SELF_TEST == 2;

} // namespace Bungee::Assert
