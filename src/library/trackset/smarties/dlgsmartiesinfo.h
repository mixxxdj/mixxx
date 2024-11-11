#pragma once
#include <QComboBox>
#include <QDialog>
#include <QVariant>
#include <QVariantList>
#include <QWidget>

// #include "library/trackset/smarties/smartiesfeature.h"
#include "library/trackset/smarties/ui_dlgsmartiesinfo.h"

class SmartiesFeature;

class dlgSmartiesInfo : public QDialog, public Ui::dlgSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgSmartiesInfo(
            SmartiesFeature* feature,
            QWidget* pParent = nullptr);
    //    ~dlgSmartiesInfo();

    void init(const QVariantList& smartiesData);

    QVariant getUpdatedData() const;

  signals:
    void dataUpdated(const QVariantList& updatedData);
    void requestPreviousSmarties();
    void requestNextSmarties();
    void requestNewSmarties();

  public slots:
    void connectConditions();
    void updateConditionState();
    void onApplyButtonClicked();    // Slot for Apply button
    void onNewButtonClicked();      // Slot for New button
    void onPreviousButtonClicked(); // Slot for Previous button
    void onNextButtonClicked();     // Slot for Next button
    void onOKButtonClicked();       // Slot for OK button
    void onCancelButtonClicked();   // Slot for Cancel button
    void initializeConditionState();

    void updateOperatorComboBox(QComboBox* fieldComboBox,
            int conditionIndex,
            const QStringList& stringFieldOptions,
            const QStringList& dateFieldOptions,
            const QStringList& numberFieldOptions);

  private:
    void populateUI(const QVariantList& smartiesData);

    QVariantList smartiesData;
    QVariantList collectUIChanges() const;
    //    void onApplyButtonClicked(); // Slot for Apply button
    //    void onOkButtonClicked();    // Slot for OK button
    //    void onCancelButtonClicked(); // Slot for OK button
    SmartiesFeature* m_feature;

    bool m_isUpdatingUI = false; // Flag to prevent signal loops

  private slots:
    void onUpdateSmartiesData(const QVariantList& smartiesData);
    void toggleLockStatus();
};
