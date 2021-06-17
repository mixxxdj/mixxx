#include "tagging/taggingcontext.h"

#include "util/cmdlineargs.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("tagging.context");

const QString kConfigFileName = QStringLiteral("mixxx_tagging_config.json");

QString defaultConfigFilePath(
        const QString& fileName = kConfigFileName) {
    return QDir(CmdlineArgs::Instance().getSettingsPath()).filePath(fileName);
}

// Using the semicolon character ";" as a separator/delimiter for encoding
// multiple genre or mood values into a single text field is the de-facto
// standard that most music players adopted and recognize, e.g. MediaMonkey,
// MusicBee and others.
const TagMappingConfig kDefaultGenreTagMappingConfig =
        TagMappingConfig(QStringLiteral(";"), 0.75);
const TagMappingConfig kDefaultMoodTagMappingConfig =
        TagMappingConfig(QStringLiteral(";"), 0.75);

bool sanitizeTaggingConfig(
        TaggingConfig* pConfig,
        const QMap<TagFacet, QString>& defaultFacetDisplayNames) {
    VERIFY_OR_DEBUG_ASSERT(pConfig) {
        return false;
    }
    bool modified = false;
    for (const auto& reservedFacet : CustomTags::kReservedFacets) {
        const auto removedCount =
                pConfig->refCustomTags().removeAllFacetedTags(reservedFacet) +
                pConfig->refCustomFacets().remove(reservedFacet);
        if (removedCount > 0) {
            kLogger.info()
                    << "Removed reserved facet"
                    << pConfig->displayFacet(
                               reservedFacet,
                               defaultFacetDisplayNames.value(reservedFacet))
                    << "from custom configuration";
            modified = true;
        }
    }
    const std::pair<const TagFacet&, const TagMappingConfig&>
            facetsWithMandatoryTagMappingConfig[] = {
                    {CustomTags::kFacetGenre, kDefaultGenreTagMappingConfig},
                    {CustomTags::kFacetMood, kDefaultMoodTagMappingConfig}};
    for (const auto [facet, defaultConfig] : facetsWithMandatoryTagMappingConfig) {
        if (!pConfig->findFacetTagMapping(facet)) {
            kLogger.info()
                    << "Restoring default tag mapping configuration for"
                    << pConfig->displayFacet(
                               facet,
                               defaultFacetDisplayNames.value(facet));
            pConfig->refCustomFacets()[facet].setTagMapping(defaultConfig);
            modified = true;
        }
    }
    return modified;
}

} // anonymous namespace

TaggingContext::TaggingContext(
        QObject* parent)
        : QObject(parent),
          m_defaultFacetDisplayNames{
                  {CustomTags::kFacetGenre, tr("Genre")},
                  {CustomTags::kFacetMood, tr("Mood")},
                  {CustomTags::kFacetDecade, tr("Decade")},
                  {CustomTags::kFacetEthno, tr("Ethno")},
                  {CustomTags::kFacetLanguage, tr("Language")},
                  {CustomTags::kFacetVibe, tr("Vibe")},
                  {CustomTags::kFacetRating, tr("Rating")},
                  {CustomTags::kFacetArousal, tr("Arousal")},
                  {CustomTags::kFacetValence, tr("Valence")},
                  {CustomTags::kFacetAcousticness, tr("Acousticness")},
                  {CustomTags::kFacetDanceability, tr("Danceability")},
                  {CustomTags::kFacetInstrumentalness, tr("Instrumentalness")},
                  {CustomTags::kFacetLiveness, tr("Liveness")},
                  {CustomTags::kFacetPopularity, tr("Popularity")},
                  {CustomTags::kFacetSpeechiness, tr("Speechiness")},
          },
          m_configFilePath(defaultConfigFilePath()),
          m_config(loadOrCreateFileConfig(m_configFilePath)) {
}

const FacetedTagConfig* TaggingContext::findFacetConfig(
        const TagFacet& facet) const {
    const auto i = getConfig().getCustomFacets().find(facet);
    if (i != getConfig().getCustomFacets().end()) {
        // Return a pointer to the referenced map entry value
        const auto& ref = *i;
        return &ref;
    }
    return nullptr;
}

FacetedTagConfigMap TaggingContext::createDefaultFacetConfigs() const {
    return {
            {CustomTags::kFacetGenre, FacetedTagConfig{kDefaultGenreTagMappingConfig}},
            {CustomTags::kFacetMood, FacetedTagConfig{kDefaultMoodTagMappingConfig}},
    };
}

void TaggingContext::restoreDefaultFacetConfigs() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    refConfig().refCustomFacets().insert(
            createDefaultFacetConfigs());
#else
    const auto defaultFacetConfigs = createDefaultFacetConfigs();
    for (auto i = defaultFacetConfigs.begin();
            i != defaultFacetConfigs.end();
            ++i) {
        refConfig().refCustomFacets().insert(i.key(), i.value());
    }
#endif
    // No duplicates allowed, i.e. no multimap
    DEBUG_ASSERT(getConfig().getCustomFacets().size() ==
            getConfig().getCustomFacets().keys().size());
}

