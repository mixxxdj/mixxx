#pragma once

#include <QImage>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QThread>
#include <future>

#include "util/fileinfo.h"

// This is a QThread class for copying the cover art.

class CoverArtCopyWorker : public QThread {
    Q_OBJECT
  public:
    enum class OverwriteAnswer {
        UPDATE,
        OVERWRITE,
        CANCEL = -1,
    };

    CoverArtCopyWorker(const QImage& coverArtImage, const QString& coverArtCopyPath)
            : m_coverArtImage(coverArtImage), m_coverArtCopyPath(coverArtCopyPath) {
    }

    virtual ~CoverArtCopyWorker(){};

    void run() override;

    QString errorMessage() const {
        return m_errorMessage;
    }

    void stop();

    QString getOldCoverArtLocation() {
        return m_coverArtCopyPath;
    };

    QImage getNewCoverArtImage() {
        return m_coverArtImage;
    }

  signals:
    void askOverwrite(
            const QString& filename,
            std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise);
    void canceled();

  private:
    void copyFile(const QImage& m_coverArtImage,
            const QString& m_coverArtCopyPath);

    OverwriteAnswer makeOverwriteRequest(const QString& filename);

    QAtomicInt m_bStop = false;
    QString m_errorMessage;

    const QImage& m_coverArtImage;
    const QString m_coverArtCopyPath;
};
