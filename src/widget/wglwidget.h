#pragma once

// This define is checked in wglwidgetqglwidget.h and wglwidgetqglwidget.h
// to make sure they are only included from this header, to enforce that
// all code includes this header wglwidget.h.
#define WGLWIDGET_H

#ifdef MIXXX_USE_QOPENGL
#include "widget/wglwidgetqopengl.h"
#else
#include "widget/wglwidgetqglwidget.h"
#endif

#undef WGLWIDGET_H
