#pragma once

#include <QRegion>
#include <QSize>
#include <QtGlobal>
#include <cmath>

namespace mixxx {
namespace qml {

enum class RenderInvalidationReason {
    Unknown,
    FullSurface,
    Viewport,
    SmallRegion,
};

struct RenderInvalidationState {
    explicit RenderInvalidationState(bool initiallyFullSurface = false)
            : fullSurface(initiallyFullSurface) {
    }

    void invalidate(RenderInvalidationReason reason,
            const QRegion& logicalRegion = {}) {
        const bool wasFullSurface = fullSurface;
        if (reason == RenderInvalidationReason::FullSurface ||
                reason == RenderInvalidationReason::Unknown || logicalRegion.isEmpty()) {
            fullSurface = true;
            dirtyRegion = QRegion();
            invalidationReason = logicalRegion.isEmpty() &&
                            (reason == RenderInvalidationReason::Viewport ||
                                    reason == RenderInvalidationReason::SmallRegion)
                    ? RenderInvalidationReason::FullSurface
                    : reason;
        } else if (!wasFullSurface) {
            dirtyRegion += logicalRegion;
            invalidationReason = reason;
        }
    }

    void clipTo(const QRegion& logicalBounds) {
        if (fullSurface) {
            return;
        }
        dirtyRegion = dirtyRegion.intersected(logicalBounds);
        if (dirtyRegion.isEmpty()) {
            fullSurface = true;
            invalidationReason = RenderInvalidationReason::FullSurface;
        }
    }

    RenderInvalidationState take() {
        RenderInvalidationState pending = *this;
        reset();
        return pending;
    }

    void reset() {
        fullSurface = false;
        dirtyRegion = QRegion();
        invalidationReason = RenderInvalidationReason::Unknown;
    }

    bool fullSurface = false;
    QRegion dirtyRegion;
    RenderInvalidationReason invalidationReason =
            RenderInvalidationReason::Unknown;
};

inline QSize physicalSizeForLogicalSize(const QSize& logicalSize, qreal dpr) {
    if (!qIsFinite(dpr) || dpr <= 0.0) {
        dpr = 1.0;
    }
    return QSize(
            qMax(1, static_cast<int>(std::ceil(logicalSize.width() * dpr))),
            qMax(1, static_cast<int>(std::ceil(logicalSize.height() * dpr))));
}

} // namespace qml
} // namespace mixxx
