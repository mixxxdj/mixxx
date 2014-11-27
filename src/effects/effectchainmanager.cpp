#include "effects/effectchainmanager.h"

#include <QtDebug>
#include <QDomDocument>
#include <QFile>
#include <QDir>

#include "effects/effectsmanager.h"
#include "xmlparse.h"

EffectChainManager::EffectChainManager(ConfigObject<ConfigValue>* pConfig,
                                       EffectsManager* pEffectsManager)
        : QObject(pEffectsManager),
          m_pConfig(pConfig),
          m_pEffectsManager(pEffectsManager) {
}

EffectChainManager::~EffectChainManager() {
    //qDebug() << debugString() << "destroyed";
}

void EffectChainManager::registerGroup(const QString& group) {
    if (m_registeredGroups.contains(group)) {
        qWarning() << debugString() << "WARNING: Group already registered:"
                   << group;
        return;
    }
    m_registeredGroups.insert(group);

    foreach (StandardEffectRackPointer pRack, m_standardEffectRacks) {
        pRack->registerGroup(group);
    }
}

StandardEffectRackPointer EffectChainManager::addStandardEffectRack() {
    StandardEffectRackPointer pRack(new StandardEffectRack(
        m_pEffectsManager, this, m_standardEffectRacks.size()));
    m_standardEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

StandardEffectRackPointer EffectChainManager::getStandardEffectRack(int i) {
    if (i < 0 || i >= m_standardEffectRacks.size()) {
        return StandardEffectRackPointer();
    }
    return m_standardEffectRacks[i];
}

EqualizerRackPointer EffectChainManager::addEqualizerRack() {
    EqualizerRackPointer pRack(new EqualizerRack(
        m_pEffectsManager, this, m_equalizerEffectRacks.size()));
    m_equalizerEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

EqualizerRackPointer EffectChainManager::getEqualizerRack(int i) {
    if (i < 0 || i >= m_equalizerEffectRacks.size()) {
        return EqualizerRackPointer();
    }
    return m_equalizerEffectRacks[i];
}

QuickEffectRackPointer EffectChainManager::addQuickEffectRack() {
    QuickEffectRackPointer pRack(new QuickEffectRack(
        m_pEffectsManager, this, m_quickEffectRacks.size()));
    m_quickEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

QuickEffectRackPointer EffectChainManager::getQuickEffectRack(int i) {
    if (i < 0 || i >= m_quickEffectRacks.size()) {
        return QuickEffectRackPointer();
    }
    return m_quickEffectRacks[i];
}

EffectRackPointer EffectChainManager::getEffectRack(const QString& group) {
    return m_effectRacksByGroup.value(group);
}

void EffectChainManager::addEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_effectChains.append(pEffectChain);
    }
}

void EffectChainManager::removeEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_effectChains.removeAll(pEffectChain);
    }
}

EffectChainPointer EffectChainManager::getNextEffectChain(EffectChainPointer pEffectChain) {
    if (m_effectChains.isEmpty())
        return EffectChainPointer();

    if (!pEffectChain) {
        return m_effectChains[0];
    }

    int indexOf = m_effectChains.lastIndexOf(pEffectChain);
    if (indexOf == -1) {
        qWarning() << debugString() << "WARNING: getNextEffectChain called for an unmanaged EffectChain";
        return m_effectChains[0];
    }

    return m_effectChains[(indexOf + 1) % m_effectChains.size()];
}

EffectChainPointer EffectChainManager::getPrevEffectChain(EffectChainPointer pEffectChain) {
    if (m_effectChains.isEmpty())
        return EffectChainPointer();

    if (!pEffectChain) {
        return m_effectChains[m_effectChains.size()-1];
    }

    int indexOf = m_effectChains.lastIndexOf(pEffectChain);
    if (indexOf == -1) {
        qWarning() << debugString() << "WARNING: getPrevEffectChain called for an unmanaged EffectChain";
        return m_effectChains[m_effectChains.size()-1];
    }

    return m_effectChains[(indexOf - 1 + m_effectChains.size()) % m_effectChains.size()];
}

bool EffectChainManager::saveEffectChains() {
    //qDebug() << debugString() << "saveEffectChains";
    QDomDocument doc("MixxxEffects");

    QString blank = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<MixxxEffects>\n"
        "</MixxxEffects>\n";
    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();

    QDomElement chains = doc.createElement("EffectChains");
    foreach (EffectChainPointer pChain, m_effectChains) {
        QDomElement chain = pChain->toXML(&doc);
        chains.appendChild(chain);
    }
    rootNode.appendChild(chains);

    QDir settingsPath(m_pConfig->getSettingsPath());

    if (!settingsPath.exists()) {
        return false;
    }

    QFile file(settingsPath.absoluteFilePath("effects.xml"));

    // TODO(rryan): overwrite the right way.
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return false;
    }

    QString effectsXml = doc.toString();
    file.write(effectsXml.toUtf8());
    file.close();
    return true;
}

bool EffectChainManager::loadEffectChains() {
    //qDebug() << debugString() << "loadEffectChains";

    QDir settingsPath(m_pConfig->getSettingsPath());
    QFile file(settingsPath.absoluteFilePath("effects.xml"));

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();

    QDomElement effectChains = XmlParse::selectElement(root, "EffectChains");
    QDomNodeList chains = effectChains.childNodes();

    for (int i = 0; i < chains.count(); ++i) {
        QDomNode chainNode = chains.at(i);

        if (chainNode.isElement()) {
            EffectChainPointer pChain = EffectChain::fromXML(
                m_pEffectsManager, chainNode.toElement());

            m_effectChains.append(pChain);
        }
    }
    return true;
}
