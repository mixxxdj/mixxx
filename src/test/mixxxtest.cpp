#include "test/mixxxtest.h"

#include "util/singleton.h"

// Specialize the Singleton template for QApplication because it doesn't have a
// 0-args constructor.
template <>
QApplication* Singleton<QApplication>::create() {
    if (!m_instance) {
        static int argc = 1;
        static char* argv[1] = { strdup("test") };
        m_instance = new QApplication(argc, argv);
    }
    return m_instance;
}

MixxxTest::MixxxTest() {
    // Create QApplication as a singleton. This prevents issues with creating
    // and destroying the QApplication multiple times in the same process.
    // http://stackoverflow.com/questions/14243858/qapplication-segfaults-in-googletest
    m_pApplication = Singleton<QApplication>::create();
    m_pConfig.reset(new ConfigObject<ConfigValue>(
        QDir::currentPath().append("/src/test/test_data/test.cfg")));

}

MixxxTest::~MixxxTest() {
    // Mixxx leaks a ton of COs normally. To make new tests not affected by
    // previous tests, we clear our all COs after every MixxxTest completion.
    QList<QSharedPointer<ControlDoublePrivate> > leakedControls;
    ControlDoublePrivate::getControls(&leakedControls);
    foreach (QSharedPointer<ControlDoublePrivate> pCDP, leakedControls) {
        if (pCDP.isNull()) {
            continue;
        }
        ConfigKey key = pCDP->getKey();
        qDebug() << "Warning: Test leaked control:" << key.group << key.item;
        delete pCDP->getCreatorCO();
    }
}
