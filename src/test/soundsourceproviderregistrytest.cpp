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
            SoundSourceProviderPriority priorityHint)
            : m_name(name),
              m_supportedFileExtensions(supportedFileExtensions),
              m_priorityHint(priorityHint) {
    }

    QString getName() const override {
        return m_name;
    }

    // A list of supported file extensions in any order.
    QStringList getSupportedFileExtensions() const override {
        return m_supportedFileExtensions;
    }

    SoundSourceProviderPriority getPriorityHint() const override {
        return m_priorityHint;
    }

    SoundSourcePointer newSoundSource(const QUrl& /*url*/) override {
        return SoundSourcePointer();
    }

private:
    const QString m_name;
    const QStringList m_supportedFileExtensions;
    const SoundSourceProviderPriority m_priorityHint;
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
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::DEFAULT) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, supportedFileExtensions, priorityHint));
    }

    SoundSourceProviderPointer createProvider(
            QString name,
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::DEFAULT) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, m_supportedFileExtensions, priorityHint));
    }

    static QStringList getAllRegisteredProviderNamesForFileExtension(
            const SoundSourceProviderRegistry& cut, QString fileExt) {
        QStringList providerNames;
        const QList<SoundSourceProviderRegistration> registrations(
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
    cut.registerProvider(createProvider("Test04", SoundSourceProviderPriority::DEFAULT));
    cut.registerProvider(createProvider("Test02", SoundSourceProviderPriority::LOWER));
    cut.registerProvider(createProvider("Test00", SoundSourceProviderPriority::LOWEST));
    cut.registerProvider(createProvider("Test01", SoundSourceProviderPriority::LOWEST));
    cut.registerProvider(createProvider("Test10", SoundSourceProviderPriority::HIGHEST));
    // 1st round - registration with explicit priority
    cut.registerProvider(createProvider("Test05"), SoundSourceProviderPriority::DEFAULT);
    cut.registerProvider(createProvider("Test11"), SoundSourceProviderPriority::HIGHEST);
    cut.registerProvider(createProvider("Test03"), SoundSourceProviderPriority::LOWER);
    cut.registerProvider(createProvider("Test08"), SoundSourceProviderPriority::HIGHER);
    cut.registerProvider(createProvider("Test09"), SoundSourceProviderPriority::HIGHER);

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
    cut.registerProvider(
            createProvider(
                    "Test06",
                    QStringList(FILE_EXT2),
                    SoundSourceProviderPriority::DEFAULT));
    // 1st round - registration with explicit priority
    cut.registerProvider(
            createProvider(
                    "Test07",
                    QStringList(FILE_EXT2),
                    // priority hint should be overridden by registration
                    SoundSourceProviderPriority::HIGHEST),
            SoundSourceProviderPriority::DEFAULT);

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
