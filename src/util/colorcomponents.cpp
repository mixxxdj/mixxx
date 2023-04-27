#include "util/colorcomponents.h"

void getHsvF(const QColor& color, float* h, float* s, float* v, float* a) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getHsvF(h, s, v, a);
#else
    qreal qh, qs, qv, qa;
    color.getHsvF(&qh, &qs, &qv, a ? &qa : nullptr);
    *h = static_cast<float>(qh);
    *s = static_cast<float>(qs);
    *v = static_cast<float>(qv);
    if (a) {
        *a = static_cast<float>(qa);
    }
#endif
}

void getHslF(const QColor& color, float* h, float* s, float* l, float* a) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getHslF(h, s, l, a);
#else
    qreal qh, qs, ql, qa;
    color.getHslF(&qh, &qs, &ql, a ? &qa : nullptr);
    *h = static_cast<float>(qh);
    *s = static_cast<float>(qs);
    *l = static_cast<float>(ql);
    if (a) {
        *a = static_cast<float>(qa);
    }
#endif
}

void getRgbF(const QColor& color, float* r, float* g, float* b, float* a) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    color.getRgbF(r, g, b, a);
#else
    qreal qr, qg, qb, qa;
    color.getRgbF(&qr, &qg, &qb, a ? &qa : nullptr);
    *r = static_cast<float>(qr);
    *g = static_cast<float>(qg);
    *b = static_cast<float>(qb);
    if (a) {
        *a = static_cast<float>(qa);
    }
#endif
}
