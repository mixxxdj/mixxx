#pragma once

#include <QAction>
#include <QMenu>
#include <QUrlQuery>

class Track;

class WFindOnMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WFindOnMenu(
            QWidget* parent = nullptr);
    ~WFindOnMenu() override = default;

    enum class Service {
        SoundCloud,
        LastFm,
        Discogs
    };

    enum class TrackProperty {
        Artist,
        ArtistTitle,
        ArtistAlbum,
        Album,
        Title
    };

  public slots:
    void openInBrowser(Service service,
            TrackProperty trackProperty,
            const QString& query);

    void createAllSubmenusForWebLookups(const Track& track);

  signals:
    void triggerBrowser(Service service,
            TrackProperty trackProperty,
            const QString& query);

  private:
    void populateWebLookUpQueries(QMenu* pServiceMenu,
            const Track& track,
            const QString& serviceTitle,
            Service service);

    void addActionsArtist(Service service,
            const QString& artist,
            QMenu* pServiceMenu,
            TrackProperty trackProperty);
    void addActionsTrackTitle(Service service,
            const QString& trackTitle,
            QMenu* pServiceMenu,
            TrackProperty trackProperty);
    void addActionsAlbum(Service service,
            const QString& album,
            QMenu* pServiceMenu,
            TrackProperty trackProperty);

    QMenu* m_pFindOnSoundCloud;
    QMenu* m_pFindOnLastFm;
    QMenu* m_pFindOnDiscogs;
};
