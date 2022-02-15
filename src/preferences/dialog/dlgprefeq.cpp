#include "preferences/dialog/dlgprefeq.h"

#include <QHBoxLayout>
#include <QString>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "effects/builtin/biquadfullkilleqeffect.h"
#include "effects/builtin/filtereffect.h"
#include "effects/effectrack.h"
#include "effects/effectslot.h"
#include "engine/filters/enginefilterbessel4.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefeq.cpp"
#include "util/math.h"

const QString kConfigKey = "[Mixer Profile]";
const QString kEnableEqs = "EnableEQs";
const QString kEqsOnly = "EQsOnly";
const QString kSingleEq = "SingleEQEffect";
const QString kDefaultEqId = BiquadFullKillEQEffect::getId();
const QString kDefaultMasterEqId = QString();
const QString kDefaultQuickEffectId = FilterEffect::getId();

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

DlgPrefEQ::DlgPrefEQ(QWidget* pParent, EffectsManager* pEffectsManager, UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_COLoFreq(kConfigKey, "LoEQFrequency"),
          m_COHiFreq(kConfigKey, "HiEQFrequency"),
          m_pConfig(pConfig),
          m_lowEqFreq(0.0),
          m_highEqFreq(0.0),
          m_pEffectsManager(pEffectsManager),
          m_firstSelectorLabel(nullptr),
          m_pNumDecks(nullptr),
          m_numDecks(0) {
    m_pEQEffectRack = m_pEffectsManager->getEqualizerRack(0);
    m_pQuickEffectRack = m_pEffectsManager->getQuickEffectRack(0);
    m_pOutputEffectRack = m_pEffectsManager->getOutputsEffectRack();

    setupUi(this);

    connect(SliderHiEQ, &QAbstractSlider::valueChanged, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QAbstractSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateHiEQ);
    connect(SliderHiEQ, &QAbstractSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateHiEQ);

    connect(SliderLoEQ, &QAbstractSlider::valueChanged, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QAbstractSlider::sliderMoved, this, &DlgPrefEQ::slotUpdateLoEQ);
    connect(SliderLoEQ, &QAbstractSlider::sliderReleased, this, &DlgPrefEQ::slotUpdateLoEQ);

    connect(CheckBoxBypass, &QCheckBox::stateChanged, this, &DlgPrefEQ::slotBypass);

    connect(CheckBoxEqOnly,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefEQ::slotPopulateDeckEffectSelectors);

    connect(CheckBoxSingleEqEffect,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefEQ::slotSingleEqChecked);

    loadSettings();

    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(this, &DlgPrefEQ::slotNumDecksChanged);
    slotNumDecksChanged(m_pNumDecks->get());

    setUpMasterEQ();

    slotApply();
}

DlgPrefEQ::~DlgPrefEQ() {
    qDeleteAll(m_deckEqEffectSelectors);
    m_deckEqEffectSelectors.clear();

    qDeleteAll(m_deckQuickEffectSelectors);
    m_deckQuickEffectSelectors.clear();

    qDeleteAll(m_filterWaveformEnableCOs);
    m_filterWaveformEnableCOs.clear();
}

// Add drop down lists for new decks, restore selection from config
void DlgPrefEQ::slotNumDecksChanged(double numDecks) {
    int oldDecks = m_deckEqEffectSelectors.size();
    m_numDecks = static_cast<int>(numDecks);
    while (m_deckEqEffectSelectors.size() < m_numDecks) {
        int deckNo = m_deckEqEffectSelectors.size() + 1;

        QLabel* label = new QLabel(QObject::tr("Deck %1").arg(deckNo), this);

        QString group = PlayerManager::groupForDeck(
                m_deckEqEffectSelectors.size());

        m_filterWaveformEnableCOs.append(
                new ControlObject(ConfigKey(group, "filterWaveformEnable")));
        m_filterWaveformEffectLoaded.append(false);

        // Create the drop down list for deck EQs
        QComboBox* eqComboBox = new QComboBox(this);
        m_deckEqEffectSelectors.append(eqComboBox);

        // Create the drop down list for Quick Effects
        QComboBox* quickEffectComboBox = new QComboBox(this);
        m_deckQuickEffectSelectors.append(quickEffectComboBox);

        if (deckNo == 1) {
            m_firstSelectorLabel = label;
            if (CheckBoxSingleEqEffect->isChecked()) {
                m_firstSelectorLabel->clear();
            }
        }

        // Setup the GUI
        gridLayout_3->addWidget(label, deckNo, 0);
        gridLayout_3->addWidget(eqComboBox, deckNo, 1);
        gridLayout_3->addWidget(quickEffectComboBox, deckNo, 2);
        gridLayout_3->addItem(new QSpacerItem(
                40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum),
                deckNo, 3, 1, 1);
    }
    slotPopulateDeckEffectSelectors();
    loadEffectSelection(oldDecks);
    applySelections();
    // This is only required to update the widgets if numDecks changed
    // while the preferences are open (slotUpdate() not called)
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());
    slotBypass(CheckBoxBypass->isChecked());
}

