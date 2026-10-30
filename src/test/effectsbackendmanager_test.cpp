#include "effects/backends/effectsbackendmanager.h"

#include <gtest/gtest.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectprocessor.h"
#include "test/mixxxtest.h"

namespace {

class MissingManifestBackend final : public EffectsBackend {
  public:
    explicit MissingManifestBackend(bool includeAvailable)
            : m_includeAvailable(includeAvailable),
              m_manifest(EffectManifestPointer::create()) {
        m_manifest->setId(QStringLiteral("available"));
        m_manifest->setName(QStringLiteral("Available test effect"));
        m_manifest->setBackendType(getType());
    }

    EffectBackendType getType() const override {
        return EffectBackendType::Unknown;
    }
    const QList<QString> getEffectIds() const override {
        return m_includeAvailable
                ? QList<QString>{QStringLiteral("missing"), QStringLiteral("available")}
                : QList<QString>{QStringLiteral("missing")};
    }
    EffectManifestPointer getManifest(const QString& id) const override {
        return id == QStringLiteral("available") ? m_manifest : EffectManifestPointer{};
    }
    const QList<EffectManifestPointer> getManifests() const override {
        return m_includeAvailable ? QList<EffectManifestPointer>{m_manifest}
                                  : QList<EffectManifestPointer>{};
    }
    bool canInstantiateEffect(const QString&) const override {
        return false;
    }
    std::unique_ptr<EffectProcessor> createProcessor(
            const EffectManifestPointer) const override {
        return {};
    }

  private:
    const bool m_includeAvailable;
    const EffectManifestPointer m_manifest;
};

} // namespace

class EffectsBackendManagerTest : public MixxxTest {
  protected:
    void checkMissingManifest(bool includeAvailable) {
        EffectsBackendManager manager;
        const auto originalCount = manager.getManifests().size();
        manager.addBackend(EffectsBackendPointer(new MissingManifestBackend(includeAvailable)));
        const auto expectedCount = originalCount + (includeAvailable ? 1 : 0);
        ASSERT_EQ(expectedCount, manager.getManifests().size());
        for (const auto& manifest : manager.getManifests()) {
            ASSERT_TRUE(manifest);
        }
        ControlProxy count(QStringLiteral("[Master]"), QStringLiteral("num_effectsavailable"));
        EXPECT_DOUBLE_EQ(expectedCount, count.get());
        EXPECT_FALSE(manager.getManifest(QStringLiteral("missing"), EffectBackendType::Unknown));
        EXPECT_EQ(includeAvailable,
                manager.getManifests().contains(manager.getManifest(
                        QStringLiteral("available"), EffectBackendType::Unknown)));
    }
};

TEST_F(EffectsBackendManagerTest, IgnoresUnavailableManifestAmongAvailableEffects) {
    checkMissingManifest(true);
}

TEST_F(EffectsBackendManagerTest, BackendWithOnlyUnavailableManifestDoesNotIncreaseCount) {
    checkMissingManifest(false);
}
