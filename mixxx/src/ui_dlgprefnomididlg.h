/********************************************************************************
** Form generated from reading UI file 'dlgprefnomididlg.ui'
**
** Created: Fri Oct 15 21:35:43 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFNOMIDIDLG_H
#define UI_DLGPREFNOMIDIDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefNoMidiDlg
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *TextLabel3_2;

    void setupUi(QWidget *DlgPrefNoMidiDlg)
    {
        if (DlgPrefNoMidiDlg->objectName().isEmpty())
            DlgPrefNoMidiDlg->setObjectName(QString::fromUtf8("DlgPrefNoMidiDlg"));
        DlgPrefNoMidiDlg->resize(462, 87);
        vboxLayout = new QVBoxLayout(DlgPrefNoMidiDlg);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setAlignment(Qt::AlignTop);
        TextLabel3_2 = new QLabel(DlgPrefNoMidiDlg);
        TextLabel3_2->setObjectName(QString::fromUtf8("TextLabel3_2"));
        TextLabel3_2->setEnabled(true);
        QFont font;
        TextLabel3_2->setFont(font);
        TextLabel3_2->setMinimumSize(QSize(100, 10));
        TextLabel3_2->setWordWrap(false);

        vboxLayout->addWidget(TextLabel3_2);


        retranslateUi(DlgPrefNoMidiDlg);

        QMetaObject::connectSlotsByName(DlgPrefNoMidiDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefNoMidiDlg)
    {
        DlgPrefNoMidiDlg->setWindowTitle(QApplication::translate("DlgPrefNoMidiDlg", "Form3", 0, QApplication::UnicodeUTF8));
        TextLabel3_2->setText(QApplication::translate("DlgPrefNoMidiDlg", "No MIDI devices available", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefNoMidiDlg: public Ui_DlgPrefNoMidiDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFNOMIDIDLG_H