static bool isMixingEQ(EffectManifest* pManifest) {
    return pManifest->isMixingEQ();
}

static bool isMasterEQ(EffectManifest* pManifest) {
    return pManifest->isMasterEQ();
}

static bool hasSuperKnobLinking(EffectManifest* pManifest) {
    for (const auto& pParameterManifest : pManifest->parameters()) {
        if (pParameterManifest->defaultLinkType() !=
            EffectManifestParameter::LinkType::NONE) {
            return true;
        }
    }
    return false;
}

// Populate comboboxes with all available effects
void DlgPrefEQ::slotPopulateDeckEffectSelectors() {
    EffectsManager::EffectManifestFilterFnc filterEQ;
    filterEQ = CheckBoxEqOnly->isChecked() ? isMixingEQ : nullptr;

    const QList<EffectManifestPointer> availableEQEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(filterEQ);
    const QList<EffectManifestPointer> availableQuickEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(hasSuperKnobLinking);

    for (QComboBox* box : qAsConst(m_deckEqEffectSelectors)) {
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1; // Nothing selected

        // Add empty item at the top: no deck EQ
        box->addItem(EffectsManager::kNoEffectString);
        int i;
        // Select the previous EQ if it's available in the updated list
        for (i = 0; i < availableEQEffects.size(); ++i) {
            EffectManifestPointer pManifest = availableEQEffects.at(i);
            box->addItem(pManifest->name(), QVariant(pManifest->id()));
            if (selectedEffectId == pManifest->id()) {
                currentIndex = i + 1;
            }
        }

        if (selectedEffectId.isEmpty()) {
            // Configured effect has no id, clear selection
            currentIndex = 0;
        } else if (currentIndex < 0 && !selectedEffectName.isEmpty() ) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName, QVariant(selectedEffectId));
            currentIndex = i + 1;
        }
        box->setCurrentIndex(currentIndex);
    }

    for (QComboBox* box : qAsConst(m_deckQuickEffectSelectors)) {
        // Save current selection
        QString selectedEffectId = box->itemData(box->currentIndex()).toString();
        QString selectedEffectName = box->itemText(box->currentIndex());
        box->clear();
        int currentIndex = -1;// Nothing selected

        // Add empty item at the top: no Quick Effect
        box->addItem(EffectsManager::kNoEffectString);

        int i;
        // Select the previous QuickEffect if it's available in the updated list
        for (i = 0; i < availableQuickEffects.size(); ++i) {
            EffectManifestPointer pManifest = availableQuickEffects.at(i);
            box->addItem(pManifest->name(), QVariant(pManifest->id()));
            if (selectedEffectId == pManifest->id()) {
                currentIndex = i + 1;
            }
        }

        if (selectedEffectId.isEmpty()) {
            // Configured effect has no id, clear selection
            currentIndex = 0;
        } else if (currentIndex < 0 && !selectedEffectName.isEmpty()) {
            // current selection is not part of the new list
            // So we need to add it
            box->addItem(selectedEffectName, QVariant(selectedEffectId));
            currentIndex = i + 1;
        }
        box->setCurrentIndex(currentIndex);
    }
}

