#include "util/colorcomponents.h"

void getHsvF(const QColor& color, float* pH, float* pS, float* pV, float* pA) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getHsvF(pH, pS, pV, pA);
#else
    qreal qh, qs, qv, qa;
    color.getHsvF(&qh, &qs, &qv, pA ? &qa : nullptr);
    *pH = static_cast<float>(qh);
    *pS = static_cast<float>(qs);
    *pV = static_cast<float>(qv);
    if (pA) {
        *pA = static_cast<float>(qa);
    }
#endif
}

void getHslF(const QColor& color, float* pH, float* pS, float* pL, float* pA) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getHslF(pH, pS, pL, pA);
#else
    qreal qh, qs, ql, qa;
    color.getHslF(&qh, &qs, &ql, pA ? &qa : nullptr);
    *pH = static_cast<float>(qh);
    *pS = static_cast<float>(qs);
    *pL = static_cast<float>(ql);
    if (pA) {
        *pA = static_cast<float>(qa);
    }
#endif
}

void getRgbF(const QColor& color, float* pR, float* pG, float* pB, float* pA) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getRgbF(pR, pG, pB, pA);
#else
    qreal qr, qg, qb, qa;
    color.getRgbF(&qr, &qg, &qb, pA ? &qa : nullptr);
    *pR = static_cast<float>(qr);
    *pG = static_cast<float>(qg);
    *pB = static_cast<float>(qb);
    if (pA) {
        *pA = static_cast<float>(qa);
    }
#endif
}
