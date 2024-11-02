#pragma once

// #include <QComboBox>
// #include <QDialog>

#include "library/trackset/smarties/smartiesid.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "library/trackset/tracksettablemodel.h"

class TrackCollectionManager;

class dlgSmartiesActions : public TrackSetTableModel {
    Q_OBJECT

  public:
    explicit dlgSmartiesActions(
            QObject* pParent,
            TrackCollectionManager* pTrackCollectionManager);
    ~dlgSmartiesActions() final = default;
  public slots:
    //    void test(SmartiesId smartiesId);

  private slots:

  private:
};
