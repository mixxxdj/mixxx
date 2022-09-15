// CoverArtCopyWizard handles copying the cover art of the track to a where track located.

#pragma once

#include <QScopedPointer>
#include <QString>

#include "library/export/coverartcopyworker.h"
#include "preferences/usersettings.h"
#include "util/imagefiledata.h"

class CoverArtCopyWizard : public QObject {
    Q_OBJECT
  public:
    CoverArtCopyWizard(QObject* parent,
            const ImageFileData& coverArtImage,
            const QString& coverArtAbsolutePath)
            : QObject(parent),
              m_coverArtImage(coverArtImage),
              m_coverArtAbsolutePath(coverArtAbsolutePath) {
    }
    virtual ~CoverArtCopyWizard() = default;

    bool copyCoverArt();

  private:
    ImageFileData m_coverArtImage;
    QString m_coverArtAbsolutePath;
    QScopedPointer<CoverArtCopyWorker> m_worker;
};