// Show comboboxes of all decks, or one for all
void DlgPrefEQ::slotSingleEqChecked(int checked) {
    bool do_hide = static_cast<bool>(checked);
    for (int i = 2; i < m_deckEqEffectSelectors.size() + 1; ++i) {
        if (do_hide) {
            gridLayout_3->itemAtPosition(i, 0)->widget()->hide();
            gridLayout_3->itemAtPosition(i, 1)->widget()->hide();
            gridLayout_3->itemAtPosition(i, 2)->widget()->hide();
        } else {
            gridLayout_3->itemAtPosition(i, 0)->widget()->show();
            gridLayout_3->itemAtPosition(i, 1)->widget()->show();
            gridLayout_3->itemAtPosition(i, 2)->widget()->show();
        }
    }

    if (m_firstSelectorLabel != nullptr) {
        if (do_hide) {
            m_firstSelectorLabel->clear();
        } else {
            m_firstSelectorLabel->setText(QObject::tr("Deck 1"));
        }
    }
}

QUrl DlgPrefEQ::helpUrl() const {
    return QUrl(MIXXX_MANUAL_EQ_URL);
}

// Load settings from config, except combobxes, update GUI
void DlgPrefEQ::loadSettings() {
    QString highEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequency"));
    QString highEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"));
    QString lowEqCourse = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequency"));
    QString lowEqPrecise = m_pConfig->getValueString(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"));
    CheckBoxEqAutoReset->setChecked(static_cast<bool>(
            m_pConfig->getValueString(ConfigKey(kConfigKey, "EqAutoReset"))
                    .toInt()));
    CheckBoxGainAutoReset->setChecked(static_cast<bool>(
            m_pConfig->getValueString(ConfigKey(kConfigKey, "GainAutoReset"))
                    .toInt()));
    CheckBoxBypass->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kEnableEqs), QString("yes")) == "no");
    CheckBoxEqOnly->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kEqsOnly), "yes") == "yes");
    CheckBoxSingleEqEffect->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigKey, kSingleEq), "yes") == "yes");
    // State may not have changed, thus stateChanged() not not emitted.
    // Call explicitly
    slotBypass(CheckBoxBypass->isChecked());
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());

    double lowEqFreq = 0.0;
    double highEqFreq = 0.0;

    // Precise takes precedence over course.
    lowEqFreq = lowEqCourse.isEmpty() ? lowEqFreq : lowEqCourse.toDouble();
    lowEqFreq = lowEqPrecise.isEmpty() ? lowEqFreq : lowEqPrecise.toDouble();
    highEqFreq = highEqCourse.isEmpty() ? highEqFreq : highEqCourse.toDouble();
    highEqFreq = highEqPrecise.isEmpty() ? highEqFreq : highEqPrecise.toDouble();

    if (lowEqFreq == 0.0 || highEqFreq == 0.0 || lowEqFreq == highEqFreq) {
        setDefaultShelves();
    } else {
        m_highEqFreq = highEqFreq;
        m_lowEqFreq = lowEqFreq;
    }

    SliderHiEQ->setValue(
            getSliderPosition(m_highEqFreq,
                    SliderHiEQ->minimum(),
                    SliderHiEQ->maximum()));
    SliderLoEQ->setValue(
            getSliderPosition(m_lowEqFreq,
                    SliderLoEQ->minimum(),
                    SliderLoEQ->maximum()));
}

// Load EQ and QuickEffect selection from config, update GUI
void DlgPrefEQ::loadEffectSelection(int oldDeckCount) {
    VERIFY_OR_DEBUG_ASSERT(oldDeckCount >= 0) {
        return;
    }
    for (int i = oldDeckCount; i < m_numDecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);

        // EQ
        QString configuredEffect = m_pConfig->getValue(
                ConfigKey(kConfigKey, "EffectForGroup_" + group), kDefaultEqId);
        int selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(configuredEffect);
        if (selectedEffectIndex < 0) {
            selectedEffectIndex = m_deckEqEffectSelectors[i]->findData(kDefaultEqId);
            configuredEffect = kDefaultEqId;
        }
        m_deckEqEffectSelectors[i]->setCurrentIndex(selectedEffectIndex);
        m_filterWaveformEffectLoaded[i] = m_pEffectsManager->isEQ(configuredEffect);

        // QuickEffect
        QString configuredQuickEffect = m_pConfig->getValue(
                ConfigKey(kConfigKey, "QuickEffectForGroup_" + group), kDefaultQuickEffectId);
        int selectedQuickEffectIndex =
                m_deckQuickEffectSelectors[i]->findData(configuredQuickEffect);
        if (selectedQuickEffectIndex < 0) {
            selectedQuickEffectIndex =
                    m_deckQuickEffectSelectors[i]->findData(kDefaultQuickEffectId);
            configuredEffect = kDefaultQuickEffectId;
        }
        m_deckQuickEffectSelectors[i]->setCurrentIndex(selectedQuickEffectIndex);
    }
}

