/********************************************************************************
** Form generated from reading UI file 'dlgbpmschemedlg.ui'
**
** Created: Fri Oct 15 21:35:30 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGBPMSCHEMEDLG_H
#define UI_DLGBPMSCHEMEDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_DlgBpmSchemeDlg
{
public:
    QDialogButtonBox *buttonBox;
    QLineEdit *txtSchemeName;
    QLabel *label;
    QGroupBox *groupBox;
    QSpinBox *spinBpmMin;
    QSpinBox *spinBpmMax;
    QLabel *label_2;
    QLabel *label_3;
    QCheckBox *chkAnalyzeEntireSong;

    void setupUi(QDialog *DlgBpmSchemeDlg)
    {
        if (DlgBpmSchemeDlg->objectName().isEmpty())
            DlgBpmSchemeDlg->setObjectName(QString::fromUtf8("DlgBpmSchemeDlg"));
        DlgBpmSchemeDlg->resize(352, 172);
        buttonBox = new QDialogButtonBox(DlgBpmSchemeDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(0, 130, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
        txtSchemeName = new QLineEdit(DlgBpmSchemeDlg);
        txtSchemeName->setObjectName(QString::fromUtf8("txtSchemeName"));
        txtSchemeName->setGeometry(QRect(110, 10, 231, 23));
        label = new QLabel(DlgBpmSchemeDlg);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 111, 18));
        groupBox = new QGroupBox(DlgBpmSchemeDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 40, 331, 51));
        spinBpmMin = new QSpinBox(groupBox);
        spinBpmMin->setObjectName(QString::fromUtf8("spinBpmMin"));
        spinBpmMin->setGeometry(QRect(80, 20, 71, 23));
        spinBpmMin->setMinimum(1);
        spinBpmMin->setMaximum(220);
        spinBpmMax = new QSpinBox(groupBox);
        spinBpmMax->setObjectName(QString::fromUtf8("spinBpmMax"));
        spinBpmMax->setGeometry(QRect(210, 20, 71, 23));
        spinBpmMax->setMinimum(1);
        spinBpmMax->setMaximum(220);
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(170, 20, 31, 18));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 20, 31, 18));
        chkAnalyzeEntireSong = new QCheckBox(DlgBpmSchemeDlg);
        chkAnalyzeEntireSong->setObjectName(QString::fromUtf8("chkAnalyzeEntireSong"));
        chkAnalyzeEntireSong->setGeometry(QRect(90, 100, 201, 22));

        retranslateUi(DlgBpmSchemeDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), DlgBpmSchemeDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DlgBpmSchemeDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(DlgBpmSchemeDlg);
    } // setupUi

    void retranslateUi(QDialog *DlgBpmSchemeDlg)
    {
        DlgBpmSchemeDlg->setWindowTitle(QApplication::translate("DlgBpmSchemeDlg", "BPM Scheme", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgBpmSchemeDlg", "Scheme Name:", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DlgBpmSchemeDlg", "BPM Range", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgBpmSchemeDlg", "Max", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DlgBpmSchemeDlg", "Min", 0, QApplication::UnicodeUTF8));
        chkAnalyzeEntireSong->setText(QApplication::translate("DlgBpmSchemeDlg", "Analyze Entire Song", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgBpmSchemeDlg: public Ui_DlgBpmSchemeDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGBPMSCHEMEDLG_H
