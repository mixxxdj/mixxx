// loopfilemixer.cpp
// Create by Carl Pillot on 8/22/13

//#include <QDebug>
//#include <QDir>
//#include <QFile>
//#include <QMutex>
#include <QObject>
#include <QDebug>
#include <sndfile.h>

#include "looprecording/loopfilemixer.h"

#include "sampleutil.h"

LoopFileMixer::LoopFileMixer(ConfigObject<ConfigValue>* pConfig, QString file1, QString file2, QString dest)
        : m_pConfig(pConfig),
        m_filePath1(file1),
        m_filePath2(file2),
        m_dest(dest) {

}

LoopFileMixer::~LoopFileMixer() {
    qDebug() << "~LoopFileMixer()";
}

void LoopFileMixer::slotProcess() {

    qDebug() << "!~!~! LoopFileMixer::slotProcess() !~!~!~!";

    SF_INFO sfInfo1;
    SF_INFO sfInfo2;

    sfInfo1.format = 0;
    sfInfo2.format = 0;
//    if (m_encodingType == ENCODING_WAVE) {
//        sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//    } else {
//        sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
//    }

    SNDFILE* pSndFile1 = sf_open(m_filePath1.toLocal8Bit(), SFM_READ, &sfInfo1);
    SNDFILE* pSndFile2 = sf_open(m_filePath2.toLocal8Bit(), SFM_READ, &sfInfo2);
    if (pSndFile1 && pSndFile2) {
        sf_command(pSndFile1, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);
        sf_command(pSndFile2, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);
    }


    sf_close(pSndFile1);
    sf_close(pSndFile2);

    emit fileFinished(m_dest);
    emit finished();
}