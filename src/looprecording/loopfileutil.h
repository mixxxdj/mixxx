#ifndef LOOPFILEUTIL_H
#define LOOPFILEUTIL_H

#include <sndfile.h>

// loopfileutil.h
// Create by Carl Pillot on 8/22/13
// Provides utilities for copying and interleaving uncompressed audio in a
// worker thread.

class LoopFileUtil : public QObject {
    Q_OBJECT
  public:
    LoopFileUtil();
    virtual ~LoopFileUtil();

    // Copies a file from one location to another using QFile.
    // returns whether copy was successful or not.
    static bool copyFile(QString oldPath, QString newPath);
};
#endif