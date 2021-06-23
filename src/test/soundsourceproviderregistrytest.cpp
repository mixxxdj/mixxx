#include <gtest/gtest.h>

#include <QtDebug>

#include "sources/soundsourceproviderregistry.h"

namespace mixxx {

class TestSoundSourceProvider : public SoundSourceProvider {
  public:
    TestSoundSourceProvider(
            const QString& displayName,
            const QStringList& supportedFileExtensions,
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
            const QString& name,
            const QStringList& supportedFileExtensions,
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::Default) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, supportedFileExtensions, priorityHint));
    }

    static QStringList getAllRegisteredProviderDisplayNamesForFileExtension(
            const SoundSourceProviderRegistry& cut, const QString& fileExt) {
        QStringList displayNames;
        const QList<SoundSourceProviderRegistration> registrations(
                cut.getRegistrationsForFileExtension(fileExt));
        displayNames.reserve(registrations.size());
        for (auto const& registration : registrations) {
            displayNames.append(registration.getProvider()->getDisplayName());
        }
        return displayNames;
    }

    static bool expectSortedStringList(const QStringList& sortedStrings) {
        QString previousString; // start with an empty string
        for (const auto& nextString : sortedStrings) {
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

    // 1st round
    cut.registerProvider(createProvider(
            "Test04",
            QStringList{FILE_EXT1, FILE_EXT2},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test02",
            QStringList{FILE_EXT1, FILE_EXT2},
            SoundSourceProviderPriority::Lower));
    cut.registerProvider(createProvider(
            "Test00",
            QStringList{FILE_EXT1, FILE_EXT2},
            SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider(
            "Test01",
            QStringList{FILE_EXT1, FILE_EXT2},
            SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider(
            "Test10",
            QStringList{FILE_EXT1, FILE_EXT2},
            SoundSourceProviderPriority::Highest));
    cut.registerProvider(createProvider(
            "Test05",
            QStringList{FILE_EXT1},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test11",
            QStringList{FILE_EXT1},
            SoundSourceProviderPriority::Highest));
    cut.registerProvider(createProvider(
            "Test03",
            QStringList{FILE_EXT1},
            SoundSourceProviderPriority::Lower));
    cut.registerProvider(createProvider(
            "Test08",
            QStringList{FILE_EXT1},
            SoundSourceProviderPriority::Higher));
    cut.registerProvider(createProvider(
            "Test09",
            QStringList{FILE_EXT1},
            SoundSourceProviderPriority::Higher));

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

    // 2nd round
    cut.registerProvider(createProvider(
            "Test06",
            QStringList{FILE_EXT2},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test07",
            QStringList{FILE_EXT2},
            SoundSourceProviderPriority::Default));

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

} // namespace mixxx
