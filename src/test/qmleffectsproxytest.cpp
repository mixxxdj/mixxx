#include <gtest/gtest.h>

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QImage>
#include <QMetaMethod>
#include <QMimeData>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QStringView>
#include <QTest>
#include <QTimer>
#include <algorithm>
#include <memory>

#include "control/controlproxy.h"
#include "effects/backends/builtin/echoeffect.h"
#include "effects/backends/builtin/parametriceqeffect.h"
#include "effects/backends/effectmanifest.h"
#include "effects/effectchain.h"
#include "effects/effectparameter.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "effects/presets/effectpreset.h"
#include "engine/channelhandle.h"
#include "qml/qmlchainpresetmodel.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmleffectslotparametersmodel.h"
#include "qml/qmleffectslotproxy.h"
#include "qml/qmleffectsmanagerproxy.h"
#include "qml/qmleffectunitproxy.h"
#include "test/mixxxtest.h"

namespace {

class QmlEffectsProxyTest : public MixxxTest {
  protected:
    void SetUp() override {
        auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pEffectsManager =
                std::make_shared<EffectsManager>(config(), pChannelHandleFactory);
        const QString mainOutputGroup = QStringLiteral("[MasterOutput]");
        m_pEffectsManager->registerInputChannel(ChannelHandleAndGroup(
                pChannelHandleFactory->getOrCreateHandle(mainOutputGroup), mainOutputGroup));
        m_pEffectsManager->setup();
        m_pProxy = std::make_unique<mixxx::qml::QmlEffectsManagerProxy>(
                m_pEffectsManager);
    }

