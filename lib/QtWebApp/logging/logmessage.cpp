/**
  @file
  @author Stefan Frings
*/

#include "logmessage.h"
#include <QThread>

using namespace stefanfrings;

LogMessage::LogMessage(const QtMsgType type, const QString& message, const QHash<QString, QString> *logVars, const QString &file, const QString &function, const int line)
{
    this->type=type;
    this->message=message;
    this->file=file;
    this->function=function;
    this->line=line;
    timestamp=QDateTime::currentDateTime();
    threadId=QThread::currentThreadId();

    // Copy the logVars if not null,
    // so that later changes in the original do not affect the copy
    if (logVars)
    {
        this->logVars=*logVars;
    }
}

QString LogMessage::toString(const QString& msgFormat, const QString& timestampFormat) const
{
    QString decorated=msgFormat+"\n";
    decorated.replace("{msg}",message);

    if (decorated.contains("{timestamp}"))
    {
        decorated.replace("{timestamp}",timestamp.toString(timestampFormat));
    }

    QString typeNr;
    typeNr.setNum(type);
    decorated.replace("{typeNr}",typeNr);

    switch (type)
    {
        case QtDebugMsg:
            decorated.replace("{type}","DEBUG   ");
            break;
        case QtWarningMsg:
            decorated.replace("{type}","WARNING ");
            break;
        case QtCriticalMsg:
            decorated.replace("{type}","CRITICAL");
            break;
        case QtFatalMsg: // or QtSystemMsg which has the same int value
            decorated.replace("{type}","FATAL   ");
            break;
    #if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QtInfoMsg:
            decorated.replace("{type}","INFO    ");
            break;
    #endif
    }

    decorated.replace("{file}",file);
    decorated.replace("{function}",function);
    decorated.replace("{line}",QString::number(line));

    QString threadId = QString("0x%1").arg(qulonglong(QThread::currentThreadId()), 8, 16, QLatin1Char('0'));
    decorated.replace("{thread}",threadId);

    // Fill in variables
    if (decorated.contains("{") && !logVars.isEmpty())
    {
        QList<QString> keys=logVars.keys();
        foreach (QString key, keys)
        {
            decorated.replace("{"+key+"}",logVars.value(key));
        }
    }

    return decorated;
}

QtMsgType LogMessage::getType() const
{
    return type;
}
