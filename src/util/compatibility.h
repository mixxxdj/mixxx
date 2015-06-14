#ifndef COMPATABILITY_H
#define COMPATABILITY_H

#include <QAtomicInt>
#include <QStringList>

#include <QLocale>
#include <QDir>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QApplication>
#else
#include <QGuiApplication>
#include <QInputMethod>
#include <QStandardPaths>
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

inline QString xdgDataHomeMixxx() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // copied from Qt5
    QString xdgDataHome = QFile::decodeName(qgetenv("XDG_DATA_HOME"));
#else
    QString xdgDataHome = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    if (!xdgDataHome.isEmpty()) {
        // Don't trust $XDG_DATA_HOME
        if (xdgDataHome.startsWith ("~/")) {
            xdgDataHome.replace (0, 1, QDir::homePath());
        }
        if (!xdgDataHome.endsWith('/')) {
            xdgDataHome.append('/');
        }
    } else {
        xdgDataHome = QDir::homePath() + "/.local/share/";
    }
    xdgDataHome.append(QCoreApplication::applicationName());
    return xdgDataHome;
}

#endif /* COMPATABILITY_H */
