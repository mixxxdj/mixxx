// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

namespace Bungee::Internal {

struct Instrumentation
{
	struct Call
	{
		Call(Instrumentation &instrumentation, int sequence);
		~Call();
	};

	bool enabled = false;
	int expected = 0;
	bool firstGrain = true;

	void log(const char *format, ...);

	void enableInstrumentation(bool enable)
	{
		this->enabled = enable;
	}
};

} // namespace Bungee::Internal