    void TearDown() override {
        m_pProxy.reset();
        for (int unitIndex = 0; unitIndex < kNumStandardEffectUnits; ++unitIndex) {
            const auto pChain = m_pEffectsManager->getStandardEffectChain(unitIndex);
            auto pEmptyPreset = EffectChainPresetPointer::create();
            pEmptyPreset->setName(QString());
            pChain->loadChainPreset(pEmptyPreset);
            EXPECT_TRUE(pChain->isEmpty());
            EXPECT_TRUE(pChain->presetName().isEmpty());
        }
        m_pEffectsManager.reset();
    }

    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::unique_ptr<mixxx::qml::QmlEffectsManagerProxy> m_pProxy;
};

TEST_F(QmlEffectsProxyTest, ParameterLabelWidth) {
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    const auto* model = slot->getParametersModel();
    QFont font(QStringLiteral("Open Sans"));
    font.setPixelSize(10);
    const QFontMetrics metrics(font);
    EXPECT_EQ(metrics.size(0, QStringLiteral("Feedback")).width(),
            model->labelWidth(QStringLiteral("Feedback"), 1, QString(), font, false));
    EXPECT_EQ(QFontMetrics(QGuiApplication::font()).size(0, QStringLiteral("Feedback")).width(),
            model->labelWidth(QStringLiteral("Feedback"), 1, QString(), font, true));
    EXPECT_EQ(std::max(metrics.size(0, QStringLiteral("999.99 Hz")).width(),
                      metrics.size(0, QStringLiteral("1000.01 Hz")).width()),
            model->labelWidth(QStringLiteral("F"), 1000, QStringLiteral("Hz"), font, false));
}

TEST_F(QmlEffectsProxyTest, ParameterDragUsesWidgetPreviewAndMixedDrop) {
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    slot->setEffectId(EchoEffect::getId());
    QQmlEngine::setObjectOwnership(slot.get(), QQmlEngine::CppOwnership);
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> destination(m_pProxy->getEffectSlot(1, 2));
    destination->setEffectId(EchoEffect::getId());
    QQmlEngine::setObjectOwnership(destination.get(), QQmlEngine::CppOwnership);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import "Effects"

Item {
    id: root
    required property var effectSlot
    required property var destinationSlot
    EffectParameterDragArea {
        x: 20
        y: 80
        width: 60
        height: 10
        controlKey: "button_parameter1"
        dragItem: root
        effectSlot: root.destinationSlot
        parameterType: 1
    }
    property var capturedGrab
    readonly property bool mixedPayloadAccepted: dragArea.draggedParameter({
        hasText: true,
        text: "Mixxx effect parameter 1\n" + root.effectSlot.uniqueEffectId + "\n0"
    }).type === 1
    readonly property bool foreignPayloadRejected: dragArea.draggedParameter({
        hasText: true, text: "Mixxx effect parameter 0\nother-effect\n0"
    }) === null
    readonly property bool malformedPayloadRejected: dragArea.draggedParameter({
        hasText: true,
        text: "Mixxx effect parameter 0\n" + root.effectSlot.uniqueEffectId + "\n-1"
    }) === null
    signal previewReady(point hotSpot)
    width: 100
    height: 100

    Rectangle {
        id: parameter
        x: 20
        y: 20
        width: 60
        height: 50
        color: "red"

        EffectParameterDragArea {
            id: dragArea
            y: 30
            width: 60
            height: 10
            controlKey: "parameter1"
            dragItem: parameter
            effectSlot: root.effectSlot
            parameterType: 0
            Drag.onDragStarted: {
                root.capturedGrab = dragArea.dragImage;
                root.previewReady(dragArea.Drag.hotSpot);
            }
        }
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(RESOURCE_FOLDER "/qml/parameterdrag_test.qml")));
    QQuickWindow window;
    window.resize(100, 100);
    std::unique_ptr<QObject> root(component.createWithInitialProperties(
            {{QStringLiteral("effectSlot"), QVariant::fromValue(slot.get())},
                    {QStringLiteral("destinationSlot"), QVariant::fromValue(destination.get())}}));
    ASSERT_TRUE(root) << qPrintable(component.errorString());
    EXPECT_TRUE(root->property("mixedPayloadAccepted").toBool());
    EXPECT_TRUE(root->property("foreignPayloadRejected").toBool());
    EXPECT_TRUE(root->property("malformedPayloadRejected").toBool());
    auto* item = qobject_cast<QQuickItem*>(root.get());
    ASSERT_TRUE(item);
    item->setParentItem(window.contentItem());
    window.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
    QSignalSpy previewSpy(root.get(),
            root->metaObject()->method(root->metaObject()->indexOfSignal("previewReady(QPointF)")));
    ASSERT_TRUE(previewSpy.isValid());
    const QPoint pressPosition(35, 55);
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, pressPosition);
    QTimer::singleShot(500, &window, [&window] {
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(60, 55));
    });
    QTest::mouseMove(&window, QPoint(60, 55));
    if (previewSpy.isEmpty()) {
        EXPECT_TRUE(previewSpy.wait(2000));
    }
    ASSERT_EQ(previewSpy.count(), 1);
    EXPECT_EQ(previewSpy.at(0).at(0).toPointF(), QPointF(15, 35));
    auto* grab = root->property("capturedGrab").value<QObject*>();
    ASSERT_TRUE(grab);
    const QImage image = grab->property("image").value<QImage>();
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.size(), QSize(60, 50));
    EXPECT_EQ(image.pixelColor(30, 25), QColor(Qt::red));
    QTest::qWait(550);
    auto* model = destination->getParametersModel();
    const int originalOrder = model->get(0).toMap().value("displayOrder").toInt();
    QMimeData mimeData;
    mimeData.setText(QStringLiteral("Mixxx effect parameter 0\n%1\n0")
                    .arg(slot->getUniqueEffectId()));
    QDragEnterEvent enter(QPoint(35, 85),
            Qt::MoveAction,
            &mimeData,
            Qt::LeftButton,
            Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &enter);
    EXPECT_TRUE(enter.isAccepted());
    QDropEvent drop(QPointF(35, 85), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &drop);
    EXPECT_TRUE(drop.isAccepted());
    EXPECT_EQ(originalOrder, model->get(0).toMap().value("displayOrder").toInt());
    QTRY_COMPARE(model->get(0).toMap().value("displayOrder").toInt(), 0);
    EXPECT_EQ(originalOrder,
            slot->getParametersModel()
                    ->get(0)
                    .toMap()
                    .value("displayOrder")
                    .toInt());
    destination->completeParameterSwap(true);
    EXPECT_EQ(0, model->get(0).toMap().value("displayOrder").toInt());
}

TEST_F(QmlEffectsProxyTest, EffectsKnobCursorStaysHiddenOutsideKnob) {
    mixxx::qml::QmlConfigProxy::registerUserSettings(config());
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(":/mixxx.org/imports"));
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import "Controls"
Item {
    width: 200
    height: 200
    Knob {
        width: 30
        height: 30
        group: "[EffectRack1_EffectUnit1_Effect1]"
        key: "parameter1"
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(
                    RESOURCE_FOLDER "/skins/LateNightQML/cursor_test.qml")));
    QQuickWindow window;
    window.resize(200, 200);
    std::unique_ptr<QObject> root(component.create());
    ASSERT_TRUE(root) << qPrintable(component.errorString());
    auto* item = qobject_cast<QQuickItem*>(root.get());
    ASSERT_TRUE(item);
    item->setParentItem(window.contentItem());
    window.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
    EXPECT_EQ(QGuiApplication::overrideCursor(), nullptr);
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
    QTRY_COMPARE(window.cursor().shape(), Qt::BlankCursor);
    QTest::mouseMove(&window, QPoint(180, 180));
    QTRY_COMPARE(window.cursor().shape(), Qt::BlankCursor);
    EXPECT_EQ(QGuiApplication::overrideCursor(), nullptr);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(180, 180));
    QTRY_COMPARE(window.cursor().shape(), Qt::ArrowCursor);
    EXPECT_EQ(QGuiApplication::overrideCursor(), nullptr);
}

