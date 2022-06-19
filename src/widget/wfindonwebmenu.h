#pragma once

#include <QAction>
#include <QMenu>
#include <QUrlQuery>

class Track;

class WFindOnWebMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WFindOnWebMenu(
            QWidget* parent = nullptr);
    ~WFindOnWebMenu() override = default;

    enum class Service {
        SoundCloud,
        LastFm,
        Discogs
    };

    enum class TrackSearchProperties {
        Artist,
        ArtistAndTitle,
        ArtistAndAlbum,
        Album,
        Title
    };

    void openInBrowser(WFindOnWebMenu::Service service,
            WFindOnWebMenu::TrackSearchProperties trackSearchProperty,
            const QString& query);

    void addSubmenusForServices(const Track& track);

  signals:
    void triggerBrowser(WFindOnWebMenu::Service service,
            WFindOnWebMenu::TrackSearchProperties trackSearchProperty,
            const QString& query);

  private:
    void populateFromTrackProperties(
            const Track& track,
            const QString& serviceTitle,
            Service service);

    void addActionsArtist(Service service,
            const QString& artist,
            QMenu* pServiceMenu,
            TrackSearchProperties trackSearchProperty);
    void addActionsTrackTitle(Service service,
            const QString& trackTitle,
            QMenu* pServiceMenu,
            TrackSearchProperties trackSearchProperty);
    void addActionsAlbum(Service service,
            const QString& album,
            QMenu* pServiceMenu,
            TrackSearchProperties trackSearchProperty);

    QMenu* m_pServiceMenu;
};
