#include "widget/wstatuslight.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "moc_wstatuslight.cpp"

WStatusLight::WStatusLight(QWidget * parent)
        : WWidget(parent),
          m_iPos(0) {
    setNoPos(0);
}

void WStatusLight::setNoPos(int iNoPos) {
    // If pixmap array is already allocated, delete it.
    if (!m_pixmaps.empty()) {
        // Clear references to existing pixmaps.
        m_pixmaps.resize(0);
    }

    // values less than 2 make no sense (need at least off, on)
    if (iNoPos < 2) {
        iNoPos = 2;
    }
    m_pixmaps.resize(iNoPos);
}

void WStatusLight::setup(const QDomNode& node, const SkinContext& context) {
    // Number of states. Add one to account for the background.
    setNoPos(context.selectInt(node, "NumberPos") + 1);

    // Set pixmaps
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        // Accept either PathStatusLight or PathStatusLight1 for value 1,
        QString nodeName = QString("PathStatusLight%1").arg(i);

        QDomElement statusLightNode;
        if (context.hasNodeSelectElement(node, nodeName, &statusLightNode) ||
                (i == 0 && context.hasNodeSelectElement(node, "PathBack", &statusLightNode)) ||
                (i == 1 && context.hasNodeSelectElement(node, "PathStatusLight", &statusLightNode))) {
            setPixmap(i, context.getPixmapSource(statusLightNode),
                      context.selectScaleMode(
                              statusLightNode,
                              Paintable::FIXED),
                              context.getScaleFactor());
        } else {
            m_pixmaps[i].clear();
        }
    }

    setFocusPolicy(Qt::NoFocus);
}

void WStatusLight::setPixmap(int iState,
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    if (iState < 0 || iState >= m_pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!pPixmap.isNull() && !pPixmap->isNull()) {
        m_pixmaps[iState] = pPixmap;
        if (mode == Paintable::FIXED) {
            setFixedSize(pPixmap->size());
        }
    } else {
        qDebug() << "WStatusLight: Error loading pixmap:" << source.getPath() << iState;
        m_pixmaps[iState].clear();
    }
}

void WStatusLight::onConnectedControlChanged(double dParameter, double dValue) {
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    Q_UNUSED(dParameter);
    int newPos = static_cast<int>(dValue);

    if (m_pixmaps.size() == 2) {
        // original behavior for two-state lights: any non-zero value is "on"
        newPos = newPos > 0 ? 1 : 0;
    } else if (newPos < m_pixmaps.size() && newPos >= 0) {
        // multi-state behavior: values lie within the correct ranges
    } else {
        qDebug() << "Warning: wstatuslight asked for invalid position:"
                 << newPos << "max val:" << m_pixmaps.size()-1;
        return;
    }

    if (newPos != m_iPos) {
        m_iPos = newPos;
        update();
    }
}

void WStatusLight::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_iPos < 0 || m_iPos >= m_pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = m_pixmaps[m_iPos];

    if (pPixmap.isNull() || pPixmap->isNull()) {
        return;
    }

    pPixmap->draw(rect(), &p);
}
