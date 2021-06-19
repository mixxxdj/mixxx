#pragma once

#include <type_traits>

#include <QtDebug>

// Helper macro for defining simple properties with setters and
// getters.
//
// Fundamental types and bool are passed and returned by value.
// Other types are passed by universal reference and returned
// by const reference.
//
// The refName() function returns a mutable reference. It can
// be used to access deeply nested properties chaining these
// functions with the dot syntax. The returned reference should
// not be stored outside of the current scope!
//
// The ptrName() function is overloaded for both mutable and
// immutable access by a pointer.
#define MIXXX_DECL_PROPERTY(TYPE, NAME, CAP_NAME)                       \
  public:                                                               \
    template<typename T>                                                \
    typename std::enable_if<(std::is_fundamental<TYPE>::value ||        \
                                    std::is_same<TYPE, bool>::value) && \
            std::is_same<TYPE, T>::value>::type set##CAP_NAME(T _val) { \
        m_##NAME = _val;                                                \
    }                                                                   \
                                                                        \
  public:                                                               \
    template<typename T>                                                \
    typename std::enable_if<!(std::is_fundamental<TYPE>::value ||       \
                                    std::is_same<TYPE, bool>::value) && \
            std::is_assignable<TYPE, T>::value>::type                   \
            set##CAP_NAME(T&& _val) {                                   \
        m_##NAME = std::forward<T>(_val);                               \
    }                                                                   \
                                                                        \
  public:                                                               \
    constexpr std::conditional<std::is_fundamental<TYPE>::value,        \
            TYPE,                                                       \
            const TYPE&>::type get##CAP_NAME() const {                  \
        return m_##NAME;                                                \
    }                                                                   \
                                                                        \
  public:                                                               \
    constexpr TYPE& ref##CAP_NAME() {                                   \
        return m_##NAME;                                                \
    }                                                                   \
                                                                        \
  public:                                                               \
    constexpr TYPE* ptr##CAP_NAME() {                                   \
        return &m_##NAME;                                               \
    }                                                                   \
                                                                        \
  public:                                                               \
    constexpr const TYPE* ptr##CAP_NAME() const {                       \
        return &m_##NAME;                                               \
    }                                                                   \
                                                                        \
  public:                                                               \
    QDebug dbg##CAP_NAME(QDebug dbg) const {                            \
        return dbg << #NAME ":" << m_##NAME;                            \
    }                                                                   \
                                                                        \
  private:                                                              \
    TYPE m_##NAME;
