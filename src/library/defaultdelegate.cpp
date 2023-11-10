#include "library/defaultdelegate.h"

#include <QCoreApplication>
#include <QPainter>

#include "mixxxapplication.h"
#include "moc_defaultdelegate.cpp"

DefaultDelegate::DefaultDelegate(QTableView* pTableView)
        : QStyledItemDelegate(pTableView), m_pTableView(pTableView) {
}

void DefaultDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    processTimeSensitiveEvents(painter);
    QStyledItemDelegate::paint(painter, option, index);
}

void DefaultDelegate::processTimeSensitiveEvents(QPainter* painter) const {
    // Drawing the table can be slow, resulting in time sensitive events not
    // being delivered in time. (The WaveformWidgetFactory not receiving its
    // render and swap signals in time). We force processing these events
    // during the drawing of the table cells to fix this.
    auto app = qobject_cast<MixxxApplication*>(QCoreApplication::instance());
    if (app->hasTimeSensitiveEvents()) {
        QPaintDevice* device = painter->device();
        if (device) {
            painter->end();
        }
        app->processTimeSensitiveEvents();
        if (device) {
            painter->begin(device);
        }
    }
}
