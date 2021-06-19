#include "effects/effectslot.h"

#include <QDebug>

#include "control/controlencoder.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "effects/effectxmlelements.h"
#include "moc_effectslot.cpp"
#include "util/math.h"
#include "util/xml.h"

// The maximum number of effect parameters we're going to support.
const unsigned int kDefaultMaxParameters = 16;

EffectSlot::EffectSlot(const QString& group,
                       const unsigned int iChainNumber,
                       const unsigned int iEffectnumber)
        : m_iChainNumber(iChainNumber),
          m_iEffectNumber(iEffectnumber),
          m_group(group) {
    m_pControlLoaded = new ControlObject(ConfigKey(m_group, "loaded"));
    m_pControlLoaded->setReadOnly();

    m_pControlNumParameters = new ControlObject(ConfigKey(m_group, "num_parameters"));
    m_pControlNumParameters->setReadOnly();

    m_pControlNumParameterSlots = new ControlObject(ConfigKey(m_group, "num_parameterslots"));
    m_pControlNumParameterSlots->setReadOnly();

    m_pControlNumButtonParameters = new ControlObject(ConfigKey(m_group, "num_button_parameters"));
    m_pControlNumButtonParameters->setReadOnly();

    m_pControlNumButtonParameterSlots = new ControlObject(ConfigKey(m_group, "num_button_parameterslots"));
    m_pControlNumButtonParameterSlots->setReadOnly();

    // Default to disabled to prevent accidental activation of effects
    // at the beginning of a set.
    m_pControlEnabled = new ControlPushButton(ConfigKey(m_group, "enabled"));
    m_pControlEnabled->setButtonMode(ControlPushButton::POWERWINDOW);
    connect(m_pControlEnabled, &ControlPushButton::valueChanged, this, &EffectSlot::slotEnabled);

    m_pControlNextEffect = new ControlPushButton(ConfigKey(m_group, "next_effect"));
    connect(m_pControlNextEffect,
            &ControlPushButton::valueChanged,
            this,
            &EffectSlot::slotNextEffect);

    m_pControlPrevEffect = new ControlPushButton(ConfigKey(m_group, "prev_effect"));
    connect(m_pControlPrevEffect,
            &ControlPushButton::valueChanged,
            this,
            &EffectSlot::slotPrevEffect);

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pControlEffectSelector = new ControlEncoder(ConfigKey(m_group, "effect_selector"), false);
    connect(m_pControlEffectSelector,
            &ControlEncoder::valueChanged,
            this,
            &EffectSlot::slotEffectSelector);

    m_pControlClear = new ControlPushButton(ConfigKey(m_group, "clear"));
    connect(m_pControlClear, &ControlPushButton::valueChanged, this, &EffectSlot::slotClear);

    for (unsigned int i = 0; i < kDefaultMaxParameters; ++i) {
        addEffectParameterSlot();
        addEffectButtonParameterSlot();
    }

    m_pControlMetaParameter = new ControlPotmeter(ConfigKey(m_group, "meta"), 0.0, 1.0);
    connect(m_pControlMetaParameter, &ControlPotmeter::valueChanged, this, [this](double value) {
        slotEffectMetaParameter(value, false);
    });
    m_pControlMetaParameter->set(0.0);
    m_pControlMetaParameter->setDefaultValue(0.0);

    m_pSoftTakeover = new SoftTakeover();

    clear();
}

EffectSlot::~EffectSlot() {
    //qDebug() << debugString() << "destroyed";
    clear();

    delete m_pControlLoaded;
    delete m_pControlNumParameters;
    delete m_pControlNumParameterSlots;
    delete m_pControlNumButtonParameters;
    delete m_pControlNumButtonParameterSlots;
    delete m_pControlNextEffect;
    delete m_pControlPrevEffect;
    delete m_pControlEffectSelector;
    delete m_pControlClear;
    delete m_pControlEnabled;
    delete m_pControlMetaParameter;
    delete m_pSoftTakeover;
}

EffectParameterSlotPointer EffectSlot::addEffectParameterSlot() {
    EffectParameterSlotPointer pParameter = EffectParameterSlotPointer(
            new EffectParameterSlot(m_group, m_parameters.size()));
    m_parameters.append(pParameter);
    m_pControlNumParameterSlots->forceSet(
            m_pControlNumParameterSlots->get() + 1);
    return pParameter;
}

EffectButtonParameterSlotPointer EffectSlot::addEffectButtonParameterSlot() {
    EffectButtonParameterSlotPointer pParameter = EffectButtonParameterSlotPointer(
            new EffectButtonParameterSlot(m_group, m_buttonParameters.size()));
    m_buttonParameters.append(pParameter);
    m_pControlNumButtonParameterSlots->forceSet(
            m_pControlNumButtonParameterSlots->get() + 1);
    return pParameter;
}