TaggingConfig TaggingContext::createDefaultConfig() const {
    auto defaultConfig = TaggingConfig(
            createDefaultFacetConfigs());
    // Predefined labels of non-faceted, simple tags
    const TagLabel::value_t tagLabels[] = {
            tr("Top40"),
    };
    for (const auto& tagLabel : tagLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(tagLabel)));
    }
    // Predefined decade labels
    const TagLabel::value_t decadeLabels[] = {
            tr("1960s"),
            tr("1970s"),
            tr("1980s"),
            tr("1990s"),
            tr("2000s"),
            tr("2010s"),
            tr("2020s"),
    };
    for (const auto& decadeLabel : decadeLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(decadeLabel)),
                CustomTags::kFacetDecade);
    }
    // Predefined ethno labels, adopted from Spotify
    const TagLabel::value_t ethnoLabels[] = {
            tr("Afro"),
            tr("Arab"),
            tr("Desi"),
    };
    for (const auto& ethnoLabel : ethnoLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(ethnoLabel)),
                CustomTags::kFacetEthno);
    }
    // Predefined genre labels
    // TODO: The expected huge number of different genre tags needs to be
    // displayed and mad accessible in a different way! Selecting genres
    // from a long list of maybe >100 entries is not feasible. This might
    // also apply to plain tags.
    const TagLabel::value_t genreLabels[] = {
            tr("Ambient"),
            tr("Classical"),
            tr("Country"),
            tr("Dance"),
            tr("Disco"),
            tr("Electronic"),
            tr("Folk"),
            tr("Funk"),
            tr("Hip Hop/Rap"),
            tr("Indie & Alternative"),
            tr("Jazz/Swing/Bigband"),
            tr("Pop"),
            tr("Punk"),
            tr("Reggae & Dancehall"),
            tr("Rock"),
            tr("R&B/Soul"),
            tr("Singer/Songwriter"),
            tr("Traditional"),
            tr("World"),
    };
    for (const auto& genreLabel : genreLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(genreLabel)),
                CustomTags::kFacetGenre);
    }
    // Predefined mood labels, adopted from Thayer's arousel-valence
    // emotion plane
    const TagLabel::value_t moodLabels[] = {
            // upper-right quadrant: energetic-positive
            tr("excited"),
            tr("happy"),
            tr("pleased"),
            // lower-right quadrant: calm-positive
            tr("relaxed"),
            tr("peaceful"),
            tr("calm"),
            // upper-left quadrant: energetic-negative
            tr("annoyed"),
            tr("angry"),
            tr("nervous"),
            // lower-left quadrant: calm-negative
            tr("sad"),
            tr("bored"),
            tr("sleepy"),
    };
    for (const auto& moodLabel : moodLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(moodLabel)),
                CustomTags::kFacetMood);
    }
    // Default vibe labels, roughly adopted from Spotify with some
    // customizations and additions
    const TagLabel::value_t vibeLabels[] = {
            tr("Campfire"),
            tr("Chill"),
            tr("Dinner"),
            tr("Focus"),
            tr("Holiday"), // xmas & new years eve
            tr("Lounge"),
            tr("Party"),
            tr("Romance"),
            tr("Sleep"),
            tr("Summer"),
            tr("Travel"), // roadtrip & cruising
            tr("Wellness"),
            tr("Workout"),
    };
    for (const auto& vibeLabel : vibeLabels) {
        defaultConfig.refCustomTags().addOrReplaceTag(
                Tag(TagLabel(vibeLabel)),
                CustomTags::kFacetVibe);
    }
    return defaultConfig;
}

TaggingConfig TaggingContext::loadOrCreateFileConfig(
        const QString& filePath) const {
    std::optional<TaggingConfig> config;
    if (QFile::exists(filePath)) {
        config = TaggingConfig::loadFromFile(m_configFilePath);
        if (!config) {
            kLogger.warning()
                    << "Failed to load configuration from file"
                    << filePath;
        }
    }
    bool newConfig = false;
    if (!config) {
        kLogger.info()
                << "Creating default configuration";
        config = createDefaultConfig();
        newConfig = true;
    }
    DEBUG_ASSERT(config);
    if (sanitizeTaggingConfig(&*config, m_defaultFacetDisplayNames) || newConfig) {
        config->saveIntoFile(filePath);
    }
    return *config;
}

bool TaggingContext::sanitizeConfig() {
    return sanitizeTaggingConfig(ptrConfig(), m_defaultFacetDisplayNames);
}

bool TaggingContext::reloadConfigFile(bool strict) {
    auto optConfig = TaggingConfig::loadFromFile(configFilePath(), strict);
    if (!optConfig) {
        kLogger.warning()
                << "Failed to reload configuration from file"
                << configFilePath();
        return false;
    }
    refConfig() = std::move(*optConfig);
    return true;
}

bool TaggingContext::saveConfigFile() const {
    if (!getConfig().saveIntoFile(configFilePath())) {
        kLogger.warning()
                << "Failed to save configuration into file"
                << configFilePath();
        return false;
    }
    return true;
}

} // namespace mixxx
