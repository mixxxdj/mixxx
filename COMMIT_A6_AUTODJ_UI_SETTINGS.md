# Commit A6: Auto DJ - UI Settings for Intelligent Crossfading

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Add user interface controls for intelligent crossfade settings in the Auto DJ panel.
All text must be properly translated (i18n) for all supported languages.

## IMPORTANT: First Analyze Existing Intro/Outro System

Before implementation, analyze how Mixxx currently handles Intro/Outro:

### Investigation Tasks (for Opus)
```bash
# 1. Find existing Auto DJ transition mode options
grep -r "intro" src/library/autodj/ --include="*.cpp" --include="*.h"
grep -r "outro" src/library/autodj/ --include="*.cpp" --include="*.h"
grep -r "TransitionMode" src/library/autodj/

# 2. Find existing dropdown/combobox for transition modes
grep -r "addItem" src/library/autodj/dlgautodj.cpp
grep -r "ComboBox" src/library/autodj/dlgautodj.ui

# 3. Find how "Full Intro + Outro" / "Skip Intro/Outro" works
grep -r "FullIntroOutro\|FadeAtOutroStart" src/
```

### Known Existing Options (to verify)
- "Full Intro + Outro" (Vollständiges Intro/Outro)
- "Fade At Outro Start"
- "Full Track"
- etc.

### Goal: Extend Existing System
Don't reinvent - extend what's already there:
1. Understand current intro/outro detection
2. Use same detection for intelligent crossfade
3. Add new options to existing dropdown if applicable

## Files to Modify

### 1. Auto DJ Dialog/Widget
**`src/library/autodj/dlgautodj.ui`** (or equivalent)

### 2. Auto DJ Dialog Code
**`src/library/autodj/dlgautodj.h/cpp`**

## UI Design

### New Settings Section in Auto DJ Panel

```
┌─────────────────────────────────────────────────────┐
│ Auto DJ Settings                                     │
├─────────────────────────────────────────────────────┤
│                                                      │
│ ☑ Enable intelligent crossfading                    │
│                                                      │
│   ├─ ☑ Require beat sync for transitions            │
│   │                                                  │
│   ├─ ☐ Require key compatibility (strict mode)      │
│   │                                                  │
│   └─ Max BPM difference: [  8  ] %                  │
│                                                      │
│ ─────────────────────────────────────────────────── │
│                                                      │
│ Crossfade Duration Limits:                          │
│   Min: [ 1.0 ] sec    Max: [ 30.0 ] sec            │
│                                                      │
└─────────────────────────────────────────────────────┘
```

## Implementation

### 1. UI File Changes

**`src/library/autodj/dlgautodj.ui`**
```xml
<!-- Add after existing Auto DJ controls -->

<widget class="QGroupBox" name="intelligentCrossfadeGroup">
  <property name="title">
    <string>Intelligent Crossfading</string>
  </property>
  <layout class="QVBoxLayout">

    <!-- Main enable checkbox -->
    <item>
      <widget class="QCheckBox" name="checkBoxIntelligentCrossfade">
        <property name="text">
          <string>Enable intelligent crossfading</string>
        </property>
        <property name="toolTip">
          <string>Automatically adjust crossfade timing based on track analysis</string>
        </property>
      </widget>
    </item>

    <!-- Sub-options (indented) -->
    <item>
      <widget class="QWidget" name="intelligentOptions">
        <layout class="QVBoxLayout">
          <property name="leftMargin">
            <number>20</number>
          </property>

          <!-- Beat sync requirement -->
          <item>
            <widget class="QCheckBox" name="checkBoxRequireBeatSync">
              <property name="text">
                <string>Require beat sync for transitions</string>
              </property>
              <property name="checked">
                <bool>true</bool>
              </property>
            </widget>
          </item>

          <!-- Key compatibility requirement -->
          <item>
            <widget class="QCheckBox" name="checkBoxRequireKeyMatch">
              <property name="text">
                <string>Require key compatibility (strict mode)</string>
              </property>
              <property name="checked">
                <bool>false</bool>
              </property>
            </widget>
          </item>

          <!-- Max BPM difference -->
          <item>
            <layout class="QHBoxLayout">
              <item>
                <widget class="QLabel">
                  <property name="text">
                    <string>Max BPM difference:</string>
                  </property>
                </widget>
              </item>
              <item>
                <widget class="QSpinBox" name="spinBoxMaxBpmDiff">
                  <property name="minimum">
                    <number>1</number>
                  </property>
                  <property name="maximum">
                    <number>20</number>
                  </property>
                  <property name="value">
                    <number>8</number>
                  </property>
                  <property name="suffix">
                    <string> %</string>
                  </property>
                </widget>
              </item>
              <item>
                <spacer>
                  <property name="orientation">
                    <enum>Qt::Horizontal</enum>
                  </property>
                </spacer>
              </item>
            </layout>
          </item>

        </layout>
      </widget>
    </item>

    <!-- Duration limits -->
    <item>
      <layout class="QHBoxLayout">
        <item>
          <widget class="QLabel">
            <property name="text">
              <string>Crossfade duration:</string>
            </property>
          </widget>
        </item>
        <item>
          <widget class="QLabel">
            <property name="text">
              <string>Min:</string>
            </property>
          </widget>
        </item>
        <item>
          <widget class="QDoubleSpinBox" name="spinBoxMinDuration">
            <property name="minimum">
              <double>0.5</double>
            </property>
            <property name="maximum">
              <double>10.0</double>
            </property>
            <property name="value">
              <double>1.0</double>
            </property>
            <property name="suffix">
              <string> sec</string>
            </property>
          </widget>
        </item>
        <item>
          <widget class="QLabel">
            <property name="text">
              <string>Max:</string>
            </property>
          </widget>
        </item>
        <item>
          <widget class="QDoubleSpinBox" name="spinBoxMaxDuration">
            <property name="minimum">
              <double>5.0</double>
            </property>
            <property name="maximum">
              <double>60.0</double>
            </property>
            <property name="value">
              <double>30.0</double>
            </property>
            <property name="suffix">
              <string> sec</string>
            </property>
          </widget>
        </item>
      </layout>
    </item>

  </layout>
</widget>
```

