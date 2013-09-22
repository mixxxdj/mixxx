// loopfilemixer.cpp
// Create by Carl Pillot on 8/22/13

//#include <QDebug>
//#include <QDir>
//#include <QFile>
//#include <QMutex>
#include <QObject>
#include <sndfile.h>

#include "looprecording/loopfilemixer.h"

LoopFileMixer::LoopFileMixer(QString file1, QString file2)
        : m_filePath1(file1),
        m_filePath2(file2) {

}

LoopFileMixer::~LoopFileMixer() {
    
}