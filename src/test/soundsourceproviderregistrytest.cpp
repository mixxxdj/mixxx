#include <QtDebug>

#include <gtest/gtest.h>

#include "sources/soundsourceproviderregistry.h"

namespace Mixxx {

namespace {

class TestSoundSourceProvider: public SoundSourceProvider {
public:
    TestSoundSourceProvider(
            QString name,
            QStringList supportedFileExtensions,
            Priority priority)
            : m_name(name),
              m_supportedFileExtensions(supportedFileExtensions),
              m_priority(priority) {
    }

    QString getName() const override {
        return m_name;
    }

    // A list of supported file extensions in any order.
    QStringList getSupportedFileExtensions() const override {
        return m_supportedFileExtensions;
    }

    Priority getPriorityHint() const override {
        return m_priority;
    }

    SoundSourcePointer newSoundSource(const QUrl& /*url*/) override {
        return SoundSourcePointer();
    }

private:
    const QString m_name;
    const QStringList m_supportedFileExtensions;
    const Priority m_priority;
};

class SoundSourceProviderRegistryTest : public testing::Test {
  protected:
    SoundSourceProviderRegistryTest() {
    }

    static const QString FILE_EXT1;
    static const QString FILE_EXT2;

    void SetUp() override {
        m_supportedFileExtensions.append(FILE_EXT1);
        m_supportedFileExtensions.append(FILE_EXT2);
    }

    void TearDown() override {
    }

    SoundSourceProviderPointer createProvider(
            QString name,
            QStringList supportedFileExtensions,
            SoundSourceProvider::Priority priority = SoundSourceProvider::DEFAULT_PRIORITY) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, supportedFileExtensions, priority));
    }

    SoundSourceProviderPointer createProvider(
            QString name,
            SoundSourceProvider::Priority priority = SoundSourceProvider::DEFAULT_PRIORITY) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, m_supportedFileExtensions, priority));
    }

    static QStringList getAllRegisteredProviderNamesForFileExtension(
            const SoundSourceProviderRegistry& cut, QString fileExt) {
        QStringList providerNames;
        const auto registrations(
                cut.getRegistrationsForFileExtension(fileExt));
        for (auto const& registration: registrations) {
            providerNames.append(registration.getProvider()->getName());
        }
        return providerNames;
    }

    static bool expectSortedStringList(const QStringList& sortedStrings) {
        QString previousString; // start with an empty string
        for (const auto& nextString: sortedStrings) {
            EXPECT_TRUE(previousString < nextString);
            if (previousString >= nextString) {
                return false;
            }
        }
        return true;
    }

    QStringList m_supportedFileExtensions;
};

/*static*/ const QString SoundSourceProviderRegistryTest::FILE_EXT1("ext1");
/*static*/ const QString SoundSourceProviderRegistryTest::FILE_EXT2("ext2");

TEST_F(SoundSourceProviderRegistryTest, registerProviders) {
    SoundSourceProviderRegistry cut;

    // 1st round - registration using priority hint
    cut.registerProvider(createProvider("Test04", SoundSourceProvider::DEFAULT_PRIORITY));
    cut.registerProvider(createProvider("Test02", SoundSourceProvider::LOWER_PRIORITY));
    cut.registerProvider(createProvider("Test00", SoundSourceProvider::LOWEST_PRIORITY));
    cut.registerProvider(createProvider("Test01", SoundSourceProvider::LOWEST_PRIORITY));
    cut.registerProvider(createProvider("Test10", SoundSourceProvider::HIGHEST_PRIORITY));
    // 1st round - registration with explicit priority
    cut.registerProvider(createProvider("Test05"), SoundSourceProvider::DEFAULT_PRIORITY);
    cut.registerProvider(createProvider("Test11"), SoundSourceProvider::HIGHEST_PRIORITY);
    cut.registerProvider(createProvider("Test03"), SoundSourceProvider::LOWER_PRIORITY);
    cut.registerProvider(createProvider("Test08"), SoundSourceProvider::HIGHER_PRIORITY);
    cut.registerProvider(createProvider("Test09"), SoundSourceProvider::HIGHER_PRIORITY);

    // 1st round - validation
    {
        EXPECT_EQ(cut.getRegisteredFileExtensions(), m_supportedFileExtensions);
        const QStringList providerNames1(
                getAllRegisteredProviderNamesForFileExtension(cut, FILE_EXT1));
        EXPECT_TRUE(expectSortedStringList(providerNames1));
        const QStringList providerNames2(
                getAllRegisteredProviderNamesForFileExtension(cut, FILE_EXT2));
        EXPECT_TRUE(expectSortedStringList(providerNames2));
        EXPECT_EQ(providerNames1, providerNames2);
    }

    // 2nd round - registration using priority hint
    cut.registerProvider(createProvider("Test06", QStringList(FILE_EXT2), SoundSourceProvider::DEFAULT_PRIORITY));
    // 1st round - registration with explicit priority
    cut.registerProvider(createProvider("Test07", QStringList(FILE_EXT2), SoundSourceProvider::HIGHEST_PRIORITY), SoundSourceProvider::DEFAULT_PRIORITY);

    // 2nd round - validation
    {
        EXPECT_EQ(cut.getRegisteredFileExtensions(), m_supportedFileExtensions);
        const QStringList providerNames1(
                getAllRegisteredProviderNamesForFileExtension(cut, FILE_EXT1));
        EXPECT_TRUE(expectSortedStringList(providerNames1));
        const QStringList providerNames2(
                getAllRegisteredProviderNamesForFileExtension(cut, FILE_EXT2));
        EXPECT_TRUE(expectSortedStringList(providerNames2));
        EXPECT_NE(providerNames1, providerNames2);
        EXPECT_EQ(providerNames1.size() + 2, providerNames2.size());
    }
}

}  // anonymous namespace

}  // namespace Mixxx
