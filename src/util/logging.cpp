#include "util/logging.h"

#include <stdio.h>
#include <iostream>
#include <signal.h>

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "util/cmdlineargs.h"
#include "util/assert.h"

namespace mixxx {
namespace {

QFile Logfile;
QMutex mutexLogfile;

// Debug message handler which outputs to both a logfile and prepends the thread
// the message came from.
void MessageHandler(QtMsgType type,
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                    const char* input) {
#else
                    const QMessageLogContext&, const QString& input) {
#endif

    // It's possible to deadlock if any method in this function can
    // qDebug/qWarning/etc. Writing to a closed QFile, for example, produces a
    // qWarning which causes a deadlock. That's why every use of Logfile is
    // wrapped with isOpen() checks.
    QMutexLocker locker(&mutexLogfile);
    QByteArray ba;
    QThread* thread = QThread::currentThread();
    if (thread) {
        ba = "[" + QThread::currentThread()->objectName().toLocal8Bit() + "]: ";
    } else {
        ba = "[?]: ";
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    bool controllerDebug = strncmp(input, Logging::kControllerDebugPrefix,
                                   strlen(Logging::kControllerDebugPrefix)) == 0;
    if (controllerDebug) {
        ba += (input + strlen(Logging::kControllerDebugPrefix) + 1);
    } else {
        ba += input;
    }
#else
    bool controllerDebug = input.startsWith(QLatin1String(Logging::kControllerDebugPrefix));
    if (controllerDebug) {
        ba += input.mid(strlen(Logging::kControllerDebugPrefix) + 1).toLocal8Bit();
    } else {
        ba += input.toLocal8Bit();
    }
#endif
    ba += "\n";

    if (!Logfile.isOpen()) {
        // This Must be done in the Message Handler itself, to guarantee that the
        // QApplication is initialized
        QString logLocation = CmdlineArgs::Instance().getSettingsPath();
        QString logFileName;

        // Rotate old logfiles
        //FIXME: cerr << doesn't get printed until after mixxx quits (???)
        for (int i = 9; i >= 0; --i) {
            if (i == 0) {
                logFileName = QDir(logLocation).filePath("mixxx.log");
            } else {
                logFileName = QDir(logLocation).filePath(QString("mixxx.log.%1").arg(i));
            }
            QFileInfo logbackup(logFileName);
            if (logbackup.exists()) {
                QString olderlogname =
                        QDir(logLocation).filePath(QString("mixxx.log.%1").arg(i + 1));
                // This should only happen with number 10
                if (QFileInfo(olderlogname).exists()) {
                    QFile::remove(olderlogname);
                }
                if (!QFile::rename(logFileName, olderlogname)) {
                    std::cerr << "Error rolling over logfile " << logFileName.toStdString();
                }
            }
        }

        // WARNING(XXX) getSettingsPath() may not be ready yet. This causes
        // Logfile writes below to print qWarnings which in turn recurse into
        // MessageHandler -- potentially deadlocking.
        // XXX will there ever be a case that we can't write to our current
        // working directory?
        Logfile.setFileName(logFileName);
        Logfile.open(QIODevice::WriteOnly | QIODevice::Text);
    }

    Logging::LogLevel logLevel = CmdlineArgs::Instance().getLogLevel();

    switch (type) {
    case QtDebugMsg:
        if (logLevel >= Logging::LogLevel::Debug || controllerDebug) {
            fprintf(stderr, "Debug %s", ba.constData());
        }
        if (Logfile.isOpen()) {
            Logfile.write("Debug ");
            Logfile.write(ba);
        }
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        if (logLevel >= Logging::LogLevel::Info) {
            fprintf(stderr, "Info %s", ba.constData());
        }
        if (Logfile.isOpen()) {
            Logfile.write("Info ");
            Logfile.write(ba);
        }
        break;
#endif
    case QtWarningMsg:
        if (logLevel >= Logging::LogLevel::Warning) {
            fprintf(stderr, "Warning %s", ba.constData());
        }
        if (Logfile.isOpen()) {
            Logfile.write("Warning ");
            Logfile.write(ba);
        }
        break;
    case QtCriticalMsg:
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            bool debugAssert = strncmp(input, kDebugAssertPrefix,
                                           strlen(kDebugAssertPrefix)) == 0;
#else
            bool debugAssert = input.startsWith(QLatin1String(kDebugAssertPrefix));
#endif
            if (debugAssert) {
                if (CmdlineArgs::Instance().getDebugAssertBreak()) {
                    fputs(ba.constData(), stderr);
                    if (Logfile.isOpen()) {
                        Logfile.write(ba);
                    }
                    raise(SIGINT);
                } else {
#ifdef MIXXX_DEBUG_ASSERTIONS_FATAL
                    // re-send as fatal
                    locker.unlock();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                    qFatal(input);
#else
                    qFatal(input.toLocal8Bit());
#endif
                    return;
#else
                    fputs(ba.constData(), stderr);
                    if (Logfile.isOpen()) {
                        Logfile.write(ba);
                    }
#endif
                }
            } else {
                // Critical errors are always shown on the console.
                fprintf(stderr, "Critical %s", ba.constData());
                if (Logfile.isOpen()) {
                    Logfile.write("Critical ");
                    Logfile.write(ba);
                }
            }
        }
        break;
    case QtFatalMsg:
        // Fatal errors are always shown on the console.
        fprintf(stderr, "Fatal %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Fatal ");
            Logfile.write(ba);
        }
        break;
    default:
        fprintf(stderr, "Unknown %s", ba.constData());
        if (Logfile.isOpen()) {
            Logfile.write("Unknown ");
            Logfile.write(ba);
        }
        break;
    }
    if (Logfile.isOpen()) {
        Logfile.flush();
    }
}

}  // namespace

// static
constexpr Logging::LogLevel Logging::kLogLevelDefault;

// static
void Logging::initialize() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(MessageHandler);
#else
    qInstallMessageHandler(MessageHandler);
#endif
}

// static
void Logging::shutdown() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(NULL);  // Reset to default.
#else
    qInstallMessageHandler(NULL);  // Reset to default.
#endif

    // Don't make any more output after this
    //    or mixxx.log will get clobbered!
    QMutexLocker locker(&mutexLogfile);
    if (Logfile.isOpen()) {
        Logfile.close();
    }
}

}  // namespace mixxx
