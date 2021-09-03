#include "track/trackmetadata.h"

#include "audio/streaminfo.h"
#include "tagging/taggingconfig.h"
#include "tagging/tagmapping.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("TrackMetadata");

} // anonymous namespace

/*static*/ constexpr int TrackMetadata::kCalendarYearInvalid;

bool TrackMetadata::updateStreamInfoFromSource(
        const audio::StreamInfo& streamInfo) {
    if (getStreamInfo() == streamInfo) {
        return false;
    }
    const auto streamChannelCount =
            streamInfo.getSignalInfo().getChannelCount();
    if (streamChannelCount.isValid() &&
            streamChannelCount != getStreamInfo().getSignalInfo().getChannelCount() &&
            getStreamInfo().getSignalInfo().getChannelCount().isValid()) {
        kLogger.debug()
                << "Modifying channel count:"
                << getStreamInfo().getSignalInfo().getChannelCount()
                << "->"
                << streamChannelCount;
    }
    const auto streamSampleRate =
            streamInfo.getSignalInfo().getSampleRate();
    if (streamSampleRate.isValid() &&
            streamSampleRate != getStreamInfo().getSignalInfo().getSampleRate() &&
            getStreamInfo().getSignalInfo().getSampleRate().isValid()) {
        kLogger.debug()
                << "Modifying sample rate:"
                << getStreamInfo().getSignalInfo().getSampleRate()
                << "->"
                << streamSampleRate;
    }
    const auto streamBitrate =
            streamInfo.getBitrate();
    if (streamBitrate.isValid() &&
            streamBitrate != getStreamInfo().getBitrate() &&
            getStreamInfo().getSignalInfo().isValid()) {
        kLogger.debug()
                << "Modifying bitrate:"
                << getStreamInfo().getSignalInfo()
                << "->"
                << streamBitrate;
    }
    const auto streamDuration =
            streamInfo.getDuration();
    if (streamDuration > Duration::empty() &&
            streamDuration != getStreamInfo().getDuration() &&
            getStreamInfo().getDuration() > Duration::empty()) {
        kLogger.debug()
                << "Modifying duration:"
                << getStreamInfo().getDuration()
                << "->"
                << streamDuration;
    }
    setStreamInfo(streamInfo);
    return true;
}

QString TrackMetadata::getBitrateText() const {
    if (!getStreamInfo().getBitrate().isValid()) {
        return QString();
    }
    return QString::number(getStreamInfo().getBitrate());
}

QString TrackMetadata::getDurationText(
        Duration::Precision precision) const {
    double durationSeconds;
    if (precision == Duration::Precision::SECONDS) {
        // Round to full seconds before formatting for consistency
        // getDurationText() should always display the same number
        // as getDurationSecondsRounded()
        durationSeconds = getDurationSecondsRounded();
    } else {
        durationSeconds = getStreamInfo().getDuration().toDoubleSeconds();
    }
    return Duration::formatTime(durationSeconds, precision);
}

int TrackMetadata::parseCalendarYear(const QString& year, bool* pValid) {
    const QDateTime dateTime(parseDateTime(year));
    if (0 < dateTime.date().year()) {
        if (pValid) {
            *pValid = true;
        }
        return dateTime.date().year();
    } else {
        bool calendarYearValid = false;
        // Ignore everything beginning with the first dash '-'
        // to successfully parse the calendar year of incomplete
        // dates like yyyy-MM or 2015-W07.
        const QString calendarYearSection(year.section('-', 0, 0).trimmed());
        const int calendarYear = calendarYearSection.toInt(&calendarYearValid);
        if (calendarYearValid) {
            calendarYearValid = 0 < calendarYear;
        }
        if (pValid) {
            *pValid = calendarYearValid;
        }
        if (calendarYearValid) {
            return calendarYear;
        } else {
            return kCalendarYearInvalid;
        }
    }
}

QString TrackMetadata::formatCalendarYear(const QString& year, bool* pValid) {
    bool calendarYearValid = false;
    int calendarYear = parseCalendarYear(year, &calendarYearValid);
    if (pValid) {
        *pValid = calendarYearValid;
    }
    if (calendarYearValid) {
        return QString::number(calendarYear);
    } else {
        return QString(); // empty string
    }
}

