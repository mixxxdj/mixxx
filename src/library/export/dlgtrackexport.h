#ifndef DLGTRACKEXPORT_H
#define DLGTRACKEXPORT_H

#include <future>

#include <QDialog>
#include <QString>
#include <QScopedPointer>
#include <QThread>

#include "configobject.h"
#include "library/export/trackexport.h"
#include "library/export/ui_dlgtrackexport.h"

class DlgTrackExport : public QDialog,
                       public Ui::DlgTrackExport {
    Q_OBJECT
  public:
    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    DlgTrackExport(QWidget *parent,
                   ConfigObject<ConfigValue>* pConfig,
                   QList<QString> filenames);
    virtual ~DlgTrackExport() { }

  public slots:
    void slotAskOverwriteMode(
            QString filename,
            std::promise<TrackExport::OverwriteAnswer>* promise);
    void slotProgress(int progress, int count);

    void skipButtonClicked();
    void skipAllButtonClicked();
    void overwriteButtonClicked();
    void overwriteAllButtonClicked();
    void cancelButtonClicked();

  private:
    void setQuestionShown(bool show);

    QScopedPointer<TrackExport> m_exporter;
    QThread m_exporterThread;
};

#endif  // DLGTRACKEXPORT_H
