#include <QtDebug>

#include <gtest/gtest.h>

#include "sources/soundsourceproviderregistry.h"

namespace mixxx {

class TestSoundSourceProvider: public SoundSourceProvider {
public:
  TestSoundSourceProvider(
          QString displayName,
          QStringList supportedFileExtensions,
          SoundSourceProviderPriority priorityHint)
          : m_displayName(displayName),
            m_supportedFileExtensions(supportedFileExtensions),
            m_priorityHint(priorityHint) {
  }

  QString getDisplayName() const override {
      return m_displayName;
  }

    // A list of supported file extensions in any order.
    QStringList getSupportedFileExtensions() const override {
        return m_supportedFileExtensions;
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override {
        Q_UNUSED(supportedFileExtension);
        return m_priorityHint;
    }

    SoundSourcePointer newSoundSource(const QUrl& /*url*/) override {
        return SoundSourcePointer();
    }

private:
  const QString m_displayName;
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

    SoundSourceProviderPointer createProvider(
            QString name,
            QStringList supportedFileExtensions,
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::Default) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, supportedFileExtensions, priorityHint));
    }

    SoundSourceProviderPointer createProvider(
            QString name,
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::Default) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, m_supportedFileExtensions, priorityHint));
    }

    static QStringList getAllRegisteredProviderDisplayNamesForFileExtension(
            const SoundSourceProviderRegistry& cut, QString fileExt) {
        QStringList displayNames;
        const QList<SoundSourceProviderRegistration> registrations(
                cut.getRegistrationsForFileExtension(fileExt));
        displayNames.reserve(registrations.size());
        for (auto const& registration: registrations) {
            displayNames.append(registration.getProvider()->getDisplayName());
        }
        return displayNames;
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
    cut.registerProvider(createProvider("Test04", SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider("Test02", SoundSourceProviderPriority::Lower));
    cut.registerProvider(createProvider("Test00", SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider("Test01", SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider("Test10", SoundSourceProviderPriority::Highest));
    // 1st round - registration with explicit priority for FILE_EXT1
    cut.registerProviderForFileExtension(FILE_EXT1,
            createProvider("Test05"),
            SoundSourceProviderPriority::Default);
    cut.registerProviderForFileExtension(FILE_EXT1,
            createProvider("Test11"),
            SoundSourceProviderPriority::Highest);
    cut.registerProviderForFileExtension(FILE_EXT1,
            createProvider("Test03"),
            SoundSourceProviderPriority::Lower);
    cut.registerProviderForFileExtension(FILE_EXT1,
            createProvider("Test08"),
            SoundSourceProviderPriority::Higher);
    cut.registerProviderForFileExtension(FILE_EXT1,
            createProvider("Test09"),
            SoundSourceProviderPriority::Higher);

    // 1st round - validation
    EXPECT_EQ(m_supportedFileExtensions, cut.getRegisteredFileExtensions());
    const QStringList displayNames1Round1(
            getAllRegisteredProviderDisplayNamesForFileExtension(cut, FILE_EXT1));
    EXPECT_EQ(10, displayNames1Round1.size());
    EXPECT_TRUE(expectSortedStringList(displayNames1Round1));
    const QStringList displayNames2Round1(
            getAllRegisteredProviderDisplayNamesForFileExtension(cut, FILE_EXT2));
    EXPECT_EQ(5, displayNames2Round1.size());
    EXPECT_TRUE(expectSortedStringList(displayNames2Round1));

    // 2nd round - registration using priority hint for FILE_EXT2
    cut.registerProvider(
            createProvider(
                    "Test06",
                    QStringList(FILE_EXT2),
                    SoundSourceProviderPriority::Default));
    // 2nd round - registration with explicit priority for FILE_EXT2
    cut.registerProviderForFileExtension(
            FILE_EXT2,
            createProvider(
                    "Test07",
                    QStringList(FILE_EXT2),
                    // priority hint should be overridden by registration
                    SoundSourceProviderPriority::Highest),
            SoundSourceProviderPriority::Default);

    // 2nd round - validation
    EXPECT_EQ(cut.getRegisteredFileExtensions(), m_supportedFileExtensions);
    const QStringList displayNames1Round2(
            getAllRegisteredProviderDisplayNamesForFileExtension(cut, FILE_EXT1));
    EXPECT_EQ(displayNames1Round1, displayNames1Round2);
    const QStringList displayNames2Round2(
            getAllRegisteredProviderDisplayNamesForFileExtension(cut, FILE_EXT2));
    EXPECT_EQ(displayNames2Round1.size() + 2, displayNames2Round2.size());
    EXPECT_TRUE(expectSortedStringList(displayNames2Round2));
}

}  // namespace mixxx
