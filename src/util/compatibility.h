#ifndef COMPATABILITY_H
#define COMPATABILITY_H

#include <QAtomicInt>
#include <QAtomicPointer>
#include <QStringList>
#include <QApplication>

#include <QLocale>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QApplication>
#else
#include <QGuiApplication>
#include <QInputMethod>
#endif

inline int load_atomic(const QAtomicInt& value) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    return value;
#else
    return value.load();
#endif
}

template <typename T>
inline T* load_atomic_pointer(const QAtomicPointer<T>& value) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    return value;
#else
    return value.load();
#endif
}

inline QLocale inputLocale() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    return QApplication::keyboardInputLocale();
#else
    // Use the default config for local keyboard
    QInputMethod* pInputMethod = QGuiApplication::inputMethod();
    return pInputMethod ? pInputMethod->locale() :
            QLocale(QLocale::English);
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

// this adds const to non-const objects (like std::as_const)
template <typename T>
struct QAddConst { typedef const T Type; };
template <typename T>
constexpr typename QAddConst<T>::Type &qAsConst(T &t) { return t; }
// prevent rvalue arguments:
template <typename T>
void qAsConst(const T &&) = delete;

#endif

#endif /* COMPATABILITY_H */
