#pragma once

#include <grantlee/context.h>

#include <QCloseEvent>
#include <QDialog>
#include <QScopedPointer>
#include <QString>
#include <future>

#include "library/export/trackexportworker.h"
#include "library/export/ui_dlgtrackexport.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class QMenu;
// A dialog for interacting with the track exporter in an interactive manner.
// Handles errors and user interactions.
class TrackExportDlg : public QDialog, public Ui::DlgTrackExport {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    /// The dialog is prepared, but not shown on construction.
    /// You can pass additional information through a Context.
    /// The dialog will take ownership of the Context.
    TrackExportDlg(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackPointerList& tracks,
            Grantlee::Context* context = nullptr,
            const QString* playlistName = nullptr);
    virtual ~TrackExportDlg();
    void open() override;

  public slots:
    void slotProgress(const QString& from, const QString& to, int progress, int count);
    void slotResult(TrackExportWorker::ExportResult result, const QString& msg);
    void slotAskOverwriteMode(
            const QString& filename,
            std::promise<TrackExportWorker::OverwriteAnswer>* promise);
    void cancelButtonClicked();
    void slotBrowseFolder() {
        browseFolder();
    };
    void slotStartExport();

  protected:
    bool browseFolder();

  private slots:
    void slotPatternSelected(int index);
    void slotPatternEdited(const QString& text);

  private:
    // Called when progress is complete or the procedure has been canceled.
    // Makes sure the exporter thread has exited.
    void finish();
    void stopWorker();
    int addStatus(const QString& status, const QString& to);
    void updatePreview();
    void setEnableControls(bool enabled);
    void closeEvent(QCloseEvent* event) override;
    void populateDefaultPatterns();
    void removeDups(const QVariant& data);

    UserSettingsPointer m_pConfig;
    TrackPointerList m_tracks;
    TrackExportWorker* m_worker;
    int m_errorCount = 0;
    int m_skippedCount = 0;
    int m_okCount = 0;
    bool m_patternComboSwitched = 0;
};
