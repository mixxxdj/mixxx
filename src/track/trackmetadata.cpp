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
            getCustomTags() != importedFromFile.getCustomTags();
}

TagLabel::value_t* TrackMetadata::ptrTextField(
        const TagFacet& facet) {
    if (facet == CustomTags::kFacetGenre) {
        return refTrackInfo().ptrGenre();
    }
#if defined(__EXTRA_METADATA__)
    if (facet == CustomTags::kFacetMood) {
        return refTrackInfo().ptrMood();
    }
#endif
    return nullptr;
}

bool TrackMetadata::synchronizeTextFieldsWithCustomTags(
        const TaggingConfig& config) {
    bool updated = false;
    updated |= synchronizeTextFieldWithCustomTags(
            config,
            CustomTags::kFacetGenre);
#if defined(__EXTRA_METADATA__)
    updated |= synchronizeTextFieldWithCustomTags(
            config,
            CustomTags::kFacetMood);
#endif // __EXTRA_METADATA__
    DEBUG_ASSERT(allTextFieldsSynchronizedWithCustomTags(config));
    return updated;
}

bool TrackMetadata::synchronizeTextFieldWithCustomTags(
        const TaggingConfig& config,
        const TagFacet& facet) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then both the custom
        // tags and the corresponding text field are independent
        // and don't need to be synchronized. Just leave them
        // unmodified here and silently return.
        return false;
    }
    return synchronizeTextFieldWithCustomTags(
            *pFacetConfig,
            facet);
}

bool TrackMetadata::synchronizeTextFieldWithCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet) {
    return synchronizeTextFieldWithCustomTags(
            config,
            facet,
            ptrTextField(facet));
}

bool TrackMetadata::synchronizeTextFieldWithCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet,
        TagLabel::value_t* pTextField) {
    VERIFY_OR_DEBUG_ASSERT(pTextField) {
        return false;
    }
    const auto joinedText =
            joinFacetTextFromCustomTags(
                    config,
                    facet);
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
                << "for facet"
                << facet
                << "by splitting"
                << *pTextField
                << "into tags according to tag mapping configuration"
                << config;
    }
    return splitFacetTextIntoCustomTags(
            config,
            facet,
            *pTextField);
}

bool TrackMetadata::isTextFieldSynchronizedWithCustomTags(
        const TaggingConfig& config,
        const TagFacet& facet) const {
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then the text field
        // is trivially synchronized with the corresponding
        // custom tags, because both are independent and thereby
        // always consistent!
        return true;
    }
    return isTextFieldSynchronizedWithCustomTags(
            *pFacetConfig,
            facet);
}

bool TrackMetadata::isTextFieldSynchronizedWithCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet) const {
    if (facet == CustomTags::kFacetGenre) {
        return isTextFieldSynchronizedWithCustomTags(
                config,
                facet,
                getTrackInfo().getGenre());
    }
#if defined(__EXTRA_METADATA__)
    if (facet == CustomTags::kFacetMood) {
        return isTextFieldSynchronizedWithCustomTags(
                config,
                facet,
                getTrackInfo().getMood());
    }
#endif
    // All other facets are implicitly synchronized
    return true;
}

bool TrackMetadata::isTextFieldSynchronizedWithCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet,
        const TagLabel::value_t& textField) const {
    return textField == joinFacetTextFromCustomTags(config, facet);
}

bool TrackMetadata::allTextFieldsSynchronizedWithCustomTags(
        const TaggingConfig& config) const {
    if (!isTextFieldSynchronizedWithCustomTags(
                config,
                CustomTags::kFacetGenre)) {
        return false;
    }
#if defined(__EXTRA_METADATA__)
    if (!isTextFieldSynchronizedWithCustomTags(
                config,
                CustomTags::kFacetMood)) {
        return false;
    }
#endif // __EXTRA_METADATA__
    return true;
}

bool TrackMetadata::updateTextField(
        const TaggingConfig& config,
        const TagFacet& facet,
        const TagLabel::value_t& newText) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        // If no tag mapping is defined then both the tags
        // and the corresponding text field are independent.
        // The text field doesn't need to be updated after
        // tags have been modified.
        return false;
    }
    return updateTextField(
            *pFacetConfig,
            facet,
            newText);
}

bool TrackMetadata::updateTextField(
        const TagMappingConfig& config,
        const TagFacet& facet,
        const TagLabel::value_t& newText) {
    return updateTextField(
            config,
            facet,
            newText,
            ptrTextField(facet));
}

bool TrackMetadata::updateTextField(
        const TagMappingConfig& config,
        const TagFacet& facet,
        const TagLabel::value_t& newText,
        TagLabel::value_t* pTextField) {
    VERIFY_OR_DEBUG_ASSERT(pTextField) {
        return false;
    }
    if (*pTextField == newText) {
        DEBUG_ASSERT(isTextFieldSynchronizedWithCustomTags(
                config,
                facet,
                *pTextField));
        return false;
    }
    *pTextField = newText;
    splitFacetTextIntoCustomTags(
            config,
            facet,
            newText);
    return true;
}

