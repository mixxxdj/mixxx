/********************************************************************************
** Form generated from reading UI file 'dlgprefplaylistdlg.ui'
**
** Created: Fri Oct 15 21:35:43 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFPLAYLISTDLG_H
#define UI_DLGPREFPLAYLISTDLG_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefPlaylistDlg
{
public:
    QGroupBox *groupBoxiPod;
    QGridLayout *gridLayout;
    QLabel *TextLabel3_3;
    QLineEdit *LineEditiPodMountPoint;
    QPushButton *PushButtonBrowseiPodMountPoint;
    QPushButton *PushButtonDetectiPodMountPoint;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxLibrary;
    QGridLayout *gridLayout1;
    QLabel *TextLabel3_2;
    QLineEdit *LineEditSongfiles;
    QPushButton *PushButtonBrowsePlaylist;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLabel *label_2;
    QPushButton *pushButton;
    QPushButton *pushButtonExtraPlugins;
    QGroupBox *groupBoxBundledSongs;
    QGridLayout *gridLayout_3;
    QCheckBox *checkBoxPromoStats;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *DlgPrefPlaylistDlg)
    {
        if (DlgPrefPlaylistDlg->objectName().isEmpty())
            DlgPrefPlaylistDlg->setObjectName(QString::fromUtf8("DlgPrefPlaylistDlg"));
        DlgPrefPlaylistDlg->resize(512, 442);
        groupBoxiPod = new QGroupBox(DlgPrefPlaylistDlg);
        groupBoxiPod->setObjectName(QString::fromUtf8("groupBoxiPod"));
        groupBoxiPod->setGeometry(QRect(0, 0, 100, 30));
        groupBoxiPod->setVisible(false);
        gridLayout = new QGridLayout(groupBoxiPod);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        TextLabel3_3 = new QLabel(groupBoxiPod);
        TextLabel3_3->setObjectName(QString::fromUtf8("TextLabel3_3"));
        TextLabel3_3->setEnabled(true);
        TextLabel3_3->setMinimumSize(QSize(100, 10));
        QFont font;
        TextLabel3_3->setFont(font);
        TextLabel3_3->setWordWrap(false);

        gridLayout->addWidget(TextLabel3_3, 0, 0, 1, 1);

        LineEditiPodMountPoint = new QLineEdit(groupBoxiPod);
        LineEditiPodMountPoint->setObjectName(QString::fromUtf8("LineEditiPodMountPoint"));
        LineEditiPodMountPoint->setMinimumSize(QSize(100, 10));
        LineEditiPodMountPoint->setFont(font);

        gridLayout->addWidget(LineEditiPodMountPoint, 0, 1, 1, 1);

        PushButtonBrowseiPodMountPoint = new QPushButton(groupBoxiPod);
        PushButtonBrowseiPodMountPoint->setObjectName(QString::fromUtf8("PushButtonBrowseiPodMountPoint"));
        PushButtonBrowseiPodMountPoint->setFont(font);

        gridLayout->addWidget(PushButtonBrowseiPodMountPoint, 0, 2, 1, 1);

        PushButtonDetectiPodMountPoint = new QPushButton(groupBoxiPod);
        PushButtonDetectiPodMountPoint->setObjectName(QString::fromUtf8("PushButtonDetectiPodMountPoint"));
        PushButtonDetectiPodMountPoint->setFont(font);

        gridLayout->addWidget(PushButtonDetectiPodMountPoint, 0, 3, 1, 1);

        verticalLayout = new QVBoxLayout(DlgPrefPlaylistDlg);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxLibrary = new QGroupBox(DlgPrefPlaylistDlg);
        groupBoxLibrary->setObjectName(QString::fromUtf8("groupBoxLibrary"));
        gridLayout1 = new QGridLayout(groupBoxLibrary);
        gridLayout1->setSpacing(6);
        gridLayout1->setContentsMargins(11, 11, 11, 11);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        TextLabel3_2 = new QLabel(groupBoxLibrary);
        TextLabel3_2->setObjectName(QString::fromUtf8("TextLabel3_2"));
        TextLabel3_2->setEnabled(true);
        TextLabel3_2->setMinimumSize(QSize(100, 10));
        TextLabel3_2->setFont(font);
        TextLabel3_2->setWordWrap(false);

        gridLayout1->addWidget(TextLabel3_2, 0, 0, 1, 1);

        LineEditSongfiles = new QLineEdit(groupBoxLibrary);
        LineEditSongfiles->setObjectName(QString::fromUtf8("LineEditSongfiles"));
        LineEditSongfiles->setMinimumSize(QSize(100, 10));
        LineEditSongfiles->setFont(font);

        gridLayout1->addWidget(LineEditSongfiles, 0, 1, 1, 1);

        PushButtonBrowsePlaylist = new QPushButton(groupBoxLibrary);
        PushButtonBrowsePlaylist->setObjectName(QString::fromUtf8("PushButtonBrowsePlaylist"));
        PushButtonBrowsePlaylist->setFont(font);

        gridLayout1->addWidget(PushButtonBrowsePlaylist, 0, 2, 1, 1);


        verticalLayout->addWidget(groupBoxLibrary);

        groupBox = new QGroupBox(DlgPrefPlaylistDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setFlat(false);
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(true);

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setEnabled(false);

        gridLayout_2->addWidget(pushButton, 0, 1, 1, 1);

        pushButtonExtraPlugins = new QPushButton(groupBox);
        pushButtonExtraPlugins->setObjectName(QString::fromUtf8("pushButtonExtraPlugins"));

        gridLayout_2->addWidget(pushButtonExtraPlugins, 1, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBoxBundledSongs = new QGroupBox(DlgPrefPlaylistDlg);
        groupBoxBundledSongs->setObjectName(QString::fromUtf8("groupBoxBundledSongs"));
        gridLayout_3 = new QGridLayout(groupBoxBundledSongs);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        checkBoxPromoStats = new QCheckBox(groupBoxBundledSongs);
        checkBoxPromoStats->setObjectName(QString::fromUtf8("checkBoxPromoStats"));

        gridLayout_3->addWidget(checkBoxPromoStats, 0, 0, 1, 1);


        verticalLayout->addWidget(groupBoxBundledSongs);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(DlgPrefPlaylistDlg);

        QMetaObject::connectSlotsByName(DlgPrefPlaylistDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefPlaylistDlg)
    {
        DlgPrefPlaylistDlg->setWindowTitle(QApplication::translate("DlgPrefPlaylistDlg", "Form3", 0, QApplication::UnicodeUTF8));
        groupBoxiPod->setTitle(QApplication::translate("DlgPrefPlaylistDlg", "iPod", 0, QApplication::UnicodeUTF8));
        TextLabel3_3->setText(QApplication::translate("DlgPrefPlaylistDlg", "iPod mountpoint", 0, QApplication::UnicodeUTF8));
        PushButtonBrowseiPodMountPoint->setText(QApplication::translate("DlgPrefPlaylistDlg", "Browse...", 0, QApplication::UnicodeUTF8));
        PushButtonDetectiPodMountPoint->setText(QApplication::translate("DlgPrefPlaylistDlg", "Detect", 0, QApplication::UnicodeUTF8));
        groupBoxLibrary->setTitle(QApplication::translate("DlgPrefPlaylistDlg", "Library", 0, QApplication::UnicodeUTF8));
        TextLabel3_2->setText(QApplication::translate("DlgPrefPlaylistDlg", "Music Directory:", 0, QApplication::UnicodeUTF8));
        PushButtonBrowsePlaylist->setText(QApplication::translate("DlgPrefPlaylistDlg", "Browse...", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DlgPrefPlaylistDlg", "Audio Playback Plugins", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgPrefPlaylistDlg", "Extra Playback Plugins:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgPrefPlaylistDlg", "MP3, Ogg Vorbis, FLAC, Wave, Aiff Playback:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("DlgPrefPlaylistDlg", "Built-in", 0, QApplication::UnicodeUTF8));
        pushButtonExtraPlugins->setText(QApplication::translate("DlgPrefPlaylistDlg", "Available Online...", 0, QApplication::UnicodeUTF8));
        groupBoxBundledSongs->setTitle(QApplication::translate("DlgPrefPlaylistDlg", "Bundled Songs", 0, QApplication::UnicodeUTF8));
        checkBoxPromoStats->setText(QApplication::translate("DlgPrefPlaylistDlg", "Support Mixxx by counting and sharing bundled songs\n"
"playback and outbound link statistics", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefPlaylistDlg: public Ui_DlgPrefPlaylistDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFPLAYLISTDLG_H