TEST_F(QmlEffectsProxyTest, UnitLookup) {
    auto* pUnit1 = m_pProxy->getEffectUnit(1);
    ASSERT_NE(nullptr, pUnit1);
    EXPECT_EQ(pUnit1, m_pProxy->getEffectUnit(1));
    EXPECT_EQ(1, pUnit1->getUnitNumber());
    EXPECT_EQ(QStringLiteral("[EffectRack1_EffectUnit1]"), pUnit1->getGroup());
    EXPECT_EQ(nullptr, m_pProxy->getEffectUnit(0));
    EXPECT_EQ(nullptr, m_pProxy->getEffectUnit(5));
}

TEST_F(QmlEffectsProxyTest, PresetModelsResetSeparately) {
    auto* pStandardModel = m_pProxy->property("standardChainPresetModel")
                                   .value<mixxx::qml::QmlChainPresetModel*>();
    auto* pQuickModel = m_pProxy->property("quickChainPresetModel")
                                .value<mixxx::qml::QmlChainPresetModel*>();
    ASSERT_NE(nullptr, pStandardModel);
    ASSERT_NE(nullptr, pQuickModel);
    EXPECT_GT(pStandardModel->rowCount({}), 0);
    EXPECT_GT(pQuickModel->rowCount({}), 0);
    EXPECT_TRUE(pStandardModel->roleNames().values().contains("name"));
    EXPECT_TRUE(pStandardModel->roleNames().values().contains("readOnly"));

    QSignalSpy standardResetSpy(pStandardModel, &QAbstractItemModel::modelReset);
    QSignalSpy quickResetSpy(pQuickModel, &QAbstractItemModel::modelReset);
    QMetaObject::invokeMethod(m_pEffectsManager->getChainPresetManager().get(),
            "effectChainPresetListUpdated");
    EXPECT_EQ(1, standardResetSpy.count());
    EXPECT_EQ(0, quickResetSpy.count());
    QMetaObject::invokeMethod(m_pEffectsManager->getChainPresetManager().get(),
            "quickEffectChainPresetListUpdated");
    EXPECT_EQ(1, standardResetSpy.count());
    EXPECT_EQ(1, quickResetSpy.count());
}

TEST_F(QmlEffectsProxyTest, UnitPresetStateAndLoading) {
    auto* pUnit1 = m_pProxy->getEffectUnit(1);
    ASSERT_NE(nullptr, pUnit1);
    QSignalSpy presetSpy(pUnit1, &mixxx::qml::QmlEffectUnitProxy::presetChanged);
    pUnit1->loadPreset(-1);
    EXPECT_EQ(0, presetSpy.count());
    pUnit1->loadPreset(0);
    EXPECT_EQ(0, pUnit1->getPresetIndex());
    EXPECT_FALSE(pUnit1->getPresetName().isEmpty());
    EXPECT_EQ(!pUnit1->isPresetReadOnly(), pUnit1->canUpdatePreset());
    EXPECT_EQ(pUnit1->canUpdatePreset() && !pUnit1->getPresetName().isEmpty(),
            pUnit1->canRenamePreset());
    EXPECT_GE(presetSpy.count(), 1);
}

