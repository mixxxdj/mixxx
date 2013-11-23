#ifndef DEBUG_H
#define DEBUG_H

#include <QtDebug>
#include <QString>

#include "errordialoghandler.h"

// Calling this will report a qFatal and quit Mixxx, possibly disgracefully. Use
// very sparingly! A modal message box will be issued to the user which allows
// the Qt event loop to continue processing. This means that you must not call
// this function from a code section which is not re-entrant (e.g. paintEvent on
// a QWidget).
inline void reportFatalErrorAndQuit(QString message) {
    QByteArray message_bytes = message.toLocal8Bit();
    qFatal("%s", message_bytes.constData());
    ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();
    if (dialogHandler) {
        dialogHandler->requestErrorDialog(DLG_FATAL, message, true);
    }
}

// Calling this will report a qCritical and quit Mixxx, possibly
// disgracefully. Use very sparingly! A modal message box will be issued to the
// user which allows the Qt event loop to continue processing. This means that
// you must not call this function from a code section which is not re-entrant
// (e.g. paintEvent on a QWidget).
inline void reportCriticalErrorAndQuit(QString message) {
    qCritical() << message;
    ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();
    if (dialogHandler) {
        dialogHandler->requestErrorDialog(DLG_CRITICAL, message, true);
    }
}

#endif /* DEBUG_H */