### 2. Dialog Code Changes

**`src/library/autodj/dlgautodj.h`**
```cpp
class DlgAutoDJ : public QWidget, public Ui::DlgAutoDJ {
    // ... existing ...

  private slots:
    // New slots
    void slotIntelligentCrossfadeToggled(bool enabled);
    void slotRequireBeatSyncToggled(bool enabled);
    void slotRequireKeyMatchToggled(bool enabled);
    void slotMaxBpmDiffChanged(int value);
    void slotMinDurationChanged(double value);
    void slotMaxDurationChanged(double value);

  private:
    void updateIntelligentOptionsEnabled();
};
```

**`src/library/autodj/dlgautodj.cpp`**
```cpp
void DlgAutoDJ::setupUi() {
    // ... existing setup ...

    // Connect intelligent crossfade controls
    connect(checkBoxIntelligentCrossfade, &QCheckBox::toggled,
            this, &DlgAutoDJ::slotIntelligentCrossfadeToggled);

    connect(checkBoxRequireBeatSync, &QCheckBox::toggled,
            this, &DlgAutoDJ::slotRequireBeatSyncToggled);

    connect(checkBoxRequireKeyMatch, &QCheckBox::toggled,
            this, &DlgAutoDJ::slotRequireKeyMatchToggled);

    connect(spinBoxMaxBpmDiff, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DlgAutoDJ::slotMaxBpmDiffChanged);

    connect(spinBoxMinDuration, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DlgAutoDJ::slotMinDurationChanged);

    connect(spinBoxMaxDuration, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DlgAutoDJ::slotMaxDurationChanged);

    // Load saved settings
    loadIntelligentCrossfadeSettings();
}

void DlgAutoDJ::slotIntelligentCrossfadeToggled(bool enabled) {
    m_pAutoDJProcessor->setIntelligentCrossfadeEnabled(enabled);
    updateIntelligentOptionsEnabled();
    saveSettings();
}

void DlgAutoDJ::updateIntelligentOptionsEnabled() {
    bool enabled = checkBoxIntelligentCrossfade->isChecked();
    intelligentOptions->setEnabled(enabled);
    spinBoxMinDuration->setEnabled(enabled);
    spinBoxMaxDuration->setEnabled(enabled);
}

void DlgAutoDJ::loadIntelligentCrossfadeSettings() {
    checkBoxIntelligentCrossfade->setChecked(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "IntelligentCrossfade"), true));

    checkBoxRequireBeatSync->setChecked(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "RequireBeatSync"), true));

    checkBoxRequireKeyMatch->setChecked(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "RequireKeyMatch"), false));

    spinBoxMaxBpmDiff->setValue(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "MaxBpmDifference"), 8));

    spinBoxMinDuration->setValue(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "MinCrossfadeDuration"), 1.0));

    spinBoxMaxDuration->setValue(
        m_pConfig->getValue(ConfigKey("[Auto DJ]", "MaxCrossfadeDuration"), 30.0));

    updateIntelligentOptionsEnabled();
}

void DlgAutoDJ::saveSettings() {
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "IntelligentCrossfade"),
        checkBoxIntelligentCrossfade->isChecked());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "RequireBeatSync"),
        checkBoxRequireBeatSync->isChecked());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "RequireKeyMatch"),
        checkBoxRequireKeyMatch->isChecked());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MaxBpmDifference"),
        spinBoxMaxBpmDiff->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MinCrossfadeDuration"),
        spinBoxMinDuration->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MaxCrossfadeDuration"),
        spinBoxMaxDuration->value());
}
```