TEST_F(QmlEffectsProxyTest, MixedParameterOrderPreservesControlSlotsAndPresets) {
    using Model = mixxx::qml::QmlEffectSlotParametersModel;
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    slot->setEffectId(EchoEffect::getId());
    auto* model = slot->getParametersModel();
    auto* orderedModel = model->orderedModel();
    const auto coreSlot = m_pEffectsManager->getStandardEffectChain(0)->getEffectSlot(0);
    const auto knobs = coreSlot->getLoadedParameters().value(EffectParameterType::Knob);
    const auto buttons = coreSlot->getLoadedParameters().value(EffectParameterType::Button);
    ASSERT_FALSE(knobs.isEmpty());
    ASSERT_FALSE(buttons.isEmpty());
    knobs.at(0)->setValue(0.37);
    const QString firstKnobId = knobs.at(0)->manifest()->id();
    const QString firstButtonId = buttons.at(0)->manifest()->id();
    const auto displayedId = [orderedModel](int row) {
        return orderedModel->data(orderedModel->index(row, 0), Model::IdRole).toString();
    };
    EXPECT_EQ(firstButtonId, displayedId(0));
    slot->queueParameterSwap(Model::Button, 0, 0, Model::Knob);
    EXPECT_EQ(firstButtonId, displayedId(0));
    slot->completeParameterSwap(false);
    EXPECT_EQ(firstButtonId, displayedId(0));
    slot->queueParameterSwap(Model::Button, 0, 0, Model::Knob);
    slot->completeParameterSwap(true);
    EXPECT_EQ(firstKnobId, displayedId(0));
    EXPECT_EQ(knobs, coreSlot->getLoadedParameters().value(EffectParameterType::Knob));
    EXPECT_EQ(buttons, coreSlot->getLoadedParameters().value(EffectParameterType::Button));
    EXPECT_EQ(QStringLiteral("parameter1"),
            orderedModel->data(orderedModel->index(0, 0), Model::ControlKeyRole).toString());

    slot->swapParameters(Model::Knob, 0, 1);
    EXPECT_EQ(knobs.at(1)->manifest()->id(), displayedId(0));
    slot->swapParameters(Model::Knob, 0, 1);
    slot->swapParameters(99, 0, 0, Model::Knob);
    slot->swapParameters(Model::Button, -1, 0, Model::Knob);
    slot->swapParameters(Model::Button, 0, 999, Model::Knob);
    EXPECT_EQ(firstKnobId, displayedId(0));

    QDomDocument document;
    const EffectPreset snapshot(coreSlot);
    const auto element = snapshot.toXml(&document);
    coreSlot->loadEffectFromPreset(EffectPresetPointer::create(element));
    EXPECT_EQ(firstKnobId, displayedId(0));
    EXPECT_DOUBLE_EQ(0.37,
            coreSlot->getLoadedParameters()
                    .value(EffectParameterType::Knob)
                    .at(0)
                    ->getValue());
    EXPECT_EQ(firstButtonId,
            coreSlot->getLoadedParameters()
                    .value(EffectParameterType::Button)
                    .at(0)
                    ->manifest()
                    ->id());

    slot->setParameterVisible(firstKnobId, false);
    slot->setParameterVisible(firstKnobId, true);
    EXPECT_EQ(firstKnobId, displayedId(model->rowCount({}) - 1));

    auto legacyElement = snapshot.toXml(&document);
    const auto parameters = legacyElement.elementsByTagName(QStringLiteral("Parameter"));
    for (int index = 0; index < parameters.size(); ++index) {
        auto parameter = parameters.at(index).toElement();
        parameter.removeChild(parameter.firstChildElement(QStringLiteral("DisplayOrder")));
    }
    coreSlot->loadEffectFromPreset(EffectPresetPointer::create(legacyElement));
    EXPECT_EQ(firstButtonId, displayedId(0));
    EXPECT_EQ(firstKnobId,
            coreSlot->getLoadedParameters()
                    .value(EffectParameterType::Knob)
                    .at(0)
                    ->manifest()
                    ->id());
    EXPECT_DOUBLE_EQ(0.37,
            coreSlot->getLoadedParameters()
                    .value(EffectParameterType::Knob)
                    .at(0)
                    ->getValue());
}

