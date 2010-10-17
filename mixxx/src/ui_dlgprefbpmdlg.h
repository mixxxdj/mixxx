/********************************************************************************
** Form generated from reading UI file 'dlgprefbpmdlg.ui'
**
** Created: Fri Oct 15 21:35:34 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFBPMDLG_H
#define UI_DLGPREFBPMDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefBPMDlg
{
public:
    QGridLayout *gridLayout;
    Q3GroupBox *GroupBox1;
    QGridLayout *gridLayout1;
    QCheckBox *chkEnableBpmDetection;
    QFrame *line1;
    QCheckBox *chkDetectOnImport;
    QCheckBox *chkWriteID3;
    QCheckBox *chkAboveRange;
    QGroupBox *grpBpmSchemes;
    QGridLayout *gridLayout2;
    QListWidget *lstSchemes;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnDefault;
    QSpacerItem *spacerItem;

    void setupUi(QWidget *DlgPrefBPMDlg)
    {
        if (DlgPrefBPMDlg->objectName().isEmpty())
            DlgPrefBPMDlg->setObjectName(QString::fromUtf8("DlgPrefBPMDlg"));
        DlgPrefBPMDlg->resize(427, 395);
        gridLayout = new QGridLayout(DlgPrefBPMDlg);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        GroupBox1 = new Q3GroupBox(DlgPrefBPMDlg);
        GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
        GroupBox1->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(GroupBox1->sizePolicy().hasHeightForWidth());
        GroupBox1->setSizePolicy(sizePolicy);
        GroupBox1->setMinimumSize(QSize(409, 150));
        QFont font;
        GroupBox1->setFont(font);
        GroupBox1->setColumnLayout(0, Qt::Vertical);
        GroupBox1->layout()->setSpacing(6);
        GroupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout1 = new QGridLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(GroupBox1->layout());
        if (boxlayout)
            boxlayout->addLayout(gridLayout1);
        gridLayout1->setAlignment(Qt::AlignTop);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        chkEnableBpmDetection = new QCheckBox(GroupBox1);
        chkEnableBpmDetection->setObjectName(QString::fromUtf8("chkEnableBpmDetection"));

        gridLayout1->addWidget(chkEnableBpmDetection, 0, 0, 1, 1);

        line1 = new QFrame(GroupBox1);
        line1->setObjectName(QString::fromUtf8("line1"));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);

        gridLayout1->addWidget(line1, 1, 0, 1, 1);

        chkDetectOnImport = new QCheckBox(GroupBox1);
        chkDetectOnImport->setObjectName(QString::fromUtf8("chkDetectOnImport"));

        gridLayout1->addWidget(chkDetectOnImport, 2, 0, 1, 1);

        chkWriteID3 = new QCheckBox(GroupBox1);
        chkWriteID3->setObjectName(QString::fromUtf8("chkWriteID3"));

        gridLayout1->addWidget(chkWriteID3, 3, 0, 1, 1);

        chkAboveRange = new QCheckBox(GroupBox1);
        chkAboveRange->setObjectName(QString::fromUtf8("chkAboveRange"));

        gridLayout1->addWidget(chkAboveRange, 4, 0, 1, 1);


        gridLayout->addWidget(GroupBox1, 0, 0, 1, 1);

        grpBpmSchemes = new QGroupBox(DlgPrefBPMDlg);
        grpBpmSchemes->setObjectName(QString::fromUtf8("grpBpmSchemes"));
        grpBpmSchemes->setMinimumSize(QSize(409, 200));
        gridLayout2 = new QGridLayout(grpBpmSchemes);
        gridLayout2->setSpacing(6);
        gridLayout2->setContentsMargins(11, 11, 11, 11);
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        lstSchemes = new QListWidget(grpBpmSchemes);
        lstSchemes->setObjectName(QString::fromUtf8("lstSchemes"));
        lstSchemes->setProperty("showDropIndicator", QVariant(false));
        lstSchemes->setDragDropMode(QAbstractItemView::NoDragDrop);
        lstSchemes->setModelColumn(0);

        gridLayout2->addWidget(lstSchemes, 0, 0, 5, 1);

        btnAdd = new QPushButton(grpBpmSchemes);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));

        gridLayout2->addWidget(btnAdd, 0, 1, 1, 1);

        btnEdit = new QPushButton(grpBpmSchemes);
        btnEdit->setObjectName(QString::fromUtf8("btnEdit"));

        gridLayout2->addWidget(btnEdit, 1, 1, 1, 1);

        btnDelete = new QPushButton(grpBpmSchemes);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));

        gridLayout2->addWidget(btnDelete, 2, 1, 1, 1);

        btnDefault = new QPushButton(grpBpmSchemes);
        btnDefault->setObjectName(QString::fromUtf8("btnDefault"));

        gridLayout2->addWidget(btnDefault, 3, 1, 1, 1);

        spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout2->addItem(spacerItem, 4, 1, 1, 1);


        gridLayout->addWidget(grpBpmSchemes, 1, 0, 1, 1);


        retranslateUi(DlgPrefBPMDlg);

        QMetaObject::connectSlotsByName(DlgPrefBPMDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefBPMDlg)
    {
        DlgPrefBPMDlg->setWindowTitle(QApplication::translate("DlgPrefBPMDlg", "BPM Detection Settings", 0, QApplication::UnicodeUTF8));
        GroupBox1->setTitle(QApplication::translate("DlgPrefBPMDlg", "BPM Detection", 0, QApplication::UnicodeUTF8));
        chkEnableBpmDetection->setText(QApplication::translate("DlgPrefBPMDlg", "Enable BPM Detection", 0, QApplication::UnicodeUTF8));
        chkDetectOnImport->setText(QApplication::translate("DlgPrefBPMDlg", "Detect Song BPM on Import", 0, QApplication::UnicodeUTF8));
        chkWriteID3->setText(QApplication::translate("DlgPrefBPMDlg", "Write BPM to ID3 Tag", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkAboveRange->setToolTip(QApplication::translate("DlgPrefBPMDlg", "If BPM can be detected but not within specified range", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkAboveRange->setText(QApplication::translate("DlgPrefBPMDlg", "Allow BPM above the range", 0, QApplication::UnicodeUTF8));
        grpBpmSchemes->setTitle(QApplication::translate("DlgPrefBPMDlg", "BPM Schemes", 0, QApplication::UnicodeUTF8));
        btnAdd->setText(QApplication::translate("DlgPrefBPMDlg", "Add", 0, QApplication::UnicodeUTF8));
        btnEdit->setText(QApplication::translate("DlgPrefBPMDlg", "Edit", 0, QApplication::UnicodeUTF8));
        btnDelete->setText(QApplication::translate("DlgPrefBPMDlg", "Delete", 0, QApplication::UnicodeUTF8));
        btnDefault->setText(QApplication::translate("DlgPrefBPMDlg", "Default", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefBPMDlg: public Ui_DlgPrefBPMDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFBPMDLG_H
