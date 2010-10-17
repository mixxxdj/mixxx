/********************************************************************************
** Form generated from reading UI file 'dlgprefmidibindingsdlg.ui'
**
** Created: Fri Oct 15 21:35:43 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFMIDIBINDINGSDLG_H
#define UI_DLGPREFMIDIBINDINGSDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableView>
#include <QtGui/QToolBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefMidiBindingsDlg
{
public:
    QGridLayout *gridLayout_4;
    QLabel *labelDeviceName;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBoxPresets;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *comboBoxPreset;
    QPushButton *btnExportXML;
    QComboBox *comboBoxOutputDevice;
    QLabel *label_2;
    QCheckBox *chkEnabledDevice;
    QToolBox *toolBox;
    QWidget *page;
    QGridLayout *gridLayout_2;
    QTableView *m_pInputMappingTableView;
    QGroupBox *groupBoxInputManagement;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnAddInputBinding;
    QPushButton *btnRemoveInputBinding;
    QPushButton *btnMidiLearnWizard;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *btnClearAllInputBindings;
    QWidget *page_2;
    QGridLayout *gridLayout_3;
    QTableView *m_pOutputMappingTableView;
    QGroupBox *groupBoxOutputs;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnAddOutputBinding;
    QPushButton *btnRemoveOutputBinding;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btnClearAllOutputBindings;

    void setupUi(QWidget *DlgPrefMidiBindingsDlg)
    {
        if (DlgPrefMidiBindingsDlg->objectName().isEmpty())
            DlgPrefMidiBindingsDlg->setObjectName(QString::fromUtf8("DlgPrefMidiBindingsDlg"));
        DlgPrefMidiBindingsDlg->resize(598, 436);
        gridLayout_4 = new QGridLayout(DlgPrefMidiBindingsDlg);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        labelDeviceName = new QLabel(DlgPrefMidiBindingsDlg);
        labelDeviceName->setObjectName(QString::fromUtf8("labelDeviceName"));
        QFont font;
        font.setPointSize(14);
        font.setBold(true);
        font.setWeight(75);
        labelDeviceName->setFont(font);

        gridLayout_4->addWidget(labelDeviceName, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(115, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 0, 2, 2, 1);

        groupBoxPresets = new QGroupBox(DlgPrefMidiBindingsDlg);
        groupBoxPresets->setObjectName(QString::fromUtf8("groupBoxPresets"));
        groupBoxPresets->setFlat(true);
        groupBoxPresets->setCheckable(false);
        gridLayout = new QGridLayout(groupBoxPresets);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(groupBoxPresets);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 1, 1, 1, 1);

        comboBoxPreset = new QComboBox(groupBoxPresets);
        comboBoxPreset->setObjectName(QString::fromUtf8("comboBoxPreset"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBoxPreset->sizePolicy().hasHeightForWidth());
        comboBoxPreset->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBoxPreset, 1, 2, 1, 1);

        btnExportXML = new QPushButton(groupBoxPresets);
        btnExportXML->setObjectName(QString::fromUtf8("btnExportXML"));
        btnExportXML->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(btnExportXML, 1, 3, 1, 1);

        comboBoxOutputDevice = new QComboBox(groupBoxPresets);
        comboBoxOutputDevice->setObjectName(QString::fromUtf8("comboBoxOutputDevice"));
        comboBoxOutputDevice->setEnabled(false);

        gridLayout->addWidget(comboBoxOutputDevice, 2, 2, 1, 1);

        label_2 = new QLabel(groupBoxPresets);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 1, 1, 1);


        gridLayout_4->addWidget(groupBoxPresets, 0, 3, 2, 1);

        chkEnabledDevice = new QCheckBox(DlgPrefMidiBindingsDlg);
        chkEnabledDevice->setObjectName(QString::fromUtf8("chkEnabledDevice"));

        gridLayout_4->addWidget(chkEnabledDevice, 1, 0, 1, 1);

        toolBox = new QToolBox(DlgPrefMidiBindingsDlg);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        toolBox->setFrameShape(QFrame::NoFrame);
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        page->setGeometry(QRect(0, 0, 580, 252));
        gridLayout_2 = new QGridLayout(page);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        m_pInputMappingTableView = new QTableView(page);
        m_pInputMappingTableView->setObjectName(QString::fromUtf8("m_pInputMappingTableView"));

        gridLayout_2->addWidget(m_pInputMappingTableView, 1, 0, 1, 2);

        groupBoxInputManagement = new QGroupBox(page);
        groupBoxInputManagement->setObjectName(QString::fromUtf8("groupBoxInputManagement"));
        groupBoxInputManagement->setMinimumSize(QSize(300, 0));
        horizontalLayout_2 = new QHBoxLayout(groupBoxInputManagement);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        btnAddInputBinding = new QPushButton(groupBoxInputManagement);
        btnAddInputBinding->setObjectName(QString::fromUtf8("btnAddInputBinding"));
        btnAddInputBinding->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(btnAddInputBinding);

        btnRemoveInputBinding = new QPushButton(groupBoxInputManagement);
        btnRemoveInputBinding->setObjectName(QString::fromUtf8("btnRemoveInputBinding"));
        btnRemoveInputBinding->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(btnRemoveInputBinding);

        btnMidiLearnWizard = new QPushButton(groupBoxInputManagement);
        btnMidiLearnWizard->setObjectName(QString::fromUtf8("btnMidiLearnWizard"));
        btnMidiLearnWizard->setStyleSheet(QString::fromUtf8(""));
        btnMidiLearnWizard->setCheckable(false);
        btnMidiLearnWizard->setChecked(false);

        horizontalLayout_2->addWidget(btnMidiLearnWizard);

        horizontalSpacer_3 = new QSpacerItem(219, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        btnClearAllInputBindings = new QPushButton(groupBoxInputManagement);
        btnClearAllInputBindings->setObjectName(QString::fromUtf8("btnClearAllInputBindings"));
        btnClearAllInputBindings->setAutoFillBackground(false);
        btnClearAllInputBindings->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(btnClearAllInputBindings);


        gridLayout_2->addWidget(groupBoxInputManagement, 2, 0, 1, 2);

        toolBox->addItem(page, QString::fromUtf8("MIDI Input"));
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        page_2->setGeometry(QRect(0, 0, 317, 174));
        gridLayout_3 = new QGridLayout(page_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        m_pOutputMappingTableView = new QTableView(page_2);
        m_pOutputMappingTableView->setObjectName(QString::fromUtf8("m_pOutputMappingTableView"));
        m_pOutputMappingTableView->setEnabled(true);

        gridLayout_3->addWidget(m_pOutputMappingTableView, 0, 0, 1, 1);

        groupBoxOutputs = new QGroupBox(page_2);
        groupBoxOutputs->setObjectName(QString::fromUtf8("groupBoxOutputs"));
        horizontalLayout = new QHBoxLayout(groupBoxOutputs);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        btnAddOutputBinding = new QPushButton(groupBoxOutputs);
        btnAddOutputBinding->setObjectName(QString::fromUtf8("btnAddOutputBinding"));
        btnAddOutputBinding->setEnabled(false);

        horizontalLayout->addWidget(btnAddOutputBinding);

        btnRemoveOutputBinding = new QPushButton(groupBoxOutputs);
        btnRemoveOutputBinding->setObjectName(QString::fromUtf8("btnRemoveOutputBinding"));

        horizontalLayout->addWidget(btnRemoveOutputBinding);

        horizontalSpacer_2 = new QSpacerItem(219, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        btnClearAllOutputBindings = new QPushButton(groupBoxOutputs);
        btnClearAllOutputBindings->setObjectName(QString::fromUtf8("btnClearAllOutputBindings"));

        horizontalLayout->addWidget(btnClearAllOutputBindings);


        gridLayout_3->addWidget(groupBoxOutputs, 1, 0, 1, 1);

        toolBox->addItem(page_2, QString::fromUtf8("MIDI Output"));

        gridLayout_4->addWidget(toolBox, 2, 0, 1, 4);


        retranslateUi(DlgPrefMidiBindingsDlg);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DlgPrefMidiBindingsDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefMidiBindingsDlg)
    {
        DlgPrefMidiBindingsDlg->setWindowTitle(QApplication::translate("DlgPrefMidiBindingsDlg", "Dialog", 0, QApplication::UnicodeUTF8));
        labelDeviceName->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Your Device Name", 0, QApplication::UnicodeUTF8));
        groupBoxPresets->setTitle(QString());
        label->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Load Preset:", 0, QApplication::UnicodeUTF8));
        btnExportXML->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Export", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Output:", 0, QApplication::UnicodeUTF8));
        chkEnabledDevice->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Enabled", 0, QApplication::UnicodeUTF8));
        groupBoxInputManagement->setTitle(QApplication::translate("DlgPrefMidiBindingsDlg", "Controls", 0, QApplication::UnicodeUTF8));
        btnAddInputBinding->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Add", 0, QApplication::UnicodeUTF8));
        btnRemoveInputBinding->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Remove", 0, QApplication::UnicodeUTF8));
        btnMidiLearnWizard->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "MIDI Learning Wizard", 0, QApplication::UnicodeUTF8));
        btnClearAllInputBindings->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Clear All", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(page), QApplication::translate("DlgPrefMidiBindingsDlg", "MIDI Input", 0, QApplication::UnicodeUTF8));
        groupBoxOutputs->setTitle(QApplication::translate("DlgPrefMidiBindingsDlg", "Outputs", 0, QApplication::UnicodeUTF8));
        btnAddOutputBinding->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Add", 0, QApplication::UnicodeUTF8));
        btnRemoveOutputBinding->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Remove", 0, QApplication::UnicodeUTF8));
        btnClearAllOutputBindings->setText(QApplication::translate("DlgPrefMidiBindingsDlg", "Clear All", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(page_2), QApplication::translate("DlgPrefMidiBindingsDlg", "MIDI Output", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefMidiBindingsDlg: public Ui_DlgPrefMidiBindingsDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFMIDIBINDINGSDLG_H
