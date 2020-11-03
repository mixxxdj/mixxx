#pragma once

#include <QMenu>

class Track;

class WSearchRelatedTracksMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WSearchRelatedTracksMenu(
            QWidget* parent = nullptr);
    ~WSearchRelatedTracksMenu() override = default;

    void addActionsForTrack(
            const Track& track);

  signals:
    void triggerSearch(
            const QString& searchQuery);

  private:
    void addTriggerSearchAction(
            bool* /*in/out*/ pAddSeparatorBeforeNextAction,
            QString searchQuery,
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix = QString());
    QString elideActionText(
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix) const;
};
