#ifdef __BROADCAST__

#include <QFile>
#include <QString>

#include "test/mixxxtest.h"
#include "preferences/broadcastsettings.h"
//#include "broadcast/defs_broadcast.h"
//#include "defs_urls.h"

namespace {

class BroadcastSettingsTest : public MixxxTest {};

TEST_F(BroadcastSettingsTest, SaveLoadAndRename) {
    QString originalProfileName("Original Profile Name");
    QString newProfileName("New Profile Name");

    BroadcastSettings settings(config());
    BroadcastProfilePtr pProfile(new BroadcastProfile(originalProfileName));
    settings.saveProfile(pProfile);

    // call saveProfile() and store the file name
    settings.saveProfile(pProfile);
    QString filename = pProfile->getLastFilename();

    // rename the profile, the file name shouldn't change
    QFile::remove(filename);
    pProfile->setProfileName(newProfileName);
    settings.saveProfile(pProfile);
    EXPECT_TRUE(pProfile->getLastFilename() == filename);
    ASSERT_TRUE(QFile::exists(filename));

    // load XML file using static loadFromFile then save it again
    pProfile = BroadcastProfile::loadFromFile(filename);
    ASSERT_NE(pProfile, nullptr);
    QFile::remove(filename);
    settings.saveProfile(pProfile);
    EXPECT_TRUE(pProfile->getLastFilename() == filename);
    ASSERT_TRUE(QFile::exists(filename));
}

TEST_F(BroadcastSettingsTest, AvoidExistingFiles) {
    QString name1("Profile? Name");
    QString name2("Profile> Name");

    BroadcastSettings settings(config());
    BroadcastProfilePtr p1(new BroadcastProfile(name1));
    BroadcastProfilePtr p2(new BroadcastProfile(name2));

    // save both profiles and make sure they have different file names
    settings.saveProfile(p1);
    settings.saveProfile(p2);

    ASSERT_FALSE(p1->getLastFilename() == p2->getLastFilename());
}

TEST_F(BroadcastSettingsTest, ReuseExistingFile) {
    QString name("Profile Name");

    BroadcastSettings settings(config());
    BroadcastProfilePtr pProfile(new BroadcastProfile(name));

    // save profile twice and make sure the name didn't change between saves
    settings.saveProfile(pProfile);
    QString filename = pProfile->getLastFilename();
    settings.saveProfile(pProfile);

    ASSERT_TRUE(pProfile->getLastFilename() == filename);
}

TEST_F(BroadcastSettingsTest, AddRemoveUpdateFromModel) {
    QString name("Profile Name");
    QString host("http://example.com");

    BroadcastSettings settings(config());
    BroadcastProfilePtr pProfile(new BroadcastProfile(name));
    BroadcastSettingsModel model;

    // add the profile to the model and apply the model to the settings
    model.addProfileToModel(pProfile);
    settings.applyModel(&model);
    ASSERT_EQ(settings.profiles().size(), 1);

    // make sure the profile was saved
    QString filename = settings.profileAt(0)->getLastFilename();
    ASSERT_TRUE(QFile::exists(filename));
    ASSERT_TRUE(settings.profileAt(0)->getProfileName() == name);

    // update the profile host and apply again
    ASSERT_FALSE(settings.profileAt(0)->getHost() == host);
    model.getProfileByName(name)->setHost(host);
    settings.applyModel(&model);
    ASSERT_EQ(settings.profiles().size(), 1);
    ASSERT_TRUE(settings.profileAt(0)->getHost() == host);

    // now remove the profile from the model and apply again
    model.deleteProfileFromModel(pProfile);
    settings.applyModel(&model);
    ASSERT_EQ(settings.profiles().size(), 0);

    // make sure the profile was removed
    ASSERT_FALSE(QFile::exists(filename));
}

} // namespace

#endif // __BROADCAST__