TEST_F(QmlEffectsProxyTest, ParameterInsertionPreservesValuesAndPersists) {
    using Model = mixxx::qml::QmlEffectSlotParametersModel;
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    slot->setEffectId(EchoEffect::getId());
    const auto core = m_pEffectsManager->getStandardEffectChain(0)->getEffectSlot(0);
    auto* model = slot->getParametersModel();
    auto* ordered = model->orderedModel();
    const auto ids = [ordered] {
        QStringList result;
        for (int row = 0; row < ordered->rowCount(); ++row) {
            const auto index = ordered->index(row, 0);
            if (ordered->data(index, Model::LoadedRole).toBool()) {
                result.append(ordered->data(index, Model::IdRole).toString());
            }
        }
        return result;
    };
    const auto original = ids();
    ASSERT_GE(original.size(), 3);
    const auto first = ordered->index(0, 0);
    const auto last = ordered->index(original.size() - 1, 0);
    const int firstType = ordered->data(first, Model::TypeRole).toInt();
    const int lastType = ordered->data(last, Model::TypeRole).toInt();
    const auto slotNumber = [ordered](const QModelIndex& index) {
        const auto key = ordered->data(index, Model::ControlKeyRole).toString();
        return QStringView(key).mid(key.lastIndexOf(QLatin1Char('r')) + 1).toInt() - 1;
    };
    const int firstSlot = slotNumber(first);
    const int lastSlot = slotNumber(last);
    const auto parameter = core->getLoadedParameters()
                                   .value(static_cast<EffectParameterType>(firstType))
                                   .at(firstSlot);
    const double value = parameter->getValue();
    slot->queueParameterMove(firstType, firstSlot, lastType, lastSlot, true);
    EXPECT_EQ(original, ids());
    slot->completeParameterSwap(false);
    EXPECT_EQ(original, ids());
    slot->queueParameterMove(firstType, firstSlot, lastType, lastSlot, true);
    slot->completeParameterSwap(true);
    auto expected = original;
    expected.move(0, expected.size() - 1);
    EXPECT_EQ(expected, ids());
    EXPECT_DOUBLE_EQ(value, parameter->getValue());
    QDomDocument document;
    const EffectPreset snapshot(core);
    core->loadEffectFromPreset(EffectPresetPointer::create(snapshot.toXml(&document)));
    EXPECT_EQ(expected, ids());
    const auto moved = ordered->index(expected.size() - 1, 0);
    const auto target = ordered->index(0, 0);
    slot->queueParameterMove(ordered->data(moved, Model::TypeRole).toInt(),
            slotNumber(moved),
            ordered->data(target, Model::TypeRole).toInt(),
            slotNumber(target),
            false);
    slot->completeParameterSwap(true);
    EXPECT_EQ(original, ids());
    slot->queueParameterMove(99, 0, lastType, 0, true);
    slot->completeParameterSwap(true);
    EXPECT_EQ(original, ids());
}

