#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QScopedPointer>

#include "library/ui_dlgtrackinfo.h"
#include "track/track.h"
#include "library/coverart.h"
#include "util/tapfilter.h"
#include "util/types.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"

// TODO(XXX): All unapplied edits in dialog fields are lost when
// reloading metadata from file tags or when fetching metadata
// from MusicBrainz. The reloaded/fetched metadata is applied
// immediately to the actual track instead of only loaded into
// the dialog's fields.
// Fixing this unexpected behavior requires that the dialog and
// all subtasks (reload, fetch) operate on a deep copy of the
// actual track. A major refactoring is required to achieve this
// goal.
class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    DlgTrackInfo(QWidget* parent);
    virtual ~DlgTrackInfo();

  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackPointer pTrack);

  signals:
    void next();
    void previous();
    void showTagFetcher(TrackPointer pTrack);

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

    void slotKeyTextChanged();

    void reloadTrackMetadata();
    void updateTrackMetadata();
    void slotOpenInFileBrowser();

    void slotCoverFound(const QObject* pRequestor, int requestReference,
                        const CoverInfo& info, QPixmap pixmap, bool fromCache);
    void slotCoverArtSelected(const CoverArt& art);
    void slotReloadCoverArt();

  private:
    void cacheCoverArt();

    void populateFields(const Track& track);
    void reloadTrackBeats(const Track& track);
    void populateCues(TrackPointer pTrack);
    void saveTrack();
    void unloadTrack(bool save);
    void clear();
    void init();
    QHash<int, CuePointer> m_cueMap;
    TrackPointer m_pLoadedTrack;
    BeatsPointer m_pBeatsClone;
    Keys m_keys;
    bool m_trackHasBeatMap;

    QScopedPointer<TapFilter> m_pTapFilter;
    double m_dLastTapedBpm;

    CoverInfo m_loadedCoverInfo;
    WCoverArtLabel* m_pWCoverArtLabel;
};

#endif /* DLGTRACKINFO_H */
