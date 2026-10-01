#pragma once

#include <QMenu>

#include "util/parented_ptr.h"
#include "widget/wmenucheckbox.h"

class Track;
class QWidgetAction;

// enum that allows reusing previously selected search criteria
enum SearchCriterion {
    Key = 1 << 1,
    Bpm = 1 << 2,
    Artist = 1 << 3,
    Title = 1 << 4,
    AlbumArtist = 1 << 5,
    Album = 1 << 6,
    Composer = 1 << 7,
    Grouping = 1 << 8,
    Year = 1 << 9,
    Genre = 1 << 10,
    Location = 1 << 11
};
Q_DECLARE_FLAGS(SearchCriteria, SearchCriterion)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchCriteria);
Q_DECLARE_METATYPE(SearchCriteria);

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
            SearchCriterion criterion,
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix = QString());
    QString elideActionText(
            const QString& actionTextPrefix,
            const QString& elidableTextSuffix) const;

    parented_ptr<QWidgetAction> m_pSearchAction;

    static SearchCriteria s_prevCriteria;
};
