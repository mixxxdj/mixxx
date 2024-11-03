/********************************************************************************
** Form generated from reading UI file 'dlgsmartiesinfo.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGSMARTIESINFO_H
#define UI_DLGSMARTIESINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_dlgSmartiesInfo {
  public:
    QVBoxLayout* verticalLayout;
    QGroupBox* groupBox;
    QLineEdit* lineEditID;
    QLabel* labelID;
    QLabel* labelName;
    QLineEdit* lineEditName;
    QLabel* labelCount;
    QSpinBox* spinBoxCount;
    QLabel* labelShow;
    QCheckBox* checkBoxShow;
    QLabel* labelLocked;
    QPushButton* buttonLock;
    QLabel* labelAutoDJ;
    QCheckBox* checkBoxAutoDJ;
    QLabel* labelSearchInput;
    QLineEdit* lineEditSearchInput;
    QLabel* labelSearchSQL;
    QLineEdit* lineEditSearchSQL;
    QLabel* labelConditions;
    QScrollArea* scrollAreaConditions;
    QWidget* scrollAreaWidgetContents;
    QVBoxLayout* verticalLayoutConditions;
    QHBoxLayout* horizontalLayoutCondition1;
    QComboBox* comboBoxCondition1Field;
    QComboBox* comboBoxCondition1Operator;
    QLineEdit* lineEditCondition1Value;
    QComboBox* comboBoxCondition1Combiner;
    QHBoxLayout* horizontalLayoutCondition2;
    QComboBox* comboBoxCondition2Field;
    QComboBox* comboBoxCondition2Operator;
    QLineEdit* lineEditCondition2Value;
    QComboBox* comboBoxCondition2Combiner;
    QHBoxLayout* horizontalLayoutCondition3;
    QComboBox* comboBoxCondition3Field;
    QComboBox* comboBoxCondition3Operator;
    QLineEdit* lineEditCondition3Value;
    QComboBox* comboBoxCondition3Combiner;
    QHBoxLayout* horizontalLayoutCondition4;
    QComboBox* comboBoxCondition4Field;
    QComboBox* comboBoxCondition4Operator;
    QLineEdit* lineEditCondition4Value;
    QComboBox* comboBoxCondition4Combiner;
    QHBoxLayout* horizontalLayoutCondition5;
    QComboBox* comboBoxCondition5Field;
    QComboBox* comboBoxCondition5Operator;
    QLineEdit* lineEditCondition5Value;
    QComboBox* comboBoxCondition5Combiner;
    QHBoxLayout* horizontalLayoutCondition6;
    QComboBox* comboBoxCondition6Field;
    QComboBox* comboBoxCondition6Operator;
    QLineEdit* lineEditCondition6Value;
    QComboBox* comboBoxCondition6Combiner;
    QHBoxLayout* horizontalLayoutCondition7;
    QComboBox* comboBoxCondition7Field;
    QComboBox* comboBoxCondition7Operator;
    QLineEdit* lineEditCondition7Value;
    QComboBox* comboBoxCondition7Combiner;
    QHBoxLayout* horizontalLayoutCondition8;
    QComboBox* comboBoxCondition8Field;
    QComboBox* comboBoxCondition8Operator;
    QLineEdit* lineEditCondition8Value;
    QComboBox* comboBoxCondition8Combiner;
    QHBoxLayout* horizontalLayoutCondition9;
    QComboBox* comboBoxCondition9Field;
    QComboBox* comboBoxCondition9Operator;
    QLineEdit* lineEditCondition9Value;
    QComboBox* comboBoxCondition9Combiner;
    QHBoxLayout* horizontalLayoutCondition10;
    QComboBox* comboBoxCondition10Field;
    QComboBox* comboBoxCondition10Operator;
    QLineEdit* lineEditCondition10Value;
    QComboBox* comboBoxCondition10Combiner;
    QHBoxLayout* horizontalLayoutCondition11;
    QComboBox* comboBoxCondition11Field;
    QComboBox* comboBoxCondition11Operator;
    QLineEdit* lineEditCondition11Value;
    QComboBox* comboBoxCondition11Combiner;
    QHBoxLayout* horizontalLayoutCondition12;
    QComboBox* comboBoxCondition12Field;
    QComboBox* comboBoxCondition12Operator;
    QLineEdit* lineEditCondition12Value;
    QComboBox* comboBoxCondition12Combiner;
    QHBoxLayout* horizontalLayoutButtons;
    QPushButton* newButton;
    QPushButton* previousButton;
    QPushButton* nextButton;
    QPushButton* applyButton;
    QPushButton* okButton;

    void setupUi(QDialog* dlgSmartiesInfo) {
        if (dlgSmartiesInfo->objectName().isEmpty())
            dlgSmartiesInfo->setObjectName("dlgSmartiesInfo");
        dlgSmartiesInfo->resize(800, 600);
        verticalLayout = new QVBoxLayout(dlgSmartiesInfo);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(dlgSmartiesInfo);
        groupBox->setObjectName("groupBox");
        groupBox->setMinimumSize(QSize(0, 150));
        lineEditID = new QLineEdit(groupBox);
        lineEditID->setObjectName("lineEditID");
        lineEditID->setGeometry(QRect(120, 20, 60, 20));
        lineEditID->setMinimumSize(QSize(60, 20));
        lineEditID->setMaximumSize(QSize(60, 20));
        lineEditID->setReadOnly(true);
        labelID = new QLabel(groupBox);
        labelID->setObjectName("labelID");
        labelID->setGeometry(QRect(20, 20, 80, 20));
        labelID->setMinimumSize(QSize(80, 20));
        labelID->setMaximumSize(QSize(80, 20));
        labelName = new QLabel(groupBox);
        labelName->setObjectName("labelName");
        labelName->setGeometry(QRect(240, 20, 50, 20));
        labelName->setMinimumSize(QSize(50, 20));
        labelName->setMaximumSize(QSize(50, 20));
        lineEditName = new QLineEdit(groupBox);
        lineEditName->setObjectName("lineEditName");
        lineEditName->setGeometry(QRect(290, 20, 400, 20));
        lineEditName->setMinimumSize(QSize(400, 20));
        lineEditName->setMaximumSize(QSize(400, 20));
        labelCount = new QLabel(groupBox);
        labelCount->setObjectName("labelCount");
        labelCount->setGeometry(QRect(240, 50, 50, 20));
        labelCount->setMinimumSize(QSize(50, 20));
        labelCount->setMaximumSize(QSize(50, 20));
        spinBoxCount = new QSpinBox(groupBox);
        spinBoxCount->setObjectName("spinBoxCount");
        spinBoxCount->setGeometry(QRect(290, 50, 60, 20));
        spinBoxCount->setMinimumSize(QSize(60, 20));
        spinBoxCount->setMaximumSize(QSize(60, 20));
        labelShow = new QLabel(groupBox);
        labelShow->setObjectName("labelShow");
        labelShow->setGeometry(QRect(600, 80, 50, 20));
        labelShow->setMinimumSize(QSize(50, 20));
        labelShow->setMaximumSize(QSize(50, 20));
        checkBoxShow = new QCheckBox(groupBox);
        checkBoxShow->setObjectName("checkBoxShow");
        checkBoxShow->setGeometry(QRect(580, 82, 30, 20));
        checkBoxShow->setMinimumSize(QSize(30, 20));
        checkBoxShow->setMaximumSize(QSize(30, 20));
        labelLocked = new QLabel(groupBox);
        labelLocked->setObjectName("labelLocked");
        labelLocked->setGeometry(QRect(20, 50, 50, 20));
        labelLocked->setMinimumSize(QSize(50, 20));
        labelLocked->setMaximumSize(QSize(50, 20));
        buttonLock = new QPushButton(groupBox);
        buttonLock->setObjectName("buttonLock");
        buttonLock->setGeometry(QRect(120, 50, 60, 20));
        buttonLock->setMinimumSize(QSize(60, 20));
        buttonLock->setMaximumSize(QSize(60, 20));
        labelAutoDJ = new QLabel(groupBox);
        labelAutoDJ->setObjectName("labelAutoDJ");
        labelAutoDJ->setGeometry(QRect(600, 110, 80, 20));
        labelAutoDJ->setMinimumSize(QSize(80, 20));
        labelAutoDJ->setMaximumSize(QSize(80, 20));
        checkBoxAutoDJ = new QCheckBox(groupBox);
        checkBoxAutoDJ->setObjectName("checkBoxAutoDJ");
        checkBoxAutoDJ->setGeometry(QRect(580, 114, 30, 13));
        checkBoxAutoDJ->setMinimumSize(QSize(30, 0));
        checkBoxAutoDJ->setMaximumSize(QSize(30, 20));
        labelSearchInput = new QLabel(groupBox);
        labelSearchInput->setObjectName("labelSearchInput");
        labelSearchInput->setGeometry(QRect(20, 80, 80, 20));
        labelSearchInput->setMinimumSize(QSize(80, 20));
        labelSearchInput->setMaximumSize(QSize(80, 20));
        lineEditSearchInput = new QLineEdit(groupBox);
        lineEditSearchInput->setObjectName("lineEditSearchInput");
        lineEditSearchInput->setGeometry(QRect(120, 80, 400, 20));
        lineEditSearchInput->setMinimumSize(QSize(400, 20));
        lineEditSearchInput->setMaximumSize(QSize(400, 20));
        labelSearchSQL = new QLabel(groupBox);
        labelSearchSQL->setObjectName("labelSearchSQL");
        labelSearchSQL->setGeometry(QRect(20, 110, 80, 20));
        labelSearchSQL->setMinimumSize(QSize(80, 20));
        labelSearchSQL->setMaximumSize(QSize(80, 20));
        lineEditSearchSQL = new QLineEdit(groupBox);
        lineEditSearchSQL->setObjectName("lineEditSearchSQL");
        lineEditSearchSQL->setGeometry(QRect(120, 110, 400, 20));
        lineEditSearchSQL->setMinimumSize(QSize(400, 20));
        lineEditSearchSQL->setMaximumSize(QSize(400, 20));

        verticalLayout->addWidget(groupBox);

        labelConditions = new QLabel(dlgSmartiesInfo);
        labelConditions->setObjectName("labelConditions");
        labelConditions->setMinimumSize(QSize(60, 15));

        verticalLayout->addWidget(labelConditions);

        scrollAreaConditions = new QScrollArea(dlgSmartiesInfo);
        scrollAreaConditions->setObjectName("scrollAreaConditions");
        scrollAreaConditions->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 780, 372));
        verticalLayoutConditions = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayoutConditions->setObjectName("verticalLayoutConditions");
        horizontalLayoutCondition1 = new QHBoxLayout();
        horizontalLayoutCondition1->setObjectName("horizontalLayoutCondition1");
        comboBoxCondition1Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition1Field->setObjectName("comboBoxCondition1Field");

        horizontalLayoutCondition1->addWidget(comboBoxCondition1Field);

        comboBoxCondition1Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition1Operator->setObjectName("comboBoxCondition1Operator");

        horizontalLayoutCondition1->addWidget(comboBoxCondition1Operator);

        lineEditCondition1Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition1Value->setObjectName("lineEditCondition1Value");

        horizontalLayoutCondition1->addWidget(lineEditCondition1Value);

        comboBoxCondition1Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition1Combiner->setObjectName("comboBoxCondition1Combiner");

        horizontalLayoutCondition1->addWidget(comboBoxCondition1Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition1);

        horizontalLayoutCondition2 = new QHBoxLayout();
        horizontalLayoutCondition2->setObjectName("horizontalLayoutCondition2");
        comboBoxCondition2Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition2Field->setObjectName("comboBoxCondition2Field");

        horizontalLayoutCondition2->addWidget(comboBoxCondition2Field);

        comboBoxCondition2Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition2Operator->setObjectName("comboBoxCondition2Operator");

        horizontalLayoutCondition2->addWidget(comboBoxCondition2Operator);

        lineEditCondition2Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition2Value->setObjectName("lineEditCondition2Value");

        horizontalLayoutCondition2->addWidget(lineEditCondition2Value);

        comboBoxCondition2Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition2Combiner->setObjectName("comboBoxCondition2Combiner");

        horizontalLayoutCondition2->addWidget(comboBoxCondition2Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition2);

        horizontalLayoutCondition3 = new QHBoxLayout();
        horizontalLayoutCondition3->setObjectName("horizontalLayoutCondition3");
        comboBoxCondition3Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition3Field->setObjectName("comboBoxCondition3Field");

        horizontalLayoutCondition3->addWidget(comboBoxCondition3Field);

        comboBoxCondition3Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition3Operator->setObjectName("comboBoxCondition3Operator");

        horizontalLayoutCondition3->addWidget(comboBoxCondition3Operator);

        lineEditCondition3Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition3Value->setObjectName("lineEditCondition3Value");

        horizontalLayoutCondition3->addWidget(lineEditCondition3Value);

        comboBoxCondition3Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition3Combiner->setObjectName("comboBoxCondition3Combiner");

        horizontalLayoutCondition3->addWidget(comboBoxCondition3Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition3);

        horizontalLayoutCondition4 = new QHBoxLayout();
        horizontalLayoutCondition4->setObjectName("horizontalLayoutCondition4");
        comboBoxCondition4Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition4Field->setObjectName("comboBoxCondition4Field");

        horizontalLayoutCondition4->addWidget(comboBoxCondition4Field);

        comboBoxCondition4Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition4Operator->setObjectName("comboBoxCondition4Operator");

        horizontalLayoutCondition4->addWidget(comboBoxCondition4Operator);

        lineEditCondition4Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition4Value->setObjectName("lineEditCondition4Value");

        horizontalLayoutCondition4->addWidget(lineEditCondition4Value);

        comboBoxCondition4Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition4Combiner->setObjectName("comboBoxCondition4Combiner");

        horizontalLayoutCondition4->addWidget(comboBoxCondition4Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition4);

        horizontalLayoutCondition5 = new QHBoxLayout();
        horizontalLayoutCondition5->setObjectName("horizontalLayoutCondition5");
        comboBoxCondition5Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition5Field->setObjectName("comboBoxCondition5Field");

        horizontalLayoutCondition5->addWidget(comboBoxCondition5Field);

        comboBoxCondition5Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition5Operator->setObjectName("comboBoxCondition5Operator");

        horizontalLayoutCondition5->addWidget(comboBoxCondition5Operator);

        lineEditCondition5Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition5Value->setObjectName("lineEditCondition5Value");

        horizontalLayoutCondition5->addWidget(lineEditCondition5Value);

        comboBoxCondition5Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition5Combiner->setObjectName("comboBoxCondition5Combiner");

        horizontalLayoutCondition5->addWidget(comboBoxCondition5Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition5);

        horizontalLayoutCondition6 = new QHBoxLayout();
        horizontalLayoutCondition6->setObjectName("horizontalLayoutCondition6");
        comboBoxCondition6Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition6Field->setObjectName("comboBoxCondition6Field");

        horizontalLayoutCondition6->addWidget(comboBoxCondition6Field);

        comboBoxCondition6Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition6Operator->setObjectName("comboBoxCondition6Operator");

        horizontalLayoutCondition6->addWidget(comboBoxCondition6Operator);

        lineEditCondition6Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition6Value->setObjectName("lineEditCondition6Value");

        horizontalLayoutCondition6->addWidget(lineEditCondition6Value);

        comboBoxCondition6Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition6Combiner->setObjectName("comboBoxCondition6Combiner");

        horizontalLayoutCondition6->addWidget(comboBoxCondition6Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition6);

        horizontalLayoutCondition7 = new QHBoxLayout();
        horizontalLayoutCondition7->setObjectName("horizontalLayoutCondition7");
        comboBoxCondition7Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition7Field->setObjectName("comboBoxCondition7Field");

        horizontalLayoutCondition7->addWidget(comboBoxCondition7Field);

        comboBoxCondition7Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition7Operator->setObjectName("comboBoxCondition7Operator");

        horizontalLayoutCondition7->addWidget(comboBoxCondition7Operator);

        lineEditCondition7Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition7Value->setObjectName("lineEditCondition7Value");

        horizontalLayoutCondition7->addWidget(lineEditCondition7Value);

        comboBoxCondition7Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition7Combiner->setObjectName("comboBoxCondition7Combiner");

        horizontalLayoutCondition7->addWidget(comboBoxCondition7Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition7);

        horizontalLayoutCondition8 = new QHBoxLayout();
        horizontalLayoutCondition8->setObjectName("horizontalLayoutCondition8");
        comboBoxCondition8Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition8Field->setObjectName("comboBoxCondition8Field");

        horizontalLayoutCondition8->addWidget(comboBoxCondition8Field);

        comboBoxCondition8Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition8Operator->setObjectName("comboBoxCondition8Operator");

        horizontalLayoutCondition8->addWidget(comboBoxCondition8Operator);

        lineEditCondition8Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition8Value->setObjectName("lineEditCondition8Value");

        horizontalLayoutCondition8->addWidget(lineEditCondition8Value);

        comboBoxCondition8Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition8Combiner->setObjectName("comboBoxCondition8Combiner");

        horizontalLayoutCondition8->addWidget(comboBoxCondition8Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition8);

        horizontalLayoutCondition9 = new QHBoxLayout();
        horizontalLayoutCondition9->setObjectName("horizontalLayoutCondition9");
        comboBoxCondition9Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition9Field->setObjectName("comboBoxCondition9Field");

        horizontalLayoutCondition9->addWidget(comboBoxCondition9Field);

        comboBoxCondition9Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition9Operator->setObjectName("comboBoxCondition9Operator");

        horizontalLayoutCondition9->addWidget(comboBoxCondition9Operator);

        lineEditCondition9Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition9Value->setObjectName("lineEditCondition9Value");

        horizontalLayoutCondition9->addWidget(lineEditCondition9Value);

        comboBoxCondition9Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition9Combiner->setObjectName("comboBoxCondition9Combiner");

        horizontalLayoutCondition9->addWidget(comboBoxCondition9Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition9);

        horizontalLayoutCondition10 = new QHBoxLayout();
        horizontalLayoutCondition10->setObjectName("horizontalLayoutCondition10");
        comboBoxCondition10Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition10Field->setObjectName("comboBoxCondition10Field");

        horizontalLayoutCondition10->addWidget(comboBoxCondition10Field);

        comboBoxCondition10Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition10Operator->setObjectName("comboBoxCondition10Operator");

        horizontalLayoutCondition10->addWidget(comboBoxCondition10Operator);

        lineEditCondition10Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition10Value->setObjectName("lineEditCondition10Value");

        horizontalLayoutCondition10->addWidget(lineEditCondition10Value);

        comboBoxCondition10Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition10Combiner->setObjectName("comboBoxCondition10Combiner");

        horizontalLayoutCondition10->addWidget(comboBoxCondition10Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition10);

        horizontalLayoutCondition11 = new QHBoxLayout();
        horizontalLayoutCondition11->setObjectName("horizontalLayoutCondition11");
        comboBoxCondition11Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition11Field->setObjectName("comboBoxCondition11Field");

        horizontalLayoutCondition11->addWidget(comboBoxCondition11Field);

        comboBoxCondition11Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition11Operator->setObjectName("comboBoxCondition11Operator");

        horizontalLayoutCondition11->addWidget(comboBoxCondition11Operator);

        lineEditCondition11Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition11Value->setObjectName("lineEditCondition11Value");

        horizontalLayoutCondition11->addWidget(lineEditCondition11Value);

        comboBoxCondition11Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition11Combiner->setObjectName("comboBoxCondition11Combiner");

        horizontalLayoutCondition11->addWidget(comboBoxCondition11Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition11);

        horizontalLayoutCondition12 = new QHBoxLayout();
        horizontalLayoutCondition12->setObjectName("horizontalLayoutCondition12");
        comboBoxCondition12Field = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition12Field->setObjectName("comboBoxCondition12Field");

        horizontalLayoutCondition12->addWidget(comboBoxCondition12Field);

        comboBoxCondition12Operator = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition12Operator->setObjectName("comboBoxCondition12Operator");

        horizontalLayoutCondition12->addWidget(comboBoxCondition12Operator);

        lineEditCondition12Value = new QLineEdit(scrollAreaWidgetContents);
        lineEditCondition12Value->setObjectName("lineEditCondition12Value");

        horizontalLayoutCondition12->addWidget(lineEditCondition12Value);

        comboBoxCondition12Combiner = new QComboBox(scrollAreaWidgetContents);
        comboBoxCondition12Combiner->setObjectName("comboBoxCondition12Combiner");

        horizontalLayoutCondition12->addWidget(comboBoxCondition12Combiner);

        verticalLayoutConditions->addLayout(horizontalLayoutCondition12);

        scrollAreaConditions->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollAreaConditions);

        horizontalLayoutButtons = new QHBoxLayout();
        horizontalLayoutButtons->setObjectName("horizontalLayoutButtons");
        newButton = new QPushButton(dlgSmartiesInfo);
        newButton->setObjectName("newButton");

        horizontalLayoutButtons->addWidget(newButton);

        previousButton = new QPushButton(dlgSmartiesInfo);
        previousButton->setObjectName("previousButton");

        horizontalLayoutButtons->addWidget(previousButton);

        nextButton = new QPushButton(dlgSmartiesInfo);
        nextButton->setObjectName("nextButton");

        horizontalLayoutButtons->addWidget(nextButton);

        applyButton = new QPushButton(dlgSmartiesInfo);
        applyButton->setObjectName("applyButton");

        horizontalLayoutButtons->addWidget(applyButton);

        okButton = new QPushButton(dlgSmartiesInfo);
        okButton->setObjectName("okButton");

        horizontalLayoutButtons->addWidget(okButton);

        verticalLayout->addLayout(horizontalLayoutButtons);

        retranslateUi(dlgSmartiesInfo);

        QMetaObject::connectSlotsByName(dlgSmartiesInfo);
    } // setupUi

    void retranslateUi(QDialog* dlgSmartiesInfo) {
        dlgSmartiesInfo->setWindowTitle(QCoreApplication::translate(
                "dlgSmartiesInfo", "Smarties Info", nullptr));
        groupBox->setTitle(QCoreApplication::translate(
                "dlgSmartiesInfo", "Smarties INFO", nullptr));
        labelID->setText(QCoreApplication::translate("dlgSmartiesInfo", "ID:", nullptr));
        labelName->setText(QCoreApplication::translate("dlgSmartiesInfo", "Name:", nullptr));
        labelCount->setText(QCoreApplication::translate("dlgSmartiesInfo", "Count:", nullptr));
        labelShow->setText(QCoreApplication::translate("dlgSmartiesInfo", "Show", nullptr));
        labelLocked->setText(QCoreApplication::translate("dlgSmartiesInfo", "Locked:", nullptr));
        buttonLock->setText(QCoreApplication::translate("dlgSmartiesInfo", "Lock", nullptr));
        labelAutoDJ->setText(QCoreApplication::translate(
                "dlgSmartiesInfo", "AutoDJ Source", nullptr));
        labelSearchInput->setText(QCoreApplication::translate(
                "dlgSmartiesInfo", "Search Input:", nullptr));
        labelSearchSQL->setText(QCoreApplication::translate(
                "dlgSmartiesInfo", "Search SQL:", nullptr));
        labelConditions->setText(QCoreApplication::translate(
                "dlgSmartiesInfo", "Smarties Conditions:", nullptr));
        newButton->setText(QCoreApplication::translate("dlgSmartiesInfo", "New", nullptr));
        previousButton->setText(QCoreApplication::translate(
                "dlgSmartiesInfo", "Previous", nullptr));
        nextButton->setText(QCoreApplication::translate("dlgSmartiesInfo", "Next", nullptr));
        applyButton->setText(QCoreApplication::translate("dlgSmartiesInfo", "Apply", nullptr));
        okButton->setText(QCoreApplication::translate("dlgSmartiesInfo", "OK", nullptr));
    } // retranslateUi
};

namespace Ui {
class dlgSmartiesInfo : public Ui_dlgSmartiesInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGSMARTIESINFO_H
