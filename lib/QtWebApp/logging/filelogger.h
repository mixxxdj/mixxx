/**
  @file
  @author Stefan Frings
*/

#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QtGlobal>
#include <QSettings>
#include <QFile>
#include <QMutex>
#include <QBasicTimer>
#include "logglobal.h"
#include "logger.h"

namespace stefanfrings {

/**
  Logger that uses a text file for output. Settings are read from a
  config file using a QSettings object. Config settings can be changed at runtime.
  <p>
  Example for the configuration settings:
  <code><pre>
  fileName=logs/QtWebApp.log
  maxSize=1000000
  maxBackups=2
  bufferSize=0
  minLevel=WARNING
  msgformat={timestamp} {typeNr} {type} thread={thread}: {msg}
  timestampFormat=dd.MM.yyyy hh:mm:ss.zzz  
  </pre></code>

  - Possible log levels are: ALL/DEBUG=0, INFO=4, WARN/WARNING=1, ERROR/CRITICAL=2, FATAL=3
  - fileName is the name of the log file, relative to the directory of the settings file.
    In case of windows, if the settings are in the registry, the path is relative to the current
    working directory.
  - maxSize is the maximum size of that file in bytes. The file will be backed up and
    replaced by a new file if it becomes larger than this limit. Please note that
    the actual file size may become a little bit larger than this limit. Default is 0=unlimited.
  - maxBackups defines the number of backup files to keep. Default is 0=unlimited.
  - bufferSize defines the size of the ring buffer. Default is 0=disabled.
  - minLevel If bufferSize=0: Messages with lower level are discarded.<br>
             If buffersize>0: Messages with lower level are buffered, messages with equal or higher
             level (except INFO) trigger writing the buffered messages into the file.<br>
             Defaults is 0=debug.
  - msgFormat defines the decoration of log messages, see LogMessage class. Default is "{timestamp} {type} {msg}".
  - timestampFormat defines the format of timestamps, see QDateTime::toString(). Default is "yyyy-MM-dd hh:mm:ss.zzz".


  @see set() describes how to set logger variables
  @see LogMessage for a description of the message decoration.
  @see Logger for a descrition of the buffer.
*/

class DECLSPEC FileLogger : public Logger {
    Q_OBJECT
    Q_DISABLE_COPY(FileLogger)
public:

    /**
      Constructor.
      @param settings Configuration settings, usually stored in an INI file. Must not be 0.
      Settings are read from the current group, so the caller must have called settings->beginGroup().
      Because the group must not change during runtime, it is recommended to provide a
      separate QSettings instance that is not used by other parts of the program.
      The FileLogger does not take over ownership of the QSettings instance, so the caller
      should destroy it during shutdown.
      @param refreshInterval Interval of checking for changed config settings in msec, or 0=disabled
      @param parent Parent object
    */
    FileLogger(QSettings* settings, const int refreshInterval=10000, QObject* parent = nullptr);

    /**
      Destructor. Closes the file.
    */
    virtual ~FileLogger();

    /** Write a message to the log file */
    virtual void write(const LogMessage* logMessage);

protected:

    /**
      Handler for timer events.
      Refreshes config settings or synchronizes I/O buffer, depending on the event.
      This method is thread-safe.
      @param event used to distinguish between the two timers.
    */
    void timerEvent(QTimerEvent* event);

private:

    /** Configured name of the log file */
    QString fileName;

    /** Configured  maximum size of the file in bytes, or 0=unlimited */
    long maxSize;

    /** Configured maximum number of backup files, or 0=unlimited */
    int maxBackups;

    /** Pointer to the configuration settings */
    QSettings* settings;

    /** Output file, or 0=disabled */
    QFile* file;

    /** Timer for refreshing configuration settings */
    QBasicTimer refreshTimer;

    /** Timer for flushing the file I/O buffer */
    QBasicTimer flushTimer;

    /** Open the output file */
    void open();

    /** Close the output file */
    void close();

    /** Rotate files and delete some backups if there are too many */
    void rotate();

    /**
      Refreshes the configuration settings.
      This method is thread-safe.
    */
    void refreshSettings();

};

} // end of namespace

#endif // FILELOGGER_H