void DlgPrefEQ::setDefaultShelves() {
    m_highEqFreq = 2500;
    m_lowEqFreq = 250;
}

void DlgPrefEQ::slotResetToDefaults() {
    slotMasterEQToDefault();

    setDefaultShelves();
    foreach(QComboBox* pCombo, m_deckEqEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultEqId));
    }
    foreach(QComboBox* pCombo, m_deckQuickEffectSelectors) {
        pCombo->setCurrentIndex(
               pCombo->findData(kDefaultQuickEffectId));
    }
    CheckBoxEqOnly->setChecked(true);

    CheckBoxSingleEqEffect->setChecked(true);
    CheckBoxEqAutoReset->setChecked(false);
    CheckBoxGainAutoReset->setChecked(false);
    CheckBoxBypass->setChecked(false);
    slotSingleEqChecked(CheckBoxSingleEqEffect->isChecked());
    slotBypass(CheckBoxBypass->isChecked());

    slotApply();
}

// Apply EQ and effect selection, write to config
void DlgPrefEQ::applySelections() {
    int deck = 0;
    QString firstEffectId;
    int firstEffectIndex = 0;
    for (QComboBox* box : qAsConst(m_deckEqEffectSelectors)) {
        QString effectId = box->itemData(box->currentIndex()).toString();
        // If "Single EQ" is checked use the first EQ/effect for all decks
        if (deck == 0) {
            firstEffectId = effectId;
            firstEffectIndex = box->currentIndex();
        } else if (CheckBoxSingleEqEffect->isChecked()) {
            effectId = firstEffectId;
            box->setCurrentIndex(firstEffectIndex);
        }
        QString group = PlayerManager::groupForDeck(deck);

        // Only apply the effect if it changed -- so first interrogate the
        // loaded effect if any.
        bool need_load = true;
        if (m_pEQEffectRack->numEffectChainSlots() > deck) {
            // It's not correct to get a chainslot by index number -- get by
            // group name instead.
            EffectChainSlotPointer chainslot =
                    m_pEQEffectRack->getGroupEffectChainSlot(group);
            if (chainslot && chainslot->numSlots()) {
                EffectPointer effectpointer =
                        chainslot->getEffectSlot(0)->getEffect();
                if (effectpointer &&
                        effectpointer->getManifest()->id() == effectId) {
                    need_load = false;
                }
            }
        }
        if (need_load) {
            EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
            m_pEQEffectRack->loadEffectToGroup(group, pEffect);
            m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_" + group),
                    ConfigValue(effectId));
            m_filterWaveformEnableCOs[deck]->set(m_pEffectsManager->isEQ(effectId));

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
    }

    deck = 0;
    for (QComboBox* box : qAsConst(m_deckQuickEffectSelectors)) {
        QString effectId = box->itemData(box->currentIndex()).toString();
        QString group = PlayerManager::groupForDeck(deck);

        if (deck == 0) {
            firstEffectId = effectId;
            firstEffectIndex = box->currentIndex();
        } else if (CheckBoxSingleEqEffect->isChecked()) {
            effectId = firstEffectId;
            box->setCurrentIndex(firstEffectIndex);
        }

        // Only apply the effect if it changed -- so first interrogate the
        // loaded effect if any.
        bool need_load = true;
        if (m_pQuickEffectRack->numEffectChainSlots() > deck) {
            // It's not correct to get a chainslot by index number -- get by
            // group name instead.
            EffectChainSlotPointer chainslot =
                    m_pQuickEffectRack->getGroupEffectChainSlot(group);
            if (chainslot && chainslot->numSlots()) {
                EffectPointer effectpointer =
                        chainslot->getEffectSlot(0)->getEffect();
                if (effectpointer &&
                        effectpointer->getManifest()->id() == effectId) {
                    need_load = false;
                }
            }
        }
        if (need_load) {
            EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
            m_pQuickEffectRack->loadEffectToGroup(group, pEffect);

            m_pConfig->set(ConfigKey(kConfigKey, "QuickEffectForGroup_" + group),
                    ConfigValue(effectId));

            // This is required to remove a previous selected effect that does not
            // fit to the current ShowAllEffects checkbox
            slotPopulateDeckEffectSelectors();
        }
        ++deck;
    }
}

