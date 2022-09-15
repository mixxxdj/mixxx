#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QThread>
#include <future>

#include "util/fileinfo.h"
#include "util/imagefiledata.h"

// This is a QThread class for copying the cover art.

class CoverArtCopyWorker : public QThread {
    Q_OBJECT
  public:
    enum class OverwriteAnswer {
        OVERWRITE,
        CANCEL = -1,
    };

    CoverArtCopyWorker(const ImageFileData& coverArtImage, const QString& coverArtAbsolutePath)
            : m_coverArtImage(coverArtImage), m_coverArtAbsolutePath(coverArtAbsolutePath) {
    }

    virtual ~CoverArtCopyWorker(){};

    void run() override;

    bool isCoverUpdated() const {
        return m_isCoverArtUpdated;
    }

  private:
    void copyFile(const ImageFileData& m_coverArtImage,
            const QString& m_coverArtAbsolutePath);

    void askOverWrite(const QString& filename,
            std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise);

    OverwriteAnswer makeOverwriteRequest(const QString& filename);

    const ImageFileData& m_coverArtImage;
    const QString m_coverArtAbsolutePath;
    bool m_isCoverArtUpdated;
};
