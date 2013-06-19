#include <QtCore>
#include <chromaprint.h>

#include "musicbrainz/chromaprinter.h"
#include "soundsourceproxy.h"
#include "defs.h"

chromaprinter::chromaprinter(QObject* parent)
             : QObject(parent){
}

QString chromaprinter::getFingerPrint(TrackPointer pTrack){
    SoundSourceProxy soundSource(pTrack);
    return calcFingerPrint(soundSource);
}

QString chromaprinter::getFingerPrint(QString location){
    SoundSourceProxy soundSource(location);
    return calcFingerPrint(soundSource);
}

QString chromaprinter::calcFingerPrint(SoundSourceProxy& soundSource){
    soundSource.open();
    m_SampleRate = soundSource.getSampleRate();
    unsigned int length = soundSource.length();
    if (m_SampleRate == 0 ){
        qDebug() << "Skipping invalid file:" << soundSource.getFilename();
        return QString();
    }

    // this is worth 2min of audio, multiply by 2 because we have 2 channels
    // AcoustID only stores a fingerprint for the first two minutes of a song 
    // on their server so we need only a fingerprint of the first two minutes
    // --kain88 July 2012
    m_NumSamples = 120*2*m_SampleRate;
    // check that the song is actually longer then the amount of audio we use
    if (m_NumSamples > length) {
        m_NumSamples = length;
    }

    SAMPLE *pData = new SAMPLE[m_NumSamples];
    QTime timerReadingFile;
    timerReadingFile.start();
    int read = soundSource.read(m_NumSamples, pData);

    if (read!=m_NumSamples) {
        qDebug() << "oh that's embarrasing I couldn't read the track";
        return QString();
    }
    qDebug("reading file took: %d ms" , timerReadingFile.elapsed());

    ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    // we have 2 channels in mixxx always
    chromaprint_start(ctx, m_SampleRate, 2);

    QTime timerGeneratingFingerPrint;
    timerGeneratingFingerPrint.start();
    int success = chromaprint_feed(ctx, pData, m_NumSamples);
    if (!success) {
        qDebug() << "could not generate fingerprint";
        return QString();
    }
    chromaprint_finish(ctx);

    void* fprint = NULL;
    int size = 0;
    int ret = chromaprint_get_raw_fingerprint(ctx, &fprint, &size);
    QByteArray fingerprint;
    if (ret == 1) {
        void* encoded = NULL;
        int encoded_size = 0;
        chromaprint_encode_fingerprint(fprint, size,
                                       CHROMAPRINT_ALGORITHM_DEFAULT,
                                       &encoded,
                                       &encoded_size, 1);

        fingerprint.append(reinterpret_cast<char*>(encoded), encoded_size);

        chromaprint_dealloc(fprint);
        chromaprint_dealloc(encoded);
    }
    chromaprint_free(ctx);
    delete pData;
    
    qDebug("generating fingerprint took: %d ms" , timerGeneratingFingerPrint.elapsed());

    return fingerprint;
}