void DlgPrefEQ::slotUpdateHiEQ() {
    if (SliderHiEQ->value() < SliderLoEQ->value())
    {
        SliderHiEQ->setValue(SliderLoEQ->value());
    }
    m_highEqFreq = getEqFreq(SliderHiEQ->value(),
                             SliderHiEQ->minimum(),
                             SliderHiEQ->maximum());
    validate_levels();
    if (m_highEqFreq < 1000) {
        TextHiEQ->setText( QString("%1 Hz").arg((int)m_highEqFreq));
    } else {
        TextHiEQ->setText( QString("%1 kHz").arg((int)m_highEqFreq / 1000.));
    }
}

void DlgPrefEQ::slotUpdateLoEQ() {
    if (SliderLoEQ->value() > SliderHiEQ->value())
    {
        SliderLoEQ->setValue(SliderHiEQ->value());
    }
    m_lowEqFreq = getEqFreq(SliderLoEQ->value(),
                            SliderLoEQ->minimum(),
                            SliderLoEQ->maximum());
    validate_levels();
    if (m_lowEqFreq < 1000) {
        TextLoEQ->setText(QString("%1 Hz").arg((int)m_lowEqFreq));
    } else {
        TextLoEQ->setText(QString("%1 kHz").arg((int)m_lowEqFreq / 1000.));
    }
}

void DlgPrefEQ::slotUpdateMasterEQParameter(int value) {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        QSlider* slider = qobject_cast<QSlider*>(sender());
        int index = slider->property("index").toInt();
        EffectParameter* param = effect->getKnobParameterForSlot(index);
        if (param) {
            double dValue = value / 100.0;
            param->setValue(dValue);
            QLabel* valueLabel = m_masterEQValues[index];
            QString valueText = QString::number(dValue);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigKey,
                    QString("EffectForGroup_[Master]_parameter%1").arg(index + 1)),
                            ConfigValue(valueText));
        }
    }
}

int DlgPrefEQ::getSliderPosition(double eqFreq, int minValue, int maxValue) {
    if (eqFreq >= kFrequencyUpperLimit) {
        return maxValue;
    } else if (eqFreq <= kFrequencyLowerLimit) {
        return minValue;
    }
    double dsliderPos = (eqFreq - kFrequencyLowerLimit) / (kFrequencyUpperLimit-kFrequencyLowerLimit);
    dsliderPos = pow(dsliderPos, 1.0 / 4.0) * (maxValue - minValue) + minValue;
    return static_cast<int>(dsliderPos);
}

void DlgPrefEQ::slotApply() {
    m_pConfig->set(ConfigKey(kConfigKey, kEqsOnly),
            CheckBoxEqOnly->isChecked() ? QString("yes") : QString("no"));
    m_pConfig->set(ConfigKey(kConfigKey, kSingleEq),
            CheckBoxSingleEqEffect->isChecked() ? QString("yes") : QString("no"));
    applySelections();

    m_COLoFreq.set(m_lowEqFreq);
    m_COHiFreq.set(m_highEqFreq);
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_highEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "HiEQFrequencyPrecise"),
            ConfigValue(QString::number(m_highEqFreq, 'f')));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequency"),
            ConfigValue(QString::number(static_cast<int>(m_lowEqFreq))));
    m_pConfig->set(ConfigKey(kConfigKey, "LoEQFrequencyPrecise"),
            ConfigValue(QString::number(m_lowEqFreq, 'f')));

    m_pConfig->set(ConfigKey(kConfigKey, "EqAutoReset"),
            ConfigValue(CheckBoxEqAutoReset->isChecked() ? 1 : 0));
    m_pConfig->set(ConfigKey(kConfigKey, "GainAutoReset"),
            ConfigValue(CheckBoxGainAutoReset->isChecked() ? 1 : 0));
    // Note the inversion: GUI label is "Bypass EQs", config key is "EnableEQs"
    m_pConfig->set(ConfigKey(kConfigKey, kEnableEqs),
            CheckBoxBypass->isChecked() ? QString("no") : QString("yes"));
}

// Reload settings and effect selection from config, update GUI
void DlgPrefEQ::slotUpdate() {
    loadSettings();
    loadEffectSelection(0);
}

