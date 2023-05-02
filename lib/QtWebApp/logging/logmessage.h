/**
  @file
  @author Stefan Frings
*/

#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H

#include <QtGlobal>
#include <QDateTime>
#include <QHash>
#include "logglobal.h"

namespace stefanfrings {

/**
  Represents a single log message together with some data
  that are used to decorate the log message.

  The following variables may be used in the message and in msgFormat:

  - {timestamp} Date and time of creation
  - {typeNr}    Type of the message in numeric format (0-3)
  - {type}      Type of the message in string format (DEBUG, WARNING, CRITICAL, FATAL)
  - {thread}    ID number of the thread
  - {msg}       Message text
  - {xxx}       For any user-defined logger variable

  Plus some new variables since QT 5.0, only filled when compiled in debug mode:

  - {file}      Filename where the message was generated
  - {function}  Function where the message was generated
  - {line}      Line number where the message was generated
*/

class DECLSPEC LogMessage
{
    Q_DISABLE_COPY(LogMessage)
public:

    /**
      Constructor. All parameters are copied, so that later changes to them do not
      affect this object.
      @param type Type of the message
      @param message Message text
      @param logVars Logger variables, 0 is allowed
      @param file Name of the source file where the message was generated
      @param function Name of the function where the message was generated
      @param line Line Number of the source file, where the message was generated
    */
    LogMessage(const QtMsgType type, const QString& message, const QHash<QString,QString>* logVars,
               const QString &file, const QString &function, const int line);

    /**
      Returns the log message as decorated string.
      @param msgFormat Format of the decoration. May contain variables and static text,
          e.g. "{timestamp} {type} thread={thread}: {msg}".
      @param timestampFormat Format of timestamp, e.g. "dd.MM.yyyy hh:mm:ss.zzz", see QDateTime::toString().
      @see QDatetime for a description of the timestamp format pattern
    */
    QString toString(const QString& msgFormat, const QString& timestampFormat) const;

    /**
      Get the message type.
    */
    QtMsgType getType() const;

private:

    /** Logger variables */
    QHash<QString,QString> logVars;

    /** Date and time of creation */
    QDateTime timestamp;

    /** Type of the message */
    QtMsgType type;

    /** ID number of the thread  */
    Qt::HANDLE threadId;

    /** Message text */
    QString message;

    /** Filename where the message was generated */
    QString file;

    /** Function name where the message was generated */
    QString function;

    /** Line number where the message was generated */
    int line;

};

} // end of namespace

#endif // LOGMESSAGE_H
