/********************************************************************************
** Form generated from reading UI file 'dlgtrackinfo.ui'
**
** Created: Fri Oct 15 21:36:01 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGTRACKINFO_H
#define UI_DLGTRACKINFO_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgTrackInfo
{
public:
    QVBoxLayout *verticalLayout_2;
    QLabel *lblSong;
    QToolBox *toolbox;
    QWidget *pgTrack;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLineEdit *txtTrackName;
    QLabel *label_3;
    QLineEdit *txtArtist;
    QLabel *label_6;
    QLineEdit *txtAlbum;
    QLabel *label_8;
    QLineEdit *txtYear;
    QLabel *label_10;
    QLabel *label_9;
    QLineEdit *txtGenre;
    QLabel *label_5;
    QLineEdit *txtFilepath;
    QLabel *label_7;
    QLabel *txtDuration;
    QLabel *Type;
    QLabel *txtType;
    QLabel *label_4;
    QTextEdit *txtComment;
    QLineEdit *txtTrackNumber;
    QVBoxLayout *verticalLayout_6;
    QPushButton *btnReloadFromFile;
    QWidget *pgBpm;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelBpm;
    QDoubleSpinBox *spinBpm;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *bpmDouble;
    QPushButton *bpmHalve;
    QPushButton *bpmTap;
    QLabel *label;
    QSpacerItem *verticalSpacer;
    QWidget *pgCue;
    QVBoxLayout *verticalLayout;
    QTableWidget *cueTable;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnCueActivate;
    QPushButton *btnCueDelete;
    QHBoxLayout *buttonLayout;
    QPushButton *btnPrev;
    QPushButton *btnApply;
    QPushButton *btnCancel;
    QPushButton *btnNext;

    void setupUi(QDialog *DlgTrackInfo)
    {
        if (DlgTrackInfo->objectName().isEmpty())
            DlgTrackInfo->setObjectName(QString::fromUtf8("DlgTrackInfo"));
        DlgTrackInfo->resize(448, 571);
        DlgTrackInfo->setSizeGripEnabled(true);
        verticalLayout_2 = new QVBoxLayout(DlgTrackInfo);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        lblSong = new QLabel(DlgTrackInfo);
        lblSong->setObjectName(QString::fromUtf8("lblSong"));
        QFont font;
        font.setBold(true);
        font.setUnderline(true);
        font.setWeight(75);
        lblSong->setFont(font);

        verticalLayout_2->addWidget(lblSong);

        toolbox = new QToolBox(DlgTrackInfo);
        toolbox->setObjectName(QString::fromUtf8("toolbox"));
        toolbox->setMinimumSize(QSize(0, 400));
        toolbox->setFrameShape(QFrame::NoFrame);
        toolbox->setFrameShadow(QFrame::Plain);
        pgTrack = new QWidget();
        pgTrack->setObjectName(QString::fromUtf8("pgTrack"));
        pgTrack->setGeometry(QRect(0, 0, 430, 410));
        verticalLayout_5 = new QVBoxLayout(pgTrack);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(pgTrack);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        txtTrackName = new QLineEdit(pgTrack);
        txtTrackName->setObjectName(QString::fromUtf8("txtTrackName"));

        gridLayout->addWidget(txtTrackName, 0, 1, 1, 3);

        label_3 = new QLabel(pgTrack);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        txtArtist = new QLineEdit(pgTrack);
        txtArtist->setObjectName(QString::fromUtf8("txtArtist"));

        gridLayout->addWidget(txtArtist, 1, 1, 1, 3);

        label_6 = new QLabel(pgTrack);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 2, 0, 1, 1);

        txtAlbum = new QLineEdit(pgTrack);
        txtAlbum->setObjectName(QString::fromUtf8("txtAlbum"));

        gridLayout->addWidget(txtAlbum, 2, 1, 1, 3);

        label_8 = new QLabel(pgTrack);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 3, 0, 1, 1);

        txtYear = new QLineEdit(pgTrack);
        txtYear->setObjectName(QString::fromUtf8("txtYear"));

        gridLayout->addWidget(txtYear, 3, 1, 1, 3);

        label_10 = new QLabel(pgTrack);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout->addWidget(label_10, 4, 0, 1, 1);

        label_9 = new QLabel(pgTrack);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 5, 0, 1, 1);

        txtGenre = new QLineEdit(pgTrack);
        txtGenre->setObjectName(QString::fromUtf8("txtGenre"));

        gridLayout->addWidget(txtGenre, 5, 1, 1, 3);

        label_5 = new QLabel(pgTrack);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 6, 0, 1, 1);

        txtFilepath = new QLineEdit(pgTrack);
        txtFilepath->setObjectName(QString::fromUtf8("txtFilepath"));
        txtFilepath->setEnabled(true);
        txtFilepath->setReadOnly(true);

        gridLayout->addWidget(txtFilepath, 6, 1, 1, 3);

        label_7 = new QLabel(pgTrack);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 8, 0, 1, 1);

        txtDuration = new QLabel(pgTrack);
        txtDuration->setObjectName(QString::fromUtf8("txtDuration"));

        gridLayout->addWidget(txtDuration, 8, 1, 1, 1);

        Type = new QLabel(pgTrack);
        Type->setObjectName(QString::fromUtf8("Type"));

        gridLayout->addWidget(Type, 8, 2, 1, 1);

        txtType = new QLabel(pgTrack);
        txtType->setObjectName(QString::fromUtf8("txtType"));

        gridLayout->addWidget(txtType, 8, 3, 1, 1);

        label_4 = new QLabel(pgTrack);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 9, 0, 1, 1);

        txtComment = new QTextEdit(pgTrack);
        txtComment->setObjectName(QString::fromUtf8("txtComment"));

        gridLayout->addWidget(txtComment, 9, 1, 1, 3);

        txtTrackNumber = new QLineEdit(pgTrack);
        txtTrackNumber->setObjectName(QString::fromUtf8("txtTrackNumber"));

        gridLayout->addWidget(txtTrackNumber, 4, 1, 1, 3);


        verticalLayout_5->addLayout(gridLayout);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        btnReloadFromFile = new QPushButton(pgTrack);
        btnReloadFromFile->setObjectName(QString::fromUtf8("btnReloadFromFile"));

        verticalLayout_6->addWidget(btnReloadFromFile);


        verticalLayout_5->addLayout(verticalLayout_6);

        toolbox->addItem(pgTrack, QString::fromUtf8("Track Information (Click to Expand)"));
        pgBpm = new QWidget();
        pgBpm->setObjectName(QString::fromUtf8("pgBpm"));
        pgBpm->setGeometry(QRect(0, 0, 430, 410));
        verticalLayout_4 = new QVBoxLayout(pgBpm);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        labelBpm = new QLabel(pgBpm);
        labelBpm->setObjectName(QString::fromUtf8("labelBpm"));

        horizontalLayout_2->addWidget(labelBpm);

        spinBpm = new QDoubleSpinBox(pgBpm);
        spinBpm->setObjectName(QString::fromUtf8("spinBpm"));
        spinBpm->setDecimals(8);
        spinBpm->setMaximum(500);
        spinBpm->setSingleStep(1);

        horizontalLayout_2->addWidget(spinBpm);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        bpmDouble = new QPushButton(pgBpm);
        bpmDouble->setObjectName(QString::fromUtf8("bpmDouble"));

        horizontalLayout_3->addWidget(bpmDouble);

        bpmHalve = new QPushButton(pgBpm);
        bpmHalve->setObjectName(QString::fromUtf8("bpmHalve"));

        horizontalLayout_3->addWidget(bpmHalve);


        verticalLayout_3->addLayout(horizontalLayout_3);


        horizontalLayout_4->addLayout(verticalLayout_3);

        bpmTap = new QPushButton(pgBpm);
        bpmTap->setObjectName(QString::fromUtf8("bpmTap"));
        bpmTap->setMinimumSize(QSize(121, 91));
        bpmTap->setMaximumSize(QSize(123, 91));

        horizontalLayout_4->addWidget(bpmTap);


        verticalLayout_4->addLayout(horizontalLayout_4);

        label = new QLabel(pgBpm);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_4->addWidget(label);

        verticalSpacer = new QSpacerItem(0, 200, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        toolbox->addItem(pgBpm, QString::fromUtf8("Track BPM"));
        pgCue = new QWidget();
        pgCue->setObjectName(QString::fromUtf8("pgCue"));
        pgCue->setEnabled(true);
        pgCue->setGeometry(QRect(0, 0, 430, 410));
        verticalLayout = new QVBoxLayout(pgCue);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        cueTable = new QTableWidget(pgCue);
        if (cueTable->columnCount() < 4)
            cueTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        cueTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        cueTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        cueTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        cueTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        cueTable->setObjectName(QString::fromUtf8("cueTable"));

        verticalLayout->addWidget(cueTable);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        btnCueActivate = new QPushButton(pgCue);
        btnCueActivate->setObjectName(QString::fromUtf8("btnCueActivate"));

        horizontalLayout->addWidget(btnCueActivate);

        btnCueDelete = new QPushButton(pgCue);
        btnCueDelete->setObjectName(QString::fromUtf8("btnCueDelete"));

        horizontalLayout->addWidget(btnCueDelete);


        verticalLayout->addLayout(horizontalLayout);

        toolbox->addItem(pgCue, QString::fromUtf8("Track Cuepoints"));

        verticalLayout_2->addWidget(toolbox);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(6);
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        btnPrev = new QPushButton(DlgTrackInfo);
        btnPrev->setObjectName(QString::fromUtf8("btnPrev"));

        buttonLayout->addWidget(btnPrev);

        btnApply = new QPushButton(DlgTrackInfo);
        btnApply->setObjectName(QString::fromUtf8("btnApply"));

        buttonLayout->addWidget(btnApply);

        btnCancel = new QPushButton(DlgTrackInfo);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setDefault(true);

        buttonLayout->addWidget(btnCancel);

        btnNext = new QPushButton(DlgTrackInfo);
        btnNext->setObjectName(QString::fromUtf8("btnNext"));

        buttonLayout->addWidget(btnNext);


        verticalLayout_2->addLayout(buttonLayout);


        retranslateUi(DlgTrackInfo);

        toolbox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DlgTrackInfo);
    } // setupUi

    void retranslateUi(QDialog *DlgTrackInfo)
    {
        DlgTrackInfo->setWindowTitle(QApplication::translate("DlgTrackInfo", "Track Editor", 0, QApplication::UnicodeUTF8));
        lblSong->setText(QApplication::translate("DlgTrackInfo", "Song:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DlgTrackInfo", "Title:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DlgTrackInfo", "Artist:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("DlgTrackInfo", "Album:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("DlgTrackInfo", "Date:", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("DlgTrackInfo", "Track #:", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("DlgTrackInfo", "Genre:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("DlgTrackInfo", "Filename:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("DlgTrackInfo", "Duration:", 0, QApplication::UnicodeUTF8));
        txtDuration->setText(QApplication::translate("DlgTrackInfo", "3:00", 0, QApplication::UnicodeUTF8));
        Type->setText(QApplication::translate("DlgTrackInfo", "File Type:", 0, QApplication::UnicodeUTF8));
        txtType->setText(QApplication::translate("DlgTrackInfo", "77", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("DlgTrackInfo", "Comments:", 0, QApplication::UnicodeUTF8));
        btnReloadFromFile->setText(QApplication::translate("DlgTrackInfo", "Reload track metadata from file.", 0, QApplication::UnicodeUTF8));
        toolbox->setItemText(toolbox->indexOf(pgTrack), QApplication::translate("DlgTrackInfo", "Track Information (Click to Expand)", 0, QApplication::UnicodeUTF8));
        labelBpm->setText(QApplication::translate("DlgTrackInfo", "Track BPM: ", 0, QApplication::UnicodeUTF8));
        bpmDouble->setText(QApplication::translate("DlgTrackInfo", "x2", 0, QApplication::UnicodeUTF8));
        bpmHalve->setText(QApplication::translate("DlgTrackInfo", "/2", 0, QApplication::UnicodeUTF8));
        bpmTap->setText(QApplication::translate("DlgTrackInfo", "Tap to Beat", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DlgTrackInfo", "Hint: Use the Library Analyze view to run BPM detection.", 0, QApplication::UnicodeUTF8));
        toolbox->setItemText(toolbox->indexOf(pgBpm), QApplication::translate("DlgTrackInfo", "Track BPM", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = cueTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("DlgTrackInfo", "Cue Id", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = cueTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("DlgTrackInfo", "Position", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = cueTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("DlgTrackInfo", "Hotcue", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = cueTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("DlgTrackInfo", "Label", 0, QApplication::UnicodeUTF8));
        btnCueActivate->setText(QApplication::translate("DlgTrackInfo", "Activate Cue", 0, QApplication::UnicodeUTF8));
        btnCueDelete->setText(QApplication::translate("DlgTrackInfo", "Delete Cue", 0, QApplication::UnicodeUTF8));
        toolbox->setItemText(toolbox->indexOf(pgCue), QApplication::translate("DlgTrackInfo", "Track Cuepoints", 0, QApplication::UnicodeUTF8));
        btnPrev->setText(QApplication::translate("DlgTrackInfo", "<< &Prev", 0, QApplication::UnicodeUTF8));
        btnApply->setText(QApplication::translate("DlgTrackInfo", "&Apply", 0, QApplication::UnicodeUTF8));
        btnCancel->setText(QApplication::translate("DlgTrackInfo", "&Cancel", 0, QApplication::UnicodeUTF8));
        btnCancel->setShortcut(QApplication::translate("DlgTrackInfo", "Alt+O", 0, QApplication::UnicodeUTF8));
        btnNext->setText(QApplication::translate("DlgTrackInfo", "&Next >>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgTrackInfo: public Ui_DlgTrackInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGTRACKINFO_H
