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

    enum class Services {
        SoundCloud,
        LastFm,
        Discogs
    };

    void openTheBrowser(const QString& serviceUrl,
            const QString& query);

    void createAllServices(const Track& track);

  signals:
    void triggerBrowser(
            const QString& searchQuery,
            const QString& query);

  private:
    void createService(QMenu* serviceMenu,
            const Track& track,
            const QString& serviceTitleDisplay,
            Services serviceTitle);

    void addActionsArtist(Services serviceTitle, const QString& artist, QMenu* m_pService);
    void addActionsTrackTitle(Services serviceTitle, const QString& trackTitle, QMenu* m_pService);
    void addActionsAlbum(Services serviceTitle, const QString& album, QMenu* m_pService);

    QMenu* m_pFindOnSoundCloud;
    QMenu* m_pFindOnLastFm;
    QMenu* m_pFindOnDiscogs;
};
