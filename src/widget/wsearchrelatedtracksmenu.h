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
    bool addTriggerSearchAction(
            bool addSeparatorBeforeNextAction,
            const QString& actionText,
            QString searchQuery);
};
