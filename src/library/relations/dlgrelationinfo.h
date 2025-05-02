#pragma once

#include <QDialog>
#include <QList>
#include <future>

#include "library/relations/ui_dlgrelationinfo.h"
#include "track/relation.h"

class DlgRelationInfo : public QDialog, public Ui::DlgRelationInfo {
    Q_OBJECT
  public:
    explicit DlgRelationInfo(Relation* relation = nullptr);
    ~DlgRelationInfo() override = default;

  private:
    void init();

    Relation* m_pRelation;
};
