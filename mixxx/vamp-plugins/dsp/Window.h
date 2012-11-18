/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM DSP library
    Centre for Digital Music, Queen Mary, University of London.
    This file Copyright 2006 Chris Cannam.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <cmath>
#include <iostream>
#include <map>
// M_PI needs to be difined for Windows builds
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

enum WindowType {
    RectangularWindow,
    BartlettWindow,
    HammingWindow,
    HanningWindow,
    BlackmanWindow,
    GaussianWindow,
    ParzenWindow
};

template <typename T>
class Window
{
public:
    /**
     * Construct a windower of the given type.
     */
    Window(WindowType type, size_t size) : m_type(type), m_size(size) { encache(); }
    Window(const Window &w) : m_type(w.m_type), m_size(w.m_size) { encache(); }
    Window &operator=(const Window &w) {
	if (&w == this) return *this;
	m_type = w.m_type;
	m_size = w.m_size;
	encache();
	return *this;
    }
    virtual ~Window() { delete[] m_cache; }
    
    void cut(T *src) const { cut(src, src); }
    void cut(const T *src, T *dst) const {
	for (size_t i = 0; i < m_size; ++i) dst[i] = src[i] * m_cache[i];
    }

    WindowType getType() const { return m_type; }
    size_t getSize() const { return m_size; }

protected:
    WindowType m_type;
    size_t m_size;
    T *m_cache;
    
    void encache();
};

template <typename T>
void Window<T>::encache()
{
    size_t n = m_size;
    T *mult = new T[n];
    size_t i;
    for (i = 0; i < n; ++i) mult[i] = 1.0;

    switch (m_type) {
		
    case RectangularWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * 0.5;
	}
	break;
	    
    case BartlettWindow:
	for (i = 0; i < n/2; ++i) {
	    mult[i] = mult[i] * (i / T(n/2));
	    mult[i + n/2] = mult[i + n/2] * (1.0 - (i / T(n/2)));
	}
	break;
	    
    case HammingWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * (0.54 - 0.46 * cos(2 * M_PI * i / n));
	}
	break;
	    
    case HanningWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * (0.50 - 0.50 * cos(2 * M_PI * i / n));
	}
	break;
	    
    case BlackmanWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * (0.42 - 0.50 * cos(2 * M_PI * i / n)
				 + 0.08 * cos(4 * M_PI * i / n));
	}
	break;
	    
    case GaussianWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * exp((-1.0 / (n*n)) * ((T(2*i) - n) *
						      (T(2*i) - n)));
	}
	break;
	    
    case ParzenWindow:
	for (i = 0; i < n; ++i) {
	    mult[i] = mult[i] * (1.0 - fabs((T(2*i) - n) / T(n + 1)));
	}
	break;
    }
	
    m_cache = mult;
}

#endif
