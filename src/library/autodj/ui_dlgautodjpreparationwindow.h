/********************************************************************************
** Form generated from reading UI file 'dlgautodjpreparationwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGAUTODJPREPARATIONWINDOW_H
#define UI_DLGAUTODJPREPARATIONWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgAutoDJPreparationWindow {
  public:
    QVBoxLayout* verticalLayout;
    QWidget* LibraryFeatureControls;
    QHBoxLayout* horizontalLayout;
    QPushButton* pushButtonAutoDJ;
    QSpacerItem* horizontalSpacer_1;
    QPushButton* pushButtonFadeNow;
    QPushButton* pushButtonSkipNext;
    QComboBox* fadeModeCombobox;
    QSpinBox* spinBoxTransition;
    QLabel* labelTransitionAppendix;
    QSpacerItem* horizontalSpacer_2;
    QPushButton* pushButtonShuffle;
    QPushButton* pushButtonAddRandomTrack;
    QPushButton* pushButtonRepeatPlaylist;
    QSpacerItem* horizontalSpacer_3;
    QLabel* labelSelectionInfo;
    QTableView* m_pTrackTablePlaceholder;

    void setupUi(QWidget* DlgAutoDJ) {
        if (DlgAutoDJ->objectName().isEmpty())
            DlgAutoDJ->setObjectName("DlgAutoDJ");
        DlgAutoDJ->resize(885, 399);
        verticalLayout = new QVBoxLayout(DlgAutoDJ);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        LibraryFeatureControls = new QWidget(DlgAutoDJ);
        LibraryFeatureControls->setObjectName("LibraryFeatureControls");
        horizontalLayout = new QHBoxLayout(LibraryFeatureControls);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pushButtonAutoDJ = new QPushButton(LibraryFeatureControls);
        pushButtonAutoDJ->setObjectName("pushButtonAutoDJ");
        pushButtonAutoDJ->setFocusPolicy(Qt::NoFocus);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButtonAutoDJ->sizePolicy().hasHeightForWidth());
        pushButtonAutoDJ->setSizePolicy(sizePolicy);
        pushButtonAutoDJ->setCheckable(true);

        horizontalLayout->addWidget(pushButtonAutoDJ);

        horizontalSpacer_1 = new QSpacerItem(10,
                1,
                QSizePolicy::Policy::Fixed,
                QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        pushButtonFadeNow = new QPushButton(LibraryFeatureControls);
        pushButtonFadeNow->setObjectName("pushButtonFadeNow");
        pushButtonFadeNow->setFocusPolicy(Qt::NoFocus);
        sizePolicy.setHeightForWidth(pushButtonFadeNow->sizePolicy().hasHeightForWidth());
        pushButtonFadeNow->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(pushButtonFadeNow);

        pushButtonSkipNext = new QPushButton(LibraryFeatureControls);
        pushButtonSkipNext->setObjectName("pushButtonSkipNext");
        pushButtonSkipNext->setFocusPolicy(Qt::NoFocus);
        sizePolicy.setHeightForWidth(pushButtonSkipNext->sizePolicy().hasHeightForWidth());
        pushButtonSkipNext->setSizePolicy(sizePolicy);
        pushButtonSkipNext->setCheckable(false);

        horizontalLayout->addWidget(pushButtonSkipNext);

        fadeModeCombobox = new QComboBox(LibraryFeatureControls);
        fadeModeCombobox->setObjectName("fadeModeCombobox");

        horizontalLayout->addWidget(fadeModeCombobox);

        spinBoxTransition = new QSpinBox(LibraryFeatureControls);
        spinBoxTransition->setObjectName("spinBoxTransition");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(spinBoxTransition->sizePolicy().hasHeightForWidth());
        spinBoxTransition->setSizePolicy(sizePolicy1);
        spinBoxTransition->setFrame(false);
        spinBoxTransition->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
        spinBoxTransition->setMinimum(-99);

        horizontalLayout->addWidget(spinBoxTransition);

        labelTransitionAppendix = new QLabel(LibraryFeatureControls);
        labelTransitionAppendix->setObjectName("labelTransitionAppendix");

        horizontalLayout->addWidget(labelTransitionAppendix);

        horizontalSpacer_2 = new QSpacerItem(10,
                1,
                QSizePolicy::Policy::Fixed,
                QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        pushButtonShuffle = new QPushButton(LibraryFeatureControls);
        pushButtonShuffle->setObjectName("pushButtonShuffle");
        pushButtonShuffle->setFocusPolicy(Qt::NoFocus);
        sizePolicy.setHeightForWidth(pushButtonShuffle->sizePolicy().hasHeightForWidth());
        pushButtonShuffle->setSizePolicy(sizePolicy);
        pushButtonShuffle->setCheckable(false);

        horizontalLayout->addWidget(pushButtonShuffle);

        pushButtonAddRandomTrack = new QPushButton(LibraryFeatureControls);
        pushButtonAddRandomTrack->setObjectName("pushButtonAddRandomTrack");
        pushButtonAddRandomTrack->setFocusPolicy(Qt::NoFocus);
        sizePolicy.setHeightForWidth(pushButtonAddRandomTrack->sizePolicy().hasHeightForWidth());
        pushButtonAddRandomTrack->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(pushButtonAddRandomTrack);

        pushButtonRepeatPlaylist = new QPushButton(LibraryFeatureControls);
        pushButtonRepeatPlaylist->setObjectName("pushButtonRepeatPlaylist");
        pushButtonRepeatPlaylist->setFocusPolicy(Qt::NoFocus);
        sizePolicy.setHeightForWidth(pushButtonRepeatPlaylist->sizePolicy().hasHeightForWidth());
        pushButtonRepeatPlaylist->setSizePolicy(sizePolicy);
        pushButtonRepeatPlaylist->setCheckable(true);

        horizontalLayout->addWidget(pushButtonRepeatPlaylist);

        horizontalSpacer_3 = new QSpacerItem(1,
                20,
                QSizePolicy::Policy::Expanding,
                QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        labelSelectionInfo = new QLabel(LibraryFeatureControls);
        labelSelectionInfo->setObjectName("labelSelectionInfo");

        horizontalLayout->addWidget(labelSelectionInfo);

        verticalLayout->addWidget(LibraryFeatureControls);

        m_pTrackTablePlaceholder = new QTableView(DlgAutoDJ);
        m_pTrackTablePlaceholder->setObjectName("m_pTrackTablePlaceholder");
        m_pTrackTablePlaceholder->setShowGrid(true);

        verticalLayout->addWidget(m_pTrackTablePlaceholder);

        retranslateUi(DlgAutoDJ);

        QMetaObject::connectSlotsByName(DlgAutoDJ);
    } // setupUi

    void retranslateUi(QWidget* DlgAutoDJ) {
        DlgAutoDJ->setWindowTitle(QCoreApplication::translate(
                "DlgAutoDJPreparationWindow", "Auto DJ", nullptr));
        labelTransitionAppendix->setText(QCoreApplication::translate(
                "DlgAutoDJPreparationWindow", "sec.", nullptr));
        labelSelectionInfo->setText(QString());
    } // retranslateUi
};

namespace Ui {
class DlgAutoDJPreparationWindow : public Ui_DlgAutoDJPreparationWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGAUTODJPREPARATIONWINDOW_H
