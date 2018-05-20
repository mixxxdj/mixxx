#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegExp>
#include <QString>
#include <QStringList>

#ifdef __QTKEYCHAIN__
#include <qtkeychain/keychain.h>
using namespace QKeychain;
#endif

// #include "broadcast/defs_broadcast.h"
#include "defs_urls.h"
#include "util/xml.h"
#include "util/memory.h"
#include "util/logger.h"

#include "preferences/effectprofile.h"

namespace {
const mixxx::Logger kLogger("EffectProfile");
} // anonymous namespace

EffectProfile::EffectProfile(EffectManifest &pManifest,
                                   QObject* parent)
    : QObject(parent) {

    m_isVisible = pManifest.isVisible();
    m_pManifest = &pManifest;
}

QString EffectProfile::getEffectId() const {
    return m_pManifest->id();
}

QString EffectProfile::getDisplayName() const {
    return m_pManifest->displayName();
}

bool EffectProfile::isVisible() const {
    return m_isVisible;
}

void EffectProfile::setVisibility(bool value) {
    m_isVisible = value;
}

EffectManifest* EffectProfile::getManifest() const {
    return m_pManifest;
}