QString TrackMetadata::reformatYear(const QString& year) {
    const QDateTime dateTime(parseDateTime(year));
    if (dateTime.isValid()) {
        // date/time
        return formatDateTime(dateTime);
    }
    const QDate date(dateTime.date());
    if (date.isValid()) {
        // only date
        return formatDate(date);
    }
    bool calendarYearValid = false;
    const QString calendarYear(formatCalendarYear(year, &calendarYearValid));
    if (calendarYearValid) {
        // only calendar year
        return calendarYear;
    }
    // just trim and simplify whitespaces
    return year.simplified();
}

void TrackMetadata::normalizeBeforeExport() {
    m_albumInfo.normalizeBeforeExport();
    m_trackInfo.normalizeBeforeExport();
    // FIXME(uklotzde): Do not discard facets after import/export of
    // file tags has been finalized. Until then we must ignore the
    // facets to avoid detecting tracks as modified again and again!
    m_facets = {};
}

bool TrackMetadata::anyFileTagsModified(
        const TrackMetadata& importedFromFile,
        Bpm::Comparison cmpBpm) const {
    // NOTE(uklotzde): The read-only audio properties that are stored
    // directly as members of this class might differ after they have
    // been updated while decoding audio data. They are read-only and
    // must not be considered when exporting metadata!
    return getAlbumInfo() != importedFromFile.getAlbumInfo() ||
            !getTrackInfo().compareEq(importedFromFile.getTrackInfo(), cmpBpm) ||
            getFacets() != importedFromFile.getFacets();
}

TagLabel::value_t* TrackMetadata::ptrTextField(
        const TagFacetId& facetId) {
    if (facetId == library::tags::kFacetGenre) {
        return refTrackInfo().ptrGenre();
    }
#if defined(__EXTRA_METADATA__)
    if (facetId == library::tags::kFacetMood) {
        return refTrackInfo().ptrMood();
    }
#endif
    return nullptr;
}

bool TrackMetadata::synchronizeTextFieldsWithFacets(
        const TaggingConfig& config) {
    bool updated = false;
    updated |= synchronizeTextFieldWithFacets(
            config,
            library::tags::kFacetGenre);
#if defined(__EXTRA_METADATA__)
    updated |= synchronizeTextFieldWithFacets(
            config,
            library::tags::kFacetMood);
#endif // __EXTRA_METADATA__
    DEBUG_ASSERT(allTextFieldsSynchronizedWithFacets(config));
    return updated;
}

bool TrackMetadata::synchronizeTextFieldWithFacets(
        const TaggingConfig& config,
        const TagFacetId& facetId) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then both the custom
        // tags and the corresponding text field are independent
        // and don't need to be synchronized. Just leave them
        // unmodified here and silently return.
        return false;
    }
    return synchronizeTextFieldWithFacets(
            *pFacetConfig,
            facetId);
}

bool TrackMetadata::synchronizeTextFieldWithFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId) {
    return synchronizeTextFieldWithFacets(
            config,
            facetId,
            ptrTextField(facetId));
}

bool TrackMetadata::synchronizeTextFieldWithFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        TagLabel::value_t* pTextField) {
    VERIFY_OR_DEBUG_ASSERT(pTextField) {
        return false;
    }
    const auto joinedText =
            joinFacetTextFromFacets(
                    config,
                    facetId);
    if (*pTextField == joinedText) {
        return false;
    }
    if (pTextField->isEmpty()) {
        *pTextField = joinedText;
        return true;
    }
    if (!joinedText.isEmpty()) {
        // The joined text and the field text are both not empty and
        // inconsistent. The field text might have been edited by an
        // external application. In this case the custom facet tags
        // are replaced, even if they already exist.
        // Note: This also depends on the current tag mapping configuration,
        // but we cannot decide if this still matches the contents of the
        // text field! The user is responsible for not messing this up.
        kLogger.info()
                << "Replacing inconsistent custom tags text"
                << joinedText
                << "for facet identifier"
                << facetId
                << "by splitting"
                << *pTextField
                << "into tags according to tag mapping configuration"
                << config;
    }
    return splitFacetTextIntoFacets(
            config,
            facetId,
            *pTextField);
}