// De/activate EQ controls when Bypass checkbox is toggled
void DlgPrefEQ::slotBypass(int state) {
    bool enabled = !static_cast<bool>(state);
    for (const auto& box : qAsConst(m_deckEqEffectSelectors)) {
        box->setEnabled(enabled);
    }
    SliderHiEQ->setEnabled(enabled);
    SliderLoEQ->setEnabled(enabled);
    CheckBoxEqOnly->setEnabled(enabled);
    CheckBoxSingleEqEffect->setEnabled(enabled);
    CheckBoxEqAutoReset->setEnabled(enabled);
}

void DlgPrefEQ::setUpMasterEQ() {
    connect(pbResetMasterEq, &QAbstractButton::clicked, this, &DlgPrefEQ::slotMasterEQToDefault);

    connect(comboBoxMasterEq,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefEQ::slotMasterEqEffectChanged);

    QString configuredEffect = m_pConfig->getValue(ConfigKey(kConfigKey,
            "EffectForGroup_[Master]"), kDefaultMasterEqId);

    const QList<EffectManifestPointer> availableMasterEQEffects =
        m_pEffectsManager->getAvailableEffectManifestsFiltered(isMasterEQ);

    // Add empty item at the top: no Master EQ
    comboBoxMasterEq->addItem(EffectsManager::kNoEffectString);
    for (const auto& pManifest : availableMasterEQEffects) {
        comboBoxMasterEq->addItem(pManifest->name(), QVariant(pManifest->id()));
    }

    int masterEqIndex = comboBoxMasterEq->findData(configuredEffect);
    if (masterEqIndex < 0) {
        // Configured effect not in list, clear selection
        masterEqIndex = 0;
    }
    comboBoxMasterEq->setCurrentIndex(masterEqIndex);

    // Load parameters from preferences:
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        int knobNum = effect->numKnobParameters();
        for (int i = 0; i < knobNum; i++) {
            EffectParameter* param = effect->getKnobParameterForSlot(i);
            if (param) {
                QString strValue = m_pConfig->getValueString(ConfigKey(kConfigKey,
                        QString("EffectForGroup_[Master]_parameter%1").arg(i + 1)));
                bool ok;
                double value = strValue.toDouble(&ok);
                if (ok) {
                    setMasterEQParameter(i, value);
                }
            }
        }
    }
}

void DlgPrefEQ::slotMasterEqEffectChanged(int effectIndex) {
    // clear parameters view first
    qDeleteAll(m_masterEQSliders);
    m_masterEQSliders.clear();
    qDeleteAll(m_masterEQValues);
    m_masterEQValues.clear();
    qDeleteAll(m_masterEQLabels);
    m_masterEQLabels.clear();

    QString effectId = comboBoxMasterEq->itemData(effectIndex).toString();

    if (effectId.isNull()) {
        pbResetMasterEq->hide();
    } else {
        pbResetMasterEq->show();
    }

    EffectChainSlotPointer pChainSlot = m_pOutputEffectRack->getEffectChainSlot(0);

    if (pChainSlot) {
        EffectChainPointer pChain = pChainSlot->getEffectChain();
        VERIFY_OR_DEBUG_ASSERT(pChain) {
            pChain = pChainSlot->getOrCreateEffectChain(m_pEffectsManager);
        }
        EffectPointer pEffect = m_pEffectsManager->instantiateEffect(effectId);
        pChain->replaceEffect(0, pEffect);

        if (pEffect) {
            pEffect->setEnabled(true);
            m_pEffectMasterEQ = pEffect;

            int knobNum = pEffect->numKnobParameters();

            // Create and set up Master EQ's sliders
            int i;
            for (i = 0; i < knobNum; i++) {
                EffectParameter* param = pEffect->getKnobParameterForSlot(i);
                if (param) {
                    // Setup Label
                    QLabel* centerFreqLabel = new QLabel(this);
                    QString labelText = param->manifest()->name();
                    m_masterEQLabels.append(centerFreqLabel);
                    centerFreqLabel->setText(labelText);
                    slidersGridLayout->addWidget(centerFreqLabel, 0, i + 1, Qt::AlignCenter);

                    QSlider* slider = new QSlider(this);
                    slider->setMinimum(static_cast<int>(param->getMinimum() * 100));
                    slider->setMaximum(static_cast<int>(param->getMaximum() * 100));
                    slider->setSingleStep(1);
                    slider->setValue(static_cast<int>(param->getDefault() * 100));
                    slider->setMinimumHeight(90);
                    // Set the index as a property because we need it inside slotUpdateFilter()
                    slider->setProperty("index", QVariant(i));
                    slidersGridLayout->addWidget(slider, 1, i + 1, Qt::AlignCenter);
                    m_masterEQSliders.append(slider);
                    connect(slider,
                            &QAbstractSlider::sliderMoved,
                            this,
                            &DlgPrefEQ::slotUpdateMasterEQParameter);

                    QLabel* valueLabel = new QLabel(this);
                    m_masterEQValues.append(valueLabel);
                    QString valueText = QString::number((double)slider->value() / 100);
                    valueLabel->setText(valueText);
                    slidersGridLayout->addWidget(valueLabel, 2, i + 1, Qt::AlignCenter);

                }
            }
        }
    }

    // Update the configured effect for the current QComboBox
    m_pConfig->set(ConfigKey(kConfigKey, "EffectForGroup_[Master]"),
            ConfigValue(effectId));
}

