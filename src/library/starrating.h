#pragma once

#include <QMetaType>
#include <QPolygonF>
#include <QSize>

QT_FORWARD_DECLARE_CLASS(QPainter);
QT_FORWARD_DECLARE_CLASS(QRect);

/*
 * The StarRating class represents a rating as a number of stars.
 * In addition to holding the data, it is also capable of painting the stars on a QPaintDevice,
 * which in this example is either a view or an editor.
 * The myStarCount member variable stores the current rating, and myMaxStarCount stores
 * the highest possible rating (typically 5).
 */
class StarRating {
  public:
    enum EditMode { Editable, ReadOnly };

    StarRating(int starCount = 1, int maxStarCount = 5);

    void paint(QPainter* painter, const QRect& rect) const;
    QSize sizeHint() const;

    int starCount() const { return m_myStarCount; }
    int maxStarCount() const { return m_myMaxStarCount; }
    void setStarCount(int starCount) { m_myStarCount = starCount; }
    void setMaxStarCount(int maxStarCount) { m_myMaxStarCount = maxStarCount; }

  private:
    QPolygonF m_starPolygon;
    QPolygonF m_diamondPolygon;
    int m_myStarCount;
    int m_myMaxStarCount;
};

Q_DECLARE_METATYPE(StarRating)
