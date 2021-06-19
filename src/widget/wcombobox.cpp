#include "widget/wcombobox.h"

#include <QIcon>
#include <QtDebug>

#include "moc_wcombobox.cpp"

WComboBox::WComboBox(QWidget* pParent)
        : QComboBox(pParent),
          WBaseWidget(this) {
    connect(this,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &WComboBox::slotCurrentIndexChanged);
}

void WComboBox::setup(const QDomNode& node, const SkinContext& context) {
    // Load pixmaps for associated states
    QDomNode state = context.selectNode(node, "State");
    while (!state.isNull()) {
        if (state.isElement() && state.nodeName() == "State") {
            int iState = 0;
            if (context.hasNodeSelectInt(state, "Number", &iState)) {
                QString text = context.selectString(state, "Text");
                QString icon = context.selectString(state, "Icon");
                addItem(QIcon(icon), text, QVariant(iState));
            } else {
                SKIN_WARNING(state, context)
                        << "WComboBox ignoring <State> without <Number> node.";
            }
        }
        state = state.nextSibling();
    }
}

bool WComboBox::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QComboBox::event(pEvent);
}

void WComboBox::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    int index = findData(static_cast<int>(dValue));
    if (index != -1) {
        setCurrentIndex(index);
    }
}

void WComboBox::slotCurrentIndexChanged(int index) {
    setControlParameter(index);
}
