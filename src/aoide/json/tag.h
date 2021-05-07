#pragma once

#include "aoide/json/json.h"
#include "tagging/migration.h"
#include "util/math.h"

namespace aoide {

// General purpose facets
extern const mixxx::TagFacetId kTagFacetCrate;
extern const mixxx::TagFacetId kTagFacetCrowd;
extern const mixxx::TagFacetId kTagFacetEvent;
extern const mixxx::TagFacetId kTagFacetStyle;
extern const mixxx::TagFacetId kTagFacetVenue;

namespace json {

class Tags : public Object {
  public:
    explicit Tags(
            QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    std::optional<mixxx::Facets> toFacets() const {
        return mixxx::Facets::fromJsonObject(m_jsonObject);
    }
    static Tags fromFacets(
            const mixxx::Facets& facets) {
        return Tags(facets.toJsonObject());
    }
};

class TagFacetCount : public Array {
  public:
    explicit TagFacetCount(QJsonArray jsonArray = Array())
            : Array(std::move(jsonArray)) {
        DEBUG_ASSERT(m_jsonArray.size() == 2);
    }

    mixxx::TagFacetId facetId() const;

    quint64 count() const;
};

class TagCount : public Object {
  public:
    explicit TagCount(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    mixxx::TagFacetId facetId() const;

    mixxx::TagLabel label() const;

    // Tag with avg. score
    mixxx::TagScore avgScore() const;

    quint64 count() const;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::Tags);
Q_DECLARE_METATYPE(aoide::json::TagFacetCount);
Q_DECLARE_METATYPE(aoide::json::TagCount);
