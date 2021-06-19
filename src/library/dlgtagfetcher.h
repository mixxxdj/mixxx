#pragma once

#include <QDialog>
#include <QList>
#include <QTreeWidget>

#include "library/trackmodel.h"
#include "library/ui_dlgtagfetcher.h"
#include "musicbrainz/tagfetcher.h"
#include "track/track_decl.h"

/// A dialog box to fetch track metadata from MusicBrainz.
/// Use TrackPointer to load a track into the dialog or
/// QModelIndex along with TrackModel to enable previous and next buttons
/// to switch tracks within the context of the TrackModel.
class DlgTagFetcher : public QDialog, public Ui::DlgTagFetcher {
    Q_OBJECT

  public:
    // TODO: Remove dependency on TrackModel
    explicit DlgTagFetcher(
            const TrackModel* pTrackModel = nullptr);
    ~DlgTagFetcher() override = default;

    void init();

  public slots:
    void loadTrack(const TrackPointer& track);
    void loadTrack(const QModelIndex& index);

  signals:
    void next();
    void previous();

  private slots:
    void fetchTagFinished(
            TrackPointer pTrack,
            const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases);
    void resultSelected();
    void fetchTagProgress(const QString&);
    void slotNetworkResult(int httpStatus, const QString& app, const QString& message, int code);
    void slotTrackChanged(TrackId trackId);
    void apply();
    void quit();
    void slotNext();
    void slotPrev();

  private:
    void loadTrackInternal(const TrackPointer& track);
    void updateStack();
    void addDivider(const QString& text, QTreeWidget* parent) const;

    const TrackModel* const m_pTrackModel;

    TagFetcher m_tagFetcher;

    TrackPointer m_track;

    QModelIndex m_currentTrackIndex;

    struct Data {
        Data()
                : m_pending(true),
                  m_selectedResult(-1) {
        }

        bool m_pending;
        int m_selectedResult;
        QList<mixxx::musicbrainz::TrackRelease> m_results;
    };
    Data m_data;

    enum class NetworkResult {
        Ok,
        HttpError,
        UnknownError,
    };
    NetworkResult m_networkResult;
};