TEST_F(QmlEffectsProxyTest, ParameterReorderingPreservesQmlAndFixedControlBindings) {
    using Model = mixxx::qml::QmlEffectSlotParametersModel;
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    slot->setEffectId(EchoEffect::getId());
    const auto core = m_pEffectsManager->getStandardEffectChain(0)->getEffectSlot(0);
    QHash<QString, EffectParameterPointer> parameters;
    QHash<QString, double> values;
    QHash<QString, QSharedPointer<ControlProxy>> fixedControls;
    QStringList expectedOrder;
    for (const auto type : {EffectParameterType::Button, EffectParameterType::Knob}) {
        const auto loaded = core->getLoadedParameters().value(type);
        for (int index = 0; index < loaded.size(); ++index) {
            const auto& parameter = loaded.at(index);
            const auto id = parameter->manifest()->id();
            const auto key = (type == EffectParameterType::Button
                            ? QStringLiteral("button_parameter%1")
                            : QStringLiteral("parameter%1"))
                                     .arg(index + 1);
            auto control = QSharedPointer<ControlProxy>::create(slot->getGroup(), key);
            ASSERT_TRUE(control->valid());
            const double value = type == EffectParameterType::Button
                    ? index % 2
                    : 0.1 * (index + 1);
            control->set(value);
            EXPECT_DOUBLE_EQ(value, parameter->getValue());
            parameters.insert(id, parameter);
            values.insert(id, value);
            fixedControls.insert(key, control);
            expectedOrder.append(id);
        }
    }
    ASSERT_EQ(6, expectedOrder.size());
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(":/mixxx.org/imports"));
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import Mixxx 1.0 as Mixxx
Item {
    id: root
    required property string group
    required property var parametersModel
    function rowFor(id) {
        for (let index = 0; index < repeater.count; ++index) {
            const row = repeater.itemAt(index);
            if (row.parameterId === id) return row;
        }
        return null;
    }
    Repeater {
        id: repeater
        model: root.parametersModel
        delegate: Item {
            id: row
            required property string parameterId
            required property string controlKey
            property alias value: control.value
            Mixxx.ControlProxy {
                id: control
                group: root.group
                key: row.controlKey
            }
        }
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(RESOURCE_FOLDER "/qml/parameter_values_test.qml")));
    auto* ordered = slot->getParametersModel()->orderedModel();
    std::unique_ptr<QObject> root(component.createWithInitialProperties(
            {{QStringLiteral("group"), slot->getGroup()},
                    {QStringLiteral("parametersModel"), QVariant::fromValue(ordered)}}));
    ASSERT_TRUE(root) << qPrintable(component.errorString());
    const auto rowFor = [&root](const QString& id) -> QObject* {
        QVariant result;
        if (!QMetaObject::invokeMethod(root.get(),
                    "rowFor",
                    Q_RETURN_ARG(QVariant, result),
                    Q_ARG(QVariant, id))) {
            return nullptr;
        }
        return result.value<QObject*>();
    };
    const auto verifyBindings = [&] {
        QStringList displayed;
        for (int row = 0; row < ordered->rowCount(); ++row) {
            displayed.append(ordered->data(ordered->index(row, 0), Model::IdRole).toString());
        }
        EXPECT_EQ(expectedOrder, displayed);
        QHash<int, int> slotNumbers;
        for (const auto& id : expectedOrder) {
            SCOPED_TRACE(id.toStdString());
            const auto parameter = parameters.value(id);
            const auto type = parameter->manifest()->parameterType();
            const int number = ++slotNumbers[static_cast<int>(type)];
            const auto key = (type == EffectParameterType::Button
                            ? QStringLiteral("button_parameter%1")
                            : QStringLiteral("parameter%1"))
                                     .arg(number);
            EXPECT_EQ(parameter, core->getLoadedParameters().value(type).at(number - 1));
            EXPECT_DOUBLE_EQ(values.value(id), parameter->getValue());
            EXPECT_DOUBLE_EQ(values.value(id), fixedControls.value(key)->get());
            auto* row = rowFor(id);
            ASSERT_NE(nullptr, row);
            EXPECT_EQ(key, row->property("controlKey").toString());
            QTRY_COMPARE(row->property("value").toDouble(), values.value(id));
        }
    };
    const auto verifyWrites = [&] {
        for (const auto& id : expectedOrder) {
            const bool button =
                    parameters.value(id)->manifest()->parameterType() ==
                    EffectParameterType::Button;
            auto* row = rowFor(id);
            ASSERT_NE(nullptr, row);
            values[id] = button ? 1.0 - values.value(id) : values.value(id) + 0.03;
            ASSERT_TRUE(row->setProperty("value", values.value(id)));
            verifyBindings();
            const auto key = row->property("controlKey").toString();
            values[id] = button ? 1.0 - values.value(id) : values.value(id) + 0.02;
            fixedControls.value(key)->set(values.value(id));
            verifyBindings();
        }
    };
    verifyBindings();
    slot->queueParameterMove(Model::Knob, 3, Model::Button, 0, false);
    slot->completeParameterSwap(false);
    verifyBindings();
    slot->queueParameterMove(Model::Knob, 3, Model::Button, 0, false);
    slot->completeParameterSwap(true);
    expectedOrder.move(5, 0);
    verifyBindings();
    verifyWrites();
    slot->queueParameterMove(Model::Button, 1, Model::Knob, 0, false);
    slot->completeParameterSwap(true);
    expectedOrder.move(2, 0);
    verifyBindings();
    verifyWrites();
    core->swapParameters(EffectParameterType::Knob, 0, 2);
    expectedOrder.swapItemsAt(1, 4);
    verifyBindings();
    verifyWrites();
    core->swapParameters(EffectParameterType::Button, 0, 1);
    expectedOrder.swapItemsAt(0, 2);
    verifyBindings();
    verifyWrites();
}

