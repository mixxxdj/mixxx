/********************************************************************************
** Form generated from reading UI file 'dlgprefvinyldlg.ui'
**
** Created: Fri Oct 15 21:35:44 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFVINYLDLG_H
#define UI_DLGPREFVINYLDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefVinylDlg
{
public:
    QGridLayout *gridLayout;
    Q3GroupBox *Input;
    QGridLayout *gridLayout1;
    QLabel *TextLabel1_2;
    QComboBox *ComboBoxDeviceDeck1;
    QComboBox *ComboBoxChannelDeck1;
    QLabel *TextLabel1_2_3;
    QComboBox *ComboBoxDeviceDeck2;
    QComboBox *ComboBoxChannelDeck2;
    QLabel *label_2;
    QLabel *textLabel1_3;
    QSlider *VinylGain;
    QLabel *textLabel2;
    QLabel *textLabel3;
    Q3GroupBox *VinylOptions;
    QGridLayout *gridLayout2;
    QLabel *TextLabel2;
    QSpacerItem *spacerItem;
    QComboBox *ComboBoxVinylType;
    QLabel *TextLabel2_2;
    QSpacerItem *spacerItem1;
    QLineEdit *LeadinTime;
    QLabel *TextLabel2_2_2;
    QGroupBox *groupBox;
    QVBoxLayout *vboxLayout;
    QRadioButton *AbsoluteMode;
    QRadioButton *RelativeMode;
    QRadioButton *ScratchMode;
    QCheckBox *NeedleSkipEnable;
    QGroupBox *groupBoxSignalQuality;
    QGridLayout *gridLayout3;
    QLabel *label;

    void setupUi(QWidget *DlgPrefVinylDlg)
    {
        if (DlgPrefVinylDlg->objectName().isEmpty())
            DlgPrefVinylDlg->setObjectName(QString::fromUtf8("DlgPrefVinylDlg"));
        DlgPrefVinylDlg->resize(442, 468);
        gridLayout = new QGridLayout(DlgPrefVinylDlg);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        Input = new Q3GroupBox(DlgPrefVinylDlg);
        Input->setObjectName(QString::fromUtf8("Input"));
        Input->setColumnLayout(0, Qt::Vertical);
        Input->layout()->setSpacing(6);
        Input->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout1 = new QGridLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(Input->layout());
        if (boxlayout)
            boxlayout->addLayout(gridLayout1);
        gridLayout1->setAlignment(Qt::AlignTop);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        TextLabel1_2 = new QLabel(Input);
        TextLabel1_2->setObjectName(QString::fromUtf8("TextLabel1_2"));
        TextLabel1_2->setEnabled(true);
        QFont font;
        TextLabel1_2->setFont(font);
        TextLabel1_2->setFrameShape(QFrame::NoFrame);
        TextLabel1_2->setFrameShadow(QFrame::Plain);
        TextLabel1_2->setWordWrap(false);

        gridLayout1->addWidget(TextLabel1_2, 0, 0, 1, 1);

        ComboBoxDeviceDeck1 = new QComboBox(Input);
        ComboBoxDeviceDeck1->setObjectName(QString::fromUtf8("ComboBoxDeviceDeck1"));
        ComboBoxDeviceDeck1->setEnabled(true);
        ComboBoxDeviceDeck1->setFont(font);

        gridLayout1->addWidget(ComboBoxDeviceDeck1, 0, 1, 1, 1);

        ComboBoxChannelDeck1 = new QComboBox(Input);
        ComboBoxChannelDeck1->setObjectName(QString::fromUtf8("ComboBoxChannelDeck1"));

        gridLayout1->addWidget(ComboBoxChannelDeck1, 0, 2, 1, 1);

        TextLabel1_2_3 = new QLabel(Input);
        TextLabel1_2_3->setObjectName(QString::fromUtf8("TextLabel1_2_3"));
        TextLabel1_2_3->setEnabled(true);
        TextLabel1_2_3->setFont(font);
        TextLabel1_2_3->setFrameShape(QFrame::NoFrame);
        TextLabel1_2_3->setFrameShadow(QFrame::Plain);
        TextLabel1_2_3->setWordWrap(false);

        gridLayout1->addWidget(TextLabel1_2_3, 1, 0, 1, 1);

        ComboBoxDeviceDeck2 = new QComboBox(Input);
        ComboBoxDeviceDeck2->setObjectName(QString::fromUtf8("ComboBoxDeviceDeck2"));
        ComboBoxDeviceDeck2->setEnabled(true);
        ComboBoxDeviceDeck2->setFont(font);

        gridLayout1->addWidget(ComboBoxDeviceDeck2, 1, 1, 1, 1);

        ComboBoxChannelDeck2 = new QComboBox(Input);
        ComboBoxChannelDeck2->setObjectName(QString::fromUtf8("ComboBoxChannelDeck2"));

        gridLayout1->addWidget(ComboBoxChannelDeck2, 1, 2, 1, 1);

        label_2 = new QLabel(Input);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setOpenExternalLinks(true);

        gridLayout1->addWidget(label_2, 2, 0, 1, 3);

        textLabel1_3 = new QLabel(Input);
        textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
        textLabel1_3->setWordWrap(false);

        gridLayout1->addWidget(textLabel1_3, 3, 0, 1, 2);

        VinylGain = new QSlider(Input);
        VinylGain->setObjectName(QString::fromUtf8("VinylGain"));
        VinylGain->setMinimum(1);
        VinylGain->setMaximum(150);
        VinylGain->setOrientation(Qt::Horizontal);

        gridLayout1->addWidget(VinylGain, 4, 0, 1, 3);

        textLabel2 = new QLabel(Input);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        textLabel2->setWordWrap(false);

        gridLayout1->addWidget(textLabel2, 5, 0, 1, 2);

        textLabel3 = new QLabel(Input);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textLabel3->sizePolicy().hasHeightForWidth());
        textLabel3->setSizePolicy(sizePolicy);
        textLabel3->setWordWrap(false);

        gridLayout1->addWidget(textLabel3, 5, 2, 1, 1);


        gridLayout->addWidget(Input, 0, 0, 1, 2);

        VinylOptions = new Q3GroupBox(DlgPrefVinylDlg);
        VinylOptions->setObjectName(QString::fromUtf8("VinylOptions"));
        VinylOptions->setColumnLayout(0, Qt::Vertical);
        VinylOptions->layout()->setSpacing(6);
        VinylOptions->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout2 = new QGridLayout();
        QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(VinylOptions->layout());
        if (boxlayout1)
            boxlayout1->addLayout(gridLayout2);
        gridLayout2->setAlignment(Qt::AlignTop);
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        TextLabel2 = new QLabel(VinylOptions);
        TextLabel2->setObjectName(QString::fromUtf8("TextLabel2"));
        TextLabel2->setEnabled(true);
        sizePolicy.setHeightForWidth(TextLabel2->sizePolicy().hasHeightForWidth());
        TextLabel2->setSizePolicy(sizePolicy);
        TextLabel2->setFont(font);
        TextLabel2->setWordWrap(false);

        gridLayout2->addWidget(TextLabel2, 0, 0, 1, 1);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout2->addItem(spacerItem, 0, 1, 1, 1);

        ComboBoxVinylType = new QComboBox(VinylOptions);
        ComboBoxVinylType->setObjectName(QString::fromUtf8("ComboBoxVinylType"));
        ComboBoxVinylType->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ComboBoxVinylType->sizePolicy().hasHeightForWidth());
        ComboBoxVinylType->setSizePolicy(sizePolicy1);
        ComboBoxVinylType->setFont(font);

        gridLayout2->addWidget(ComboBoxVinylType, 0, 2, 1, 2);

        TextLabel2_2 = new QLabel(VinylOptions);
        TextLabel2_2->setObjectName(QString::fromUtf8("TextLabel2_2"));
        TextLabel2_2->setEnabled(true);
        sizePolicy.setHeightForWidth(TextLabel2_2->sizePolicy().hasHeightForWidth());
        TextLabel2_2->setSizePolicy(sizePolicy);
        TextLabel2_2->setFont(font);
        TextLabel2_2->setWordWrap(false);

        gridLayout2->addWidget(TextLabel2_2, 1, 0, 1, 1);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout2->addItem(spacerItem1, 1, 1, 1, 1);

        LeadinTime = new QLineEdit(VinylOptions);
        LeadinTime->setObjectName(QString::fromUtf8("LeadinTime"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(LeadinTime->sizePolicy().hasHeightForWidth());
        LeadinTime->setSizePolicy(sizePolicy2);

        gridLayout2->addWidget(LeadinTime, 1, 2, 1, 1);

        TextLabel2_2_2 = new QLabel(VinylOptions);
        TextLabel2_2_2->setObjectName(QString::fromUtf8("TextLabel2_2_2"));
        TextLabel2_2_2->setEnabled(true);
        TextLabel2_2_2->setFont(font);
        TextLabel2_2_2->setWordWrap(false);

        gridLayout2->addWidget(TextLabel2_2_2, 1, 3, 1, 1);


        gridLayout->addWidget(VinylOptions, 1, 0, 1, 2);

        groupBox = new QGroupBox(DlgPrefVinylDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy3);
        vboxLayout = new QVBoxLayout(groupBox);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        AbsoluteMode = new QRadioButton(groupBox);
        AbsoluteMode->setObjectName(QString::fromUtf8("AbsoluteMode"));

        vboxLayout->addWidget(AbsoluteMode);

        RelativeMode = new QRadioButton(groupBox);
        RelativeMode->setObjectName(QString::fromUtf8("RelativeMode"));

        vboxLayout->addWidget(RelativeMode);

        ScratchMode = new QRadioButton(groupBox);
        ScratchMode->setObjectName(QString::fromUtf8("ScratchMode"));

        vboxLayout->addWidget(ScratchMode);

        NeedleSkipEnable = new QCheckBox(groupBox);
        NeedleSkipEnable->setObjectName(QString::fromUtf8("NeedleSkipEnable"));

        vboxLayout->addWidget(NeedleSkipEnable);


        gridLayout->addWidget(groupBox, 2, 0, 2, 1);

        groupBoxSignalQuality = new QGroupBox(DlgPrefVinylDlg);
        groupBoxSignalQuality->setObjectName(QString::fromUtf8("groupBoxSignalQuality"));
        gridLayout3 = new QGridLayout(groupBoxSignalQuality);
        gridLayout3->setSpacing(6);
        gridLayout3->setContentsMargins(11, 11, 11, 11);
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));

        gridLayout->addWidget(groupBoxSignalQuality, 2, 1, 1, 1);

        label = new QLabel(DlgPrefVinylDlg);
        label->setObjectName(QString::fromUtf8("label"));
        label->setEnabled(false);
        label->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        gridLayout->addWidget(label, 3, 1, 1, 1);


        retranslateUi(DlgPrefVinylDlg);

        QMetaObject::connectSlotsByName(DlgPrefVinylDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefVinylDlg)
    {
        DlgPrefVinylDlg->setWindowTitle(QApplication::translate("DlgPrefVinylDlg", "Form1", 0, QApplication::UnicodeUTF8));
        Input->setTitle(QApplication::translate("DlgPrefVinylDlg", "Input", 0, QApplication::UnicodeUTF8));
        TextLabel1_2->setText(QApplication::translate("DlgPrefVinylDlg", "Deck 1", 0, QApplication::UnicodeUTF8));
        TextLabel1_2_3->setText(QApplication::translate("DlgPrefVinylDlg", "Deck 2", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefVinylDlg", "<a href=\"http://www.mixxx.org/wiki/doku.php/vinyl_control#troubleshooting\">Troubleshooting</a>", 0, QApplication::UnicodeUTF8));
        textLabel1_3->setText(QApplication::translate("DlgPrefVinylDlg", "Turntable Input Preamp", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("DlgPrefVinylDlg", "1 (Off)", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("DlgPrefVinylDlg", "150", 0, QApplication::UnicodeUTF8));
        VinylOptions->setTitle(QApplication::translate("DlgPrefVinylDlg", "Vinyl Configuration", 0, QApplication::UnicodeUTF8));
        TextLabel2->setText(QApplication::translate("DlgPrefVinylDlg", "Vinyl Type", 0, QApplication::UnicodeUTF8));
        TextLabel2_2->setText(QApplication::translate("DlgPrefVinylDlg", "Lead-in time", 0, QApplication::UnicodeUTF8));
        TextLabel2_2_2->setText(QApplication::translate("DlgPrefVinylDlg", "seconds", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DlgPrefVinylDlg", "Control Mode", 0, QApplication::UnicodeUTF8));
        AbsoluteMode->setText(QApplication::translate("DlgPrefVinylDlg", "Absolute Mode", 0, QApplication::UnicodeUTF8));
        RelativeMode->setText(QApplication::translate("DlgPrefVinylDlg", "Relative Mode", 0, QApplication::UnicodeUTF8));
        ScratchMode->setText(QApplication::translate("DlgPrefVinylDlg", "Scratch Mode", 0, QApplication::UnicodeUTF8));
        NeedleSkipEnable->setText(QApplication::translate("DlgPrefVinylDlg", "Enable Needle Skip Prevention", 0, QApplication::UnicodeUTF8));
        groupBoxSignalQuality->setTitle(QApplication::translate("DlgPrefVinylDlg", "Signal Quality", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label->setToolTip(QApplication::translate("DlgPrefVinylDlg", "http://www.xwax.co.uk", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("DlgPrefVinylDlg", "Powered by xwax", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefVinylDlg: public Ui_DlgPrefVinylDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFVINYLDLG_H
