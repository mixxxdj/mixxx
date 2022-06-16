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

    void openInBrowser(const QString& serviceUrl,
            const QString& query);

    void createAllSubmenusForWebLookups(const Track& track);

  signals:
    void triggerBrowser(
            const QString& serviceUrl,
            const QString& query);

  private:
    enum class Service {
        SoundCloud,
        LastFm,
        Discogs
    };

    void populateWebLookUpQueries(QMenu* pServiceMenu,
            const Track& track,
            const QString& serviceTitle,
            Service service);

    void addActionsArtist(Service service, const QString& artist, QMenu* pServiceMenu);
    void addActionsTrackTitle(Service service, const QString& trackTitle, QMenu* pServiceMenu);
    void addActionsAlbum(Service service, const QString& album, QMenu* pServiceMenu);

    QMenu* m_pFindOnSoundCloud;
    QMenu* m_pFindOnLastFm;
    QMenu* m_pFindOnDiscogs;
};
