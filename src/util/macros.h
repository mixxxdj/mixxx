#pragma once

#include <QtDebug>


// Helper for defining simple properties with setters and getters that are
// passed by value using move assignment in the setter. The getter returns
// a const reference. The type must have a default constructor for proper
// initialization during construction and a move assignment operator for
// efficient passing and setting by value.
//
// The refName() function returns a mutable reference. It is only needed
// for direct and efficient access to properties when nesting property
// classes.
#define PROPERTY_SET_BYVAL_GET_BYREF(TYPE, NAME, CAP_NAME) \
public: void set##CAP_NAME(TYPE NAME) { m_##NAME = std::move(NAME); } \
public: TYPE const& get##CAP_NAME() const { return m_##NAME; } \
public: TYPE& ref##CAP_NAME() { return m_##NAME; } \
public: QDebug dbg##CAP_NAME(QDebug dbg) const { return dbg << #NAME ":" << m_##NAME; } \
private: TYPE m_##NAME;
