#pragma once
#include <QComboBox>
#include <QDialog>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QWidget>
// #include <QRegularExpressionValidator>

// #include "library/trackset/smarties/smartiesfeature.h"
#include "library/trackset/smarties/ui_dlgsmartiesinfo.h"

class GroupedSmartiesFeature;
class SmartiesSetBlockerOff;

class dlgGroupedSmartiesInfo : public QDialog, public Ui::dlgSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgGroupedSmartiesInfo(
            GroupedSmartiesFeature* feature,
            QWidget* pParent = nullptr);
    //    ~dlgGroupedSmartiesInfo();

    void init(const QVariantList& smartiesData, const QVariantList& playlistsCratesData);

    QVariant getUpdatedData() const;

  signals:
    void dataUpdated(const QVariantList& updatedData);
    void requestPreviousSmarties();
    void requestNextSmarties();
    void requestNewSmarties();
    void requestDeleteSmarties();

  public slots:
    void connectConditions();
    void updateConditionState();
    void onApplyButtonClicked();    // Slot for Apply button
    void onNewButtonClicked();      // Slot for New button
    void onDeleteButtonClicked();   // Slot for Delete button
    void onPreviousButtonClicked(); // Slot for Previous button
    void onNextButtonClicked();     // Slot for Next button
    void onOKButtonClicked();       // Slot for OK button
    void onCancelButtonClicked();   // Slot for Cancel button
    // void initializeConditionState();

    // validation
    bool validationCheck();
    // narriw operator combobox
    void onFieldComboBoxChanged();
    void onOperatorComboBoxChanged();
    void onValueComboBoxChanged();
    // functions to move conditions up/down/insert/delete
    void insertCondition(int index);
    void removeCondition(int index);
    void swapConditionWithAbove(int index);
    void swapConditionWithBelow(int index);
    void copyCondition(int fromIndex, int toIndex);
    void clearCondition(int index);
    void swapConditions(int index1, int index2);

  private:
    void setupConditionUI();
    void populateUI();
    void initHeaderTable(const QVariantList& smartiesData);
    void initConditionsTable(const QVariantList& smartiesData);
    void initPlaylistCrateTable(const QVariantList& playlistsCratesData);
    QString buildWhereClause();
    // void populateUI(const QVariantList& smartiesData);

    QVariantList smartiesData;
    QVariantList collectUIChanges() const;
    //    void onApplyButtonClicked(); // Slot for Apply button
    //    void onOkButtonClicked();    // Slot for OK button
    //    void onCancelButtonClicked(); // Slot for OK button
    GroupedSmartiesFeature* m_feature;
    bool m_isUpdatingUI; // Flag to prevent signal loops
    bool m_sendDelete;   // Flag to prevent signal loops from delete
    bool m_sendPrevious; // Flag to prevent signal loops from onPreviousButton
    bool m_sendNext;     // Flag to prevent signal loops from onNextButton
    bool m_sendNew;      // Flag to prevent signal loops from onNewButton
    QString headerTable[8];
    QString conditionsTable[13][5];
    QList<QPair<QString, QString>> playlistTable;
    QHash<QString, QString> playlistNameHash;
    QHash<QString, QString> playlistIdHash;
    QList<QPair<QString, QString>> crateTable;
    QHash<QString, QString> crateNameHash;
    QHash<QString, QString> crateIdHash;
    QList<QPair<QString, QString>> historyTable;
    QHash<QString, QString> historyNameHash;
    QHash<QString, QString> historyIdHash;

  private slots:
    void onSetBlockerOff(const QString& blocker);
    void onUpdateSmartiesData(const QVariantList& smartiesData);
    void toggleLockStatus();
    void storeUIIn2Table();
};
