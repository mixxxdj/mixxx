#include "effects/visibleeffectslist.h"

#include <QDomDocument>

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
        const QDomDocument& doc, EffectsBackendManagerPointer pBackendManager) {
    QDomElement root = doc.documentElement();
    QDomElement visibleEffectsElement = XmlParse::selectElement(root, EffectXml::kVisibleEffects);
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

    if (!list.isEmpty()) {
        setList(list);
    } else {
        setList(pBackendManager->getManifestsForBackend(EffectBackendType::BuiltIn));
    }
}

void VisibleEffectsList::saveEffectsXml(QDomDocument* pDoc) {
    QDomElement root = pDoc->documentElement();
    QDomElement visibleEffectsElement = pDoc->createElement(EffectXml::kVisibleEffects);
    root.appendChild(visibleEffectsElement);
    for (const auto& pManifest : std::as_const(m_list)) {
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
