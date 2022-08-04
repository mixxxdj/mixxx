#pragma once

#include <QMenu>

class Track;

class WFindOnWebMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WFindOnWebMenu(
            QWidget* parent = nullptr);
    ~WFindOnWebMenu() override = default;

    void addActionToServiceMenu(QMenu* serviceMenu,
            const QString& actionText,
            const QUrl& serviceUrl);

    static bool hasEntriesForTrack(const Track& track);

  protected:
    QString composeActionText(const QString& prefix, const QString& trackProperty);

    QString composeSearchQuery(const QString& artist, const QString& trackAlbumOrTitle);

    void openInBrowser(const QUrl& url);
};
