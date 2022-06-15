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

    void openTheBrowser(const QString& serviceUrl,
            const QString& query);

    void createAllServices(const Track& track);

  signals:
    void triggerBrowser(
            const QString& searchQuery,
            const QString& query);

  private:
    enum class Service {
        SoundCloud,
        LastFm,
        Discogs
    };

    void createService(QMenu* serviceMenu,
            const Track& track,
            const QString& serviceTitle,
            Service service);

    void addActionsArtist(Service service, const QString& artist, QMenu* m_pService);
    void addActionsTrackTitle(Service service, const QString& trackTitle, QMenu* m_pService);
    void addActionsAlbum(Service service, const QString& album, QMenu* m_pService);

    QMenu* m_pFindOnSoundCloud;
    QMenu* m_pFindOnLastFm;
    QMenu* m_pFindOnDiscogs;
};
