#include <QtDebug>

#include "effects/effectmanifestparameter.h"

EffectManifestParameter::EffectManifestParameter(QObject* pParent)
        : QObject() {
}

EffectManifestParameter::~EffectManifestParameter() {
    qDebug() << *this << "destroyed";
}

const QString EffectManifestParameter::id() const {
    return m_id;
}

void EffectManifestParameter::setId(QString id) {
    m_id = id;
}

const QString EffectManifestParameter::name() const {
    return m_name;
}

void EffectManifestParameter::setName(QString name) {
    m_name = name;
}

const QString EffectManifestParameter::description() const {
    return m_description;
}

void EffectManifestParameter::setDescription(QString description) {
    m_description = description;
}

EffectManifestParameter::ControlHint EffectManifestParameter::controlHint() const {
    return m_controlHint;
}

void EffectManifestParameter::setControlHint(ControlHint controlHint) {
    m_controlHint = controlHint;
}

EffectManifestParameter::ValueHint EffectManifestParameter::valueHint() const {
    return m_valueHint;
}

void EffectManifestParameter::setValueHint(ValueHint valueHint) {
    m_valueHint = valueHint;
}

EffectManifestParameter::SemanticHint EffectManifestParameter::semanticHint() const {
    return m_semanticHint;
}

void EffectManifestParameter::setSemanticHint(SemanticHint semanticHint) {
    m_semanticHint = semanticHint;
}

EffectManifestParameter::UnitsHint EffectManifestParameter::unitsHint() const {
    return m_unitsHint;
}

void EffectManifestParameter::setUnitsHint(UnitsHint unitsHint) {
    m_unitsHint = unitsHint;
}

bool EffectManifestParameter::hasDefault() const {
    return m_default.isValid();
}

QVariant EffectManifestParameter::getDefault() const {
    return m_default;
}

void EffectManifestParameter::setDefault(QVariant defaultValue) {
    m_default = defaultValue;
}

bool EffectManifestParameter::hasMinimum() const {
    return m_minimum.isValid();
}

QVariant EffectManifestParameter::getMinimum() const {
    return m_minimum;
}

void EffectManifestParameter::setMinimum(QVariant minimum) {
    m_minimum = minimum;
}

bool EffectManifestParameter::hasMaximum() const {
    return m_maximum.isValid();
}

QVariant EffectManifestParameter::getMaximum() const {
    return m_maximum;
}

void EffectManifestParameter::setMaximum(QVariant maximum) {
    m_maximum = maximum;
}

QDebug operator<<(QDebug dbg, const EffectManifestParameter &parameter) {
    return dbg.maybeSpace() << QString("EffectManifestParameter(%1)").arg(parameter.id());
}