bool TrackMetadata::isTextFieldSynchronizedWithFacets(
        const TaggingConfig& config,
        const TagFacetId& facetId) const {
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then the text field
        // is trivially synchronized with the corresponding
        // custom tags, because both are independent and thereby
        // always consistent!
        return true;
    }
    return isTextFieldSynchronizedWithFacets(
            *pFacetConfig,
            facetId);
}

bool TrackMetadata::isTextFieldSynchronizedWithFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId) const {
    if (facetId == library::tags::kFacetGenre) {
        return isTextFieldSynchronizedWithFacets(
                config,
                facetId,
                getTrackInfo().getGenre());
    }
#if defined(__EXTRA_METADATA__)
    if (facetId == library::tags::kFacetMood) {
        return isTextFieldSynchronizedWithFacets(
                config,
                facetId,
                getTrackInfo().getMood());
    }
#endif
    // All other facets are implicitly synchronized
    return true;
}

bool TrackMetadata::isTextFieldSynchronizedWithFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        const TagLabel::value_t& textField) const {
    const auto joinedText = joinFacetTextFromFacets(config, facetId);
    // Special case: Missing tracks might not have custom any tags.
    // If no custom tags are available the corresponding text
    // field is considered synchronized.
    return joinedText == QString{} || textField == joinedText;
}

bool TrackMetadata::allTextFieldsSynchronizedWithFacets(
        const TaggingConfig& config) const {
    if (!isTextFieldSynchronizedWithFacets(
                config,
                library::tags::kFacetGenre)) {
        return false;
    }
#if defined(__EXTRA_METADATA__)
    if (!isTextFieldSynchronizedWithFacets(
                config,
                library::tags::kFacetMood)) {
        return false;
    }
#endif // __EXTRA_METADATA__
    return true;
}

bool TrackMetadata::updateTextField(
        const TaggingConfig& config,
        const TagFacetId& facetId,
        const TagLabel::value_t& newText) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then both the tags
        // and the corresponding text field are independent.
        // The text field doesn't need to be updated after
        // tags have been modified.
        return false;
    }
    return updateTextField(
            *pFacetConfig,
            facetId,
            newText);
}

bool TrackMetadata::updateTextField(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        const TagLabel::value_t& newText) {
    return updateTextField(
            config,
            facetId,
            newText,
            ptrTextField(facetId));
}

bool TrackMetadata::updateTextField(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        const TagLabel::value_t& newText,
        TagLabel::value_t* pTextField) {
    VERIFY_OR_DEBUG_ASSERT(pTextField) {
        return false;
    }
    if (*pTextField == newText) {
        DEBUG_ASSERT(isTextFieldSynchronizedWithFacets(
                config,
                facetId,
                *pTextField));
        return false;
    }
    *pTextField = newText;
    splitFacetTextIntoFacets(
            config,
            facetId,
            newText);
    return true;
}

bool TrackMetadata::updateTextFieldsFromFacets(
        const TaggingConfig& config) {
    bool updated = false;
    updated |= updateTextFieldFromFacets(
            config,
            library::tags::kFacetGenre);
#if defined(__EXTRA_METADATA__)
    updated |= updateTextFieldFromFacets(
            config,
            library::tags::kFacetMood);
#endif // __EXTRA_METADATA__
    DEBUG_ASSERT(allTextFieldsSynchronizedWithFacets(config));
    return updated;
}

bool TrackMetadata::updateTextFieldFromFacets(
        const TaggingConfig& config,
        const TagFacetId& facetId) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        return false;
    }
    return updateTextFieldFromFacets(
            *pFacetConfig,
            facetId);
}

