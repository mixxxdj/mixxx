#pragma once

#include "aoide/json/json.h"
#include "tagging/customtags.h"
#include "util/math.h"

namespace aoide {

// General purpose facets
extern const mixxx::TagFacet kTagFacetCrate;
extern const mixxx::TagFacet kTagFacetCrowd;
extern const mixxx::TagFacet kTagFacetEvent;
extern const mixxx::TagFacet kTagFacetStyle;
extern const mixxx::TagFacet kTagFacetVenue;

namespace json {

class Tags : public Object {
  public:
    explicit Tags(
            QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    std::optional<mixxx::CustomTags> toCustomTags() const {
        return mixxx::CustomTags::fromJsonObject(m_jsonObject);
    }
    static Tags fromCustomTags(
            const mixxx::CustomTags& customTags) {
        return Tags(customTags.toJsonObject());
    }
};

class TagFacetCount : public Array {
  public:
    explicit TagFacetCount(QJsonArray jsonArray = Array())
            : Array(std::move(jsonArray)) {
        DEBUG_ASSERT(m_jsonArray.size() == 2);
    }

    mixxx::TagFacet facet() const;

    quint64 count() const;
};

class TagCount : public Object {
  public:
    explicit TagCount(QJsonObject jsonObject = QJsonObject())
            : Object(std::move(jsonObject)) {
    }

    mixxx::TagFacet facet() const;

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
