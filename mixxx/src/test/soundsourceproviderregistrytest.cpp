#include <gtest/gtest.h>

#include <QtDebug>

#include "sources/soundsourceproviderregistry.h"

namespace mixxx {

class TestSoundSourceProvider : public SoundSourceProvider {
  public:
    TestSoundSourceProvider(
            const QString& displayName,
            const QStringList& supportedFileTypes,
            SoundSourceProviderPriority priorityHint)
            : m_displayName(displayName),
              m_supportedFileTypes(supportedFileTypes),
              m_priorityHint(priorityHint) {
    }

    QString getDisplayName() const override {
        return m_displayName;
    }

    // A list of supported file extensions in any order.
    QStringList getSupportedFileTypes() const override {
        return m_supportedFileTypes;
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileSuffix) const override {
        Q_UNUSED(supportedFileSuffix);
        return m_priorityHint;
    }

    SoundSourcePointer newSoundSource(const QUrl& /*url*/) override {
        return SoundSourcePointer();
    }

  private:
    const QString m_displayName;
    const QStringList m_supportedFileTypes;
    const SoundSourceProviderPriority m_priorityHint;
};

class SoundSourceProviderRegistryTest : public testing::Test {
  protected:
    SoundSourceProviderRegistryTest() {
    }

    static const QString FILE_TYPE1;
    static const QString FILE_TYPE2;

    void SetUp() override {
        m_supportedFileTypes.append(FILE_TYPE1);
        m_supportedFileTypes.append(FILE_TYPE2);
    }

    SoundSourceProviderPointer createProvider(
            const QString& name,
            const QStringList& supportedFileTypes,
            SoundSourceProviderPriority priorityHint = SoundSourceProviderPriority::Default) {
        return SoundSourceProviderPointer(
                new TestSoundSourceProvider(
                        name, supportedFileTypes, priorityHint));
    }

    static QStringList getAllRegisteredProviderDisplayNamesForFileType(
            const SoundSourceProviderRegistry& cut, const QString& fileType) {
        QStringList displayNames;
        const QList<SoundSourceProviderRegistration> registrations(
                cut.getRegistrationsForFileType(fileType));
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

    QStringList m_supportedFileTypes;
};

/*static*/ const QString SoundSourceProviderRegistryTest::FILE_TYPE1("ext1");
/*static*/ const QString SoundSourceProviderRegistryTest::FILE_TYPE2("ext2");

TEST_F(SoundSourceProviderRegistryTest, registerProviders) {
    SoundSourceProviderRegistry cut;

    // 1st round
    cut.registerProvider(createProvider(
            "Test04",
            QStringList{FILE_TYPE1, FILE_TYPE2},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test02",
            QStringList{FILE_TYPE1, FILE_TYPE2},
            SoundSourceProviderPriority::Lower));
    cut.registerProvider(createProvider(
            "Test00",
            QStringList{FILE_TYPE1, FILE_TYPE2},
            SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider(
            "Test01",
            QStringList{FILE_TYPE1, FILE_TYPE2},
            SoundSourceProviderPriority::Lowest));
    cut.registerProvider(createProvider(
            "Test10",
            QStringList{FILE_TYPE1, FILE_TYPE2},
            SoundSourceProviderPriority::Highest));
    cut.registerProvider(createProvider(
            "Test05",
            QStringList{FILE_TYPE1},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test11",
            QStringList{FILE_TYPE1},
            SoundSourceProviderPriority::Highest));
    cut.registerProvider(createProvider(
            "Test03",
            QStringList{FILE_TYPE1},
            SoundSourceProviderPriority::Lower));
    cut.registerProvider(createProvider(
            "Test08",
            QStringList{FILE_TYPE1},
            SoundSourceProviderPriority::Higher));
    cut.registerProvider(createProvider(
            "Test09",
            QStringList{FILE_TYPE1},
            SoundSourceProviderPriority::Higher));

    // 1st round - validation
    EXPECT_EQ(m_supportedFileTypes, cut.getRegisteredFileTypes());
    const QStringList displayNames1Round1(
            getAllRegisteredProviderDisplayNamesForFileType(cut, FILE_TYPE1));
    EXPECT_EQ(10, displayNames1Round1.size());
    EXPECT_TRUE(expectSortedStringList(displayNames1Round1));
    const QStringList displayNames2Round1(
            getAllRegisteredProviderDisplayNamesForFileType(cut, FILE_TYPE2));
    EXPECT_EQ(5, displayNames2Round1.size());
    EXPECT_TRUE(expectSortedStringList(displayNames2Round1));

    // 2nd round
    cut.registerProvider(createProvider(
            "Test06",
            QStringList{FILE_TYPE2},
            SoundSourceProviderPriority::Default));
    cut.registerProvider(createProvider(
            "Test07",
            QStringList{FILE_TYPE2},
            SoundSourceProviderPriority::Default));

    // 2nd round - validation
    EXPECT_EQ(cut.getRegisteredFileTypes(), m_supportedFileTypes);
    const QStringList displayNames1Round2(
            getAllRegisteredProviderDisplayNamesForFileType(cut, FILE_TYPE1));
    EXPECT_EQ(displayNames1Round1, displayNames1Round2);
    const QStringList displayNames2Round2(
            getAllRegisteredProviderDisplayNamesForFileType(cut, FILE_TYPE2));
    EXPECT_EQ(displayNames2Round1.size() + 2, displayNames2Round2.size());
    EXPECT_TRUE(expectSortedStringList(displayNames2Round2));
}

} // namespace mixxx
