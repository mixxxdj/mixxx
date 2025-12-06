# Commit 3: Deck UI - 432Hz/440Hz Toggle Button (Reference Skin)

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Add a toggle button (not just display) for 432Hz/440Hz tuning mode in the deck UI.
This allows DJs to switch between tuning modes directly from the deck to hear the difference.
Start with one reference skin (Deere) before porting to others.

## Why a Button Instead of Just Display?
- **A/B Comparison**: DJs can toggle to hear the difference between 432Hz and 440Hz
- **Quick Access**: No need to go to preferences to change tuning mode
- **Per-Deck Control**: Potentially different tuning per deck (future enhancement)

## Reference Skin: Deere
Located in: `res/skins/Deere/`

## Files to Modify

### 1. Find Key Display Widget
```bash
# Search for key display in Deere skin
grep -r "key" res/skins/Deere/*.xml
```

Likely location: `res/skins/Deere/deck.xml` or similar

### 2. Skin XML Changes

**Add toggle button next to key display:**
```xml
<!-- 432Hz/440Hz Tuning Toggle Button -->
<PushButton>
  <ObjectName>TuningToggleButton</ObjectName>
  <TooltipId>432hz_pitch_lock</TooltipId>
  <Size>45f,18f</Size>
  <NumberStates>2</NumberStates>
  <State>
    <Number>0</Number>
    <Text>440Hz</Text>
  </State>
  <State>
    <Number>1</Number>
    <Text>432Hz</Text>
  </State>
  <Connection>
    <ConfigKey>[ChannelN],432hz_pitch_lock</ConfigKey>
    <ButtonState>LeftButton</ButtonState>
  </Connection>
</PushButton>
```

**Usage:**
- Click to toggle between 440Hz and 432Hz tuning
- DJ can A/B compare the sound difference instantly
- Visual feedback shows current state

### 3. Skin CSS/Stylesheet

**Add styling in skin's style.qss:**
```css
#TuningToggleButton {
  font-size: 10px;
  font-weight: bold;
  padding: 2px 6px;
  border-radius: 3px;
  border: 1px solid #555;
}

/* 440Hz state (default) - subtle appearance */
#TuningToggleButton[displayValue="0"] {
  background-color: #333;
  color: #888;
}

#TuningToggleButton[displayValue="0"]:hover {
  background-color: #444;
  color: #aaa;
}

/* 432Hz state (active) - golden highlight */
#TuningToggleButton[displayValue="1"] {
  background-color: #DAA520;
  color: #000;
  border-color: #B8860B;
}

#TuningToggleButton[displayValue="1"]:hover {
  background-color: #FFD700;
}
```

## Alternative: Create New Widget (C++)

If skin XML approach is too limited, create a dedicated C++ toggle button widget:

### `src/widget/wtuningtoggle.h`
```cpp
#pragma once

#include "widget/wpushbutton.h"

class WTuningToggle : public WPushButton {
    Q_OBJECT
  public:
    explicit WTuningToggle(QWidget* parent = nullptr);
    void setup(const QDomNode& node, const SkinContext& context) override;

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  private slots:
    void slotPitchLockChanged(double value);

  private:
    void updateDisplay();
    parented_ptr<ControlProxy> m_p432HzPitchLock;
    bool m_bIs432Hz = false;
};
```

### `src/widget/wtuningtoggle.cpp`
```cpp
#include "widget/wtuningtoggle.h"
#include "control/controlproxy.h"
#include <QMouseEvent>

WTuningToggle::WTuningToggle(QWidget* parent)
    : WPushButton(parent) {
    setCheckable(true);
}

void WTuningToggle::setup(const QDomNode& node, const SkinContext& context) {
    WPushButton::setup(node, context);

    QString group = context.selectString(node, "Group");
    m_p432HzPitchLock = make_parented<ControlProxy>(
        group, "432hz_pitch_lock", this);
    m_p432HzPitchLock->connectValueChanged(
        this, &WTuningToggle::slotPitchLockChanged);

    // Initial state
    slotPitchLockChanged(m_p432HzPitchLock->get());
}

void WTuningToggle::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        // Toggle the 432Hz pitch lock
        m_p432HzPitchLock->set(m_bIs432Hz ? 0.0 : 1.0);
    }
    WPushButton::mousePressEvent(e);
}

void WTuningToggle::slotPitchLockChanged(double value) {
    m_bIs432Hz = (value > 0);
    updateDisplay();
}

void WTuningToggle::updateDisplay() {
    if (m_bIs432Hz) {
        setText("432Hz");
        setChecked(true);
    } else {
        setText("440Hz");
        setChecked(false);
    }
}
```

## Control Object
Already exists from Commit 1:
- `[ChannelN],432hz_pitch_lock` - created in DlgPrefDeck

## Placement Guidelines
- Next to key display (Dm, Am, etc.)
- Small, unobtrusive
- Clear visual distinction between 432Hz (golden) and 440Hz (gray)

## Testing
1. Load any track in deck
2. Button should show "440Hz" (default state, gray)
3. Click the button to toggle to "432Hz"
4. Button should show "432Hz" in golden
5. Listen for the pitch difference (~32 cents lower)
6. Click again to toggle back to "440Hz"
7. Listen for the pitch returning to normal
8. Verify setting persists when changing tracks
9. Verify A/B comparison is smooth (no audio glitches)
