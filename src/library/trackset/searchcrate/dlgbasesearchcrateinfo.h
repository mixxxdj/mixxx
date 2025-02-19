#pragma once
#include <QDialog>
#include <QString>
#include <QVariantList>

#include "library/trackset/searchcrate/ui_dlgsearchcrateinfo.h"

class BaseTrackSetFeature;
class SearchCrateSetBlockerOff;

class dlgBaseSearchCrateInfo : public QDialog, public Ui::dlgSearchCrateInfo {
    Q_OBJECT

  public:
    explicit dlgBaseSearchCrateInfo(BaseTrackSetFeature* feature, QWidget* parent = nullptr);
    virtual ~dlgBaseSearchCrateInfo() = default;

    void init(const QVariantList& searchCrateData, const QVariantList& playlistsCratesData);

    QVariant getUpdatedData() const;

  signals:
    void dataUpdated(const QVariantList& updatedData);
    void requestPreviousSearchCrate();
    void requestNextSearchCrate();
    void requestNewSearchCrate();
    void requestDeleteSearchCrate();

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
    void initHeaderTable(const QVariantList& searchCrateData);
    void initConditionsTable(const QVariantList& searchCrateData);
    void initPrevNextTable(const QVariantList& searchCrateData);
    void initPlaylistCrateTable(const QVariantList& playlistsCratesData);
    QString buildWhereClause();
    QVariantList searchCrateData;
    QVariantList collectUIChanges() const;
    //    void onApplyButtonClicked(); // Slot for Apply button
    //    void onOkButtonClicked();    // Slot for OK button
    //    void onCancelButtonClicked(); // Slot for OK button
    BaseTrackSetFeature* m_feature;
    bool m_isUpdatingUI; // Flag to prevent signal loops
    bool m_sendDelete;   // Flag to prevent signal loops from delete
    bool m_sendPrevious; // Flag to prevent signal loops from onPreviousButton
    bool m_sendNext;     // Flag to prevent signal loops from onNextButton
    bool m_sendNew;      // Flag to prevent signal loops from onNewButton
    QString headerTable[8];
    QString conditionsTable[13][5];
    QString prevnextTable[8];
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
    void onUpdateSearchCrateData(const QVariantList& searchCrateData);
    void toggleLockStatus();
    void storeUIIn2Table();
};
