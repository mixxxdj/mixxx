#pragma once

#include <QMenu>

#include "util/parented_ptr.h"

class Track;
class QWidgetAction;

class WSearchRelatedTracksMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WSearchRelatedTracksMenu(
            QWidget* pParent = nullptr);
    ~WSearchRelatedTracksMenu() override = default;

    void addActionsForTrack(
            const Track& track);
    bool eventFilter(QObject* pObj, QEvent* e) override;

  signals:
    void triggerSearch(
            const QString& searchQuery);

  private slots:
    void updateSearchButton();
    void combineQueriesTriggerSearch();

  private:
    void addTriggerSearchAction(
            bool* /*in/out*/ pAddSeparatorBeforeNextAction,
            QString searchQuery,
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix = QString());
    QString elideActionText(
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix) const;

    parented_ptr<QWidgetAction> m_pSearchAction;
};
