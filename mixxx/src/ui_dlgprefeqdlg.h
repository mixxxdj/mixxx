/********************************************************************************
** Form generated from reading UI file 'dlgprefeqdlg.ui'
**
** Created: Fri Oct 15 21:35:42 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFEQDLG_H
#define UI_DLGPREFEQDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefEQDlg
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QCheckBox *CheckBoxLoFi;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem;
    QPushButton *PushButtonReset;
    Q3GroupBox *GroupBoxHiEQ;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout2;
    QSpacerItem *spacerItem1;
    QLabel *TextHiEQ;
    QSlider *SliderHiEQ;
    QHBoxLayout *hboxLayout3;
    QLabel *TextLabel8_3;
    QSpacerItem *spacerItem2;
    QLabel *TextLabel8_2_3;
    QSpacerItem *spacerItem3;
    QLabel *TextLabel8_2_2;
    Q3GroupBox *GroupBoxLoEQ;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout4;
    QSpacerItem *spacerItem4;
    QLabel *TextLoEQ;
    QSlider *SliderLoEQ;
    QHBoxLayout *hboxLayout5;
    QLabel *TextLabel8_3_2;
    QSpacerItem *spacerItem5;
    QLabel *TextLabel8_2_4;
    QSpacerItem *spacerItem6;
    QLabel *TextLabel8_2_2_2;
    QSpacerItem *spacerItem7;

    void setupUi(QWidget *DlgPrefEQDlg)
    {
        if (DlgPrefEQDlg->objectName().isEmpty())
            DlgPrefEQDlg->setObjectName(QString::fromUtf8("DlgPrefEQDlg"));
        DlgPrefEQDlg->resize(568, 520);
        gridLayout = new QGridLayout(DlgPrefEQDlg);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        CheckBoxLoFi = new QCheckBox(DlgPrefEQDlg);
        CheckBoxLoFi->setObjectName(QString::fromUtf8("CheckBoxLoFi"));

        hboxLayout->addWidget(CheckBoxLoFi);


        gridLayout->addLayout(hboxLayout, 0, 0, 1, 1);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);

        PushButtonReset = new QPushButton(DlgPrefEQDlg);
        PushButtonReset->setObjectName(QString::fromUtf8("PushButtonReset"));

        hboxLayout1->addWidget(PushButtonReset);


        gridLayout->addLayout(hboxLayout1, 4, 0, 1, 1);

        GroupBoxHiEQ = new Q3GroupBox(DlgPrefEQDlg);
        GroupBoxHiEQ->setObjectName(QString::fromUtf8("GroupBoxHiEQ"));
        GroupBoxHiEQ->setEnabled(true);
        QFont font;
        GroupBoxHiEQ->setFont(font);
        GroupBoxHiEQ->setColumnLayout(0, Qt::Vertical);
        GroupBoxHiEQ->layout()->setSpacing(6);
        GroupBoxHiEQ->layout()->setContentsMargins(11, 11, 11, 11);
        vboxLayout = new QVBoxLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(GroupBoxHiEQ->layout());
        if (boxlayout)
            boxlayout->addLayout(vboxLayout);
        vboxLayout->setAlignment(Qt::AlignTop);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem1);

        TextHiEQ = new QLabel(GroupBoxHiEQ);
        TextHiEQ->setObjectName(QString::fromUtf8("TextHiEQ"));
        TextHiEQ->setWordWrap(false);

        hboxLayout2->addWidget(TextHiEQ);


        vboxLayout->addLayout(hboxLayout2);

        SliderHiEQ = new QSlider(GroupBoxHiEQ);
        SliderHiEQ->setObjectName(QString::fromUtf8("SliderHiEQ"));
        SliderHiEQ->setMinimum(80);
        SliderHiEQ->setMaximum(480);
        SliderHiEQ->setSingleStep(1);
        SliderHiEQ->setPageStep(1);
        SliderHiEQ->setValue(80);
        SliderHiEQ->setOrientation(Qt::Horizontal);
        SliderHiEQ->setTickInterval(5);

        vboxLayout->addWidget(SliderHiEQ);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        TextLabel8_3 = new QLabel(GroupBoxHiEQ);
        TextLabel8_3->setObjectName(QString::fromUtf8("TextLabel8_3"));
        TextLabel8_3->setFont(font);
        TextLabel8_3->setWordWrap(false);

        hboxLayout3->addWidget(TextLabel8_3);

        spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem2);

        TextLabel8_2_3 = new QLabel(GroupBoxHiEQ);
        TextLabel8_2_3->setObjectName(QString::fromUtf8("TextLabel8_2_3"));
        TextLabel8_2_3->setFont(font);
        TextLabel8_2_3->setWordWrap(false);

        hboxLayout3->addWidget(TextLabel8_2_3);

        spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem3);

        TextLabel8_2_2 = new QLabel(GroupBoxHiEQ);
        TextLabel8_2_2->setObjectName(QString::fromUtf8("TextLabel8_2_2"));
        TextLabel8_2_2->setFont(font);
        TextLabel8_2_2->setWordWrap(false);

        hboxLayout3->addWidget(TextLabel8_2_2);


        vboxLayout->addLayout(hboxLayout3);


        gridLayout->addWidget(GroupBoxHiEQ, 1, 0, 1, 1);

        GroupBoxLoEQ = new Q3GroupBox(DlgPrefEQDlg);
        GroupBoxLoEQ->setObjectName(QString::fromUtf8("GroupBoxLoEQ"));
        GroupBoxLoEQ->setEnabled(true);
        GroupBoxLoEQ->setFont(font);
        GroupBoxLoEQ->setColumnLayout(0, Qt::Vertical);
        GroupBoxLoEQ->layout()->setSpacing(6);
        GroupBoxLoEQ->layout()->setContentsMargins(11, 11, 11, 11);
        vboxLayout1 = new QVBoxLayout();
        QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(GroupBoxLoEQ->layout());
        if (boxlayout1)
            boxlayout1->addLayout(vboxLayout1);
        vboxLayout1->setAlignment(Qt::AlignTop);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        spacerItem4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout4->addItem(spacerItem4);

        TextLoEQ = new QLabel(GroupBoxLoEQ);
        TextLoEQ->setObjectName(QString::fromUtf8("TextLoEQ"));
        TextLoEQ->setWordWrap(false);

        hboxLayout4->addWidget(TextLoEQ);


        vboxLayout1->addLayout(hboxLayout4);

        SliderLoEQ = new QSlider(GroupBoxLoEQ);
        SliderLoEQ->setObjectName(QString::fromUtf8("SliderLoEQ"));
        SliderLoEQ->setMinimum(80);
        SliderLoEQ->setMaximum(480);
        SliderLoEQ->setSingleStep(1);
        SliderLoEQ->setPageStep(1);
        SliderLoEQ->setValue(80);
        SliderLoEQ->setOrientation(Qt::Horizontal);
        SliderLoEQ->setTickInterval(5);

        vboxLayout1->addWidget(SliderLoEQ);

        hboxLayout5 = new QHBoxLayout();
        hboxLayout5->setSpacing(6);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        TextLabel8_3_2 = new QLabel(GroupBoxLoEQ);
        TextLabel8_3_2->setObjectName(QString::fromUtf8("TextLabel8_3_2"));
        TextLabel8_3_2->setFont(font);
        TextLabel8_3_2->setWordWrap(false);

        hboxLayout5->addWidget(TextLabel8_3_2);

        spacerItem5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout5->addItem(spacerItem5);

        TextLabel8_2_4 = new QLabel(GroupBoxLoEQ);
        TextLabel8_2_4->setObjectName(QString::fromUtf8("TextLabel8_2_4"));
        TextLabel8_2_4->setFont(font);
        TextLabel8_2_4->setWordWrap(false);

        hboxLayout5->addWidget(TextLabel8_2_4);

        spacerItem6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout5->addItem(spacerItem6);

        TextLabel8_2_2_2 = new QLabel(GroupBoxLoEQ);
        TextLabel8_2_2_2->setObjectName(QString::fromUtf8("TextLabel8_2_2_2"));
        TextLabel8_2_2_2->setFont(font);
        TextLabel8_2_2_2->setWordWrap(false);

        hboxLayout5->addWidget(TextLabel8_2_2_2);


        vboxLayout1->addLayout(hboxLayout5);


        gridLayout->addWidget(GroupBoxLoEQ, 2, 0, 1, 1);

        spacerItem7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem7, 5, 0, 1, 1);


        retranslateUi(DlgPrefEQDlg);

        QMetaObject::connectSlotsByName(DlgPrefEQDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefEQDlg)
    {
        DlgPrefEQDlg->setWindowTitle(QApplication::translate("DlgPrefEQDlg", "Form1", 0, QApplication::UnicodeUTF8));
        CheckBoxLoFi->setText(QApplication::translate("DlgPrefEQDlg", "Static EQs (for slower CPUs)", 0, QApplication::UnicodeUTF8));
        PushButtonReset->setText(QApplication::translate("DlgPrefEQDlg", "Reset", 0, QApplication::UnicodeUTF8));
        GroupBoxHiEQ->setTitle(QApplication::translate("DlgPrefEQDlg", "High Shelf EQ", 0, QApplication::UnicodeUTF8));
        TextHiEQ->setText(QApplication::translate("DlgPrefEQDlg", "textLabel1", 0, QApplication::UnicodeUTF8));
        TextLabel8_3->setText(QApplication::translate("DlgPrefEQDlg", "16 Hz", 0, QApplication::UnicodeUTF8));
        TextLabel8_2_3->setText(QApplication::translate("DlgPrefEQDlg", "2.045 kHz", 0, QApplication::UnicodeUTF8));
        TextLabel8_2_2->setText(QApplication::translate("DlgPrefEQDlg", "20.05 kHz", 0, QApplication::UnicodeUTF8));
        GroupBoxLoEQ->setTitle(QApplication::translate("DlgPrefEQDlg", "Low Shelf EQ", 0, QApplication::UnicodeUTF8));
        TextLoEQ->setText(QApplication::translate("DlgPrefEQDlg", "textLabel2", 0, QApplication::UnicodeUTF8));
        TextLabel8_3_2->setText(QApplication::translate("DlgPrefEQDlg", "16 Hz", 0, QApplication::UnicodeUTF8));
        TextLabel8_2_4->setText(QApplication::translate("DlgPrefEQDlg", "2.045 kHz", 0, QApplication::UnicodeUTF8));
        TextLabel8_2_2_2->setText(QApplication::translate("DlgPrefEQDlg", "20.05 kHz", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefEQDlg: public Ui_DlgPrefEQDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFEQDLG_H
