#include "aoide/json/tag.h"

#include <QRegularExpression>

#include "util/assert.h"

namespace aoide {

/*extern*/ const mixxx::TagFacet kTagFacetCrate = mixxx::TagFacet(QStringLiteral("crate"));
/*extern*/ const mixxx::TagFacet kTagFacetCrowd = mixxx::TagFacet(QStringLiteral("crowd"));
/*extern*/ const mixxx::TagFacet kTagFacetDecade = mixxx::TagFacet(QStringLiteral("decade"));
/*extern*/ const mixxx::TagFacet kTagFacetEvent = mixxx::TagFacet(QStringLiteral("event"));
/*extern*/ const mixxx::TagFacet kTagFacetStyle = mixxx::TagFacet(QStringLiteral("style"));
/*extern*/ const mixxx::TagFacet kTagFacetVenue = mixxx::TagFacet(QStringLiteral("venue"));
/*extern*/ const mixxx::TagFacet kTagFacetVibe = mixxx::TagFacet(QStringLiteral("vibe"));

namespace json {

mixxx::TagFacet TagFacetCount::facet() const {
    return mixxx::TagFacet(m_jsonArray.at(0).toString());
}

quint64 TagFacetCount::count() const {
    return m_jsonArray.at(1).toVariant().toULongLong();
}

mixxx::TagFacet TagCount::facet() const {
    const auto facet = mixxx::TagFacet(
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
