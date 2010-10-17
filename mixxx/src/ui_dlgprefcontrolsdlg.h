/********************************************************************************
** Form generated from reading UI file 'dlgprefcontrolsdlg.ui'
**
** Created: Fri Oct 15 21:35:36 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFCONTROLSDLG_H
#define UI_DLGPREFCONTROLSDLG_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefControlsDlg
{
public:
    QGridLayout *gridLayout_3;
    QGridLayout *gridLayout;
    QLabel *TextLabel6_2_2;
    QComboBox *ComboBoxSkinconf;
    QLabel *TextLabel6_2_2_4;
    QComboBox *ComboBoxSchemeconf;
    QLabel *TextLabel6_2_2_2;
    QComboBox *ComboBoxVisuals;
    QLabel *TextLabel6_2_2_3;
    QComboBox *ComboBoxPosition;
    QLabel *TextLabel6_2_2_3_2;
    QComboBox *ComboBoxTooltips;
    QLabel *TextLabel6;
    QComboBox *ComboBoxRateRange;
    QLabel *TextLabel6_2;
    QComboBox *ComboBoxRateDir;
    QLabel *label;
    QComboBox *ComboBoxCueDefault;
    QLabel *textLabelCueRecall;
    QComboBox *ComboBoxCueRecall;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_4;
    QLabel *TextLabel6_2_3_2;
    QLabel *TextLabel6_2_3_2_2;
    QDoubleSpinBox *spinBoxPermRateLeft;
    QDoubleSpinBox *spinBoxPermRateRight;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout1;
    QLabel *TextLabel6_2_3_2_3;
    QLabel *TextLabel6_2_3_2_2_2;
    QDoubleSpinBox *spinBoxTempRateLeft;
    QDoubleSpinBox *spinBoxTempRateRight;
    QGroupBox *groupBoxRateRamp;
    QGridLayout *gridLayout_5;
    QLabel *label_2;
    QSlider *SliderRateRampSensitivity;
    QSpinBox *SpinBoxRateRampSensitivity;

    void setupUi(QWidget *DlgPrefControlsDlg)
    {
        if (DlgPrefControlsDlg->objectName().isEmpty())
            DlgPrefControlsDlg->setObjectName(QString::fromUtf8("DlgPrefControlsDlg"));
        DlgPrefControlsDlg->resize(499, 489);
        gridLayout_3 = new QGridLayout(DlgPrefControlsDlg);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        TextLabel6_2_2 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2_2->setObjectName(QString::fromUtf8("TextLabel6_2_2"));
        TextLabel6_2_2->setEnabled(true);
        QFont font;
        TextLabel6_2_2->setFont(font);
        TextLabel6_2_2->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2_2, 0, 0, 1, 1);

        ComboBoxSkinconf = new QComboBox(DlgPrefControlsDlg);
        ComboBoxSkinconf->setObjectName(QString::fromUtf8("ComboBoxSkinconf"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ComboBoxSkinconf->sizePolicy().hasHeightForWidth());
        ComboBoxSkinconf->setSizePolicy(sizePolicy);
        ComboBoxSkinconf->setFont(font);

        gridLayout->addWidget(ComboBoxSkinconf, 0, 1, 1, 2);

        TextLabel6_2_2_4 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2_2_4->setObjectName(QString::fromUtf8("TextLabel6_2_2_4"));
        TextLabel6_2_2_4->setEnabled(true);
        TextLabel6_2_2_4->setFont(font);
        TextLabel6_2_2_4->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2_2_4, 1, 0, 1, 1);

        ComboBoxSchemeconf = new QComboBox(DlgPrefControlsDlg);
        ComboBoxSchemeconf->setObjectName(QString::fromUtf8("ComboBoxSchemeconf"));
        sizePolicy.setHeightForWidth(ComboBoxSchemeconf->sizePolicy().hasHeightForWidth());
        ComboBoxSchemeconf->setSizePolicy(sizePolicy);
        ComboBoxSchemeconf->setFont(font);

        gridLayout->addWidget(ComboBoxSchemeconf, 1, 1, 1, 2);

        TextLabel6_2_2_2 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2_2_2->setObjectName(QString::fromUtf8("TextLabel6_2_2_2"));
        TextLabel6_2_2_2->setEnabled(true);
        TextLabel6_2_2_2->setFont(font);
        TextLabel6_2_2_2->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2_2_2, 2, 0, 1, 1);

        ComboBoxVisuals = new QComboBox(DlgPrefControlsDlg);
        ComboBoxVisuals->setObjectName(QString::fromUtf8("ComboBoxVisuals"));
        sizePolicy.setHeightForWidth(ComboBoxVisuals->sizePolicy().hasHeightForWidth());
        ComboBoxVisuals->setSizePolicy(sizePolicy);
        ComboBoxVisuals->setFont(font);

        gridLayout->addWidget(ComboBoxVisuals, 2, 1, 1, 2);

        TextLabel6_2_2_3 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2_2_3->setObjectName(QString::fromUtf8("TextLabel6_2_2_3"));
        TextLabel6_2_2_3->setEnabled(true);
        TextLabel6_2_2_3->setFont(font);
        TextLabel6_2_2_3->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2_2_3, 3, 0, 1, 1);

        ComboBoxPosition = new QComboBox(DlgPrefControlsDlg);
        ComboBoxPosition->setObjectName(QString::fromUtf8("ComboBoxPosition"));
        sizePolicy.setHeightForWidth(ComboBoxPosition->sizePolicy().hasHeightForWidth());
        ComboBoxPosition->setSizePolicy(sizePolicy);
        ComboBoxPosition->setFont(font);

        gridLayout->addWidget(ComboBoxPosition, 3, 1, 1, 2);

        TextLabel6_2_2_3_2 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2_2_3_2->setObjectName(QString::fromUtf8("TextLabel6_2_2_3_2"));
        TextLabel6_2_2_3_2->setEnabled(true);
        TextLabel6_2_2_3_2->setFont(font);
        TextLabel6_2_2_3_2->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2_2_3_2, 5, 0, 1, 1);

        ComboBoxTooltips = new QComboBox(DlgPrefControlsDlg);
        ComboBoxTooltips->setObjectName(QString::fromUtf8("ComboBoxTooltips"));
        sizePolicy.setHeightForWidth(ComboBoxTooltips->sizePolicy().hasHeightForWidth());
        ComboBoxTooltips->setSizePolicy(sizePolicy);
        ComboBoxTooltips->setFont(font);

        gridLayout->addWidget(ComboBoxTooltips, 5, 1, 1, 2);

        TextLabel6 = new QLabel(DlgPrefControlsDlg);
        TextLabel6->setObjectName(QString::fromUtf8("TextLabel6"));
        TextLabel6->setEnabled(true);
        TextLabel6->setFont(font);
        TextLabel6->setWordWrap(false);

        gridLayout->addWidget(TextLabel6, 6, 0, 1, 1);

        ComboBoxRateRange = new QComboBox(DlgPrefControlsDlg);
        ComboBoxRateRange->setObjectName(QString::fromUtf8("ComboBoxRateRange"));
        sizePolicy.setHeightForWidth(ComboBoxRateRange->sizePolicy().hasHeightForWidth());
        ComboBoxRateRange->setSizePolicy(sizePolicy);
        ComboBoxRateRange->setFont(font);

        gridLayout->addWidget(ComboBoxRateRange, 6, 1, 1, 2);

        TextLabel6_2 = new QLabel(DlgPrefControlsDlg);
        TextLabel6_2->setObjectName(QString::fromUtf8("TextLabel6_2"));
        TextLabel6_2->setEnabled(true);
        TextLabel6_2->setFont(font);
        TextLabel6_2->setWordWrap(false);

        gridLayout->addWidget(TextLabel6_2, 7, 0, 1, 1);

        ComboBoxRateDir = new QComboBox(DlgPrefControlsDlg);
        ComboBoxRateDir->setObjectName(QString::fromUtf8("ComboBoxRateDir"));
        sizePolicy.setHeightForWidth(ComboBoxRateDir->sizePolicy().hasHeightForWidth());
        ComboBoxRateDir->setSizePolicy(sizePolicy);
        ComboBoxRateDir->setFont(font);

        gridLayout->addWidget(ComboBoxRateDir, 7, 1, 1, 2);

        label = new QLabel(DlgPrefControlsDlg);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 10, 0, 1, 1);

        ComboBoxCueDefault = new QComboBox(DlgPrefControlsDlg);
        ComboBoxCueDefault->setObjectName(QString::fromUtf8("ComboBoxCueDefault"));

        gridLayout->addWidget(ComboBoxCueDefault, 10, 1, 1, 2);

        textLabelCueRecall = new QLabel(DlgPrefControlsDlg);
        textLabelCueRecall->setObjectName(QString::fromUtf8("textLabelCueRecall"));

        gridLayout->addWidget(textLabelCueRecall, 9, 0, 1, 1);

        ComboBoxCueRecall = new QComboBox(DlgPrefControlsDlg);
        ComboBoxCueRecall->setObjectName(QString::fromUtf8("ComboBoxCueRecall"));

        gridLayout->addWidget(ComboBoxCueRecall, 9, 1, 1, 2);


        gridLayout_3->addLayout(gridLayout, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(DlgPrefControlsDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setCheckable(false);
        gridLayout_4 = new QGridLayout(groupBox);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        TextLabel6_2_3_2 = new QLabel(groupBox);
        TextLabel6_2_3_2->setObjectName(QString::fromUtf8("TextLabel6_2_3_2"));
        TextLabel6_2_3_2->setEnabled(true);
        TextLabel6_2_3_2->setFont(font);
        TextLabel6_2_3_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabel6_2_3_2->setWordWrap(false);

        gridLayout_4->addWidget(TextLabel6_2_3_2, 0, 0, 1, 1);

        TextLabel6_2_3_2_2 = new QLabel(groupBox);
        TextLabel6_2_3_2_2->setObjectName(QString::fromUtf8("TextLabel6_2_3_2_2"));
        TextLabel6_2_3_2_2->setEnabled(true);
        TextLabel6_2_3_2_2->setFont(font);
        TextLabel6_2_3_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabel6_2_3_2_2->setWordWrap(false);

        gridLayout_4->addWidget(TextLabel6_2_3_2_2, 1, 0, 1, 1);

        spinBoxPermRateLeft = new QDoubleSpinBox(groupBox);
        spinBoxPermRateLeft->setObjectName(QString::fromUtf8("spinBoxPermRateLeft"));
        spinBoxPermRateLeft->setAccelerated(true);
        spinBoxPermRateLeft->setDecimals(2);
        spinBoxPermRateLeft->setMinimum(0.1);
        spinBoxPermRateLeft->setMaximum(10);
        spinBoxPermRateLeft->setSingleStep(0.1);

        gridLayout_4->addWidget(spinBoxPermRateLeft, 0, 1, 1, 1);

        spinBoxPermRateRight = new QDoubleSpinBox(groupBox);
        spinBoxPermRateRight->setObjectName(QString::fromUtf8("spinBoxPermRateRight"));
        spinBoxPermRateRight->setAccelerated(true);
        spinBoxPermRateRight->setDecimals(2);
        spinBoxPermRateRight->setMinimum(0.1);
        spinBoxPermRateRight->setMaximum(10);
        spinBoxPermRateRight->setSingleStep(0.1);

        gridLayout_4->addWidget(spinBoxPermRateRight, 1, 1, 1, 1);


        gridLayout_2->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(DlgPrefControlsDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout1 = new QGridLayout(groupBox_2);
        gridLayout1->setSpacing(6);
        gridLayout1->setContentsMargins(11, 11, 11, 11);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        TextLabel6_2_3_2_3 = new QLabel(groupBox_2);
        TextLabel6_2_3_2_3->setObjectName(QString::fromUtf8("TextLabel6_2_3_2_3"));
        TextLabel6_2_3_2_3->setEnabled(true);
        TextLabel6_2_3_2_3->setFont(font);
        TextLabel6_2_3_2_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabel6_2_3_2_3->setWordWrap(false);

        gridLayout1->addWidget(TextLabel6_2_3_2_3, 0, 0, 1, 1);

        TextLabel6_2_3_2_2_2 = new QLabel(groupBox_2);
        TextLabel6_2_3_2_2_2->setObjectName(QString::fromUtf8("TextLabel6_2_3_2_2_2"));
        TextLabel6_2_3_2_2_2->setEnabled(true);
        TextLabel6_2_3_2_2_2->setFont(font);
        TextLabel6_2_3_2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabel6_2_3_2_2_2->setWordWrap(false);

        gridLayout1->addWidget(TextLabel6_2_3_2_2_2, 1, 0, 1, 1);

        spinBoxTempRateLeft = new QDoubleSpinBox(groupBox_2);
        spinBoxTempRateLeft->setObjectName(QString::fromUtf8("spinBoxTempRateLeft"));
        spinBoxTempRateLeft->setAccelerated(true);
        spinBoxTempRateLeft->setDecimals(2);
        spinBoxTempRateLeft->setMinimum(0.1);
        spinBoxTempRateLeft->setMaximum(10);
        spinBoxTempRateLeft->setSingleStep(0.1);

        gridLayout1->addWidget(spinBoxTempRateLeft, 0, 1, 1, 1);

        spinBoxTempRateRight = new QDoubleSpinBox(groupBox_2);
        spinBoxTempRateRight->setObjectName(QString::fromUtf8("spinBoxTempRateRight"));
        spinBoxTempRateRight->setAccelerated(true);
        spinBoxTempRateRight->setDecimals(2);
        spinBoxTempRateRight->setMinimum(0.1);
        spinBoxTempRateRight->setMaximum(10);
        spinBoxTempRateRight->setSingleStep(0.1);

        gridLayout1->addWidget(spinBoxTempRateRight, 1, 1, 1, 1);


        gridLayout_2->addWidget(groupBox_2, 0, 1, 1, 1);


        horizontalLayout->addLayout(gridLayout_2);


        gridLayout_3->addLayout(horizontalLayout, 1, 0, 1, 1);

        groupBoxRateRamp = new QGroupBox(DlgPrefControlsDlg);
        groupBoxRateRamp->setObjectName(QString::fromUtf8("groupBoxRateRamp"));
        groupBoxRateRamp->setEnabled(true);
        groupBoxRateRamp->setCheckable(true);
        groupBoxRateRamp->setChecked(false);
        gridLayout_5 = new QGridLayout(groupBoxRateRamp);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        label_2 = new QLabel(groupBoxRateRamp);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_5->addWidget(label_2, 0, 0, 1, 1);

        SliderRateRampSensitivity = new QSlider(groupBoxRateRamp);
        SliderRateRampSensitivity->setObjectName(QString::fromUtf8("SliderRateRampSensitivity"));
        SliderRateRampSensitivity->setEnabled(false);
        SliderRateRampSensitivity->setMinimum(100);
        SliderRateRampSensitivity->setMaximum(2500);
        SliderRateRampSensitivity->setSingleStep(50);
        SliderRateRampSensitivity->setValue(250);
        SliderRateRampSensitivity->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(SliderRateRampSensitivity, 0, 1, 1, 1);

        SpinBoxRateRampSensitivity = new QSpinBox(groupBoxRateRamp);
        SpinBoxRateRampSensitivity->setObjectName(QString::fromUtf8("SpinBoxRateRampSensitivity"));
        SpinBoxRateRampSensitivity->setEnabled(false);
        SpinBoxRateRampSensitivity->setMinimum(100);
        SpinBoxRateRampSensitivity->setMaximum(2500);
        SpinBoxRateRampSensitivity->setSingleStep(1);
        SpinBoxRateRampSensitivity->setValue(250);

        gridLayout_5->addWidget(SpinBoxRateRampSensitivity, 0, 2, 1, 1);


        gridLayout_3->addWidget(groupBoxRateRamp, 2, 0, 1, 1);


        retranslateUi(DlgPrefControlsDlg);
        QObject::connect(SliderRateRampSensitivity, SIGNAL(valueChanged(int)), SpinBoxRateRampSensitivity, SLOT(setValue(int)));
        QObject::connect(SpinBoxRateRampSensitivity, SIGNAL(valueChanged(int)), SliderRateRampSensitivity, SLOT(setValue(int)));

        QMetaObject::connectSlotsByName(DlgPrefControlsDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefControlsDlg)
    {
        DlgPrefControlsDlg->setWindowTitle(QApplication::translate("DlgPrefControlsDlg", "Form1", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_2->setText(QApplication::translate("DlgPrefControlsDlg", "Skin", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_2_4->setText(QApplication::translate("DlgPrefControlsDlg", "Scheme", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_2_2->setText(QApplication::translate("DlgPrefControlsDlg", "Waveform display", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_2_3->setText(QApplication::translate("DlgPrefControlsDlg", "Position display", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_2_3_2->setText(QApplication::translate("DlgPrefControlsDlg", "Tool tips", 0, QApplication::UnicodeUTF8));
        ComboBoxTooltips->clear();
        ComboBoxTooltips->insertItems(0, QStringList()
         << QApplication::translate("DlgPrefControlsDlg", "On", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("DlgPrefControlsDlg", "Off", 0, QApplication::UnicodeUTF8)
        );
        TextLabel6->setText(QApplication::translate("DlgPrefControlsDlg", "Pitch/Rate slider range", 0, QApplication::UnicodeUTF8));
        TextLabel6_2->setText(QApplication::translate("DlgPrefControlsDlg", "Pitch/Rate slider direction", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgPrefControlsDlg", "Cue behaviour", 0, QApplication::UnicodeUTF8));
        textLabelCueRecall->setText(QApplication::translate("DlgPrefControlsDlg", "Auto Recall Cue", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DlgPrefControlsDlg", "Permanent Pitch/Rate Buttons", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_3_2->setText(QApplication::translate("DlgPrefControlsDlg", "Left click", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_3_2_2->setText(QApplication::translate("DlgPrefControlsDlg", "Right click", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinBoxPermRateLeft->setToolTip(QApplication::translate("DlgPrefControlsDlg", "Permanent rate change (between 1 and 8000) when left clicking", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        spinBoxPermRateLeft->setSuffix(QApplication::translate("DlgPrefControlsDlg", "%", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinBoxPermRateRight->setToolTip(QApplication::translate("DlgPrefControlsDlg", "Permanent rate change (between 1 and 8000) when right clicking", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        spinBoxPermRateRight->setSuffix(QApplication::translate("DlgPrefControlsDlg", "%", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("DlgPrefControlsDlg", "Temporary Pitch/Rate Buttons", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_3_2_3->setText(QApplication::translate("DlgPrefControlsDlg", "Left click", 0, QApplication::UnicodeUTF8));
        TextLabel6_2_3_2_2_2->setText(QApplication::translate("DlgPrefControlsDlg", "Right click", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinBoxTempRateLeft->setToolTip(QApplication::translate("DlgPrefControlsDlg", "Temporary rate change (between 1 and 8000) when left clicking", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        spinBoxTempRateLeft->setSuffix(QApplication::translate("DlgPrefControlsDlg", "%", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinBoxTempRateRight->setToolTip(QApplication::translate("DlgPrefControlsDlg", "Temporary rate change (between 1 and 8000) when right clicking", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        spinBoxTempRateRight->setSuffix(QApplication::translate("DlgPrefControlsDlg", "%", 0, QApplication::UnicodeUTF8));
        groupBoxRateRamp->setTitle(QApplication::translate("DlgPrefControlsDlg", "Ramping Pitchbend", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefControlsDlg", "Pitchbend sensitivity", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefControlsDlg: public Ui_DlgPrefControlsDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFCONTROLSDLG_H
