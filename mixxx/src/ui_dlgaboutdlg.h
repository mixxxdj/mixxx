/********************************************************************************
** Form generated from reading UI file 'dlgaboutdlg.ui'
**
** Created: Fri Oct 15 21:35:28 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGABOUTDLG_H
#define UI_DLGABOUTDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_DlgAboutDlg
{
public:
    QDialogButtonBox *buttonBox;
    QTextBrowser *textBrowser;
    QLabel *label;
    QLabel *mixxx_logo;
    QLabel *version_label;
    QLabel *label_2;

    void setupUi(QDialog *DlgAboutDlg)
    {
        if (DlgAboutDlg->objectName().isEmpty())
            DlgAboutDlg->setObjectName(QString::fromUtf8("DlgAboutDlg"));
        DlgAboutDlg->resize(249, 322);
        DlgAboutDlg->setMinimumSize(QSize(249, 322));
        buttonBox = new QDialogButtonBox(DlgAboutDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(130, 280, 111, 41));
        buttonBox->setLayoutDirection(Qt::LeftToRight);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        textBrowser = new QTextBrowser(DlgAboutDlg);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setGeometry(QRect(10, 100, 231, 171));
        textBrowser->setTextInteractionFlags(Qt::NoTextInteraction);
        label = new QLabel(DlgAboutDlg);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(14, 10, 51, 61));
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/mixxx-icon.png")));
        mixxx_logo = new QLabel(DlgAboutDlg);
        mixxx_logo->setObjectName(QString::fromUtf8("mixxx_logo"));
        mixxx_logo->setGeometry(QRect(69, 7, 170, 68));
        mixxx_logo->setPixmap(QPixmap(QString::fromUtf8(":/images/templates/logo_mixxx.png")));
        mixxx_logo->setScaledContents(true);
        version_label = new QLabel(DlgAboutDlg);
        version_label->setObjectName(QString::fromUtf8("version_label"));
        version_label->setGeometry(QRect(40, 70, 200, 20));
        version_label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_2 = new QLabel(DlgAboutDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 290, 131, 21));
        label_2->setOpenExternalLinks(true);
        label_2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        retranslateUi(DlgAboutDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), DlgAboutDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DlgAboutDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(DlgAboutDlg);
    } // setupUi

    void retranslateUi(QDialog *DlgAboutDlg)
    {
        DlgAboutDlg->setWindowTitle(QApplication::translate("DlgAboutDlg", "About Mixxx", 0, QApplication::UnicodeUTF8));
        textBrowser->setHtml(QApplication::translate("DlgAboutDlg", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">Credits go here</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Sans Serif'; font-size:9pt;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        mixxx_logo->setText(QString());
        version_label->setText(QApplication::translate("DlgAboutDlg", "1.x.x", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgAboutDlg", "<a href=\"http://mixxx.org/\">Official Website</a>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgAboutDlg: public Ui_DlgAboutDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGABOUTDLG_H
