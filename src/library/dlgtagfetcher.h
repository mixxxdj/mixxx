#pragma once

#include <QDialog>
#include <QList>
#include <QTreeWidget>

#include "library/trackmodel.h"
#include "library/ui_dlgtagfetcher.h"
#include "musicbrainz/tagfetcher.h"
#include "track/track_decl.h"
#include "track/trackrecord.h"
#include "util/parented_ptr.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"

/// A dialog box to fetch track metadata from MusicBrainz.
/// Use TrackPointer to load a track into the dialog or
/// QModelIndex along with TrackModel to enable previous and next buttons
/// to switch tracks within the context of the TrackModel.
class DlgTagFetcher : public QDialog, public Ui::DlgTagFetcher {
    Q_OBJECT

  public:
    // TODO: Remove dependency on TrackModel
    explicit DlgTagFetcher(
            UserSettingsPointer pConfig, const TrackModel* pTrackModel = nullptr);
    ~DlgTagFetcher() override = default;

    void init();

  public slots:
    void loadTrack(const TrackPointer& pTrack);
    void loadTrack(const QModelIndex& index);

  signals:
    void next();
    void previous();

  private slots:
    void fetchTagFinished(
            TrackPointer pTrack,
            const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases);
    void tagSelected();
    void showProgressOfConstantTask(const QString&);
    void setPercentOfEachRecordings(int totalRecordingsFound);
    void showProgressOfRecordingTask();
    void slotNetworkResult(int httpStatus, const QString& app, const QString& message, int code);
    // Called when apply is pressed.
    void slotTrackChanged(TrackId trackId);
    void apply();
    void retry();
    void quit();
    void reject() override;
    void slotNext();
    void slotPrev();
    void slotCoverFound(
            const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap,
            mixxx::cache_key_t requestedCacheKey,
            bool coverInfoUpdated);
    void slotStartFetchCoverArt(const QList<QString>& allUrls);
    void slotLoadBytesToLabel(const QByteArray& data);
    void slotCoverArtLinkNotFound();
    void slotWorkerStarted();
    void slotWorkerAskOverwrite(const QString& coverArtAbsolutePath,
            std::promise<CoverArtCopyWorker::OverwriteAnswer>* pPromise);
    void slotWorkerCoverArtCopyFailed(const QString& errorMessage);
    void slotWorkerCoverArtUpdated(const CoverInfoRelative& coverInfo);
    void slotWorkerFinished();

  private:
    // Called on population or changed via buttons Next&Prev.
    void loadTrackInternal(const TrackPointer& pTrack);
    void addDivider(const QString& text, QTreeWidget* pParent) const;
    void getCoverArt(const QString& url);
    void loadCurrentTrackCover();

    UserSettingsPointer m_pConfig;

    const TrackModel* const m_pTrackModel;

    TagFetcher m_tagFetcher;

    bool m_isCoverArtCopyWorkerRunning;

    TrackPointer m_pTrack;

    QModelIndex m_currentTrackIndex;

    int m_percentForOneRecording;

    parented_ptr<WCoverArtLabel> m_pWCurrentCoverArtLabel;
    parented_ptr<WCoverArtLabel> m_pWFetchedCoverArtLabel;

    mixxx::TrackRecord m_trackRecord;

    struct Data {
        Data()
                : m_selectedTag(-1) {
        }
        int m_selectedTag;
        QList<mixxx::musicbrainz::TrackRelease> m_tags;
    };
    Data m_data;

    QByteArray m_fetchedCoverArtByteArrays;

    QScopedPointer<CoverArtCopyWorker> m_pWorker;
};
