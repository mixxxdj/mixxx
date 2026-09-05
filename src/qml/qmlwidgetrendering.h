#pragma once

#include <QPainter>
#include <QRegion>
#include <QWidget>

namespace mixxx::qml {

inline void renderWidgetRegion(QWidget* pWidget,
        QPainter* pPainter,
        const QRegion& logicalRegion,
        const QColor& backgroundColor) {
    if (logicalRegion.isEmpty()) {
        return;
    }
    pPainter->save();
    pPainter->setClipRegion(logicalRegion, Qt::IntersectClip);
    pPainter->fillRect(logicalRegion.boundingRect(), backgroundColor);
    pWidget->render(pPainter, logicalRegion.boundingRect().topLeft(), logicalRegion);
    pPainter->restore();
}

} // namespace mixxx::qml
