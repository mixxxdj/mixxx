# Auto DJ - Intelligent Crossfading / Intelligente Überblendung

## Status: Planning / In Planung
**Branch**: TBD (to be created after review)

## Ziel / Goal
Verbesserung der Intro/Outro-Erkennung im Auto DJ für intelligentes, tanzunterbrechungsfreies Überblenden basierend auf Waveform-Analyse und musikalischen Eigenschaften.

---

## Anforderungen / Requirements

### 1. Intelligente Intro/Outro-Erkennung
- **Aktuell**: Feste Überblendungsdauer (unabhängig vom Track)
- **Ziel**: Dynamische Erkennung wo Intro endet und Outro beginnt
- Neuer Track wird eingeblendet **genau dann**, wenn das Intro des neuen Tracks endet
- Vermeidung von Tanzunterbrechungen

### 2. Waveform-basierte Überblendungsdauer
**Keine feste Überblendungsdauer**, sondern individuell nach Beschaffenheit der Waveform:

#### Schneller/Harter Übergang
- **Bedingung**: Nach Intro springt Waveform stark nach oben (hohe Energie)
- **Verhalten**: Schneller, harter Crossfade
- **Beispiel**: Track startet direkt mit Drop/Hauptteil

#### Langsamer/Sanfter Übergang
- **Bedingung**: Nach Intro steigt Waveform langsam an
- **Verhalten**: Längerer, sanfter Crossfade
- **Beispiel**: Track baut sich langsam auf

#### Überblendungsgeschwindigkeit-Parameter
- Analysiere Energie-Anstieg nach Intro-Ende
- Berechne optimale Crossfade-Dauer basierend auf:
  - Waveform-Amplitude
  - Energie-Gradient (wie schnell steigt die Energie)
  - Spektrale Eigenschaften (Frequenzgehalt)

### 3. Musikalische Bedingungen
Der intelligente Crossfade funktioniert nur wenn:
- ✅ **Korrekter Takt-Sync**: Beide Tracks sind im Beat
- ✅ **Kompatible Tonart**: Tracks sind harmonisch kompatibel
- ❌ Falls Bedingungen nicht erfüllt: Fallback auf klassischen Crossfade

---

## Technische Überlegungen

### Zu analysierende Komponenten

1. **Intro/Outro Detection**
   - Aktuelle Implementierung prüfen: `src/library/autodj/autodjprocessor.cpp`
   - Bestehende Cue-Point-Logik evaluieren
   - Waveform-Daten aus `WaveformData` nutzen

2. **Waveform-Analyse**
   - RMS/Peak-Analyse für Energie-Erkennung
   - Gradient-Berechnung nach Intro
   - Spektral-Analyse für Frequenzgehalt (optional)

3. **Beat-Matching**
   - Nutzung von `BpmControl` für Takt-Sync
   - Beat-Grid Alignment prüfen

4. **Key-Matching**
   - Integration mit vorhandener Key-Erkennung
   - Harmonic-Mixing-Regeln (Camelot Wheel)

### Implementierungs-Ideen

```cpp
// Pseudo-Code
struct CrossfadeConfig {
    double startTime;        // Wann Crossfade beginnt (= Intro-Ende)
    double duration;         // Dynamisch berechnet
    CurveType curve;         // Linear, exponentiell, etc.
};

CrossfadeConfig calculateIntelligentCrossfade(
    TrackPointer currentTrack,
    TrackPointer nextTrack
) {
    // 1. Intro-Ende des nächsten Tracks bestimmen
    double introEnd = detectIntroEnd(nextTrack);

    // 2. Waveform nach Intro analysieren
    double energyGradient = analyzeEnergyGradient(nextTrack, introEnd);

    // 3. Crossfade-Dauer berechnen
    double duration;
    if (energyGradient > HIGH_ENERGY_THRESHOLD) {
        duration = SHORT_CROSSFADE;  // z.B. 2 Sekunden
    } else {
        duration = LONG_CROSSFADE;   // z.B. 8 Sekunden
    }

    // 4. Beat- und Key-Matching prüfen
    if (!isBeatMatched(currentTrack, nextTrack) ||
        !isKeyCompatible(currentTrack, nextTrack)) {
        return getDefaultCrossfade();  // Fallback
    }

    return {introEnd, duration, calculateOptimalCurve(energyGradient)};
}
```

### Relevante Dateien

- `src/library/autodj/autodjprocessor.h/cpp` - Auto DJ Haupt-Logik
- `src/library/autodj/dlgautodj.cpp` - Auto DJ UI
- `src/engine/controls/cuecontrol.h/cpp` - Cue-Point Management
- `src/waveform/waveform.h/cpp` - Waveform-Daten
- `src/analyzer/analyzerebur128.cpp` - Lautheit/Energie-Analyse
- `src/engine/controls/bpmcontrol.h/cpp` - BPM/Beat-Sync
- `src/track/keyutils.h/cpp` - Key-Compatibility

---

## Offene Fragen

1. **Intro/Outro-Marker**:
   - Sind bereits Intro/Outro-Cues vorhanden?
   - Falls nein: Automatische Erkennung implementieren oder manuell setzen?

2. **Performance**:
   - Waveform-Analyse in Echtzeit oder pre-analyzed?
   - Caching von Crossfade-Konfigurationen?

3. **User Control**:
   - Sollen User die intelligente Überblendung manuell überschreiben können?
   - Einstellungen für aggressiven vs. sanften Crossfade?

4. **Taktgenauigkeit**:
   - Quantisierung auf Beat-Grid?
   - Wie mit ungenauen Beat-Grids umgehen?

---

## Nächste Schritte

1. ✅ Anforderungen dokumentieren (dieser Commit)
2. ⏳ Bestehenden Auto DJ Code analysieren
3. ⏳ Waveform-Analyse-Strategie definieren
4. ⏳ Prototyp für Intro-Ende-Erkennung
5. ⏳ Dynamische Crossfade-Berechnung implementieren
6. ⏳ Testing mit verschiedenen Track-Kombinationen

---

## Notes

- Keine Implementierung in diesem Commit
- Erst Durchsprache der Anforderungen und Architektur
- Dann Schritt-für-Schritt Umsetzung
