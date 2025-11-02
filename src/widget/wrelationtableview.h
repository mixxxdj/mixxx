#pragma once

#include "preferences/usersettings.h"
#include "track/relation.h"
#include "widget/wtracktableview.h"

class Library;
class Relation;

class WRelationTableView : public WTrackTableView {
    Q_OBJECT
  public:
    WRelationTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            double trackTableBackgroundColorOpacity,
            bool relationPairView);

    QList<DbId> getSelectedRelationIds() const;
    RelationPointer getSelectedRelation();

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    Library* m_pLibrary;
    bool m_bRelationPairView;
};
