# Commit 4: Deck UI - Port Tuning Status to All Skins

## Status: 📋 Prepared (Ready for Implementation)

## Summary
Port the 432Hz/440Hz tuning status display from reference skin (Deere) to all other skins.

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
<!-- Add after key display -->
<WidgetGroup>
  <ObjectName>TuningStatus</ObjectName>
  <Layout>horizontal</Layout>
  <SizePolicy>f,min</SizePolicy>
  <Children>
    <Label>
      <ObjectName>TuningLabel432</ObjectName>
      <Text>432Hz</Text>
      <Connection>
        <ConfigKey>[ChannelN],432hz_pitch_lock</ConfigKey>
        <BindProperty>visible</BindProperty>
      </Connection>
    </Label>
    <Label>
      <ObjectName>TuningLabel440</ObjectName>
      <Text>440Hz</Text>
      <Connection>
        <ConfigKey>[ChannelN],432hz_pitch_lock</ConfigKey>
        <BindProperty>visible</BindProperty>
        <Transform><Not/></Transform>
      </Connection>
    </Label>
  </Children>
</WidgetGroup>
```

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
