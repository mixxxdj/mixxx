#pragma once

#include <QDialog>
#include <QModelIndex>
#include <memory>

#include "library/coverart.h"
#include "library/ui_dlgtrackinfo.h"
#include "track/beats.h"
#include "track/keys.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "util/tapfilter.h"

class TrackModel;
class DlgTagFetcher;
class WCoverArtLabel;
class WStarRating;

/// A dialog box to display and edit track properties.
/// Use TrackPointer to load a track into the dialog or
/// QModelIndex along with TrackModel to enable previous and next buttons
/// to switch tracks within the context of the TrackModel.
class DlgTrackInfo : public QDialog, public Ui::DlgTrackInfo {
    Q_OBJECT
  public:
    // TODO: Remove dependency on TrackModel
    explicit DlgTrackInfo(
            const TrackModel* trackModel = nullptr);
    ~DlgTrackInfo() override;

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
            quint16 requestedHash,
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

    const TrackModel* const m_pTrackModel;

    TrackPointer m_pLoadedTrack;

    QModelIndex m_currentTrackIndex;

    mixxx::BeatsPointer m_pBeatsClone;
    Keys m_keysClone;
    bool m_trackHasBeatMap;

    TapFilter m_tapFilter;
    double m_dLastTapedBpm;

    CoverInfo m_loadedCoverInfo;

    parented_ptr<WCoverArtLabel> m_pWCoverArtLabel;
    parented_ptr<WStarRating> m_pWStarRating;

    std::unique_ptr<DlgTagFetcher> m_pDlgTagFetcher;
};
