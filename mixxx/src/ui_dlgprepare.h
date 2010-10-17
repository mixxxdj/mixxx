/********************************************************************************
** Form generated from reading UI file 'dlgprepare.ui'
**
** Created: Fri Oct 15 21:36:00 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREPARE_H
#define UI_DLGPREPARE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrepare
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_3;
    QRadioButton *radioButtonRecentlyAdded;
    QRadioButton *radioButtonAllSongs;
    QSpacerItem *horizontalSpacer;
    QLabel *labelProgress;
    QPushButton *pushButtonSelectAll;
    QPushButton *pushButtonAnalyze;
    QSpacerItem *horizontalSpacer_2;
    QTableView *m_pTrackTablePlaceholder;

    void setupUi(QWidget *DlgPrepare)
    {
        if (DlgPrepare->objectName().isEmpty())
            DlgPrepare->setObjectName(QString::fromUtf8("DlgPrepare"));
        DlgPrepare->resize(582, 399);
        verticalLayout = new QVBoxLayout(DlgPrepare);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_3 = new QSpacerItem(12, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        radioButtonRecentlyAdded = new QRadioButton(DlgPrepare);
        radioButtonRecentlyAdded->setObjectName(QString::fromUtf8("radioButtonRecentlyAdded"));

        horizontalLayout->addWidget(radioButtonRecentlyAdded);

        radioButtonAllSongs = new QRadioButton(DlgPrepare);
        radioButtonAllSongs->setObjectName(QString::fromUtf8("radioButtonAllSongs"));

        horizontalLayout->addWidget(radioButtonAllSongs);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        labelProgress = new QLabel(DlgPrepare);
        labelProgress->setObjectName(QString::fromUtf8("labelProgress"));

        horizontalLayout->addWidget(labelProgress);

        pushButtonSelectAll = new QPushButton(DlgPrepare);
        pushButtonSelectAll->setObjectName(QString::fromUtf8("pushButtonSelectAll"));

        horizontalLayout->addWidget(pushButtonSelectAll);

        pushButtonAnalyze = new QPushButton(DlgPrepare);
        pushButtonAnalyze->setObjectName(QString::fromUtf8("pushButtonAnalyze"));

        horizontalLayout->addWidget(pushButtonAnalyze);

        horizontalSpacer_2 = new QSpacerItem(12, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

        m_pTrackTablePlaceholder = new QTableView(DlgPrepare);
        m_pTrackTablePlaceholder->setObjectName(QString::fromUtf8("m_pTrackTablePlaceholder"));
        m_pTrackTablePlaceholder->setShowGrid(true);

        verticalLayout->addWidget(m_pTrackTablePlaceholder);


        retranslateUi(DlgPrepare);

        QMetaObject::connectSlotsByName(DlgPrepare);
    } // setupUi

    void retranslateUi(QWidget *DlgPrepare)
    {
        DlgPrepare->setWindowTitle(QApplication::translate("DlgPrepare", "Manage", 0, QApplication::UnicodeUTF8));
        radioButtonRecentlyAdded->setText(QApplication::translate("DlgPrepare", "Recently Added", 0, QApplication::UnicodeUTF8));
        radioButtonAllSongs->setText(QApplication::translate("DlgPrepare", "All Songs", 0, QApplication::UnicodeUTF8));
        labelProgress->setText(QApplication::translate("DlgPrepare", "Progress", 0, QApplication::UnicodeUTF8));
        pushButtonSelectAll->setText(QApplication::translate("DlgPrepare", "Select All", 0, QApplication::UnicodeUTF8));
        pushButtonAnalyze->setText(QApplication::translate("DlgPrepare", "Analyze", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrepare: public Ui_DlgPrepare {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREPARE_H
