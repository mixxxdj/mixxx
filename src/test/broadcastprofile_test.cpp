#ifdef __BROADCAST__

#include <QFile>
#include <QString>

#include "test/mixxxtest.h"
#include "preferences/broadcastprofile.h"
#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"

namespace {

TEST(BroadcastProfileTest, ConstructWithName) {
    // instantiate BroadcastProfile with a specific name and
    // assert its case-sensitive equality when getting it from getProfileName
    QString name("unit testing profile");
    BroadcastProfile profile(name);
    ASSERT_TRUE(profile.getProfileName() == name);
}

TEST(BroadcastProfileTest, ForbiddenChars) {
    // Test if validName works properly with valid values
    ASSERT_TRUE(BroadcastProfile::validName("Default Profile"));
    ASSERT_TRUE(BroadcastProfile::validName("This is a profile"));
    ASSERT_TRUE(BroadcastProfile::validName("Radio1 - MP3 128k"));
    ASSERT_TRUE(BroadcastProfile::validName("Radio1 - AACplus 96k"));

    // Test if validName works properly with invalid values
    ASSERT_FALSE(BroadcastProfile::validName("Radio1 / MP3 128k"));
    ASSERT_FALSE(BroadcastProfile::validName("Radio1 \\ AAC+ 96k"));
    ASSERT_FALSE(BroadcastProfile::validName("Radio1 | OGG 96k"));
    ASSERT_FALSE(BroadcastProfile::validName("Hello ?"));
    ASSERT_FALSE(BroadcastProfile::validName("3 * 3"));
    ASSERT_FALSE(BroadcastProfile::validName("Here it is: a profile"));
    ASSERT_FALSE(BroadcastProfile::validName("<marquee>Scrolltext<marquee/>"));
    ASSERT_FALSE(BroadcastProfile::validName("It's called a \"profile\"."));

    // Test if forbidden chars are properly stripped
    ASSERT_FALSE(BroadcastProfile::stripForbiddenChars(
                        "This is an invalid profile name: ?/").contains("?"));
}

TEST(BroadcastProfileTest, SaveAndLoadXML) {
    // Preliminary: set a discriminating value in one of the profile fields
    QString streamName("unit testing in progress");

    BroadcastProfile profile("Broadcast Profile test");
    profile.setStreamName(streamName);

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    QString filename = tempDir.filePath(profile.getProfileName() + QString(".bcp.xml"));

    profile.save(filename);
    ASSERT_TRUE(QFile::exists(filename));

    // Load XML file using static loadFromFile and assert
    // the discriminating value is present
    BroadcastProfilePtr savedProfile = BroadcastProfile::loadFromFile(filename);
    EXPECT_NE(savedProfile, nullptr);
    EXPECT_TRUE(savedProfile->getStreamName() == streamName);
}

TEST(BroadcastProfileTest, SaveAndLoadXMLDotName) {
    QString profileName("broadcast profile has a dot. (in the name) test");
    BroadcastProfile profile(profileName);

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    QString filename = tempDir.filePath(profile.getProfileName() + QString(".bcp.xml"));

    profile.save(filename);
    ASSERT_TRUE(QFile::exists(filename));

    // Load XML file using static loadFromFile and assert
    // the discriminating value is present
    BroadcastProfilePtr savedProfile = BroadcastProfile::loadFromFile(filename);
    EXPECT_NE(savedProfile, nullptr);
    EXPECT_TRUE(savedProfile->getProfileName() == profileName);
}

TEST(BroadcastProfileTest, SetGetValues) {
    // For each attribute:
    // - use its setter to set a specific value
    // - use its getter to check if the specific value has been set

    BroadcastProfile profile("Get and set values Test");

    // Long method ahead!
    QString profileName("new name");
    profile.setProfileName(profileName);
    ASSERT_TRUE(profile.getProfileName() == profileName);

    bool secureCredentials = true;
    profile.setSecureCredentialStorage(secureCredentials);
    ASSERT_EQ(profile.secureCredentialStorage(), secureCredentials);

    bool enabled = false;
    profile.setEnabled(enabled);
    ASSERT_EQ(profile.getEnabled(), enabled);

    QString hostname("streaming.website.com");
    profile.setHost(hostname);
    ASSERT_TRUE(profile.getHost() == hostname);

    int port = 1238;
    profile.setPort(port);
    ASSERT_EQ(profile.getPort(), port);

    QString servertype("Shoutcast 1");
    profile.setServertype(servertype);
    ASSERT_TRUE(profile.getServertype() == servertype);

    QString login("myusername");
    profile.setLogin(login);
    ASSERT_TRUE(profile.getLogin() == login);

    QString password("changemepassword");
    profile.setPassword(password);
    ASSERT_TRUE(profile.getPassword() == password);

    bool enableReconnect = false;
    profile.setEnableReconnect(enableReconnect);
    ASSERT_EQ(profile.getEnableReconnect(), enableReconnect);

    double reconnectPeriod = 3.14;
    profile.setReconnectPeriod(reconnectPeriod);
    ASSERT_EQ(profile.getReconnectPeriod(), reconnectPeriod);

    bool limitReconnects = false;
    profile.setLimitReconnects(limitReconnects);
    ASSERT_EQ(profile.getLimitReconnects(), limitReconnects);

    int maximumRetries = 3;
    profile.setMaximumRetries(maximumRetries);
    ASSERT_EQ(profile.getMaximumRetries(), maximumRetries);

    bool noDelayFirstReconnect = false;
    profile.setNoDelayFirstReconnect(noDelayFirstReconnect);
    ASSERT_EQ(profile.getNoDelayFirstReconnect(), noDelayFirstReconnect);

    double reconnectFirstDelay = 6.28;
    profile.setReconnectFirstDelay(reconnectFirstDelay);
    ASSERT_EQ(profile.getReconnectFirstDelay(), reconnectFirstDelay);

    QString format("Ogg Vorbis");
    profile.setFormat(format);
    ASSERT_TRUE(profile.getFormat() == format);

    int bitrate = 320;
    profile.setBitrate(bitrate);
    ASSERT_EQ(profile.getBitrate(), bitrate);

    int channels = 1;
    profile.setChannels(channels);
    ASSERT_EQ(profile.getChannels(), channels);

    QString mountpoint("stream.ogg");
    profile.setMountPoint(mountpoint);
    ASSERT_TRUE(profile.getMountpoint() == mountpoint);

    QString streamName("This is a test stream");
    profile.setStreamName(streamName);
    ASSERT_TRUE(profile.getStreamName() == streamName);

    QString streamDesc("this is a stream description");
    profile.setStreamDesc(streamDesc);
    ASSERT_TRUE(profile.getStreamDesc() == streamDesc);

    QString streamGenre("unit testing");
    profile.setStreamGenre(streamGenre);
    ASSERT_TRUE(profile.getStreamGenre() == streamGenre);

    bool streamPublic = true;
    profile.setStreamPublic(streamPublic);
    ASSERT_EQ(profile.getStreamPublic(), streamPublic);

    QString streamWebsite("www.website.com");
    profile.setStreamWebsite(streamWebsite);
    ASSERT_TRUE(profile.getStreamWebsite() == streamWebsite);

    bool enableMetadata = true;
    profile.setEnableMetadata(enableMetadata);
    ASSERT_EQ(profile.getEnableMetadata(), enableMetadata);

    QString metadataCharset("UTF-8");
    profile.setMetadataCharset(metadataCharset);
    ASSERT_TRUE(profile.getMetadataCharset() == metadataCharset);

    QString customArtist("this is an artist");
    profile.setCustomArtist(customArtist);
    ASSERT_TRUE(profile.getCustomArtist() == customArtist);

    QString customTitle("this is a title");
    profile.setCustomTitle(customTitle);
    ASSERT_TRUE(profile.getCustomTitle() == customTitle);

    QString metadataFormat("no particular format");
    profile.setMetadataFormat(metadataFormat);
    ASSERT_TRUE(profile.getMetadataFormat() == metadataFormat);

    bool oggDynamicUpdate = true;
    profile.setOggDynamicUpdate(oggDynamicUpdate);
    ASSERT_EQ(profile.getOggDynamicUpdate(), oggDynamicUpdate);
}

TEST(BroadcastProfileTest, DefaultValues) {
    BroadcastProfile profile("Unit Testing Default Values");

    // Check if some select attributes have non-empty default values
    ASSERT_FALSE(profile.getEnabled());
    ASSERT_EQ(profile.getPort(), BROADCAST_DEFAULT_PORT);
    ASSERT_TRUE(profile.getStreamWebsite() == QString(MIXXX_WEBSITE_URL));
    ASSERT_FALSE(profile.getStreamDesc().isEmpty());
    ASSERT_FALSE(profile.getStreamGenre().isEmpty());
    ASSERT_FALSE(profile.getMetadataFormat().isEmpty());
}

} // namespace

#endif // __BROADCAST__
