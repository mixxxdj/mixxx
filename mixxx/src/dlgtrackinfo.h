
#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>

#include "ui_dlgtrackinfo.h"

class TrackInfoObject;

class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent);
    virtual ~DlgTrackInfo();

  public slots:
    void apply();
    void trackUpdated();

    void loadTrack(TrackInfoObject* pTrack);
    void unloadTrack(TrackInfoObject* pTrack);

  private:
    TrackInfoObject* m_pLoadedTrack;
    QMutex m_mutex;
};

#endif /* DLGTRACKINFO_H */

