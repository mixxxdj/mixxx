#include "widget/wcolorpickeractionmenu.h"

#include <QKeyEvent>

#include "moc_wcolorpickeractionmenu.cpp"
#include "widget/wcolorpickeraction.h"

WColorPickerActionMenu::WColorPickerActionMenu(
        WColorPicker::Options options,
        const ColorPalette& palette,
        QWidget* parent)
        : QMenu(parent),
          m_pColorPickerAction(
                  make_parented<WColorPickerAction>(options, palette, this)) {
    connect(m_pColorPickerAction.get(),
            &WColorPickerAction::colorPicked,
            this,
            &WColorPickerActionMenu::colorPicked);

    addAction(m_pColorPickerAction.get());
}

void WColorPickerActionMenu::setColorPalette(const ColorPalette& palette) {
    m_pColorPickerAction->setColorPalette(palette);
    updateGeometry();
}

void WColorPickerActionMenu::setSelectedColor(const mixxx::RgbColor::optional_t& color) {
    m_pColorPickerAction->setSelectedColor(color);
}

void WColorPickerActionMenu::resetSelectedColor() {
    m_pColorPickerAction->resetSelectedColor();
}

bool WColorPickerActionMenu::focusNextPrevChild(bool next) {
    // Override the behavior of QMenu::focusNextPrevChild
    // which would convert this into a keyPressEvent()
    return QWidget::focusNextPrevChild(next);
}

void WColorPickerActionMenu::keyPressEvent(QKeyEvent* event) {
    qDebug() << "WColorPickerActionMenu::keyPressEvent()" << event << focusWidget()
             << m_pColorPickerAction->defaultWidget()->hasFocus()
             << m_pColorPickerAction->defaultWidget()->focusPolicy()
             << m_pColorPickerAction->defaultWidget()->focusWidget();
    if (event->key() == Qt::Key_F) {
        m_pColorPickerAction->defaultWidget()->setFocus();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_A) {
        setActiveAction(m_pColorPickerAction);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}
