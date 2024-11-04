#pragma once
#include <QComboBox>
#include <QDialog>
#include <QVariant>
#include <QVariantList>
#include <QWidget>

#include "library/trackset/smarties/ui_dlgsmartiesinfo.h"
#include "library/trackset/smarties/smartiesfeature.h"

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
    void onApplyButtonClicked();
    void onNewButtonClicked();
    void onPreviousButtonClicked();
    void onNextButtonClicked();
    void onOKButtonClicked();
    void initializeConditionState();

  private:
    void populateUI(const QVariantList& smartiesData);

    QVariantList smartiesData;
    QVariantList collectUIChanges() const;
    void onApplyButton_clicked(); // Slot for Apply button
    void onOkButton_clicked();    // Slot for OK button
    void onCancelButtonClicked(); // Slot for OK button
    SmartiesFeature* m_feature;

    bool m_isUpdatingUI = false; // Flag to prevent signal loops

  private slots:
    void onUpdateSmartiesData(const QVariantList& smartiesData);
};
