
#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QList>

#include "ui_dlgtrackinfo.h"

class TrackInfoObject;
class Cue;

class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent);
    virtual ~DlgTrackInfo();


  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackInfoObject* pTrack);
    void unloadTrack(TrackInfoObject* pTrack);

  signals:
    void next();
    void previous();

  private slots:
    void slotNext();
    void slotPrev();
    void apply();
    void cancel();
    void trackUpdated();

    void cueActivate();
    void cueDelete();

  private:
    void clear();

    QList<Cue*> m_cueList;
    TrackInfoObject* m_pLoadedTrack;
    QMutex m_mutex;
};

#endif /* DLGTRACKINFO_H */

