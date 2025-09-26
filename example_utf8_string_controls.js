// Example script demonstrating UTF-8 string controls in Mixxx
// This script shows how to use the new engine.setStringValue/getStringValue methods

// Example 1: Setting hotcue labels with UTF-8 text (emojis, international characters)
function demonstrateUTF8HotcueLabels() {
    const group = "[Channel1]";
    
    // Set various UTF-8 labels
    engine.setStringValue(group, "hotcue_1_label_text", "🎵 Intro");
    engine.setStringValue(group, "hotcue_2_label_text", "🔥 Drop");
    engine.setStringValue(group, "hotcue_3_label_text", "🎤 Vocals");
    engine.setStringValue(group, "hotcue_4_label_text", "🎸 Guitar Solo");
    engine.setStringValue(group, "hotcue_5_label_text", "🥁 Drum Break");
    engine.setStringValue(group, "hotcue_6_label_text", "🎺 Brass Section");
    engine.setStringValue(group, "hotcue_7_label_text", "💯 Climax");
    engine.setStringValue(group, "hotcue_8_label_text", "🏁 Outro");
    
    // International text examples
    engine.setStringValue(group, "hotcue_9_label_text", "Καταπληκτικό"); // Greek
    engine.setStringValue(group, "hotcue_10_label_text", "すばらしい");    // Japanese
    engine.setStringValue(group, "hotcue_11_label_text", "Fantástico");   // Spanish
    engine.setStringValue(group, "hotcue_12_label_text", "Потрясающий"); // Russian
    
    print("UTF-8 hotcue labels set successfully!");
}

// Example 2: Reading hotcue labels back
function readHotcueLabels() {
    const group = "[Channel1]";
    
    for (let i = 1; i <= 12; i++) {
        const label = engine.getStringValue(group, "hotcue_" + i + "_label_text");
        if (label) {
            print("Hotcue " + i + " label: " + label);
        }
    }
}

// Example 3: Creating labeled hotcues programmatically
function createLabeledHotcue(group, hotcueNumber, position, label, color) {
    // Set the hotcue at the position
    engine.setValue(group, "hotcue_" + hotcueNumber + "_set", 1);
    
    // Set the color
    if (color) {
        engine.setValue(group, "hotcue_" + hotcueNumber + "_color", color);
    }
    
    // Set the UTF-8 label
    engine.setStringValue(group, "hotcue_" + hotcueNumber + "_label_text", label);
    
    print("Created hotcue " + hotcueNumber + " with label: " + label);
}

// Example 4: Bulk operations
function setupMixLabels() {
    const group = "[Channel1]";
    
    // Standard DJ mix structure with emojis
    const mixStructure = [
        {hotcue: 1, label: "🎵 Intro Start", color: 0x00FF00},
        {hotcue: 2, label: "🔊 Intro End", color: 0x00FF00},
        {hotcue: 3, label: "🎤 Verse 1", color: 0x0000FF},
        {hotcue: 4, label: "🎶 Chorus 1", color: 0xFF0000},
        {hotcue: 5, label: "🎸 Break", color: 0xFFFF00},
        {hotcue: 6, label: "🎶 Chorus 2", color: 0xFF0000},
        {hotcue: 7, label: "🔥 Drop", color: 0xFF8000},
        {hotcue: 8, label: "🏁 Outro", color: 0x800080}
    ];
    
    mixStructure.forEach(function(item) {
        engine.setStringValue(group, "hotcue_" + item.hotcue + "_label_text", item.label);
        engine.setValue(group, "hotcue_" + item.hotcue + "_color", item.color);
    });
    
    print("Mix structure labels applied!");
}

// Example 5: Error handling
function safeSetLabel(group, hotcueNumber, label) {
    try {
        engine.setStringValue(group, "hotcue_" + hotcueNumber + "_label_text", label);
        return true;
    } catch (e) {
        print("Error setting label for hotcue " + hotcueNumber + ": " + e);
        return false;
    }
}

function safeGetLabel(group, hotcueNumber) {
    try {
        return engine.getStringValue(group, "hotcue_" + hotcueNumber + "_label_text");
    } catch (e) {
        print("Error getting label for hotcue " + hotcueNumber + ": " + e);
        return "";
    }
}

// Run demonstrations
print("=== UTF-8 String Controls Demo ===");
demonstrateUTF8HotcueLabels();
readHotcueLabels();
setupMixLabels();
print("=== Demo Complete ===");
