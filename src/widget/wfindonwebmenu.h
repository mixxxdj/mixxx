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

    static bool hasEntriesForTrack(const Track& track);

    void addSubmenusForServices(const Track& track);

  private:
    void openInBrowser(WFindOnWebMenu::Service service,
            WFindOnWebMenu::TrackSearchProperties trackSearchProperties,
            const QString& query);

    void populateFromTrackProperties(
            const Track& track,
            const QString& serviceTitle,
            Service service);

    void addActions(Service service,
            const QString& queryValue,
            QMenu* pServiceMenu,
            TrackSearchProperties trackSearchProperties);
};
