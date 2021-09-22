#pragma once

#include <QDateTime>

#include "audio/streaminfo.h"
#include "tagging/migration.h"
#include "track/albuminfo.h"
#include "track/trackinfo.h"

namespace mixxx {

class TaggingConfig;
class TagMappingConfig;

class TrackMetadata final {
    // Audio properties
    //  - read-only
    //  - stored in file tags
    //  - adjusted when opening the audio stream (if available)
    MIXXX_DECL_PROPERTY(audio::StreamInfo, streamInfo, StreamInfo)

    // Track properties
    //   - read-write
    //   - stored in file tags
    MIXXX_DECL_PROPERTY(AlbumInfo, albumInfo, AlbumInfo)
    MIXXX_DECL_PROPERTY(TrackInfo, trackInfo, TrackInfo)

    MIXXX_DECL_PROPERTY(Facets, facets, Facets)

  public:
    TrackMetadata() = default;
    TrackMetadata(TrackMetadata&&) = default;
    TrackMetadata(const TrackMetadata&) = default;
    /*non-virtual*/ ~TrackMetadata() = default;

    TrackMetadata& operator=(TrackMetadata&&) = default;
    TrackMetadata& operator=(const TrackMetadata&) = default;

    bool updateStreamInfoFromSource(
            const audio::StreamInfo& streamInfo);

    // Adjusts floating-point values to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport();

    // Returns true if the current metadata differs from the imported metadata
    // and needs to be exported. A result of false indicates that no export
    // is needed.
    // NOTE: Some tag formats like ID3v1/v2 only support integer precision
    // for storing bpm values. To avoid re-exporting unmodified track metadata
    // with fractional bpm values over and over again the comparison of bpm
    // values should be restricted to integer.
    bool anyFileTagsModified(
            const TrackMetadata& importedFromFile,
            Bpm::Comparison cmpBpm = Bpm::Comparison::Default) const;

    QString getBitrateText() const;

    double getDurationSecondsRounded() const {
        return std::round(getStreamInfo().getDuration().toDoubleSeconds());
    }
    QString getDurationText(
            Duration::Precision precision) const;

    // Custom tags and their corresponding text fields
    //
    // Every modification of custom tags will in turn also
    // update the corresponding text fields.
    bool hasFacets() const {
        return !getFacets().isEmpty();
    }
    bool updateFacets(
            const TaggingConfig& config,
            const mixxx::Facets& facets);
    bool mergeReplaceFacets(
            const TaggingConfig& config,
            const mixxx::Facets& facets);
    bool replaceCustomTag(
            const TaggingConfig& config,
            const Tag& tag,
            const TagFacetId& facetId = TagFacetId{});
    bool appendCustomTag(
            const TaggingConfig& config,
            const TagLabel& newLabel,
            const TagFacetId& facetId);
    bool removeCustomTag(
            const TaggingConfig& config,
            const TagLabel& oldLabel,
            const TagFacetId& facetId = TagFacetId{});

    bool updateTextFieldsFromFacets(
            const TaggingConfig& config);
    bool allTextFieldsSynchronizedWithFacets(
            const TaggingConfig& config) const;

    TagLabel joinFacetTextFromFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId) const;
    bool splitFacetTextIntoFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            const TagLabel::value_t& text);

    // Update text fields and the corresponding custom tags
    bool updateGenre(
            const TaggingConfig& config,
            const TagLabel::value_t& genre) {
        return updateTextField(
                config,
                library::tags::kFacetGenre,
                genre);
    }
#if defined(__EXTRA_METADATA__)
    bool updateMood(
            const TaggingConfig& config,
            const mixxx::TagLabel::value_t& mood) {
        return updateTextField(
                config,
                library::tags::kFacetMood,
                mood);
    }
#endif // __EXTRA_METADATA__

    // Parse an format date/time values according to ISO 8601
    static QDate parseDate(const QString& str) {
        return QDate::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QDateTime parseDateTime(const QString& str) {
        return QDateTime::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QString formatDate(QDate date) {
        return date.toString(Qt::ISODate);
    }
    static QString formatDateTime(const QDateTime& dateTime) {
        return dateTime.toString(Qt::ISODate);
    }

    // Parse and format the calendar year (for simplified display)
    static constexpr int kCalendarYearInvalid = 0;
    static int parseCalendarYear(const QString& year, bool* pValid = nullptr);
    static QString formatCalendarYear(const QString& year, bool* pValid = nullptr);

    static QString reformatYear(const QString& year);

  private:
    TagLabel::value_t* ptrTextField(
            const TagFacetId& facetId);

    // Complete the corresponding text fields or custom tags if either
    // of both is empty or missing respectively. Replace the custom tags
    // if inconsistent with their corresponding text field.
    //
    // Needed for both the initial migration of the genre text property
    // into custom genre tags and later for synchronization in case
    // one of them has been lost along the way.
    //
    // Returns true if track has been updated and false otherwise.
    friend class TrackRecord;
    bool synchronizeTextFieldsWithFacets(
            const TaggingConfig& config);

    bool isTextFieldSynchronizedWithFacets(
            const TaggingConfig& config,
            const TagFacetId& facetId) const;
    bool isTextFieldSynchronizedWithFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId) const;
    bool isTextFieldSynchronizedWithFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            const TagLabel::value_t& textField) const;

    bool synchronizeTextFieldWithFacets(
            const TaggingConfig& config,
            const TagFacetId& facetId);
    bool synchronizeTextFieldWithFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId);
    bool synchronizeTextFieldWithFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            TagLabel::value_t* pTextField);

    bool updateTextField(
            const TaggingConfig& config,
            const TagFacetId& facetId,
            const TagLabel::value_t& newText);
    bool updateTextField(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            const TagLabel::value_t& newText);
    bool updateTextField(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            const TagLabel::value_t& newText,
            TagLabel::value_t* pTextField);

    bool updateTextFieldFromFacets(
            const TaggingConfig& config,
            const TagFacetId& facetId);
    bool updateTextFieldFromFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId);
    bool updateTextFieldFromFacets(
            const TagMappingConfig& config,
            const TagFacetId& facetId,
            TagLabel::value_t* pTextField);
};

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs);

inline bool operator!=(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackMetadata)
