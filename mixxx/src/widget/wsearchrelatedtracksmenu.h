#pragma once

#include <QMenu>

#include "util/parented_ptr.h"
#include "widget/wmenucheckbox.h"

class Track;
class QWidgetAction;

/// Extension of WMenuCheckBox with a vertical separator bar in between the
/// label and the indicator box. This is supposed to clarify the different
/// behavior of clicks in these two regions.
class WSearchRelatedCheckBox : public WMenuCheckBox {
    Q_OBJECT
  public:
    explicit WSearchRelatedCheckBox(const QString& label,
            QWidget* pParent = nullptr)
            : WMenuCheckBox(label, pParent) {
    }

    Q_PROPERTY(QColor separatorColor
                    MEMBER m_separatorColor
                            DESIGNABLE true);

    void paintEvent(QPaintEvent*) override;

  private:
    QColor m_separatorColor;
};

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
