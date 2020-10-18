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
    bool addSeparatorBeforeAction(
            bool addSeparator) {
        if (addSeparator) {
            this->addSeparator();
        }
        // Reset flag
        return false;
    }
};
