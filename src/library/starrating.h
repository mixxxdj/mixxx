#pragma once

#include <QMetaType>
#include <QPolygonF>
#include <QSize>

#include "track/trackrecord.h"

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

    static constexpr int kMinStarCount = 0;

    explicit StarRating(
            int starCount = kMinStarCount,
            int maxStarCount = mixxx::TrackRecord::kMaxRating - mixxx::TrackRecord::kMinRating);

    void paint(QPainter* painter, const QRect& rect) const;
    QSize sizeHint() const;

    int starCount() const {
        return m_starCount;
    }
    int maxStarCount() const {
        return m_maxStarCount;
    }
    void setStarCount(int starCount) {
        DEBUG_ASSERT(starCount >= kMinStarCount);
        DEBUG_ASSERT(starCount <= m_maxStarCount);
        m_starCount = starCount;
    }

  private:
    QPolygonF m_starPolygon;
    QPolygonF m_diamondPolygon;
    int m_starCount;
    int m_maxStarCount;
};

Q_DECLARE_METATYPE(StarRating)
