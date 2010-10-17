/********************************************************************************
** Form generated from reading UI file 'dlgmidilearning.ui'
**
** Created: Fri Oct 15 21:35:31 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGMIDILEARNING_H
#define UI_DLGMIDILEARNING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgMidiLearning
{
public:
    QGridLayout *gridLayout;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonBegin;
    QSpacerItem *horizontalSpacer_2;
    QWidget *page_2;
    QGridLayout *gridLayout_3;
    QLabel *label;
    QLabel *labelMixxxControl;
    QLabel *labelMappedTo;
    QPushButton *pushButtonPrevious;
    QPushButton *pushButtonSkip;
    QLabel *label_6;
    QSpacerItem *verticalSpacer;
    QWidget *page_3;
    QGridLayout *gridLayout_4;
    QLabel *label_4;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButtonFinish;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QDialog *DlgMidiLearning)
    {
        if (DlgMidiLearning->objectName().isEmpty())
            DlgMidiLearning->setObjectName(QString::fromUtf8("DlgMidiLearning"));
        DlgMidiLearning->resize(366, 271);
        gridLayout = new QGridLayout(DlgMidiLearning);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        stackedWidget = new QStackedWidget(DlgMidiLearning);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        gridLayout_2 = new QGridLayout(page);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_2 = new QLabel(page);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_2, 0, 0, 1, 3);

        label_3 = new QLabel(page);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        gridLayout_2->addWidget(label_3, 1, 0, 1, 3);

        horizontalSpacer = new QSpacerItem(116, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 2, 0, 1, 1);

        pushButtonBegin = new QPushButton(page);
        pushButtonBegin->setObjectName(QString::fromUtf8("pushButtonBegin"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButtonBegin->sizePolicy().hasHeightForWidth());
        pushButtonBegin->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(pushButtonBegin, 2, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(116, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 2, 2, 1, 1);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        gridLayout_3 = new QGridLayout(page_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label = new QLabel(page_2);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_3->addWidget(label, 0, 0, 1, 2);

        labelMixxxControl = new QLabel(page_2);
        labelMixxxControl->setObjectName(QString::fromUtf8("labelMixxxControl"));
        labelMixxxControl->setFont(font);
        labelMixxxControl->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(labelMixxxControl, 1, 0, 1, 2);

        labelMappedTo = new QLabel(page_2);
        labelMappedTo->setObjectName(QString::fromUtf8("labelMappedTo"));
        labelMappedTo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_3->addWidget(labelMappedTo, 2, 0, 1, 2);

        pushButtonPrevious = new QPushButton(page_2);
        pushButtonPrevious->setObjectName(QString::fromUtf8("pushButtonPrevious"));
        pushButtonPrevious->setEnabled(false);

        gridLayout_3->addWidget(pushButtonPrevious, 4, 0, 1, 1);

        pushButtonSkip = new QPushButton(page_2);
        pushButtonSkip->setObjectName(QString::fromUtf8("pushButtonSkip"));

        gridLayout_3->addWidget(pushButtonSkip, 4, 1, 1, 1);

        label_6 = new QLabel(page_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_6, 5, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer, 3, 0, 1, 1);

        stackedWidget->addWidget(page_2);
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        gridLayout_4 = new QGridLayout(page_3);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label_4 = new QLabel(page_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_4, 0, 0, 1, 3);

        label_8 = new QLabel(page_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setWordWrap(true);

        gridLayout_4->addWidget(label_8, 1, 0, 1, 3);

        horizontalSpacer_3 = new QSpacerItem(109, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_3, 2, 0, 1, 1);

        pushButtonFinish = new QPushButton(page_3);
        pushButtonFinish->setObjectName(QString::fromUtf8("pushButtonFinish"));

        gridLayout_4->addWidget(pushButtonFinish, 2, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(109, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_4, 2, 2, 1, 1);

        stackedWidget->addWidget(page_3);

        gridLayout->addWidget(stackedWidget, 0, 0, 1, 1);


        retranslateUi(DlgMidiLearning);

        stackedWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(DlgMidiLearning);
    } // setupUi

    void retranslateUi(QDialog *DlgMidiLearning)
    {
        DlgMidiLearning->setWindowTitle(QApplication::translate("DlgMidiLearning", "MIDI Learning Wizard", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgMidiLearning", "Welcome to the MIDI Learning Wizard", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DlgMidiLearning", "This wizard allows you to easily map the controls on your MIDI controller to Mixxx's controls.", 0, QApplication::UnicodeUTF8));
        pushButtonBegin->setText(QApplication::translate("DlgMidiLearning", "Begin", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgMidiLearning", "Please tweak the control for:", 0, QApplication::UnicodeUTF8));
        labelMixxxControl->setText(QApplication::translate("DlgMidiLearning", "Mixxx Control", 0, QApplication::UnicodeUTF8));
        labelMappedTo->setText(QApplication::translate("DlgMidiLearning", "Successfully mapped to: ", 0, QApplication::UnicodeUTF8));
        pushButtonPrevious->setText(QApplication::translate("DlgMidiLearning", "Previous", 0, QApplication::UnicodeUTF8));
        pushButtonSkip->setText(QApplication::translate("DlgMidiLearning", "Skip", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("DlgMidiLearning", "Press spacebar to proceed or skip.", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("DlgMidiLearning", "MIDI learning complete!", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("DlgMidiLearning", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Lucida Grande'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">This wizard can be re-run at any time.</p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br />Now go spin some beats!</p></body></html>", 0, QApplication::UnicodeUTF8));
        pushButtonFinish->setText(QApplication::translate("DlgMidiLearning", "Finito!", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgMidiLearning: public Ui_DlgMidiLearning {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGMIDILEARNING_H
