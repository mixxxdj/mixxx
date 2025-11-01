#pragma once

#include <QMenu>

class Track;
class FindOnWebLast;

class WFindOnWebMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WFindOnWebMenu(QWidget* pParent, FindOnWebLast* pFindOnWebLast);
    ~WFindOnWebMenu() override = default;

    void addActionToServiceMenu(
            const QString& actionId,
            const QString& actionText,
            const QUrl& serviceUrl);

    static bool hasEntriesForTrack(const Track& track);

  protected:
    QString composeSearchQuery(const QString& artist, const QString& trackAlbumOrTitle);
    void openInBrowser(const QUrl& url);

  private:
    FindOnWebLast* m_pFindOnWebLast;
};
