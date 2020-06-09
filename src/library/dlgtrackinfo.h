#ifndef DLGTRACKINFO_H
#define DLGTRACKINFO_H

#include <QDialog>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QScopedPointer>

#include "library/coverart.h"
#include "library/dlgtagfetcher.h"
#include "library/trackmodel.h"
#include "library/ui_dlgtrackinfo.h"
#include "track/track.h"
#include "util/tapfilter.h"
#include "util/types.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"

/// A dialog box to display and edit track properties.
/// Use TrackPointer to load a track into the dialog or
/// QModelIndex along with TrackModel to enable previous and next buttons
/// to switch tracks within the context of the TrackModel.
class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    // TODO: Remove dependency on TrackModel
    DlgTrackInfo(QWidget* parent,
            UserSettingsPointer pConfig,
            const TrackModel* trackModel = nullptr);
    virtual ~DlgTrackInfo();

  public slots:
    // Not thread safe. Only invoke via AutoConnection or QueuedConnection, not
    // directly!
    void loadTrack(TrackPointer pTrack);
    void loadTrack(QModelIndex index);

  signals:
    void next();
    void previous();

  private slots:
    void slotNextButton();
    void slotPrevButton();
    void slotNextDlgTagFetcher();
    void slotPrevDlgTagFetcher();
    void slotOk();
    void slotApply();
    void slotCancel();

    void trackUpdated();

    void slotBpmDouble();
    void slotBpmHalve();
    void slotBpmTwoThirds();
    void slotBpmThreeFourth();
    void slotBpmFourThirds();
    void slotBpmThreeHalves();
    void slotBpmClear();
    void slotBpmConstChanged(int state);
    void slotBpmTap(double averageLength, int numSamples);
    void slotSpinBpmValueChanged(double value);

    void slotKeyTextChanged();

    void slotImportMetadataFromFile();
    void slotImportMetadataFromMusicBrainz();

    void slotTrackChanged(TrackId trackId);
    void slotOpenInFileBrowser();

    void slotCoverFound(
            const QObject* pRequestor,
            const CoverInfo& info,
            const QPixmap& pixmap,
            mixxx::cache_key_t requestedCacheKey,
            bool coverInfoUpdated);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();

  private:
    void loadNextTrack();
    void loadPrevTrack();
    void loadTrackInternal(const TrackPointer& pTrack);
    void populateFields(const Track& track);
    void reloadTrackBeats(const Track& track);
    void saveTrack();
    void unloadTrack(bool save);
    void clear();
    void init();
    TrackPointer m_pLoadedTrack;
    mixxx::BeatsPointer m_pBeatsClone;
    Keys m_keysClone;
    bool m_trackHasBeatMap;

    QScopedPointer<TapFilter> m_pTapFilter;
    QScopedPointer<DlgTagFetcher> m_pTagFetcher;
    double m_dLastTapedBpm;

    CoverInfo m_loadedCoverInfo;
    WCoverArtLabel* m_pWCoverArtLabel;
    UserSettingsPointer m_pConfig;

    const TrackModel* m_pTrackModel;
    QModelIndex m_currentTrackIndex;
};

#endif /* DLGTRACKINFO_H */
