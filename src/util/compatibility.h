#ifndef COMPATABILITY_H
#define COMPATABILITY_H

#include <QAtomicInt>
#include <QStringList>

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

#endif /* COMPATABILITY_H */
