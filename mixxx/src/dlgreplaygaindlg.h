/********************************************************************************
** Form generated from reading UI file 'dlgreplaygaindlg.ui'
**
** Created: Sat Oct 16 18:48:21 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGREPLAYGAINDLG_H
#define DLGREPLAYGAINDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
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
#include <QtGui/QRadioButton>
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
    QRadioButton *AffectVolume;
    QRadioButton *AffectGain;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout2;
    QHBoxLayout *hboxLayout1;
    QLabel *label_2;
    QSpacerItem *spacerItem;
    QLabel *label;
    QSlider *SliderBoost;
    QHBoxLayout *hboxLayout2;
    QLabel *TextLabel8;
    QSpacerItem *spacerItem1;
    QLabel *TextLabel8_2;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem2;
    QPushButton *PushButtonReset;
    QSpacerItem *spacerItem3;

    void setupUi(QWidget *DlgPrefReplayGainDlg)
    {
        if (DlgPrefReplayGainDlg->objectName().isEmpty())
            DlgPrefReplayGainDlg->setObjectName(QString::fromUtf8("DlgPrefReplayGainDlg"));
        DlgPrefReplayGainDlg->resize(409, 418);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DlgPrefReplayGainDlg->sizePolicy().hasHeightForWidth());
        DlgPrefReplayGainDlg->setSizePolicy(sizePolicy);
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

        AffectVolume = new QRadioButton(GroupBox1);
        AffectVolume->setObjectName(QString::fromUtf8("AffectVolume"));

        vboxLayout1->addWidget(AffectVolume);

        AffectGain = new QRadioButton(GroupBox1);
        AffectGain->setObjectName(QString::fromUtf8("AffectGain"));

        vboxLayout1->addWidget(AffectGain);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(6);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        label_2 = new QLabel(GroupBox1);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        hboxLayout1->addWidget(label_2);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);

        label = new QLabel(GroupBox1);
        label->setObjectName(QString::fromUtf8("label"));

        hboxLayout1->addWidget(label);


        vboxLayout2->addLayout(hboxLayout1);

        SliderBoost = new QSlider(GroupBox1);
        SliderBoost->setObjectName(QString::fromUtf8("SliderBoost"));
        SliderBoost->setMinimum(0);
        SliderBoost->setMaximum(100);
        SliderBoost->setSingleStep(1);
        SliderBoost->setPageStep(1);
        SliderBoost->setValue(50);
        SliderBoost->setOrientation(Qt::Horizontal);
        SliderBoost->setTickInterval(5);

        vboxLayout2->addWidget(SliderBoost);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        TextLabel8 = new QLabel(GroupBox1);
        TextLabel8->setObjectName(QString::fromUtf8("TextLabel8"));
        TextLabel8->setFont(font);
        TextLabel8->setWordWrap(false);

        hboxLayout2->addWidget(TextLabel8);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem1);

        TextLabel8_2 = new QLabel(GroupBox1);
        TextLabel8_2->setObjectName(QString::fromUtf8("TextLabel8_2"));
        TextLabel8_2->setFont(font);
        TextLabel8_2->setWordWrap(false);

        hboxLayout2->addWidget(TextLabel8_2);


        vboxLayout2->addLayout(hboxLayout2);


        hboxLayout->addLayout(vboxLayout2);


        vboxLayout1->addLayout(hboxLayout);


        vboxLayout->addWidget(GroupBox1);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem2);

        PushButtonReset = new QPushButton(DlgPrefReplayGainDlg);
        PushButtonReset->setObjectName(QString::fromUtf8("PushButtonReset"));

        hboxLayout3->addWidget(PushButtonReset);


        vboxLayout->addLayout(hboxLayout3);

        spacerItem3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem3);


        retranslateUi(DlgPrefReplayGainDlg);

        QMetaObject::connectSlotsByName(DlgPrefReplayGainDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefReplayGainDlg)
    {
        DlgPrefReplayGainDlg->setWindowTitle(QApplication::translate("DlgPrefReplayGainDlg", "Form1", 0, QApplication::UnicodeUTF8));
        GroupBox1->setTitle(QApplication::translate("DlgPrefReplayGainDlg", "Replay Gain Normalization", 0, QApplication::UnicodeUTF8));
        EnableGain->setText(QApplication::translate("DlgPrefReplayGainDlg", "Enable Replay Gain", 0, QApplication::UnicodeUTF8));
        EnableAnalyser->setText(QApplication::translate("DlgPrefReplayGainDlg", "Enable Replay Gain Analyser", 0, QApplication::UnicodeUTF8));
        AffectVolume->setText(QApplication::translate("DlgPrefReplayGainDlg", "Affect Desk Volume", 0, QApplication::UnicodeUTF8));
        AffectGain->setText(QApplication::translate("DlgPrefReplayGainDlg", "Affect Desk Pregain", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefReplayGainDlg", "Initial Boost", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        TextLabel8->setText(QApplication::translate("DlgPrefReplayGainDlg", "0  Db", 0, QApplication::UnicodeUTF8));
        TextLabel8_2->setText(QApplication::translate("DlgPrefReplayGainDlg", "+15 Db ", 0, QApplication::UnicodeUTF8));
        PushButtonReset->setText(QApplication::translate("DlgPrefReplayGainDlg", "Reset", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefReplayGainDlg: public Ui_DlgPrefReplayGainDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGREPLAYGAINDLG_H