TEST_F(QmlEffectsProxyTest, LiveParameterPreviewPreservesDelegatesAndCancels) {
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import "Effects"
Item {
    width: 250
    height: 60
    property var capturedItem
    function capture() { capturedItem = repeater.itemAt(0); }
    function preview() { return visualModel.preview(0, 0, "parameter3"); }
    function cancel() { visualModel.restore(); }
    function replaceModel() { visualModel.sourceModel = visualModel.sourceModel.slice(); }
    function capturedIndex() {
        for (let i = 0; i < repeater.count; ++i) {
            if (repeater.itemAt(i) === capturedItem) return i;
        }
        return -1;
    }
    function keyAt(index) { return repeater.itemAt(index).modelData.controlKey; }
    readonly property string draggedKey: visualModel.draggedKey
    Row {
        Repeater {
            id: repeater
            model: visualModel
        }
    }
    EffectParameterReorderModel {
        id: visualModel
        sourceModel: [
            {controlKey: "parameter1", type: 0},
            {controlKey: "button_parameter1", type: 1},
            {controlKey: "parameter2", type: 0},
            {controlKey: "parameter3", type: 0}
        ]
        delegate: Item {
            required property var modelData
            width: 50
            height: 50
        }
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(RESOURCE_FOLDER "/qml/reorder_test.qml")));
    std::unique_ptr<QObject> root(component.create());
    ASSERT_TRUE(root) << qPrintable(component.errorString());
    QQuickWindow window;
    window.resize(250, 60);
    auto* item = qobject_cast<QQuickItem*>(root.get());
    ASSERT_TRUE(item);
    item->setParentItem(window.contentItem());
    window.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "capture"));
    QVariant result;
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "preview", Q_RETURN_ARG(QVariant, result)));
    EXPECT_TRUE(result.toBool());
    EXPECT_EQ(QStringLiteral("parameter1"), root->property("draggedKey").toString());
    QTest::qWait(20);
    ASSERT_TRUE(QMetaObject::invokeMethod(
            root.get(), "capturedIndex", Q_RETURN_ARG(QVariant, result)));
    EXPECT_EQ(3, result.toInt());
    EXPECT_EQ(150, root->property("capturedItem").value<QObject*>()->property("x").toInt());
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(),
            "keyAt",
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(QVariant, 0)));
    EXPECT_EQ(QStringLiteral("button_parameter1"), result.toString());
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "cancel"));
    QTest::qWait(20);
    EXPECT_TRUE(root->property("draggedKey").toString().isEmpty());
    ASSERT_TRUE(QMetaObject::invokeMethod(
            root.get(), "capturedIndex", Q_RETURN_ARG(QVariant, result)));
    EXPECT_EQ(0, result.toInt());
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "preview", Q_RETURN_ARG(QVariant, result)));
    EXPECT_TRUE(result.toBool());
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "replaceModel"));
    EXPECT_TRUE(root->property("draggedKey").toString().isEmpty());
}

TEST_F(QmlEffectsProxyTest, ParameterDropPreviewsInsertionBeforeCommit) {
    using Model = mixxx::qml::QmlEffectSlotParametersModel;
    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> slot(m_pProxy->getEffectSlot(1, 1));
    slot->setEffectId(EchoEffect::getId());
    QQmlEngine::setObjectOwnership(slot.get(), QQmlEngine::CppOwnership);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import "Effects"
Item {
    id: root
    required property var effectSlot
    readonly property var rows: {
        const rows = [];
        const model = effectSlot.parametersModel;
        for (let i = 0; i < model.rowCount(); ++i) {
            const row = model.get(i);
            if (row.loaded) rows.push(row);
        }
        return rows.sort((a, b) => a.displayOrder - b.displayOrder);
    }
    readonly property int count: rows.length
    readonly property string draggedKey: visualModel.draggedKey
    width: count * 60
    height: 50
    function firstKey() { return repeater.itemAt(0).modelData.controlKey; }
    Row {
        Repeater { id: repeater; model: visualModel }
    }
    EffectParameterReorderModel {
        id: visualModel
        sourceModel: root.rows
        delegate: Item {
            id: parameter
            required property var modelData
            width: 60
            height: 50
            EffectParameterDragArea {
                anchors.fill: parent
                dragItem: parameter
                controlKey: parameter.modelData.controlKey
                parameterType: parameter.modelData.type
                effectSlot: root.effectSlot
                reorderModel: visualModel
            }
        }
    }
}
)",
            QUrl::fromLocalFile(QStringLiteral(RESOURCE_FOLDER "/qml/drop_reorder_test.qml")));
    std::unique_ptr<QObject> root(component.createWithInitialProperties(
            {{QStringLiteral("effectSlot"), QVariant::fromValue(slot.get())}}));
    ASSERT_TRUE(root) << qPrintable(component.errorString());
    auto* item = qobject_cast<QQuickItem*>(root.get());
    ASSERT_TRUE(item);
    QQuickWindow window;
    window.resize(root->property("count").toInt() * 60, 50);
    item->setParentItem(window.contentItem());
    window.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
    auto* model = slot->getParametersModel()->orderedModel();
    const auto first = model->index(0, 0);
    const auto firstId = model->data(first, Model::IdRole).toString();
    const auto firstKey = model->data(first, Model::ControlKeyRole).toString();
    QMimeData mime;
    mime.setText(QStringLiteral("Mixxx effect parameter %1\n%2\n0")
                    .arg(model->data(first, Model::TypeRole).toInt())
                    .arg(slot->getUniqueEffectId()));
    const QPoint position(window.width() - 30, 25);
    QDragEnterEvent enter(position, Qt::MoveAction, &mime, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &enter);
    ASSERT_TRUE(enter.isAccepted());
    QTest::qWait(20);
    EXPECT_EQ(firstKey, root->property("draggedKey").toString());
    QVariant result;
    ASSERT_TRUE(QMetaObject::invokeMethod(root.get(), "firstKey", Q_RETURN_ARG(QVariant, result)));
    EXPECT_NE(firstKey, result.toString());
    EXPECT_EQ(firstId, model->data(first, Model::IdRole).toString());
    QDragLeaveEvent leave;
    QCoreApplication::sendEvent(&window, &leave);
    EXPECT_TRUE(root->property("draggedKey").toString().isEmpty());
    QCoreApplication::sendEvent(&window, &enter);
    ASSERT_TRUE(enter.isAccepted());
    QDropEvent drop(QPointF(position), Qt::MoveAction, &mime, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&window, &drop);
    ASSERT_TRUE(drop.isAccepted());
    EXPECT_EQ(firstId, model->data(first, Model::IdRole).toString());
    const int last = root->property("count").toInt() - 1;
    QTRY_COMPARE(model->data(model->index(last, 0), Model::IdRole).toString(), firstId);
    EXPECT_TRUE(root->property("draggedKey").toString().isEmpty());
}

