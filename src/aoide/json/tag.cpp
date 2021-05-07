#include "aoide/json/tag.h"

#include <QRegularExpression>

#include "util/assert.h"

namespace aoide {

//extern
const mixxx::TagFacetId kTagFacetCrate =
        mixxx::TagFacetId::staticConst(QStringLiteral("crate"));
//extern
const mixxx::TagFacetId kTagFacetCrowd =
        mixxx::TagFacetId::staticConst(QStringLiteral("crowd"));
//extern
const mixxx::TagFacetId kTagFacetDecade =
        mixxx::TagFacetId::staticConst(QStringLiteral("decade"));
//extern
const mixxx::TagFacetId kTagFacetEvent =
        mixxx::TagFacetId::staticConst(QStringLiteral("event"));
//extern
const mixxx::TagFacetId kTagFacetStyle =
        mixxx::TagFacetId::staticConst(QStringLiteral("style"));
//extern
const mixxx::TagFacetId kTagFacetVenue =
        mixxx::TagFacetId::staticConst(QStringLiteral("venue"));
//extern
const mixxx::TagFacetId kTagFacetVibe =
        mixxx::TagFacetId::staticConst(QStringLiteral("vibe"));

namespace json {

mixxx::TagFacetId TagFacetCount::facetId() const {
    return mixxx::TagFacetId(m_jsonArray.at(0).toString());
}

quint64 TagFacetCount::count() const {
    return m_jsonArray.at(1).toVariant().toULongLong();
}

mixxx::TagFacetId TagCount::facetId() const {
    const auto facet = mixxx::TagFacetId(
            m_jsonObject.value(QLatin1String("fct")).toString());
    DEBUG_ASSERT(facet.isEmpty() || facet.isValid());
    return facet;
}

mixxx::TagLabel TagCount::label() const {
    const auto label = mixxx::TagLabel(
            m_jsonObject.value(QLatin1String("lbl")).toString());
    DEBUG_ASSERT(label.isEmpty() || label.isValid());
    return label;
}

mixxx::TagScore TagCount::avgScore() const {
    const auto avgScore = mixxx::TagScore(
            m_jsonObject.value(QLatin1String("avg")).toDouble(mixxx::TagScore::kDefaultValue));
    DEBUG_ASSERT(avgScore.isValid());
    return avgScore;
}

quint64 TagCount::count() const {
    return m_jsonObject.value(QLatin1String("cnt")).toVariant().toULongLong();
}

} // namespace json

} // namespace aoide
