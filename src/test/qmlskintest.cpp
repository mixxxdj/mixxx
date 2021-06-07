#include <QWidget>

#include "coreservices.h"
#include "mixxxapplication.h"
#include "skin/qml/qmlskin.h"
#include "test/mixxxtest.h"
#include "util/cmdlineargs.h"
#include "waveform/guitick.h"
#include "waveform/visualsmanager.h"
#include "waveform/waveformwidgetfactory.h"

class QmlSkinTest : public MixxxTest {
  public:
    QmlSkinTest()
            : m_pConfig(config()) {
    }

    virtual ~QmlSkinTest() {
    }

  protected:
    UserSettingsPointer m_pConfig;
};

/// This is just a "smoke test" to check if the QML skin can be loaded.
TEST_F(QmlSkinTest, TestDemoSkinIsLoadable) {
    QDir skinDir(m_pConfig->getResourcePath());
    ASSERT_TRUE(skinDir.cd("skins"));
    ASSERT_TRUE(skinDir.cd("QMLDemo"));

    mixxx::skin::SkinPointer pSkin =
            mixxx::skin::qml::QmlSkin::fromDirectory(skinDir.absolutePath());
    ASSERT_TRUE(pSkin);
    ASSERT_TRUE(pSkin->isValid());

    // Initialize the app
    int argc = 0;
    CmdlineArgs cmdlineArgs;
    cmdlineArgs.parse(argc, nullptr);

    auto* pCoreServices = new mixxx::CoreServices(cmdlineArgs);
    pCoreServices->initializeSettings();
    pCoreServices->initializeKeyboard();

    auto* pGuiTick = new GuiTick();
    auto* pVisualsManager = new VisualsManager();

    WaveformWidgetFactory::createInstance(); // takes a long time
    WaveformWidgetFactory::instance()->setConfig(pCoreServices->getSettings());
    WaveformWidgetFactory::instance()->startVSync(pGuiTick, pVisualsManager);

    pCoreServices->initialize(application());

    QSet<ControlObject*> skinCreatedControls;
    QWidget* pSkinWidget = pSkin->loadSkin(nullptr, m_pConfig, &skinCreatedControls, pCoreServices);
    ASSERT_TRUE(pSkinWidget != nullptr);

    // Run the main loop for 3 seconds
    QScopedPointer<QTimer> timer(new QTimer());
    timer->setSingleShot(true);
    bool ok = timer->connect(timer.data(),
            &QTimer::timeout,
            qApp,
            &QApplication::quit,
            Qt::QueuedConnection);
    timer->start(3 * 1000); // 3 seconds timeout
    timer.take()->setParent(qApp);
    ASSERT_TRUE(ok);
    application()->exec();

    delete pSkinWidget;
    pSkin.reset();

    delete pVisualsManager;
    delete pGuiTick;

    pCoreServices->shutdown();
    delete pCoreServices;
}
