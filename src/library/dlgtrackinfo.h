#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QScopedPointer>

#include "library/ui_dlgtrackinfo.h"
#include "trackinfoobject.h"
#include "library/dlgtagfetcher.h"
#include "library/coverart.h"
#include "util/tapfilter.h"
#include "util/types.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"

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
    void slotBpmClear();
    void slotBpmConstChanged(int state);
    void slotBpmTap(double averageLength, int numSamples);
    void slotSpinBpmValueChanged(double value);

    void reloadTrackMetadata();
    void updateTrackMetadata();
    void slotOpenInFileBrowser();

    void slotCoverFound(const QObject* pRequestor, int requestReference,
                        const CoverInfo& info, QPixmap pixmap, bool fromCache);
    void slotCoverArtSelected(const CoverArt& art);
    void slotReloadCoverArt();

  private:
    void populateFields(const TrackInfoObject& track);
    void reloadTrackBeats(const TrackInfoObject& track);
    void populateCues(TrackPointer pTrack);
    void saveTrack();
    void unloadTrack(bool save);
    void clear();
    void init();
    QHash<int, CuePointer> m_cueMap;
    TrackPointer m_pLoadedTrack;
    BeatsPointer m_pBeatsClone;
    bool m_trackHasBeatMap;

    QScopedPointer<TapFilter> m_pTapFilter;
    double m_dLastTapedBpm;

    DlgTagFetcher& m_DlgTagFetcher;

    CoverInfo m_loadedCoverInfo;
    WCoverArtLabel* m_pWCoverArtLabel;
};

#endif /* DLGTRACKINFO_H */
