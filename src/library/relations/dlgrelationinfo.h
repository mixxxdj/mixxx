#pragma once

#include <QDialog>
#include <QList>

#include "library/relations/ui_dlgrelationinfo.h"
#include "track/relation.h"

class Library;

class DlgRelationInfo : public QDialog, public Ui::DlgRelationInfo {
    Q_OBJECT
  public:
    explicit DlgRelationInfo(
            Relation* relation = nullptr);
    ~DlgRelationInfo() override = default;

  private slots:
    void slotOk();
    void slotCancel();

  private:
    void init();
    void saveRelation();
    void updateRelationMetadataFields();

    Relation* m_pRelation;
    Library* m_pLibrary;

    TrackPointer m_pTrackA;
    TrackPointer m_pTrackB;
};
