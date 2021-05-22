#include "widget/wscrollable.h"

WScrollable::WScrollable(QWidget* pParent)
        : QScrollArea(pParent),
          WBaseWidget(this) {
}
