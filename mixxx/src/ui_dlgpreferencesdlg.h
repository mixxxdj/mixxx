/********************************************************************************
** Form generated from reading UI file 'dlgpreferencesdlg.ui'
**
** Created: Fri Oct 15 21:35:36 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFERENCESDLG_H
#define UI_DLGPREFERENCESDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPreferencesDlg
{
public:
    QHBoxLayout *horizontalLayout;
    QTreeWidget *contentsTreeWidget;
    QVBoxLayout *vboxLayout;
    QStackedWidget *pagesWidget;
    QWidget *page_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DlgPreferencesDlg)
    {
        if (DlgPreferencesDlg->objectName().isEmpty())
            DlgPreferencesDlg->setObjectName(QString::fromUtf8("DlgPreferencesDlg"));
        DlgPreferencesDlg->resize(681, 445);
        horizontalLayout = new QHBoxLayout(DlgPreferencesDlg);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        contentsTreeWidget = new QTreeWidget(DlgPreferencesDlg);
        contentsTreeWidget->setObjectName(QString::fromUtf8("contentsTreeWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(contentsTreeWidget->sizePolicy().hasHeightForWidth());
        contentsTreeWidget->setSizePolicy(sizePolicy);
        contentsTreeWidget->setMinimumSize(QSize(0, 0));
        contentsTreeWidget->setMaximumSize(QSize(170, 16777215));
        contentsTreeWidget->setProperty("showDropIndicator", QVariant(false));
        contentsTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        contentsTreeWidget->setRootIsDecorated(true);

        horizontalLayout->addWidget(contentsTreeWidget);

        vboxLayout = new QVBoxLayout();
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        pagesWidget = new QStackedWidget(DlgPreferencesDlg);
        pagesWidget->setObjectName(QString::fromUtf8("pagesWidget"));
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        pagesWidget->addWidget(page_2);

        vboxLayout->addWidget(pagesWidget);

        buttonBox = new QDialogButtonBox(DlgPreferencesDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        vboxLayout->addWidget(buttonBox);


        horizontalLayout->addLayout(vboxLayout);


        retranslateUi(DlgPreferencesDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), DlgPreferencesDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DlgPreferencesDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(DlgPreferencesDlg);
    } // setupUi

    void retranslateUi(QDialog *DlgPreferencesDlg)
    {
        DlgPreferencesDlg->setWindowTitle(QApplication::translate("DlgPreferencesDlg", "Dialog", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem = contentsTreeWidget->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("DlgPreferencesDlg", "1", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPreferencesDlg: public Ui_DlgPreferencesDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFERENCESDLG_H
