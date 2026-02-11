// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#define BUNGEE_MODES_RESAMPLE \
	X_BEGIN(Resample, resample) \
	X_ITEM(Resample, resample, autoOut, \
		"output resampling, activated as needed (default, " \
		")") \
	X_ITEM(Resample, resample, autoIn, "input resampling, activated as needed") \
	X_ITEM(Resample, resample, autoInOut, "input resampling on input when downsampling and on output when upsampling") \
	X_ITEM(Resample, resample, forceOut, "output resampling, always active") \
	X_ITEM(Resample, resample, forceIn, "input resampling, always active") \
	X_END(Resample, resample)

#define BUNGEE_MODES \
	BUNGEE_MODES_RESAMPLE
//

namespace Bungee {

#define X_BEGIN(Type, type) \
	struct Type##Mode \
	{ \
		enum Enum \
		{
#define X_ITEM(Type, type, mode, description) \
	mode,
#define X_END(Type, type) \
		}; \
	};

BUNGEE_MODES

#undef X_BEGIN
#undef X_ITEM
#undef X_END

} // namespace Bungee