bool TrackMetadata::updateTextFieldFromFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId) {
    if (facetId == library::tags::kFacetGenre) {
        return updateTextFieldFromFacets(
                config,
                facetId,
                refTrackInfo().ptrGenre());
#if defined(__EXTRA_METADATA__)
    } else if (facetId == library::tags::kFacetMood) {
        return updateTextFieldFromFacets(
                config,
                facetId,
                refTrackInfo().ptrMood());
#endif // __EXTRA_METADATA__
    }
    return false;
}

bool TrackMetadata::updateTextFieldFromFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        TagLabel::value_t* pTextField) {
    DEBUG_ASSERT(pTextField);
    const auto joinedText = joinFacetTextFromFacets(
            config,
            facetId);
    if (*pTextField == joinedText) {
        return false;
    }
    *pTextField = joinedText;
    return true;
}

TagLabel TrackMetadata::joinFacetTextFromFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId) const {
    return getFacets().joinTagsLabel(
            config.getLabelSeparator(),
            facetId);
}

bool TrackMetadata::splitFacetTextIntoFacets(
        const TagMappingConfig& config,
        const TagFacetId& facetId,
        const TagLabel::value_t& text) {
    const auto oldTagsOrdered =
            getFacets().collectTagsOrdered(
                    Facets::ScoreOrdering::Descending,
                    facetId);
    const auto newTagsOrdered =
            config.splitLabelIntoOrderedTags(text);
    if (oldTagsOrdered == newTagsOrdered) {
        return false;
    }
    auto& refTags = refFacets().refTags(facetId);
    refTags.clear();
    for (const auto& tag : newTagsOrdered) {
        refTags.insert(
                tag.getLabel(),
                tag.getScore());
    }
    return true;
}

bool TrackMetadata::updateFacets(
        const TaggingConfig& config,
        const Facets& facets) {
    if (facets == getFacets()) {
        return false;
    }
    refFacets() = facets;
    updateTextFieldsFromFacets(config);
    return true;
}

bool TrackMetadata::mergeReplaceFacets(
        const TaggingConfig& config,
        const Facets& facets) {
    if (ptrFacets() == &facets) {
        return false;
    }
    refFacets().addOrReplaceAllFacets(facets);
    updateTextFieldsFromFacets(config);
    return true;
}

bool TrackMetadata::replaceCustomTag(
        const TaggingConfig& config,
        const Tag& tag,
        const TagFacetId& facetId) {
    if (!refFacets().addOrUpdateTag(
                tag,
                facetId)) {
        return false;
    }
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    if (pFacetConfig) {
        updateTextFieldFromFacets(
                *pFacetConfig,
                facetId);
    }
    return true;
}

bool TrackMetadata::appendCustomTag(
        const TaggingConfig& config,
        const TagLabel& newLabel,
        const TagFacetId& facetId) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        return false;
    }
    if (!pFacetConfig->appendLabelToOrderedFacetsAndRescore(
                ptrFacets(),
                facetId,
                newLabel)) {
        return false;
    }
    updateTextFieldFromFacets(
            *pFacetConfig,
            facetId);
    return true;
}

bool TrackMetadata::removeCustomTag(
        const TaggingConfig& config,
        const TagLabel& oldLabel,
        const TagFacetId& facetId) {
    if (!facetId.isEmpty()) {
        const auto* const pFacetConfig = config.findFacetTagMapping(facetId);
        if (pFacetConfig) {
            if (!pFacetConfig->removeLabelFromOrderedFacetsAndRescore(
                        ptrFacets(),
                        facetId,
                        oldLabel)) {
                return false;
            }
            updateTextFieldFromFacets(
                    *pFacetConfig,
                    facetId);
            return true;
        }
    }
    return refFacets().removeTagLabeled(
            oldLabel,
            facetId);
}

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return lhs.getStreamInfo() == rhs.getStreamInfo() &&
            lhs.getAlbumInfo() == rhs.getAlbumInfo() &&
            lhs.getTrackInfo() == rhs.getTrackInfo() &&
            lhs.getFacets() == rhs.getFacets();
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg) {
    dbg << "TrackMetadata{";
    arg.dbgStreamInfo(dbg);
    arg.dbgTrackInfo(dbg);
    arg.dbgAlbumInfo(dbg);
    arg.dbgFacets(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