TEST_F(QmlEffectsProxyTest, SlotEffectAndParameterVisibility) {
    constexpr int kExpectedParameterCount = 6;
    constexpr int kParameter1Index = 0;
    constexpr int kParameter2Index = 1;
    constexpr int kParameter3Index = 2;
    constexpr int kButtonParameter1Index = 4;

    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> pSlot(
            m_pProxy->getEffectSlot(1, 1));
    ASSERT_NE(nullptr, pSlot);
    QSignalSpy effectSpy(
            pSlot.get(), &mixxx::qml::QmlEffectSlotProxy::effectIdChanged);
    auto* pModel = pSlot->getParametersModel();
    ASSERT_NE(nullptr, pModel);
    QSignalSpy modelResetSpy(pModel, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(pModel, &QAbstractItemModel::dataChanged);

    pSlot->setEffectId(EchoEffect::getId());
    ASSERT_TRUE(pSlot->isLoaded());
    EXPECT_GE(effectSpy.count(), 1);
    EXPECT_GE(modelResetSpy.count(), 1);

    ASSERT_EQ(kExpectedParameterCount, pModel->rowCount({}));
    EXPECT_TRUE(pModel->roleNames().values().contains("unitString"));
    EXPECT_TRUE(pModel->roleNames().values().contains("maximum"));
    EXPECT_TRUE(pModel->roleNames().values().contains("neutralPoint"));
    const auto pManifest = EchoEffect::getManifest();
    EXPECT_DOUBLE_EQ(pManifest->metaknobDefault(), pSlot->getMetaDefault());
    for (int row = 0; row < kExpectedParameterCount; ++row) {
        const auto parameter = pModel->get(row).toMap();
        EXPECT_DOUBLE_EQ(pManifest->parameters().at(row)->getMaximum(),
                parameter.value("maximum").toDouble());
        EXPECT_DOUBLE_EQ(pManifest->parameters().at(row)->neutralPointOnScale(),
                parameter.value("neutralPoint").toDouble());
    }
    EXPECT_EQ(QString::number(-0.000001), pModel->formatNumber(-0.000001));
    EXPECT_EQ(QStringLiteral("parameter1"),
            pModel->get(kParameter1Index).toMap().value("controlKey").toString());
    EXPECT_EQ(QStringLiteral("parameter2"),
            pModel->get(kParameter2Index).toMap().value("controlKey").toString());
    EXPECT_EQ(QStringLiteral("button_parameter1"),
            pModel->get(kButtonParameter1Index)
                    .toMap()
                    .value("controlKey")
                    .toString());

    pSlot->setParameterVisible(QStringLiteral("feedback_amount"), false);
    EXPECT_GE(dataChangedSpy.count(), 1);
    EXPECT_FALSE(pModel->get(kParameter2Index).toMap().value("loaded").toBool());
    EXPECT_TRUE(pModel->get(kParameter2Index)
                    .toMap()
                    .value("controlKey")
                    .toString()
                    .isEmpty());
    EXPECT_EQ(QStringLiteral("parameter2"),
            pModel->get(kParameter3Index).toMap().value("controlKey").toString());

    pSlot->setParameterVisible(QStringLiteral("feedback_amount"), true);
    EXPECT_TRUE(pModel->get(kParameter2Index).toMap().value("loaded").toBool());
    pSlot->saveDefaultSnapshot();

    pSlot->setEffectId(ParametricEQEffect::getId());
    EXPECT_EQ(QStringLiteral("dB"),
            pModel->get(kParameter1Index).toMap().value("unitString").toString());
    pSlot->setEffectId(QString());
    EXPECT_DOUBLE_EQ(-1.0, pSlot->getMetaDefault());
}

} // namespace