double DlgPrefEQ::getEqFreq(int sliderVal, int minValue, int maxValue) {
    // We're mapping f(x) = x^4 onto the range kFrequencyLowerLimit,
    // kFrequencyUpperLimit with x [minValue, maxValue]. First translate x into
    // [0.0, 1.0], raise it to the 4th power, and then scale the result from
    // [0.0, 1.0] to [kFrequencyLowerLimit, kFrequencyUpperLimit].
    double normValue = static_cast<double>(sliderVal - minValue) /
            (maxValue - minValue);
    // Use a non-linear mapping between slider and frequency.
    normValue = normValue * normValue * normValue * normValue;
    double result = normValue * (kFrequencyUpperLimit - kFrequencyLowerLimit) +
            kFrequencyLowerLimit;
    return result;
}

void DlgPrefEQ::validate_levels() {
    m_highEqFreq = math_clamp<double>(m_highEqFreq, kFrequencyLowerLimit,
                                      kFrequencyUpperLimit);
    m_lowEqFreq = math_clamp<double>(m_lowEqFreq, kFrequencyLowerLimit,
                                     kFrequencyUpperLimit);
    if (m_lowEqFreq == m_highEqFreq) {
        if (m_lowEqFreq == kFrequencyLowerLimit) {
            ++m_highEqFreq;
        } else if (m_highEqFreq == kFrequencyUpperLimit) {
            --m_lowEqFreq;
        } else {
            ++m_highEqFreq;
        }
    }
}

QString DlgPrefEQ::getEQEffectGroupForDeck(int deck) const {
    // The EQ effect is loaded in effect slot 0.
    if (m_pEQEffectRack) {
        return m_pEQEffectRack->formatEffectSlotGroupString(
            0, PlayerManager::groupForDeck(deck));
    }
    return QString();
}

QString DlgPrefEQ::getQuickEffectGroupForDeck(int deck) const {
    // The quick effect is loaded in effect slot 0.
    if (m_pQuickEffectRack) {
        return m_pQuickEffectRack->formatEffectSlotGroupString(
            0, PlayerManager::groupForDeck(deck));
    }
    return QString();
}

void DlgPrefEQ::slotMasterEQToDefault() {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        int knobNum = effect->numKnobParameters();
        for (int i = 0; i < knobNum; i++) {
            EffectParameter* param = effect->getKnobParameterForSlot(i);
            if (param) {
                double defaultValue = param->getDefault();
                setMasterEQParameter(i, defaultValue);
            }
        }
    }
}

void DlgPrefEQ::setMasterEQParameter(int i, double value) {
    EffectPointer effect(m_pEffectMasterEQ);
    if (!effect.isNull()) {
        EffectParameter* param = effect->getKnobParameterForSlot(i);
        if (param) {
            param->setValue(value);
            m_masterEQSliders[i]->setValue(static_cast<int>(value * 100));

            QLabel* valueLabel = m_masterEQValues[i];
            QString valueText = QString::number(value);
            valueLabel->setText(valueText);

            m_pConfig->set(ConfigKey(kConfigKey,
                    QString("EffectForGroup_[Master]_parameter%1").arg(i + 1)),
                            ConfigValue(valueText));
        }
    }
}
