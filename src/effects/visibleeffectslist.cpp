#include "effects/visibleeffectslist.h"

#include <QDomDocument>

#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectsbackend.h"
#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectxmlelements.h"
#include "moc_visibleeffectslist.cpp"
#include "util/xml.h"

void VisibleEffectsList::setList(const QList<EffectManifestPointer>& newList) {
    m_list = newList;
    emit visibleEffectsListChanged();
}

int VisibleEffectsList::indexOf(EffectManifestPointer pManifest) const {
    return m_list.indexOf(pManifest);
}

const EffectManifestPointer VisibleEffectsList::at(int index) const {
    VERIFY_OR_DEBUG_ASSERT(index >= 0) {
        index = 0;
    }
    VERIFY_OR_DEBUG_ASSERT(index < m_list.size()) {
        index = m_list.size() - 1;
    }
    return m_list.at(index);
}

const EffectManifestPointer VisibleEffectsList::next(const EffectManifestPointer pManifest) const {
    int index = m_list.indexOf(pManifest);
    index++;
    if (index == m_list.size()) {
        index = 0;
    }
    return m_list.at(index);
}

const EffectManifestPointer VisibleEffectsList::previous(
        const EffectManifestPointer pManifest) const {
    int index = m_list.indexOf(pManifest);
    index--;
    if (index < 0) {
        index = m_list.size() - 1;
    }
    return m_list.at(index);
}

void VisibleEffectsList::readEffectsXml(
        const QDomDocument& doc,
        EffectsBackendManagerPointer pBackendManager) {
    QList<EffectManifestPointer> visibleEffects = readEffectsList(
            doc,
            pBackendManager,
            EffectXml::kVisibleEffects);
    const QList<EffectManifestPointer> hiddenEffects = readEffectsList(
            doc,
            pBackendManager,
            EffectXml::kHiddenEffects);

    // New effects will remain hidden since they are neither in the VisibleEffects
    // nor in the newly introduced HiddenEffects list.
    // Unhide all effects that are not in either list.
    const auto manifests = pBackendManager->getManifestsForBackend(EffectBackendType::BuiltIn);
    for (const EffectManifestPointer& pManifest : std::as_const(manifests)) {
        if (!visibleEffects.contains(pManifest) &&
                !hiddenEffects.contains(pManifest)) {
            // prepend so un-hidden effects are discoverable
            visibleEffects.prepend(pManifest);
        }
    }
    setList(visibleEffects);
}

QList<EffectManifestPointer> VisibleEffectsList::readEffectsList(
        const QDomDocument& doc,
        EffectsBackendManagerPointer pBackendManager,
        const QString& xmlElementName) {
    QDomElement root = doc.documentElement();
    QDomElement visibleEffectsElement = XmlParse::selectElement(root, xmlElementName);
    QDomNodeList effectsElementsList = visibleEffectsElement.elementsByTagName(EffectXml::kEffect);
    QList<EffectManifestPointer> list;

    for (int i = 0; i < effectsElementsList.count(); ++i) {
        QDomNode effectNode = effectsElementsList.at(i);
        if (effectNode.isElement()) {
            QString id = XmlParse::selectNodeQString(effectNode, EffectXml::kEffectId);
            QString backendString = XmlParse::selectNodeQString(
                    effectNode, EffectXml::kEffectBackendType);
            EffectBackendType backendType = EffectsBackend::backendTypeFromString(backendString);
            EffectManifestPointer pManifest = pBackendManager->getManifest(id, backendType);
            if (pManifest) {
                list.append(pManifest);
            }
        }
    }
    return list;
}

void VisibleEffectsList::saveEffectsXml(QDomDocument* pDoc,
        EffectsBackendManagerPointer pBackendManager) {
    saveEffectsListXml(pDoc, m_list, EffectXml::kVisibleEffects);
    auto hiddenEffects = pBackendManager->getManifests();
    for (const auto& pManifest : std::as_const(m_list)) {
        hiddenEffects.removeAll(pManifest);
    }
    saveEffectsListXml(pDoc, hiddenEffects, EffectXml::kHiddenEffects);
}

void VisibleEffectsList::saveEffectsListXml(QDomDocument* pDoc,
        const QList<EffectManifestPointer>& list,
        const QString& xmlElementName) {
    QDomElement root = pDoc->documentElement();
    QDomElement visibleEffectsElement = pDoc->createElement(xmlElementName);
    root.appendChild(visibleEffectsElement);
    for (const auto& pManifest : std::as_const(list)) {
        VERIFY_OR_DEBUG_ASSERT(pManifest) {
            continue;
        }
        QDomElement effectElement = pDoc->createElement(EffectXml::kEffect);
        visibleEffectsElement.appendChild(effectElement);
        XmlParse::addElement(*pDoc, effectElement, EffectXml::kEffectId, pManifest->id());
        XmlParse::addElement(*pDoc,
                effectElement,
                EffectXml::kEffectBackendType,
                EffectsBackend::backendTypeToString(pManifest->backendType()));
    }
}