bool TrackMetadata::updateTextFieldsFromCustomTags(
        const TaggingConfig& config) {
    bool updated = false;
    updated |= updateTextFieldFromCustomTags(
            config,
            CustomTags::kFacetGenre);
#if defined(__EXTRA_METADATA__)
    updated |= updateTextFieldFromCustomTags(
            config,
            CustomTags::kFacetMood);
#endif // __EXTRA_METADATA__
    DEBUG_ASSERT(allTextFieldsSynchronizedWithCustomTags(config));
    return updated;
}

bool TrackMetadata::updateTextFieldFromCustomTags(
        const TaggingConfig& config,
        const TagFacet& facet) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        return false;
    }
    return updateTextFieldFromCustomTags(
            *pFacetConfig,
            facet);
}

bool TrackMetadata::updateTextFieldFromCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet) {
    if (facet == CustomTags::kFacetGenre) {
        return updateTextFieldFromCustomTags(
                config,
                facet,
                refTrackInfo().ptrGenre());
#if defined(__EXTRA_METADATA__)
    } else if (facet == CustomTags::kFacetMood) {
        return updateTextFieldFromCustomTags(
                config,
                facet,
                refTrackInfo().ptrMood());
#endif // __EXTRA_METADATA__
    }
    return false;
}

bool TrackMetadata::updateTextFieldFromCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet,
        TagLabel::value_t* pTextField) {
    DEBUG_ASSERT(pTextField);
    const auto joinedText = joinFacetTextFromCustomTags(
            config,
            facet);
    if (*pTextField == joinedText) {
        return false;
    }
    *pTextField = joinedText;
    return true;
}

TagLabel TrackMetadata::joinFacetTextFromCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet) const {
    return getCustomTags().joinFacetedTagsLabel(
            facet,
            config.getLabelSeparator());
}

bool TrackMetadata::splitFacetTextIntoCustomTags(
        const TagMappingConfig& config,
        const TagFacet& facet,
        const TagLabel::value_t& text) {
    const auto oldTagsOrdered =
            getCustomTags().getFacetedTagsOrdered(facet);
    const auto newTagsOrdered =
            config.splitLabelIntoOrderedTags(text);
    if (oldTagsOrdered == newTagsOrdered) {
        return false;
    }
    auto& refTags = refCustomTags().refFacetedTags(facet);
    refTags.clear();
    for (const auto& tag : newTagsOrdered) {
        refTags.insert(
                tag.getLabel(),
                tag.getScore());
    }
    return true;
}

bool TrackMetadata::updateCustomTags(
        const TaggingConfig& config,
        const CustomTags& customTags) {
    if (customTags == getCustomTags()) {
        return false;
    }
    refCustomTags() = customTags;
    updateTextFieldsFromCustomTags(
            config);
    return true;
}

bool TrackMetadata::mergeReplaceCustomTags(
        const TaggingConfig& config,
        const CustomTags& customTags) {
    if (!refCustomTags().addOrReplaceAllTags(customTags)) {
        return false;
    }
    updateTextFieldsFromCustomTags(
            config);
    return true;
}

bool TrackMetadata::replaceCustomTag(
        const TaggingConfig& config,
        const Tag& tag,
        const TagFacet& facet) {
    if (!refCustomTags().addOrReplaceTag(
                tag,
                facet)) {
        return false;
    }
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    if (pFacetConfig) {
        updateTextFieldFromCustomTags(
                *pFacetConfig,
                facet);
    }
    return true;
}

bool TrackMetadata::appendCustomTag(
        const TaggingConfig& config,
        const TagLabel& newLabel,
        const TagFacet& facet) {
    const auto* const pFacetConfig = config.findFacetTagMapping(facet);
    VERIFY_OR_DEBUG_ASSERT(pFacetConfig) {
        return false;
    }
    if (!pFacetConfig->appendLabelToOrderedCustomTagsAndRescore(
                ptrCustomTags(),
                facet,
                newLabel)) {
        return false;
    }
    updateTextFieldFromCustomTags(
            *pFacetConfig,
            facet);
    return true;
}

bool TrackMetadata::removeCustomTag(
        const TaggingConfig& config,
        const TagLabel& oldLabel,
        const TagFacet& facet) {
    if (!facet.isEmpty()) {
        const auto* const pFacetConfig = config.findFacetTagMapping(facet);
        if (pFacetConfig) {
            if (!pFacetConfig->removeLabelFromOrderedCustomTagsAndRescore(
                        ptrCustomTags(),
                        facet,
                        oldLabel)) {
                return false;
            }
            updateTextFieldFromCustomTags(
                    *pFacetConfig,
                    facet);
            return true;
        }
    }
    return refCustomTags().removeTag(
            oldLabel,
            facet);
}

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return lhs.getStreamInfo() == rhs.getStreamInfo() &&
            lhs.getAlbumInfo() == rhs.getAlbumInfo() &&
            lhs.getTrackInfo() == rhs.getTrackInfo() &&
            lhs.getCustomTags() == rhs.getCustomTags();
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg) {
    dbg << "TrackMetadata{";
    arg.dbgStreamInfo(dbg);
    arg.dbgTrackInfo(dbg);
    arg.dbgAlbumInfo(dbg);
    arg.dbgCustomTags(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
