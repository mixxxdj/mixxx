#include "widget/wbasewidget.h"

WBaseWidget::WBaseWidget(QWidget* pWidget)
        : m_pWidget(pWidget),
          m_bDisabled(false) {
}

WBaseWidget::~WBaseWidget() {
}
