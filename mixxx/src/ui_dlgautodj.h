/********************************************************************************
** Form generated from reading UI file 'dlgautodj.ui'
**
** Created: Fri Oct 15 21:35:29 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGAUTODJ_H
#define UI_DLGAUTODJ_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgAutoDJ
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QPushButton *pushButtonAutoDJ;
    QSpacerItem *horizontalSpacer_2;
    QTableView *m_pTrackTablePlaceholder;

    void setupUi(QWidget *DlgAutoDJ)
    {
        if (DlgAutoDJ->objectName().isEmpty())
            DlgAutoDJ->setObjectName(QString::fromUtf8("DlgAutoDJ"));
        DlgAutoDJ->resize(582, 399);
        verticalLayout = new QVBoxLayout(DlgAutoDJ);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(DlgAutoDJ);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        pushButtonAutoDJ = new QPushButton(DlgAutoDJ);
        pushButtonAutoDJ->setObjectName(QString::fromUtf8("pushButtonAutoDJ"));
        pushButtonAutoDJ->setCheckable(true);

        horizontalLayout->addWidget(pushButtonAutoDJ);

        horizontalSpacer_2 = new QSpacerItem(12, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

        m_pTrackTablePlaceholder = new QTableView(DlgAutoDJ);
        m_pTrackTablePlaceholder->setObjectName(QString::fromUtf8("m_pTrackTablePlaceholder"));
        m_pTrackTablePlaceholder->setShowGrid(true);

        verticalLayout->addWidget(m_pTrackTablePlaceholder);


        retranslateUi(DlgAutoDJ);

        QMetaObject::connectSlotsByName(DlgAutoDJ);
    } // setupUi

    void retranslateUi(QWidget *DlgAutoDJ)
    {
        DlgAutoDJ->setWindowTitle(QApplication::translate("DlgAutoDJ", "Manage", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgAutoDJ", "Add tracks to the queue below...", 0, QApplication::UnicodeUTF8));
        pushButtonAutoDJ->setText(QApplication::translate("DlgAutoDJ", "Enable Auto DJ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgAutoDJ: public Ui_DlgAutoDJ {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGAUTODJ_H
