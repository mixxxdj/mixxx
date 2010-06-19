
#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
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

    void slotBpmDouble();
    void slotBpmHalve();
    void slotBpmTap();

  private:
    void unloadTrack(bool save);
    void clear();

    QHash<int, Cue*> m_cueMap;
    TrackInfoObject* m_pLoadedTrack;
    QMutex m_mutex;
};

#endif /* DLGTRACKINFO_H */

