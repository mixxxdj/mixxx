#include "dialog/dlgreplacecolor.h"

#include <QAbstractButton>
#include <QColor>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

namespace {

constexpr int kColorButtonLightnessThreshold = 0x80;
const QString kColorButtonStyleSheetLight = QStringLiteral(
        "QPushButton { background-color: %1; }");
const QString kColorButtonStyleSheetDark = QStringLiteral(
        "QPushButton { background-color: %1; color: white; }");

void setButtonColor(QPushButton* button, const QColor& color) {
    button->setText(color.name());
    button->setStyleSheet((
            (color.lightness() >= kColorButtonLightnessThreshold)
                    ? kColorButtonStyleSheetLight
                    : kColorButtonStyleSheetDark)
                                  .arg(color.name()));
}

} // namespace

DlgReplaceColor::DlgReplaceColor(
        QWidget* pParent)
        : QDialog(pParent) {
    setupUi(this);

    setButtonColor(pushButtonNewColor, QColor(0, 0, 0));
    setButtonColor(pushButtonCurrentColor, QColor(0, 0, 0));

    connect(pushButtonNewColor,
            &QPushButton::clicked,
            [this] {
                slotSelectColor(pushButtonNewColor);
            });
    connect(pushButtonCurrentColor,
            &QPushButton::clicked,
            [this] {
                slotSelectColor(pushButtonCurrentColor);
            });
    connect(buttonBox,
            &QDialogButtonBox::clicked,
            [this](QAbstractButton* button) {
                switch (buttonBox->buttonRole(button)) {
                case QDialogButtonBox::RejectRole:
                    hide();
                    break;
                case QDialogButtonBox::ApplyRole:
                    slotApply();
                    break;
                default:
                    break;
                };
            });
}

void DlgReplaceColor::slotSelectColor(QPushButton* button) {
    QColor initialColor = QColor(button->text());
    QColor color = QColorDialog::getColor(initialColor, this);
    setButtonColor(button, color);
}

void DlgReplaceColor::slotApply() {
    QMessageBox::warning(this, "Not Implemented", "This function has not been implemented yet.");
}
