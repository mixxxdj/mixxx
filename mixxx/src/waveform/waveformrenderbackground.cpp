
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>
#include <QLine>
#include <QPixmap>

#include "waveformrenderbackground.h"

#include "waveformwidgetrenderer.h"

#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderBackground::WaveformRenderBackground( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer),
    m_backgroungColor(0,0,0)
{
    m_backgroundPixmap = QPixmap();
}

void WaveformRenderBackground::init()
{
}

void WaveformRenderBackground::setup( const QDomNode& node)
{
    m_backgroungColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    m_backgroungColor = WSkinColor::getCorrectColor(m_backgroungColor);
    qDebug() << m_backgroungColor.name();
}

void WaveformRenderBackground::draw( QPainter* painter, QPaintEvent* /*event*/)
{
    if( isDirty())
        generatePixmap();

    painter->drawPixmap( QPoint(0,0), m_backgroundPixmap);
}

void WaveformRenderBackground::generatePixmap()
{
    qDebug() << "WaveformRenderBackground::generatePixmap";

    m_backgroundPixmap = QPixmap( m_waveformWidget->getWidth(), m_waveformWidget->getHeight());

    QLinearGradient linearGrad(QPointF(0,0), QPointF(0,m_waveformWidget->getHeight()));
    linearGrad.setColorAt(0.0, m_backgroungColor);
    linearGrad.setColorAt(0.5, m_backgroungColor.lighter(200));
    linearGrad.setColorAt(1.0, m_backgroungColor);

    QBrush brush(linearGrad);

    QPainter painter;
    painter.begin(&m_backgroundPixmap);
    painter.fillRect(m_backgroundPixmap.rect(), brush);
    painter.end();

    setDirty(false);
}
