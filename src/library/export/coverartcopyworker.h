#pragma once

#include <QObject>
#include <QString>
#include <QThread>
#include <future>

#include "library/coverart.h"

// This is a QThread class for copying the cover art.

class CoverArtCopyWorker : public QThread {
    Q_OBJECT
  public:
    enum class OverwriteAnswer {
        Overwrite,
        Cancel = -1,
    };

    CoverArtCopyWorker(const QString& selectedCoverArtFilePath,
            const QString& oldCoverArtFilePath,
            const QByteArray& fetchedCoverArtByteArray = nullptr)
            : m_selectedCoverArtFilePath(selectedCoverArtFilePath),
              m_oldCoverArtFilePath(oldCoverArtFilePath),
              m_fetchedCoverArtByteArray(fetchedCoverArtByteArray) {
        qRegisterMetaType<CoverInfoRelative>("CoverInfoRelative");
    }

    ~CoverArtCopyWorker() override;

  signals:
    void askOverwrite(const QString& filename,
            std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise);
    void coverArtUpdated(const CoverInfoRelative& coverInfo);
    void coverArtCopyFailed(const QString& errorMessage);

  protected:
    void run() override;

  private:
    void copyFile(const QString& m_selectedCoverArtFilePath,
            const QString& m_oldCoverArtFilePath);

    OverwriteAnswer makeOverwriteRequest(const QString& filename);

    CoverInfoRelative m_coverInfo;
    const QString m_selectedCoverArtFilePath;
    const QString m_oldCoverArtFilePath;
    const QByteArray m_fetchedCoverArtByteArray;
};
