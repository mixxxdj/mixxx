#pragma once

#include <QDialog>
#include <QScopedPointer>
#include <QString>
#include <future>

#include "library/export/coverartcopyworker.h"
#include "library/export/ui_dlgcoverartcopy.h"
#include "preferences/usersettings.h"

class DlgCoverArtCopy : public QDialog, public Ui::CoverArtCopyDlg {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE,
        UPDATE,
    };

    DlgCoverArtCopy(QWidget* parent, UserSettingsPointer pConfig, CoverArtCopyWorker* worker);
    virtual ~DlgCoverArtCopy() {
    }

  signals:
    void clicked();

  public slots:
    void slotAskOverwrite(
            const QString& filename,
            std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise);
    void btnCancelClicked();

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    void finish();

    bool m_coverOverwritten;
    QString m_coverOverwrittenPath;

    UserSettingsPointer m_pConfig;

    CoverArtCopyWorker* m_worker;
};
