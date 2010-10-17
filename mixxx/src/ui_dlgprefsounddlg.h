/********************************************************************************
** Form generated from reading UI file 'dlgprefsounddlg.ui'
**
** Created: Fri Oct 15 21:35:44 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFSOUNDDLG_H
#define UI_DLGPREFSOUNDDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefSoundDlg
{
public:
    QVBoxLayout *vboxLayout;
    QGroupBox *audioGroupBox;
    QGridLayout *gridLayout;
    QLabel *TextLabel1;
    QComboBox *ComboBoxSoundcardMaster;
    QLabel *label;
    QComboBox *ComboBoxSoundcardHeadphones;
    QLabel *TextLabel2;
    QComboBox *ComboBoxSamplerates;
    QLabel *TextLabel3_2;
    QComboBox *ComboBoxSoundApi;
    QSpacerItem *spacerItem;
    QComboBox *ComboBoxChannelMaster;
    QComboBox *ComboBoxChannelHeadphones;
    QSpacerItem *spacerItem1;
    QGroupBox *stretchGroupBox;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout2;
    QRadioButton *radioButtonVinylEmu;
    QRadioButton *radioButtonPitchIndp;
    QVBoxLayout *vboxLayout3;
    QLabel *textLabelSoundScaling;
    QHBoxLayout *hboxLayout1;
    Q3GroupBox *GroupBox1;
    QVBoxLayout *vboxLayout4;
    QHBoxLayout *hboxLayout2;
    QSpacerItem *spacerItem2;
    QLabel *TextLabelLatency;
    QSlider *SliderLatency;
    QHBoxLayout *hboxLayout3;
    QLabel *TextLabel8;
    QSpacerItem *spacerItem3;
    QLabel *TextLabel8_2;
    QHBoxLayout *hboxLayout4;
    QLabel *TextLabel1_4_3_2;
    QSpacerItem *spacerItem4;
    QLabel *TextLabelLatencyMaster_3_2;
    QLabel *textLabelSoundScaling_2;
    QSpacerItem *spacerItem5;

    void setupUi(QWidget *DlgPrefSoundDlg)
    {
        if (DlgPrefSoundDlg->objectName().isEmpty())
            DlgPrefSoundDlg->setObjectName(QString::fromUtf8("DlgPrefSoundDlg"));
        DlgPrefSoundDlg->resize(520, 490);
        vboxLayout = new QVBoxLayout(DlgPrefSoundDlg);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        audioGroupBox = new QGroupBox(DlgPrefSoundDlg);
        audioGroupBox->setObjectName(QString::fromUtf8("audioGroupBox"));
        gridLayout = new QGridLayout(audioGroupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        TextLabel1 = new QLabel(audioGroupBox);
        TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));
        TextLabel1->setEnabled(true);
        TextLabel1->setMinimumSize(QSize(140, 0));
        TextLabel1->setMaximumSize(QSize(16777215, 16777215));
        QFont font;
        TextLabel1->setFont(font);
        TextLabel1->setWordWrap(false);

        gridLayout->addWidget(TextLabel1, 0, 0, 1, 1);

        ComboBoxSoundcardMaster = new QComboBox(audioGroupBox);
        ComboBoxSoundcardMaster->setObjectName(QString::fromUtf8("ComboBoxSoundcardMaster"));
        ComboBoxSoundcardMaster->setEnabled(true);
        ComboBoxSoundcardMaster->setMinimumSize(QSize(210, 0));
        ComboBoxSoundcardMaster->setFont(font);

        gridLayout->addWidget(ComboBoxSoundcardMaster, 0, 1, 1, 1);

        label = new QLabel(audioGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        ComboBoxSoundcardHeadphones = new QComboBox(audioGroupBox);
        ComboBoxSoundcardHeadphones->setObjectName(QString::fromUtf8("ComboBoxSoundcardHeadphones"));

        gridLayout->addWidget(ComboBoxSoundcardHeadphones, 1, 1, 1, 1);

        TextLabel2 = new QLabel(audioGroupBox);
        TextLabel2->setObjectName(QString::fromUtf8("TextLabel2"));
        TextLabel2->setEnabled(true);
        TextLabel2->setMinimumSize(QSize(140, 0));
        TextLabel2->setMaximumSize(QSize(16777215, 16777215));
        TextLabel2->setFont(font);
        TextLabel2->setWordWrap(false);

        gridLayout->addWidget(TextLabel2, 2, 0, 1, 1);

        ComboBoxSamplerates = new QComboBox(audioGroupBox);
        ComboBoxSamplerates->setObjectName(QString::fromUtf8("ComboBoxSamplerates"));
        ComboBoxSamplerates->setEnabled(true);
        ComboBoxSamplerates->setMinimumSize(QSize(120, 0));
        ComboBoxSamplerates->setFont(font);

        gridLayout->addWidget(ComboBoxSamplerates, 2, 1, 1, 1);

        TextLabel3_2 = new QLabel(audioGroupBox);
        TextLabel3_2->setObjectName(QString::fromUtf8("TextLabel3_2"));
        TextLabel3_2->setEnabled(true);
        TextLabel3_2->setMinimumSize(QSize(140, 0));
        TextLabel3_2->setMaximumSize(QSize(16777215, 16777215));
        TextLabel3_2->setFont(font);
        TextLabel3_2->setWordWrap(false);

        gridLayout->addWidget(TextLabel3_2, 3, 0, 1, 1);

        ComboBoxSoundApi = new QComboBox(audioGroupBox);
        ComboBoxSoundApi->setObjectName(QString::fromUtf8("ComboBoxSoundApi"));
        ComboBoxSoundApi->setEnabled(true);
        ComboBoxSoundApi->setMinimumSize(QSize(120, 0));
        ComboBoxSoundApi->setFont(font);

        gridLayout->addWidget(ComboBoxSoundApi, 3, 1, 1, 1);

        spacerItem = new QSpacerItem(81, 26, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 3, 2, 1, 1);

        ComboBoxChannelMaster = new QComboBox(audioGroupBox);
        ComboBoxChannelMaster->setObjectName(QString::fromUtf8("ComboBoxChannelMaster"));

        gridLayout->addWidget(ComboBoxChannelMaster, 0, 2, 1, 1);

        ComboBoxChannelHeadphones = new QComboBox(audioGroupBox);
        ComboBoxChannelHeadphones->setObjectName(QString::fromUtf8("ComboBoxChannelHeadphones"));

        gridLayout->addWidget(ComboBoxChannelHeadphones, 1, 2, 1, 1);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem1, 2, 2, 1, 1);


        vboxLayout->addWidget(audioGroupBox);

        stretchGroupBox = new QGroupBox(DlgPrefSoundDlg);
        stretchGroupBox->setObjectName(QString::fromUtf8("stretchGroupBox"));
        vboxLayout1 = new QVBoxLayout(stretchGroupBox);
        vboxLayout1->setSpacing(6);
        vboxLayout1->setContentsMargins(11, 11, 11, 11);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(6);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        radioButtonVinylEmu = new QRadioButton(stretchGroupBox);
        radioButtonVinylEmu->setObjectName(QString::fromUtf8("radioButtonVinylEmu"));

        vboxLayout2->addWidget(radioButtonVinylEmu);

        radioButtonPitchIndp = new QRadioButton(stretchGroupBox);
        radioButtonPitchIndp->setObjectName(QString::fromUtf8("radioButtonPitchIndp"));

        vboxLayout2->addWidget(radioButtonPitchIndp);


        hboxLayout->addLayout(vboxLayout2);

        vboxLayout3 = new QVBoxLayout();
        vboxLayout3->setSpacing(6);
        vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
        textLabelSoundScaling = new QLabel(stretchGroupBox);
        textLabelSoundScaling->setObjectName(QString::fromUtf8("textLabelSoundScaling"));
        textLabelSoundScaling->setAlignment(Qt::AlignTop);
        textLabelSoundScaling->setWordWrap(true);

        vboxLayout3->addWidget(textLabelSoundScaling);


        hboxLayout->addLayout(vboxLayout3);


        vboxLayout1->addLayout(hboxLayout);


        vboxLayout->addWidget(stretchGroupBox);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        GroupBox1 = new Q3GroupBox(DlgPrefSoundDlg);
        GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
        GroupBox1->setEnabled(true);
        GroupBox1->setFont(font);
        GroupBox1->setColumnLayout(0, Qt::Vertical);
        GroupBox1->layout()->setSpacing(6);
        GroupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        vboxLayout4 = new QVBoxLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(GroupBox1->layout());
        if (boxlayout)
            boxlayout->addLayout(vboxLayout4);
        vboxLayout4->setAlignment(Qt::AlignTop);
        vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem2);

        TextLabelLatency = new QLabel(GroupBox1);
        TextLabelLatency->setObjectName(QString::fromUtf8("TextLabelLatency"));
        TextLabelLatency->setFont(font);
        TextLabelLatency->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabelLatency->setWordWrap(false);

        hboxLayout2->addWidget(TextLabelLatency);


        vboxLayout4->addLayout(hboxLayout2);

        SliderLatency = new QSlider(GroupBox1);
        SliderLatency->setObjectName(QString::fromUtf8("SliderLatency"));
        SliderLatency->setMinimum(6);
        SliderLatency->setMaximum(13);
        SliderLatency->setSingleStep(1);
        SliderLatency->setPageStep(1);
        SliderLatency->setValue(13);
        SliderLatency->setOrientation(Qt::Horizontal);
        SliderLatency->setTickInterval(5);

        vboxLayout4->addWidget(SliderLatency);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        TextLabel8 = new QLabel(GroupBox1);
        TextLabel8->setObjectName(QString::fromUtf8("TextLabel8"));
        TextLabel8->setFont(font);
        TextLabel8->setWordWrap(false);

        hboxLayout3->addWidget(TextLabel8);

        spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem3);

        TextLabel8_2 = new QLabel(GroupBox1);
        TextLabel8_2->setObjectName(QString::fromUtf8("TextLabel8_2"));
        TextLabel8_2->setFont(font);
        TextLabel8_2->setWordWrap(false);

        hboxLayout3->addWidget(TextLabel8_2);


        vboxLayout4->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        TextLabel1_4_3_2 = new QLabel(GroupBox1);
        TextLabel1_4_3_2->setObjectName(QString::fromUtf8("TextLabel1_4_3_2"));
        TextLabel1_4_3_2->setFont(font);
        TextLabel1_4_3_2->setWordWrap(false);

        hboxLayout4->addWidget(TextLabel1_4_3_2);

        spacerItem4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout4->addItem(spacerItem4);

        TextLabelLatencyMaster_3_2 = new QLabel(GroupBox1);
        TextLabelLatencyMaster_3_2->setObjectName(QString::fromUtf8("TextLabelLatencyMaster_3_2"));
        TextLabelLatencyMaster_3_2->setFont(font);
        TextLabelLatencyMaster_3_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabelLatencyMaster_3_2->setWordWrap(false);

        hboxLayout4->addWidget(TextLabelLatencyMaster_3_2);


        vboxLayout4->addLayout(hboxLayout4);


        hboxLayout1->addWidget(GroupBox1);

        textLabelSoundScaling_2 = new QLabel(DlgPrefSoundDlg);
        textLabelSoundScaling_2->setObjectName(QString::fromUtf8("textLabelSoundScaling_2"));
        textLabelSoundScaling_2->setAlignment(Qt::AlignTop);
        textLabelSoundScaling_2->setWordWrap(true);

        hboxLayout1->addWidget(textLabelSoundScaling_2);


        vboxLayout->addLayout(hboxLayout1);

        spacerItem5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem5);

        QWidget::setTabOrder(ComboBoxSoundcardMaster, ComboBoxChannelMaster);
        QWidget::setTabOrder(ComboBoxChannelMaster, ComboBoxSoundcardHeadphones);
        QWidget::setTabOrder(ComboBoxSoundcardHeadphones, ComboBoxChannelHeadphones);
        QWidget::setTabOrder(ComboBoxChannelHeadphones, ComboBoxSamplerates);
        QWidget::setTabOrder(ComboBoxSamplerates, ComboBoxSoundApi);
        QWidget::setTabOrder(ComboBoxSoundApi, radioButtonVinylEmu);
        QWidget::setTabOrder(radioButtonVinylEmu, radioButtonPitchIndp);
        QWidget::setTabOrder(radioButtonPitchIndp, SliderLatency);

        retranslateUi(DlgPrefSoundDlg);

        QMetaObject::connectSlotsByName(DlgPrefSoundDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefSoundDlg)
    {
        DlgPrefSoundDlg->setWindowTitle(QApplication::translate("DlgPrefSoundDlg", "Form1", 0, QApplication::UnicodeUTF8));
        audioGroupBox->setTitle(QApplication::translate("DlgPrefSoundDlg", "Audio Output", 0, QApplication::UnicodeUTF8));
        TextLabel1->setText(QApplication::translate("DlgPrefSoundDlg", "Master", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgPrefSoundDlg", "Headphones", 0, QApplication::UnicodeUTF8));
        TextLabel2->setText(QApplication::translate("DlgPrefSoundDlg", "Sample rate (Hz)", 0, QApplication::UnicodeUTF8));
        TextLabel3_2->setText(QApplication::translate("DlgPrefSoundDlg", "Sound API", 0, QApplication::UnicodeUTF8));
        stretchGroupBox->setTitle(QApplication::translate("DlgPrefSoundDlg", "Pitch behaviour", 0, QApplication::UnicodeUTF8));
        radioButtonVinylEmu->setText(QApplication::translate("DlgPrefSoundDlg", "Vinyl emulation", 0, QApplication::UnicodeUTF8));
        radioButtonPitchIndp->setText(QApplication::translate("DlgPrefSoundDlg", "Pitch independent time stretch", 0, QApplication::UnicodeUTF8));
        textLabelSoundScaling->setText(QApplication::translate("DlgPrefSoundDlg", "Stretching beyond +-10% with pitch independent time stretch is not advised, and is likely to result in bad sound quality.", 0, QApplication::UnicodeUTF8));
        GroupBox1->setTitle(QApplication::translate("DlgPrefSoundDlg", "Latency", 0, QApplication::UnicodeUTF8));
        TextLabelLatency->setText(QApplication::translate("DlgPrefSoundDlg", "256 ms", 0, QApplication::UnicodeUTF8));
        TextLabel8->setText(QApplication::translate("DlgPrefSoundDlg", "Low", 0, QApplication::UnicodeUTF8));
        TextLabel8_2->setText(QApplication::translate("DlgPrefSoundDlg", "High", 0, QApplication::UnicodeUTF8));
        TextLabel1_4_3_2->setText(QApplication::translate("DlgPrefSoundDlg", "Fast CPU", 0, QApplication::UnicodeUTF8));
        TextLabelLatencyMaster_3_2->setText(QApplication::translate("DlgPrefSoundDlg", "Slow CPU", 0, QApplication::UnicodeUTF8));
        textLabelSoundScaling_2->setText(QApplication::translate("DlgPrefSoundDlg", "Tips:<ul><li>Increase your latency if you hear pops during playback</li><li>Reduce your latency to improve Mixxx's responsiveness</li></ul>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefSoundDlg: public Ui_DlgPrefSoundDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFSOUNDDLG_H
