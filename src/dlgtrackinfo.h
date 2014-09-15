#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
#include <QList>

#include "ui_dlgtrackinfo.h"
#include "trackinfoobject.h"
#include "dlgtagfetcher.h"
#include "library/coverartcache.h"
#include "util/types.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"

const int kFilterLength = 5;

class Cue;

class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent, DlgTagFetcher& DlgTagFetcher);
    virtual ~DlgTrackInfo();

  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackPointer pTrack, CoverInfo info);

  signals:
    void next();
    void previous();

  private slots:
    void slotNext();
    void slotPrev();
    void OK();
    void apply();
    void cancel();
    void trackUpdated();
    void fetchTag();

    void cueActivate();
    void cueDelete();

    void slotBpmDouble();
    void slotBpmHalve();
    void slotBpmTwoThirds();
    void slotBpmThreeFourth();
    void slotBpmTap();

    void reloadTrackMetadata();
    void updateTrackMetadata();
    void slotOpenInFileBrowser();

    void slotPixmapFound(int trackId, QPixmap pixmap);
    void slotCoverLocationUpdated(const QString& newLoc,
                                  const QString& oldLoc,
                                  QPixmap pixmap);

  private:
    void populateFields(TrackPointer pTrack);
    void populateCues(TrackPointer pTrack);
    void saveTrack();
    void unloadTrack(bool save);
    void clear();
    void init();

    QHash<int, Cue*> m_cueMap;
    TrackPointer m_pLoadedTrack;

    CSAMPLE m_bpmTapFilter[kFilterLength];
    QTime m_bpmTapTimer;

    DlgTagFetcher& m_DlgTagFetcher;

    CoverInfo m_loadedCover;
    WCoverArtLabel* m_pWCoverArtLabel;
};

#endif /* DLGTRACKINFO_H */
