
#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
#include <QList>

#include "ui_dlgtrackinfo.h"
#include "trackinfoobject.h"
#include "dlgtagfetcher.h"

/** Minimum allowed Beat per minute (BPM) */
const int minBPM = 30;
/** Maximum allowed bpm */
const int maxBPM = 240;
/** Maximum allowed interval between beats in milli seconds (calculated from minBPM) */
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBPM));
/** Filter length */
const int filterLength = 5;

class Cue;

class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent, DlgTagFetcher& DlgTagFetcher);
    virtual ~DlgTrackInfo();

  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackPointer pTrack);

  signals:
    void next();
    void previous();

  private slots:
    void slotNext();
    void slotPrev();
    void apply();
    void cancel();
    void trackUpdated();
    void fetchTag();

    void cueActivate();
    void cueDelete();

    void slotBpmDouble();
    void slotBpmHalve();
    void slotBpmTap();

    void reloadTrackMetadata();

  private:
    void populateFields(TrackPointer pTrack);
    void populateCues(TrackPointer pTrack);
    void unloadTrack(bool save);
    void clear();
    void init();

    QHash<int, Cue*> m_cueMap;
    TrackPointer m_pLoadedTrack;

    CSAMPLE m_bpmTapFilter[filterLength];
    QTime m_bpmTapTimer;

    QMutex m_mutex;
    DlgTagFetcher& m_DlgTagFetcher;

};

#endif /* DLGTRACKINFO_H */