EffectPointer EffectSlot::getEffect() const {
    return m_pEffect;
}

unsigned int EffectSlot::numParameterSlots() const {
    return m_parameters.size();
}

unsigned int EffectSlot::numButtonParameterSlots() const {
    return m_buttonParameters.size();
}

void EffectSlot::slotEnabled(double v) {
    //qDebug() << debugString() << "slotEnabled" << v;
    if (m_pEffect) {
        m_pEffect->setEnabled(v > 0);
    }
}

void EffectSlot::slotEffectEnabledChanged(bool enabled) {
    m_pControlEnabled->set(enabled);
}

EffectParameterSlotPointer EffectSlot::getEffectParameterSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectParameterSlot" << slotNumber;
    if (slotNumber >= static_cast<unsigned int>(m_parameters.size())) {
        qWarning() << "WARNING: slotNumber out of range";
        return EffectParameterSlotPointer();
    }
    return m_parameters[slotNumber];
}

EffectButtonParameterSlotPointer EffectSlot::getEffectButtonParameterSlot(unsigned int slotNumber) {
    //qDebug() << debugString() << "getEffectParameterSlot" << slotNumber;
    if (slotNumber >= static_cast<unsigned int>(m_buttonParameters.size())) {
        qWarning() << "WARNING: slotNumber out of range";
        return EffectButtonParameterSlotPointer();
    }
    return m_buttonParameters[slotNumber];
}

void EffectSlot::loadEffect(EffectPointer pEffect, bool adoptMetaknobPosition) {
    //qDebug() << debugString() << "loadEffect"
    //         << (pEffect ? pEffect->getManifest().name() : "(null)");
    if (pEffect) {
        m_pEffect = pEffect;
        m_pControlLoaded->forceSet(1.0);
        m_pControlNumParameters->forceSet(pEffect->numKnobParameters());
        m_pControlNumButtonParameters->forceSet(pEffect->numButtonParameters());

        // The enabled status persists in the EffectSlot when loading a new
        // EffectPointer to the EffectSlot. Effects and EngineEffects default to
        // disabled, so if this EffectSlot was enabled, enable the Effect and EngineEffect.
        pEffect->setEnabled(m_pControlEnabled->toBool());

        connect(pEffect.data(),
                &Effect::enabledChanged,
                this,
                &EffectSlot::slotEffectEnabledChanged);

        while (static_cast<unsigned int>(m_parameters.size())
                < pEffect->numKnobParameters()) {
            addEffectParameterSlot();
        }

        while (static_cast<unsigned int>(m_buttonParameters.size())
                < pEffect->numButtonParameters()) {
            addEffectButtonParameterSlot();
        }

        for (const auto& pParameter : qAsConst(m_parameters)) {
            pParameter->loadEffect(pEffect);
        }
        for (const auto& pParameter : qAsConst(m_buttonParameters)) {
            pParameter->loadEffect(pEffect);
        }

        if (adoptMetaknobPosition) {
            slotEffectMetaParameter(m_pControlMetaParameter->get(), true);
        } else {
            m_pControlMetaParameter->set(pEffect->getMetaknobDefault());
            slotEffectMetaParameter(pEffect->getMetaknobDefault(), true);
        }

        emit effectLoaded(pEffect, m_iEffectNumber);
    } else {
        clear();
        // Broadcasts a null effect pointer
        emit effectLoaded(EffectPointer(), m_iEffectNumber);
    }
    emit updated();
}

void EffectSlot::clear() {
    if (m_pEffect) {
        m_pEffect->disconnect(this);
    }
    m_pControlLoaded->forceSet(0.0);
    m_pControlNumParameters->forceSet(0.0);
    m_pControlNumButtonParameters->forceSet(0.0);
    for (const auto& pParameter : qAsConst(m_parameters)) {
        pParameter->clear();
    }
    for (const auto& pParameter : qAsConst(m_buttonParameters)) {
        pParameter->clear();
    }
    m_pEffect.clear();
    emit updated();
}

void EffectSlot::slotPrevEffect(double v) {
    if (v > 0) {
        slotEffectSelector(-1);
    }
}

void EffectSlot::slotNextEffect(double v) {
    if (v > 0) {
        slotEffectSelector(1);
    }
}

void EffectSlot::slotEffectSelector(double v) {
    if (v > 0) {
        emit nextEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    } else if (v < 0) {
        emit prevEffect(m_iChainNumber, m_iEffectNumber, m_pEffect);
    }
}

void EffectSlot::slotClear(double v) {
    if (v > 0) {
        emit clearEffect(m_iEffectNumber);
    }
}

void EffectSlot::syncSofttakeover() {
    for (const auto& pParameterSlot : qAsConst(m_parameters)) {
        pParameterSlot->syncSofttakeover();
    }
}

double EffectSlot::getMetaParameter() const {
    return m_pControlMetaParameter->get();
}

