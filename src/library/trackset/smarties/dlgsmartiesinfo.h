#pragma once
#include <QComboBox>
#include <QDialog>
#include <QVariant>
#include <QVariantList>
#include <QWidget>

#include "library/trackset/smarties/ui_dlgsmartiesinfo.h"
// #include "library/trackcollectionmanager.h"
// #include "library/trackset/smarties/dlgsmartiesinfohelper.h"

// namespace Ui {
// class dlgSmartiesInfo;
// }

// class dlgSmartiesInfoHelper; // Forward declaration

class dlgSmartiesInfo : public QDialog, public Ui::dlgSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgSmartiesInfo(
            QWidget* pParent = nullptr);
    //    ~dlgSmartiesInfo();
    //    void loadSmartiesData(const QList<QVariant>& smartiesData, int smartiesId);
    //    void updateComboBoxes(const QStringList& libraryFields);
    void init(const QVariantList& smartiesData);
    // void init(int smartiesId, const QVariant& smartiesData);
    //    QVariant getUpdatedData() const; // Returns updated data after editing

    //    void init(SmartiesId smartiesId, const dlgSmartiesActions& actions);

    QVariant getUpdatedData() const;

  signals:
    void dataUpdated(const QVariantList& updatedData);
    void requestPreviousSmarties();
    void requestNextSmarties();

  public slots:
    void connectConditions();
    void updateConditionState();
    void onApplyButtonClicked();
    void onNewButtonClicked();
    void onPreviousButtonClicked();
    void onNextButtonClicked();
    void onOKButtonClicked();
    void handleButtonFunctions();
    void initializeConditionState();

    //    void saveUIChangesToData(int smartiesId, const QVariantList& smartiesData);
    // void saveUIChangesToData(int smartiesId); // update UI -> List

  private:
    void populateUI(const QVariantList& smartiesData);
    //    dlgSmartiesActions* m_pActions; // Pointer to actions class for database operations
    //    QComboBox* m_artistComboBox;    // Combo box for artists
    // Other UI-related members (e.g., QUiLoader, combo boxes, etc.)
    //    void setupUi(); // Function to set up the UI elements

    //    Ui::dlgSmartiesInfo* m_pUI;
    //    TrackCollectionManager* m_pTrackCollectionManager;
    //    int m_currentSmartiesId;
    QVariantList smartiesData;
    QVariantList collectUIChanges() const;
    void onApplyButton_clicked(); // Slot for Apply button
    void onOkButton_clicked();    // Slot for OK button

    // Ui::dlgSmartiesInfo* m_pUI;
    //    dlgSmartiesInfoHelper* m_pDlgSmartiesInfoHelper;
};
