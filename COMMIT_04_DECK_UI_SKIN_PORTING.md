# Commit 4: Deck UI - Port Tuning Toggle Button to All Skins

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Port the 432Hz/440Hz tuning toggle button from reference skin (Deere) to all other skins.

## Skins to Update

### Legacy Skins (res/skins/)
```
res/skins/Deere/          ✅ Reference (Commit 3)
res/skins/LateNight/      ⏳ To port
res/skins/Shade/          ⏳ To port
res/skins/Tango/          ⏳ To port
```

### QML Skins (if applicable)
Check for QML-based skins in `res/qml/`

## Porting Process for Each Skin

### Step 1: Locate Key Display
```bash
grep -r "file_key\|visual_key" res/skins/[SKIN_NAME]/
```

### Step 2: Add Tuning Status Widget
Copy the widget template from Deere and adapt to skin's style:
- Match font sizes
- Match color scheme
- Match layout patterns

### Step 3: Update Stylesheet
Add TuningStatus and TuningLabel styles matching skin theme

## Skin-Specific Considerations

### LateNight
- Dark theme
- Compact layout
- Colors: Use darker gold, lighter gray

### Shade
- Minimal design
- May need smaller font
- Consider placement carefully

### Tango
- Colorful theme
- Match existing color palette

## Template (copy to each skin)

```xml
<!-- 432Hz/440Hz Tuning Toggle Button - Add next to key display -->
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

**Note:** Adjust `<Size>` to fit each skin's design language.

## Testing
For each skin:
1. Switch to skin in Preferences
2. Enable 432Hz Pitch Lock
3. Verify "432Hz" displays correctly
4. Disable Pitch Lock
5. Verify "440Hz" displays correctly
6. Check layout doesn't break
7. Check visibility in different deck sizes

## Notes
- Keep changes minimal and consistent with each skin's design language
- If a skin has unusual structure, document deviations
- Consider creating a shared template if possible
