/**
  @file
  @author Stefan Frings
*/

#include "dualfilelogger.h"

using namespace stefanfrings;

DualFileLogger::DualFileLogger(QSettings *firstSettings, QSettings* secondSettings, const int refreshInterval, QObject* parent)
    :Logger(parent)
{
     firstLogger=new FileLogger(firstSettings, refreshInterval, this);
     secondLogger=new FileLogger(secondSettings, refreshInterval, this);
}

void DualFileLogger::log(const QtMsgType type, const QString& message, const QString &file, const QString &function, const int line)
{
    firstLogger->log(type,message,file,function,line);
    secondLogger->log(type,message,file,function,line);
}

void DualFileLogger::clear(const bool buffer, const bool variables)
{
    firstLogger->clear(buffer,variables);
    secondLogger->clear(buffer,variables);
}
