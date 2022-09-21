#include "library/export/coverartsaver.h"

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QtDebug>

CoverArtSaver::CoverArtSaver(const QString& fileName, const QImage& origCoverArt)
        : m_fileName(fileName),
          m_origCoverArt(origCoverArt) {
}

bool CoverArtSaver::saveCoverArt() {
    QByteArray coverArtByteArray;
    QBuffer bufferCoverArt(&coverArtByteArray);
    bufferCoverArt.open(QIODevice::WriteOnly);
    // Question;
    // This can be changed according to the cover art extension?
    // There could be Enums with the related extensions in this class etc.
    m_origCoverArt.save(&bufferCoverArt, "JPG");
    QFileInfo coverArtInfo(m_fileName);
    QString originalCoverArtPath = coverArtInfo.absolutePath() + "/" +
            coverArtInfo.completeBaseName() + ".jpg";
    QFile coverArtFile(originalCoverArtPath);
    qDebug() << "Sleeping 5 secs. If there was a existence before, There "
                "should be 2 original cover arts.";
    QThread::msleep(5000);
    coverArtFile.open(QIODevice::WriteOnly);
    if (!coverArtFile.write(coverArtByteArray)) {
        qDebug() << "Failed to write cover art file";
        return false;
    }
    qDebug() << "Sleeping 5 secs. If there was a existence before, there "
                "should be the new cover art and temp "
                "cover art in the folder";
    QThread::msleep(5000);
    bufferCoverArt.close();
    coverArtFile.close();
    return true;
}
