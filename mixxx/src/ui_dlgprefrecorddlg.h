/********************************************************************************
** Form generated from reading UI file 'dlgprefrecorddlg.ui'
**
** Created: Fri Oct 15 21:35:44 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPREFRECORDDLG_H
#define UI_DLGPREFRECORDDLG_H

#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgPrefRecordDlg
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QLabel *TextLabel3_2;
    QLineEdit *LineEditRecPath;
    QPushButton *PushButtonBrowse;
    QLabel *TextLabel3_2_2;
    QComboBox *comboBoxEncoding;
    QSpacerItem *spacerItem;
    QSpacerItem *spacerItem1;
    Q3GroupBox *groupBoxQuality;
    QGridLayout *gridLayout2;
    QLabel *TextQuality;
    QSlider *SliderQuality;
    QLabel *TextLabel8_3_2;
    QLabel *TextLabel8_2_2_2;
    Q3GroupBox *groupBox1;
    QGridLayout *gridLayout3;
    QLabel *TextLabel3_2_3;
    QLineEdit *LineEditTitle;
    QLabel *TextLabel3_2_3_2;
    QLineEdit *LineEditAuthor;
    QLabel *TextLabel3_2_3_3;
    QLineEdit *LineEditAlbum;
    QSpacerItem *spacerItem2;

    void setupUi(QWidget *DlgPrefRecordDlg)
    {
        if (DlgPrefRecordDlg->objectName().isEmpty())
            DlgPrefRecordDlg->setObjectName(QString::fromUtf8("DlgPrefRecordDlg"));
        DlgPrefRecordDlg->resize(412, 409);
        gridLayout = new QGridLayout(DlgPrefRecordDlg);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout1 = new QGridLayout();
        gridLayout1->setSpacing(6);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        TextLabel3_2 = new QLabel(DlgPrefRecordDlg);
        TextLabel3_2->setObjectName(QString::fromUtf8("TextLabel3_2"));
        TextLabel3_2->setEnabled(true);
        QFont font;
        TextLabel3_2->setFont(font);
        TextLabel3_2->setWordWrap(false);

        gridLayout1->addWidget(TextLabel3_2, 0, 0, 1, 1);

        LineEditRecPath = new QLineEdit(DlgPrefRecordDlg);
        LineEditRecPath->setObjectName(QString::fromUtf8("LineEditRecPath"));
        LineEditRecPath->setFont(font);

        gridLayout1->addWidget(LineEditRecPath, 0, 2, 1, 1);

        PushButtonBrowse = new QPushButton(DlgPrefRecordDlg);
        PushButtonBrowse->setObjectName(QString::fromUtf8("PushButtonBrowse"));
        PushButtonBrowse->setFont(font);

        gridLayout1->addWidget(PushButtonBrowse, 0, 3, 1, 1);

        TextLabel3_2_2 = new QLabel(DlgPrefRecordDlg);
        TextLabel3_2_2->setObjectName(QString::fromUtf8("TextLabel3_2_2"));
        TextLabel3_2_2->setEnabled(true);
        TextLabel3_2_2->setFont(font);
        TextLabel3_2_2->setWordWrap(false);

        gridLayout1->addWidget(TextLabel3_2_2, 1, 0, 1, 1);

        comboBoxEncoding = new QComboBox(DlgPrefRecordDlg);
        comboBoxEncoding->setObjectName(QString::fromUtf8("comboBoxEncoding"));

        gridLayout1->addWidget(comboBoxEncoding, 1, 2, 1, 2);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem, 0, 1, 1, 1);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem1, 1, 1, 1, 1);


        gridLayout->addLayout(gridLayout1, 0, 0, 1, 1);

        groupBoxQuality = new Q3GroupBox(DlgPrefRecordDlg);
        groupBoxQuality->setObjectName(QString::fromUtf8("groupBoxQuality"));
        groupBoxQuality->setEnabled(true);
        groupBoxQuality->setFont(font);
        groupBoxQuality->setOrientation(Qt::Vertical);
        groupBoxQuality->setColumnLayout(0, Qt::Vertical);
        groupBoxQuality->layout()->setSpacing(6);
        groupBoxQuality->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout2 = new QGridLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(groupBoxQuality->layout());
        if (boxlayout)
            boxlayout->addLayout(gridLayout2);
        gridLayout2->setAlignment(Qt::AlignTop);
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        TextQuality = new QLabel(groupBoxQuality);
        TextQuality->setObjectName(QString::fromUtf8("TextQuality"));
        TextQuality->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextQuality->setWordWrap(false);

        gridLayout2->addWidget(TextQuality, 0, 1, 1, 1);

        SliderQuality = new QSlider(groupBoxQuality);
        SliderQuality->setObjectName(QString::fromUtf8("SliderQuality"));
        SliderQuality->setMinimum(1);
        SliderQuality->setMaximum(10);
        SliderQuality->setSingleStep(1);
        SliderQuality->setPageStep(1);
        SliderQuality->setValue(9);
        SliderQuality->setOrientation(Qt::Horizontal);
        SliderQuality->setTickInterval(5);

        gridLayout2->addWidget(SliderQuality, 1, 0, 1, 2);

        TextLabel8_3_2 = new QLabel(groupBoxQuality);
        TextLabel8_3_2->setObjectName(QString::fromUtf8("TextLabel8_3_2"));
        TextLabel8_3_2->setFont(font);
        TextLabel8_3_2->setWordWrap(false);

        gridLayout2->addWidget(TextLabel8_3_2, 2, 0, 1, 1);

        TextLabel8_2_2_2 = new QLabel(groupBoxQuality);
        TextLabel8_2_2_2->setObjectName(QString::fromUtf8("TextLabel8_2_2_2"));
        TextLabel8_2_2_2->setFont(font);
        TextLabel8_2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        TextLabel8_2_2_2->setWordWrap(false);

        gridLayout2->addWidget(TextLabel8_2_2_2, 2, 1, 1, 1);


        gridLayout->addWidget(groupBoxQuality, 1, 0, 1, 1);

        groupBox1 = new Q3GroupBox(DlgPrefRecordDlg);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        groupBox1->setOrientation(Qt::Vertical);
        groupBox1->setColumnLayout(0, Qt::Vertical);
        groupBox1->layout()->setSpacing(6);
        groupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout3 = new QGridLayout();
        QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(groupBox1->layout());
        if (boxlayout1)
            boxlayout1->addLayout(gridLayout3);
        gridLayout3->setAlignment(Qt::AlignTop);
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
        TextLabel3_2_3 = new QLabel(groupBox1);
        TextLabel3_2_3->setObjectName(QString::fromUtf8("TextLabel3_2_3"));
        TextLabel3_2_3->setEnabled(true);
        TextLabel3_2_3->setFont(font);
        TextLabel3_2_3->setWordWrap(false);

        gridLayout3->addWidget(TextLabel3_2_3, 0, 0, 1, 1);

        LineEditTitle = new QLineEdit(groupBox1);
        LineEditTitle->setObjectName(QString::fromUtf8("LineEditTitle"));

        gridLayout3->addWidget(LineEditTitle, 0, 1, 1, 1);

        TextLabel3_2_3_2 = new QLabel(groupBox1);
        TextLabel3_2_3_2->setObjectName(QString::fromUtf8("TextLabel3_2_3_2"));
        TextLabel3_2_3_2->setEnabled(true);
        TextLabel3_2_3_2->setFont(font);
        TextLabel3_2_3_2->setWordWrap(false);

        gridLayout3->addWidget(TextLabel3_2_3_2, 1, 0, 1, 1);

        LineEditAuthor = new QLineEdit(groupBox1);
        LineEditAuthor->setObjectName(QString::fromUtf8("LineEditAuthor"));

        gridLayout3->addWidget(LineEditAuthor, 1, 1, 1, 1);

        TextLabel3_2_3_3 = new QLabel(groupBox1);
        TextLabel3_2_3_3->setObjectName(QString::fromUtf8("TextLabel3_2_3_3"));
        TextLabel3_2_3_3->setEnabled(true);
        TextLabel3_2_3_3->setFont(font);
        TextLabel3_2_3_3->setWordWrap(false);

        gridLayout3->addWidget(TextLabel3_2_3_3, 2, 0, 1, 1);

        LineEditAlbum = new QLineEdit(groupBox1);
        LineEditAlbum->setObjectName(QString::fromUtf8("LineEditAlbum"));

        gridLayout3->addWidget(LineEditAlbum, 2, 1, 1, 1);


        gridLayout->addWidget(groupBox1, 2, 0, 1, 1);

        spacerItem2 = new QSpacerItem(20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem2, 4, 0, 1, 1);


        retranslateUi(DlgPrefRecordDlg);

        QMetaObject::connectSlotsByName(DlgPrefRecordDlg);
    } // setupUi

    void retranslateUi(QWidget *DlgPrefRecordDlg)
    {
        DlgPrefRecordDlg->setWindowTitle(QApplication::translate("DlgPrefRecordDlg", "Form3", 0, QApplication::UnicodeUTF8));
        TextLabel3_2->setText(QApplication::translate("DlgPrefRecordDlg", "File", 0, QApplication::UnicodeUTF8));
        PushButtonBrowse->setText(QApplication::translate("DlgPrefRecordDlg", "Browse...", 0, QApplication::UnicodeUTF8));
        TextLabel3_2_2->setText(QApplication::translate("DlgPrefRecordDlg", "Encoding", 0, QApplication::UnicodeUTF8));
        groupBoxQuality->setTitle(QApplication::translate("DlgPrefRecordDlg", "Quality", 0, QApplication::UnicodeUTF8));
        TextQuality->setText(QApplication::translate("DlgPrefRecordDlg", "Quality", 0, QApplication::UnicodeUTF8));
        TextLabel8_3_2->setText(QApplication::translate("DlgPrefRecordDlg", "Low", 0, QApplication::UnicodeUTF8));
        TextLabel8_2_2_2->setText(QApplication::translate("DlgPrefRecordDlg", "High", 0, QApplication::UnicodeUTF8));
        groupBox1->setTitle(QApplication::translate("DlgPrefRecordDlg", "Tags", 0, QApplication::UnicodeUTF8));
        TextLabel3_2_3->setText(QApplication::translate("DlgPrefRecordDlg", "Title", 0, QApplication::UnicodeUTF8));
        TextLabel3_2_3_2->setText(QApplication::translate("DlgPrefRecordDlg", "Author", 0, QApplication::UnicodeUTF8));
        TextLabel3_2_3_3->setText(QApplication::translate("DlgPrefRecordDlg", "Album", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DlgPrefRecordDlg: public Ui_DlgPrefRecordDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPREFRECORDDLG_H
