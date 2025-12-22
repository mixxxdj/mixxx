# Commit 3: Deck UI - 432Hz/440Hz Status Display (Reference Skin)

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Display the active tuning mode (432Hz or 440Hz) in the deck UI, next to the key/note display.
Start with one reference skin (Deere) before porting to others.

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

**Add new widget next to key display:**
```xml
<!-- 432Hz/440Hz Tuning Status -->
<WidgetGroup>
  <ObjectName>TuningStatus</ObjectName>
  <Layout>horizontal</Layout>
  <Children>
    <Label>
      <ObjectName>TuningLabel</ObjectName>
      <Text>432Hz</Text>
      <Connection>
        <ConfigKey>[ChannelN],432hz_pitch_lock</ConfigKey>
        <Transform>
          <IsEqual>1</IsEqual>
        </Transform>
      </Connection>
    </Label>
    <Label>
      <ObjectName>TuningLabel</ObjectName>
      <Text>440Hz</Text>
      <Connection>
        <ConfigKey>[ChannelN],432hz_pitch_lock</ConfigKey>
        <Transform>
          <IsEqual>0</IsEqual>
        </Transform>
      </Connection>
    </Label>
  </Children>
</WidgetGroup>
```

### 3. Skin CSS/Stylesheet

**Add styling in skin's style.qss:**
```css
#TuningStatus {
  margin-left: 4px;
}

#TuningLabel {
  font-size: 10px;
  font-weight: bold;
  padding: 2px 4px;
  border-radius: 3px;
}

/* 432Hz active - golden highlight */
#TuningLabel[value="432Hz"] {
  background-color: #DAA520;
  color: #000;
}

/* 440Hz (default) - subtle */
#TuningLabel[value="440Hz"] {
  background-color: #444;
  color: #888;
}
```

## Alternative: Create New Widget

If skin XML approach is too limited, create a C++ widget:

### `src/widget/wtuningdisplay.h`
```cpp
#pragma once

#include "widget/wlabel.h"

class WTuningDisplay : public WLabel {
    Q_OBJECT
  public:
    explicit WTuningDisplay(QWidget* parent = nullptr);
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void slotPitchLockChanged(double value);

  private:
    parented_ptr<ControlProxy> m_p432HzPitchLock;
};
```

### `src/widget/wtuningdisplay.cpp`
```cpp
#include "widget/wtuningdisplay.h"
#include "control/controlproxy.h"

WTuningDisplay::WTuningDisplay(QWidget* parent)
    : WLabel(parent) {
}

void WTuningDisplay::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    QString group = context.selectString(node, "Group");
    m_p432HzPitchLock = make_parented<ControlProxy>(
        group, "432hz_pitch_lock", this);
    m_p432HzPitchLock->connectValueChanged(
        this, &WTuningDisplay::slotPitchLockChanged);

    // Initial state
    slotPitchLockChanged(m_p432HzPitchLock->get());
}

void WTuningDisplay::slotPitchLockChanged(double value) {
    if (value > 0) {
        setText("432Hz");
        setStyleSheet("background-color: #DAA520; color: #000;");
    } else {
        setText("440Hz");
        setStyleSheet("background-color: #444; color: #888;");
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
1. Enable 432Hz Pitch Lock in Preferences → Decks
2. Load any track
3. Deck should show "432Hz" in golden
4. Disable Pitch Lock
5. Deck should show "440Hz" in gray
