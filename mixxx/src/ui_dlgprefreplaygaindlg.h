/********************************************************************************
** Form generated from reading UI file 'dlgprefreplaygaindlg.ui'
**
** Created: Mon Oct 18 15:20:48 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFREPLAYGAINDLG_H
#define UI_DLGPREFREPLAYGAINDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefReplayGainDlg
{
public:
    QVBoxLayout *vboxLayout;
    Q3GroupBox *GroupBox1;
    QVBoxLayout *vboxLayout1;
    QCheckBox *EnableGain;
    QFrame *line;
    QCheckBox *EnableAnalyser;
    QLabel *bigfatwarning;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout2;
    QHBoxLayout *hboxLayout1;
    QLabel *label_boost;
    QSpacerItem *spacerItem;
    QLabel *label;
    QSlider *SliderBoost;
    QHBoxLayout *hboxLayout2;
    QLabel *label0;
    QSpacerItem *spacerItem1;
    QLabel *label15;
    QSpacerItem *spacerItem2;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem3;
    QPushButton *PushButtonReset;

    void setupUi(QWidget *DlgPrefReplayGainDlg)
    {
        if (DlgPrefReplayGainDlg->objectName().isEmpty())
            DlgPrefReplayGainDlg->setObjectName(QString::fromUtf8("DlgPrefReplayGainDlg"));
        DlgPrefReplayGainDlg->resize(407, 418);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DlgPrefReplayGainDlg->sizePolicy().hasHeightForWidth());
        DlgPrefReplayGainDlg->setSizePolicy(sizePolicy);
        DlgPrefReplayGainDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        vboxLayout = new QVBoxLayout(DlgPrefReplayGainDlg);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        GroupBox1 = new Q3GroupBox(DlgPrefReplayGainDlg);
        GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
        GroupBox1->setEnabled(true);
        sizePolicy.setHeightForWidth(GroupBox1->sizePolicy().hasHeightForWidth());
        GroupBox1->setSizePolicy(sizePolicy);
        QFont font;
        GroupBox1->setFont(font);
        GroupBox1->setColumnLayout(0, Qt::Vertical);
        GroupBox1->layout()->setSpacing(6);
        GroupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        vboxLayout1 = new QVBoxLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(GroupBox1->layout());
        if (boxlayout)
            boxlayout->addLayout(vboxLayout1);
        vboxLayout1->setAlignment(Qt::AlignTop);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        EnableGain = new QCheckBox(GroupBox1);
        EnableGain->setObjectName(QString::fromUtf8("EnableGain"));

        vboxLayout1->addWidget(EnableGain);

        line = new QFrame(GroupBox1);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        vboxLayout1->addWidget(line);

        EnableAnalyser = new QCheckBox(GroupBox1);
        EnableAnalyser->setObjectName(QString::fromUtf8("EnableAnalyser"));

        vboxLayout1->addWidget(EnableAnalyser);

        bigfatwarning = new QLabel(GroupBox1);
        bigfatwarning->setObjectName(QString::fromUtf8("bigfatwarning"));
        QFont font1;
        font1.setBold(false);
        font1.setItalic(true);
        font1.setWeight(50);
        font1.setStrikeOut(false);
        bigfatwarning->setFont(font1);

        vboxLayout1->addWidget(bigfatwarning);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout1->addItem(verticalSpacer);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(6);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        label_boost = new QLabel(GroupBox1);
        label_boost->setObjectName(QString::fromUtf8("label_boost"));

        hboxLayout1->addWidget(label_boost);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);

        label = new QLabel(GroupBox1);
        label->setObjectName(QString::fromUtf8("label"));

        hboxLayout1->addWidget(label);


        vboxLayout2->addLayout(hboxLayout1);

        SliderBoost = new QSlider(GroupBox1);
        SliderBoost->setObjectName(QString::fromUtf8("SliderBoost"));
        SliderBoost->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        SliderBoost->setMinimum(0);
        SliderBoost->setMaximum(15);
        SliderBoost->setSingleStep(1);
        SliderBoost->setPageStep(1);
        SliderBoost->setValue(6);
        SliderBoost->setOrientation(Qt::Horizontal);
        SliderBoost->setTickInterval(5);

        vboxLayout2->addWidget(SliderBoost);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        label0 = new QLabel(GroupBox1);
        label0->setObjectName(QString::fromUtf8("label0"));
        label0->setFont(font);
        label0->setWordWrap(false);

        hboxLayout2->addWidget(label0);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem1);

        label15 = new QLabel(GroupBox1);
        label15->setObjectName(QString::fromUtf8("label15"));
        label15->setFont(font);
        label15->setWordWrap(false);

        hboxLayout2->addWidget(label15);


        vboxLayout2->addLayout(hboxLayout2);


        hboxLayout->addLayout(vboxLayout2);


        vboxLayout1->addLayout(hboxLayout);


        vboxLayout->addWidget(GroupBox1);

        spacerItem2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem2);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem3);

        PushButtonReset = new QPushButton(DlgPrefReplayGainDlg);
        PushButtonReset->setObjectName(QString::fromUtf8("PushButtonReset"));

        hboxLayout3->addWidget(PushButtonReset);


        vboxLayout->addLayout(hboxLayout3);


        retranslateUi(DlgPrefReplayGainDlg);

        QMetaObject::connectSlotsByName(DlgPrefReplayGainDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefReplayGainDlg)
    {
        DlgPrefReplayGainDlg->setWindowTitle(QApplication::translate("DlgPrefReplayGainDlg", "Form1", 0, QApplication::UnicodeUTF8));
        GroupBox1->setTitle(QApplication::translate("DlgPrefReplayGainDlg", "Replay Gain Normalization", 0, QApplication::UnicodeUTF8));
        EnableGain->setText(QApplication::translate("DlgPrefReplayGainDlg", "Enable Replay Gain", 0, QApplication::UnicodeUTF8));
        EnableAnalyser->setText(QApplication::translate("DlgPrefReplayGainDlg", "Enable Replay Gain Analyser", 0, QApplication::UnicodeUTF8));
        bigfatwarning->setText(QApplication::translate("DlgPrefReplayGainDlg", "Big Fat Warning: Tell something about\n"
"volume changing when ReplayGain is on.", 0, QApplication::UnicodeUTF8));
        label_boost->setText(QApplication::translate("DlgPrefReplayGainDlg", "Initial Boost", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
#ifndef QT_NO_TOOLTIP
        SliderBoost->setToolTip(QApplication::translate("DlgPrefReplayGainDlg", "Initial Gain Boost", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label0->setText(QApplication::translate("DlgPrefReplayGainDlg", "0  dB", 0, QApplication::UnicodeUTF8));
        label15->setText(QApplication::translate("DlgPrefReplayGainDlg", "+15 dB ", 0, QApplication::UnicodeUTF8));
        PushButtonReset->setText(QApplication::translate("DlgPrefReplayGainDlg", "Reset", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefReplayGainDlg: public Ui_DlgPrefReplayGainDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFREPLAYGAINDLG_H