// This function is for the superknob to update individual effects' meta knobs
// slotEffectMetaParameter does not need to update m_pControlMetaParameter's value
void EffectSlot::setMetaParameter(double v, bool force) {
    if (!m_pSoftTakeover->ignore(m_pControlMetaParameter, v)
            || !m_pControlEnabled->toBool()
            || force) {
        m_pControlMetaParameter->set(v);
        slotEffectMetaParameter(v, force);
    }
}

void EffectSlot::slotEffectMetaParameter(double v, bool force) {
    // Clamp to [0.0, 1.0]
    if (v < 0.0 || v > 1.0) {
        qWarning() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0, 1.0);
        m_pControlMetaParameter->set(v);
    }
    if (!m_pControlEnabled->toBool()) {
        force = true;
    }
    for (const auto& pParameterSlot : qAsConst(m_parameters)) {
        pParameterSlot->onEffectMetaParameterChanged(v, force);
    }
}

QDomElement EffectSlot::toXml(QDomDocument* doc) const {
    QDomElement effectElement = doc->createElement(EffectXml::Effect);
    if (!m_pEffect) {
        return effectElement;
    }

    QDomElement metaKnobElement = doc->createElement(EffectXml::EffectMetaParameter);
    XmlParse::addElement(*doc, effectElement,
                         EffectXml::EffectMetaParameter,
                         QString::number(m_pControlMetaParameter->get()));
    EffectManifestPointer pManifest = m_pEffect->getManifest();
    XmlParse::addElement(*doc, effectElement,
                         EffectXml::EffectId, pManifest->id());
    XmlParse::addElement(*doc, effectElement,
                         EffectXml::EffectVersion, pManifest->version());

    QDomElement parametersElement = doc->createElement(EffectXml::ParametersRoot);

    for (const auto& pParameter : m_parameters) {
        QDomElement parameterElement = pParameter->toXml(doc);
        if (!parameterElement.hasChildNodes()) {
            continue;
        }
        EffectManifestParameterPointer manifest = pParameter->getManifest();
        if (!manifest) {
            continue;
        }
        XmlParse::addElement(*doc, parameterElement,
                             EffectXml::ParameterId,
                             manifest->id());
        parametersElement.appendChild(parameterElement);
    }
    for (const auto& pParameter : m_buttonParameters) {
        QDomElement parameterElement = pParameter->toXml(doc);
        if (!parameterElement.hasChildNodes()) {
            continue;
        }
        EffectManifestParameterPointer manifest = pParameter->getManifest();
        if (!manifest) {
            continue;
        }
        XmlParse::addElement(*doc, parameterElement,
                             EffectXml::ParameterId,
                             pParameter->getManifest()->id());
        parametersElement.appendChild(parameterElement);
    }

    effectElement.appendChild(parametersElement);

    return effectElement;
}

void EffectSlot::loadEffectSlotFromXml(const QDomElement& effectElement) {
    if (!m_pEffect) {
        return;
    }

    if (!effectElement.hasChildNodes()) {
        return;
    }

    QDomElement effectIdElement = XmlParse::selectElement(effectElement,
                                                          EffectXml::EffectId);
    if (m_pEffect->getManifest()->id() != effectIdElement.text()) {
        qWarning() << "EffectSlot::loadEffectSlotFromXml"
                   << "effect ID in XML does not match presently loaded effect, ignoring.";
        return;
    }

    m_pControlMetaParameter->set(XmlParse::selectNodeDouble(
            effectElement, EffectXml::EffectMetaParameter));
    QDomElement parametersElement = XmlParse::selectElement(
            effectElement, EffectXml::ParametersRoot);
    if (!parametersElement.hasChildNodes()) {
        return;
    }

    QMap<QString, EffectParameterSlotBasePointer> parametersById;
    for (const auto& pParameter : qAsConst(m_parameters)) {
        EffectManifestParameterPointer manifest = pParameter->getManifest();
        if (manifest) {
            parametersById.insert(manifest->id(), pParameter);
        }
    }
    for (const auto& pParameter : qAsConst(m_buttonParameters)) {
        EffectManifestParameterPointer manifest = pParameter->getManifest();
        if (manifest) {
            parametersById.insert(manifest->id(), pParameter);
        }
    }

    QDomNodeList parametersNodeList = parametersElement.childNodes();
    for (int i = 0; i < parametersNodeList.size(); ++i) {
        QDomNode parameterNode = parametersNodeList.at(i);
        if (parameterNode.isElement()) {
            const QString id = XmlParse::selectNodeQString(parameterNode,
                                                           EffectXml::ParameterId);
            EffectParameterSlotBasePointer pParameterSlot = parametersById.value(id);
            if (pParameterSlot != nullptr) {
                pParameterSlot->loadParameterSlotFromXml(parameterNode.toElement());
            }
        }
    }
}
