#include <gtest/gtest.h>

#include <memory>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "qml/qmlskincontrolcreator.h"
#include "test/helpers/log_test.h"
#include "test/mixxxtest.h"

using namespace mixxx::qml;

namespace {

class QmlSkinControlCreatorTest : public MixxxTest {
  protected:
    std::unique_ptr<QmlSkinControlCreator> createControl(
            const QString& group,
            const QString& key,
            double defaultValue = 0.0) {
        auto creator = std::make_unique<QmlSkinControlCreator>();
        creator->setGroup(group);
        creator->setKey(key);
        creator->setDefaultValue(defaultValue);
        creator->componentComplete();
        return creator;
    }
};

TEST_F(QmlSkinControlCreatorTest, CreatesSkinControl) {
    const ConfigKey key(QStringLiteral("[Skin]"),
            QStringLiteral("qml_skin_control_creator_test"));

    auto creator = createControl(key.group, key.item, 1.0);

    EXPECT_TRUE(ControlObject::exists(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));

    creator.reset();

    EXPECT_FALSE(ControlObject::exists(key));
}

TEST_F(QmlSkinControlCreatorTest, AppliesDefaultBeforeComponentComplete) {
    const ConfigKey key(QStringLiteral("[Skin]"),
            QStringLiteral("qml_skin_control_creator_precomplete_default_test"));

    auto creator = std::make_unique<QmlSkinControlCreator>();
    creator->setGroup(key.group);
    creator->setKey(key.item);
    creator->setPersist(true);
    creator->setDefaultValue(1.0);

    EXPECT_TRUE(ControlObject::exists(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));

    creator->componentComplete();

    EXPECT_TRUE(ControlObject::exists(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));
}

TEST_F(QmlSkinControlCreatorTest, AppliesDefaultBeforeComponentCompleteInAnyOrder) {
    const ConfigKey key(QStringLiteral("[Skin]"),
            QStringLiteral("qml_skin_control_creator_precomplete_order_test"));

    auto creator = std::make_unique<QmlSkinControlCreator>();
    creator->setDefaultValue(1.0);
    creator->setGroup(key.group);
    creator->setKey(key.item);

    EXPECT_FALSE(ControlObject::exists(key));

    creator->setPersist(true);

    EXPECT_TRUE(ControlObject::exists(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));

    creator->componentComplete();

    EXPECT_TRUE(ControlObject::exists(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));
}

TEST_F(QmlSkinControlCreatorTest, RejectsExistingSkinControl) {
    const ConfigKey key(QStringLiteral("[Skin]"),
            QStringLiteral("qml_skin_control_creator_duplicate_test"));
    ControlPushButton existingControl(key);

    LogCaptureGuard logCapture;
    EXPECT_LOG_MSG(QtWarningMsg,
            QStringLiteral("QmlSkinControlCreator: Cannot create already existing skin control.*%1")
                    .arg(key.item));

    auto creator = createControl(key.group, key.item, 1.0);

    ASSERT_ALL_EXPECTED_MSG();
    EXPECT_EQ(&existingControl, ControlObject::getControl(key));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(key));
}

TEST_F(QmlSkinControlCreatorTest, RejectsDuplicateSkinControlCreator) {
    const ConfigKey key(QStringLiteral("[Skin]"),
            QStringLiteral("qml_skin_control_creator_duplicate_creator_test"));

    auto creator = createControl(key.group, key.item, 1.0);

    ASSERT_NE(nullptr, creator.get());
    ASSERT_TRUE(ControlObject::exists(key));
    ControlObject* pOriginalControl = ControlObject::getControl(key);
    ASSERT_NE(nullptr, pOriginalControl);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));

    LogCaptureGuard logCapture;
    EXPECT_LOG_MSG(QtWarningMsg,
            QStringLiteral("QmlSkinControlCreator: Cannot create already existing skin control.*%1")
                    .arg(key.item));

    auto duplicateCreator = createControl(key.group, key.item, 2.0);

    ASSERT_NE(nullptr, duplicateCreator.get());
    ASSERT_ALL_EXPECTED_MSG();
    EXPECT_EQ(pOriginalControl, ControlObject::getControl(key));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(key));
}

TEST_F(QmlSkinControlCreatorTest, RejectsNonSkinControl) {
    const ConfigKey key(QStringLiteral("[TestQmlSkinControlCreator]"),
            QStringLiteral("non_skin_test"));

    LogCaptureGuard logCapture;
    EXPECT_LOG_MSG(QtWarningMsg,
            QStringLiteral("QmlSkinControlCreator: Cannot create non-skin control.*%1")
                    .arg(key.item));

    auto creator = createControl(key.group, key.item, 1.0);

    ASSERT_ALL_EXPECTED_MSG();
    EXPECT_FALSE(ControlObject::exists(key));
}

} // namespace