### 3. Config Keys

**Add to configuration system:**
```cpp
// Config keys for intelligent crossfade
namespace AutoDJConfigKeys {
    const ConfigKey kIntelligentCrossfade("[Auto DJ]", "IntelligentCrossfade");
    const ConfigKey kRequireBeatSync("[Auto DJ]", "RequireBeatSync");
    const ConfigKey kRequireKeyMatch("[Auto DJ]", "RequireKeyMatch");
    const ConfigKey kMaxBpmDifference("[Auto DJ]", "MaxBpmDifference");
    const ConfigKey kMinCrossfadeDuration("[Auto DJ]", "MinCrossfadeDuration");
    const ConfigKey kMaxCrossfadeDuration("[Auto DJ]", "MaxCrossfadeDuration");
}
```

## Internationalization (i18n) - IMPORTANT

All UI strings must use `tr()` for translation support.

### Strings to Translate

```cpp
// Checkbox labels
tr("Enable intelligent crossfading")
tr("Require beat sync for transitions")
tr("Require key compatibility (strict mode)")

// Labels
tr("Max BPM difference:")
tr("Crossfade duration:")
tr("Min:")
tr("Max:")

// Group box title
tr("Intelligent Crossfading")

// Tooltips (English)
checkBoxIntelligentCrossfade->setToolTip(tr(
    "Automatically adjust crossfade timing based on track intro/outro analysis "
    "and energy levels. Ensures smooth, beat-matched transitions."));

checkBoxRequireBeatSync->setToolTip(tr(
    "Only use intelligent crossfading when tracks can be beat-synced. "
    "Falls back to standard crossfade otherwise."));

checkBoxRequireKeyMatch->setToolTip(tr(
    "Only use intelligent crossfading when tracks are harmonically compatible. "
    "Enable for professional harmonic mixing."));

spinBoxMaxBpmDiff->setToolTip(tr(
    "Maximum BPM difference allowed for beat synchronization. "
    "Higher values allow more flexibility but may sound less natural."));
```

### Translation File Updates Required
After implementation, update translation templates:
```bash
# Update .ts files in res/translations/
lupdate src/ -ts res/translations/mixxx_de.ts
lupdate src/ -ts res/translations/mixxx_es.ts
# ... etc for all languages
```

### Dropdown Options (if added to existing combobox)
If adding to existing Auto DJ transition mode dropdown:
```cpp
// New option to add
comboBoxTransitionMode->addItem(tr("Intelligent (auto-detect)"));
```

### German Translations (example)
```
"Enable intelligent crossfading" = "Intelligente Überblendung aktivieren"
"Require beat sync for transitions" = "Beat-Sync für Übergänge erfordern"
"Require key compatibility" = "Tonart-Kompatibilität erfordern"
"Intelligent Crossfading" = "Intelligente Überblendung"
"Max BPM difference" = "Maximale BPM-Differenz"
```

### Spanish Translations (example)
```
"Enable intelligent crossfading" = "Activar crossfade inteligente"
"Require beat sync for transitions" = "Requerir sincronización de beat"
"Require key compatibility" = "Requerir compatibilidad de tonalidad"
"Intelligent Crossfading" = "Crossfade Inteligente"
"Max BPM difference" = "Diferencia máxima de BPM"
```

## Status Display (Optional Enhancement)

### Show Current Transition Info
```xml
<!-- Status display during Auto DJ -->
<widget class="QLabel" name="labelTransitionStatus">
  <property name="text">
    <string>Next transition: Intelligent (8 beats, S-curve)</string>
  </property>
</widget>
```

```cpp
void DlgAutoDJ::updateTransitionStatus() {
    if (m_pAutoDJProcessor->isUsingIntelligentCrossfade()) {
        auto config = m_pAutoDJProcessor->getCurrentCrossfadeConfig();
        QString status = tr("Next: Intelligent (%1s, %2)")
            .arg(config.duration, 0, 'f', 1)
            .arg(curveTypeToString(config.curve));
        labelTransitionStatus->setText(status);
    } else {
        labelTransitionStatus->setText(tr("Next: Standard crossfade"));
    }
}
```

## Testing

### UI Tests

1. **Enable/disable toggle**
   - Toggle checkbox
   - Verify sub-options enable/disable
   - Verify processor receives setting

2. **Settings persistence**
   - Change settings
   - Restart Mixxx
   - Verify settings restored

3. **Value validation**
   - Enter extreme values
   - Verify bounds respected
   - Verify min < max for duration

4. **Integration**
   - Enable intelligent crossfade
   - Queue tracks
   - Verify settings affect behavior

## Dependencies
- Requires: Commit A5 (Auto DJ Integration)
- Integrates with: `AutoDJProcessor`
