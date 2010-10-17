/********************************************************************************
** Form generated from reading UI file 'dlgprefcrossfaderdlg.ui'
**
** Created: Fri Oct 15 21:35:41 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFCROSSFADERDLG_H
#define UI_DLGPREFCROSSFADERDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGraphicsView>
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

class Ui_DlgPrefCrossfaderDlg
{
public:
    QVBoxLayout *vboxLayout;
    Q3GroupBox *GroupBox1;
    QVBoxLayout *vboxLayout1;
    QRadioButton *radioButtonSlowFade;
    QRadioButton *radioButtonFastCut;
    QRadioButton *radioButtonConstantPower;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout2;
    QHBoxLayout *hboxLayout1;
    QLabel *label_2;
    QSpacerItem *spacerItem;
    QLabel *label;
    QSlider *SliderXFader;
    QHBoxLayout *hboxLayout2;
    QLabel *TextLabel8;
    QSpacerItem *spacerItem1;
    QLabel *TextLabel8_2;
    QGraphicsView *graphicsViewXfader;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem2;
    QPushButton *PushButtonReset;
    QSpacerItem *spacerItem3;

    void setupUi(QWidget *DlgPrefCrossfaderDlg)
    {
        if (DlgPrefCrossfaderDlg->objectName().isEmpty())
            DlgPrefCrossfaderDlg->setObjectName(QString::fromUtf8("DlgPrefCrossfaderDlg"));
        DlgPrefCrossfaderDlg->resize(409, 418);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DlgPrefCrossfaderDlg->sizePolicy().hasHeightForWidth());
        DlgPrefCrossfaderDlg->setSizePolicy(sizePolicy);
        vboxLayout = new QVBoxLayout(DlgPrefCrossfaderDlg);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        GroupBox1 = new Q3GroupBox(DlgPrefCrossfaderDlg);
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
        radioButtonSlowFade = new QRadioButton(GroupBox1);
        radioButtonSlowFade->setObjectName(QString::fromUtf8("radioButtonSlowFade"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(radioButtonSlowFade->sizePolicy().hasHeightForWidth());
        radioButtonSlowFade->setSizePolicy(sizePolicy1);

        vboxLayout1->addWidget(radioButtonSlowFade);

        radioButtonFastCut = new QRadioButton(GroupBox1);
        radioButtonFastCut->setObjectName(QString::fromUtf8("radioButtonFastCut"));

        vboxLayout1->addWidget(radioButtonFastCut);

        radioButtonConstantPower = new QRadioButton(GroupBox1);
        radioButtonConstantPower->setObjectName(QString::fromUtf8("radioButtonConstantPower"));

        vboxLayout1->addWidget(radioButtonConstantPower);

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

        SliderXFader = new QSlider(GroupBox1);
        SliderXFader->setObjectName(QString::fromUtf8("SliderXFader"));
        SliderXFader->setMinimum(0);
        SliderXFader->setMaximum(100);
        SliderXFader->setSingleStep(1);
        SliderXFader->setPageStep(1);
        SliderXFader->setValue(50);
        SliderXFader->setOrientation(Qt::Horizontal);
        SliderXFader->setTickInterval(5);

        vboxLayout2->addWidget(SliderXFader);

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

        graphicsViewXfader = new QGraphicsView(GroupBox1);
        graphicsViewXfader->setObjectName(QString::fromUtf8("graphicsViewXfader"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(graphicsViewXfader->sizePolicy().hasHeightForWidth());
        graphicsViewXfader->setSizePolicy(sizePolicy2);
        graphicsViewXfader->setMinimumSize(QSize(125, 70));
        graphicsViewXfader->setMaximumSize(QSize(125, 80));
        graphicsViewXfader->setBaseSize(QSize(300, 0));
        graphicsViewXfader->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        graphicsViewXfader->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        hboxLayout->addWidget(graphicsViewXfader);


        vboxLayout1->addLayout(hboxLayout);


        vboxLayout->addWidget(GroupBox1);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(spacerItem2);

        PushButtonReset = new QPushButton(DlgPrefCrossfaderDlg);
        PushButtonReset->setObjectName(QString::fromUtf8("PushButtonReset"));

        hboxLayout3->addWidget(PushButtonReset);


        vboxLayout->addLayout(hboxLayout3);

        spacerItem3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem3);


        retranslateUi(DlgPrefCrossfaderDlg);

        QMetaObject::connectSlotsByName(DlgPrefCrossfaderDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefCrossfaderDlg)
    {
        DlgPrefCrossfaderDlg->setWindowTitle(QApplication::translate("DlgPrefCrossfaderDlg", "Form1", 0, QApplication::UnicodeUTF8));
        GroupBox1->setTitle(QApplication::translate("DlgPrefCrossfaderDlg", "Crossfader Curve", 0, QApplication::UnicodeUTF8));
        radioButtonSlowFade->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Slow fade", 0, QApplication::UnicodeUTF8));
        radioButtonFastCut->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Fast cut", 0, QApplication::UnicodeUTF8));
        radioButtonConstantPower->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Constant power", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Mixing", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Scratching", 0, QApplication::UnicodeUTF8));
        TextLabel8->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Linear", 0, QApplication::UnicodeUTF8));
        TextLabel8_2->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Logarithmic", 0, QApplication::UnicodeUTF8));
        PushButtonReset->setText(QApplication::translate("DlgPrefCrossfaderDlg", "Reset", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefCrossfaderDlg: public Ui_DlgPrefCrossfaderDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFCROSSFADERDLG_H
