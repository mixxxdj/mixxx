// CoverArtCopyWizard handles copying the cover art of the track to a where track located.

#pragma once

#include <QScopedPointer>
#include <QString>

#include "library/export/coverartcopyworker.h"
#include "library/export/dlgcoverartcopy.h"
#include "preferences/usersettings.h"

class CoverArtCopyWizard : public QObject {
    Q_OBJECT
  public:
    CoverArtCopyWizard(QWidget* parent,
            UserSettingsPointer pConfig,
            const QImage& coverArtImage,
            const QString& coverArtCopyPath)
            : m_parent(parent),
              m_pConfig(pConfig),
              m_coverArtImage(coverArtImage),
              m_coverArtCopyPath(coverArtCopyPath) {
    }
    virtual ~CoverArtCopyWizard() = default;

    bool copyCoverArt();

  private:
    QWidget* m_parent;
    UserSettingsPointer m_pConfig;
    QImage m_coverArtImage;
    QString m_coverArtCopyPath;
    QScopedPointer<DlgCoverArtCopy> m_dialog;
    QScopedPointer<CoverArtCopyWorker> m_worker;
};
