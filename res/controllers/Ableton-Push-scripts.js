var AbletonPush = {};

/**
 * Settings
 */
AbletonPush.KnobSensitivity = 0.01; // Define the global encoders sensitivity to MIXXX knobs
AbletonPush.Debug = false; // Bypass loading screen, disable vu-meter and show more logs

/**
 * Create controls list. For each control:
 *  - Status = midi status
 *  - Control = midi control
 *  - Type = Buttons, Knobs, KnobsTouch, ColorButtons, RGBButtons, RGBPads
 *  - Zone = Common, Left, Right (TODO : split Right and Left in 3 parts : top, middle and bottom depending on Page layer)
 */
AbletonPush.controls = [
    {status:0x90, control:0x00, type:"KnobsTouch", zone:"Left", defaultColor:""},
    {status:0x90, control:0x01, type:"KnobsTouch", zone:"Left", defaultColor:""},
    {status:0x90, control:0x02, type:"KnobsTouch", zone:"Left", defaultColor:""},
    {status:0x90, control:0x03, type:"KnobsTouch", zone:"Left", defaultColor:""},
    {status:0x90, control:0x04, type:"KnobsTouch", zone:"Right", defaultColor:""},
    {status:0x90, control:0x05, type:"KnobsTouch", zone:"Right", defaultColor:""},
    {status:0x90, control:0x06, type:"KnobsTouch", zone:"Right", defaultColor:""},
    {status:0x90, control:0x07, type:"KnobsTouch", zone:"Right", defaultColor:""},
    {status:0x90, control:0x08, type:"KnobsTouch", zone:"Common", defaultColor:""},
    {status:0x90, control:0x09, type:"KnobsTouch", zone:"Common", defaultColor:""},
    {status:0x90, control:0x0A, type:"KnobsTouch", zone:"Common", defaultColor:""},
    {status:0x90, control:0x0C, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0x90, control:0x24, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x25, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x26, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x27, type:"RGBPads", zone:"Left", defaultColor:"R_white"},
    {status:0x90, control:0x28, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x29, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x2A, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x2B, type:"RGBPads", zone:"Right", defaultColor:"R_grey"},
    {status:0x90, control:0x2C, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x2D, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x2E, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x2F, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x30, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x31, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x32, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x33, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x34, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x35, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x36, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x37, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x38, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x39, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x3A, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x3B, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x3C, type:"RGBPads", zone:"Left", defaultColor:"R_red_dim"},
    {status:0x90, control:0x3D, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x3E, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x3F, type:"RGBPads", zone:"Left", defaultColor:"R_red_dim"},
    {status:0x90, control:0x40, type:"RGBPads", zone:"Right", defaultColor:"R_red_dim"},
    {status:0x90, control:0x41, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x42, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x43, type:"RGBPads", zone:"Right", defaultColor:"R_red_dim"},
    {status:0x90, control:0x44, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x45, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x46, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x47, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x48, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x49, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x4A, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x4B, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x4C, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x4D, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x4E, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x4F, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x50, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x51, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x52, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x53, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x54, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x55, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x56, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x57, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x58, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x59, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x5A, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x5B, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x5C, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x5D, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x5E, type:"RGBPads", zone:"Left", defaultColor:"R_orange_dim"},
    {status:0x90, control:0x5F, type:"RGBPads", zone:"Left", defaultColor:""},
    {status:0x90, control:0x60, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x61, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0x90, control:0x62, type:"RGBPads", zone:"Right", defaultColor:"R_orange_dim"},
    {status:0x90, control:0x63, type:"RGBPads", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x24, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x25, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x26, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x27, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x28, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x29, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x2A, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x2B, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x2C, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x2D, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x2E, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x2F, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x30, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x31, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x32, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x33, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x34, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x35, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x36, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x37, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x38, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x39, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x3A, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x3B, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x3C, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x3D, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x3E, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x3F, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x40, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x41, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x42, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x43, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x44, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x45, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x46, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x47, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x48, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x49, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x4A, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x4B, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x4C, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x4D, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x4E, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x4F, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x50, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x51, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x52, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x53, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x54, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x55, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x56, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x57, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x58, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x59, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x5A, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x5B, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x5C, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x5D, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x5E, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x5F, type:"RGBPadsPressure", zone:"Left", defaultColor:""},
    {status:0xA0, control:0x60, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x61, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x62, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xA0, control:0x63, type:"RGBPadsPressure", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x03, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x09, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x0E, type:"Knobs", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x0F, type:"Knobs", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x14, type:"ColorButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x15, type:"ColorButtons", zone:"Left", defaultColor:"C_red_dim"},
    {status:0xB0, control:0x16, type:"ColorButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x17, type:"ColorButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x18, type:"ColorButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x19, type:"ColorButtons", zone:"Right", defaultColor:"C_red_dim"},
    {status:0xB0, control:0x1A, type:"ColorButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x1B, type:"ColorButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x1C, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x1D, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x24, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x25, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x26, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x27, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x28, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x29, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2A, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2B, type:"ColorButtons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2C, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2D, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2E, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x2F, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x30, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x31, type:"Buttons", zone:"Common", defaultColor:"M_dim"},
    {status:0xB0, control:0x32, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x33, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x34, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x35, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x36, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x37, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x38, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x39, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3A, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3B, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3C, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3D, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3E, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x3F, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x47, type:"Knobs", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x48, type:"Knobs", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x49, type:"Knobs", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x4A, type:"Knobs", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x4B, type:"Knobs", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x4C, type:"Knobs", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x4D, type:"Knobs", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x4E, type:"Knobs", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x4F, type:"Knobs", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x55, type:"Buttons", zone:"Common", defaultColor:"M_light"},
    {status:0xB0, control:0x56, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x57, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x58, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x59, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x5A, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x66, type:"RGBButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x67, type:"RGBButtons", zone:"Left", defaultColor:"R_pink_dim"},
    {status:0xB0, control:0x68, type:"RGBButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x69, type:"RGBButtons", zone:"Left", defaultColor:""},
    {status:0xB0, control:0x6A, type:"RGBButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x6B, type:"RGBButtons", zone:"Right", defaultColor:"R_pink_dim"},
    {status:0xB0, control:0x6C, type:"RGBButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x6D, type:"RGBButtons", zone:"Right", defaultColor:""},
    {status:0xB0, control:0x6E, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x6F, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x70, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x71, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x72, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x73, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x74, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x75, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x76, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xB0, control:0x77, type:"Buttons", zone:"Common", defaultColor:""},
    {status:0xE0, control:0x00, type:"PitchBend", zone:"Common", defaultColor:""},
];

AbletonPush.image_MIXXX = [
    // Offset 0x00000000 to 0x000003BF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5F, 0x60, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5F, 0x60, 0x62, 0x00, 0x00, 0x00, 0x5F, 0x60, 0x62, 0x00, 0x00, 0x00,
    0x5C, 0x5D, 0x5F, 0x5C, 0x5D, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x82, 0x28, 0x24, 0x5F, 0x60, 0x62, 0x5C, 0x5D, 0x5F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x4F, 0x29, 0x53, 0x5F, 0x60, 0x62, 0x5F, 0x60, 0x62,
    0x3D, 0x3D, 0x3F, 0x00, 0x00, 0x00, 0x39, 0xB5, 0x4A, 0x39, 0xB5, 0x4A,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x85, 0x86, 0x88, 0x84, 0x85, 0x87, 0x00, 0x00, 0x00, 0x72, 0x73, 0x75,
    0x85, 0x86, 0x88, 0x00, 0x00, 0x00, 0x85, 0x86, 0x88, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x85, 0x86, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xC9, 0x29, 0x27, 0x82, 0x28, 0x24, 0x85, 0x86, 0x88, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x75, 0x2A, 0x74, 0x4F, 0x29, 0x53, 0x85, 0x86, 0x88,
    0x82, 0x83, 0x85, 0x00, 0x00, 0x00, 0x39, 0xB5, 0x4A, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xB1, 0xB2, 0xB4, 0xB1, 0xB2, 0xB4, 0xA2, 0xA3, 0xA5, 0xB1, 0xB2, 0xB4,
    0xB1, 0xB2, 0xB4, 0x00, 0x00, 0x00, 0xB1, 0xB2, 0xB4, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xAB, 0xAD, 0xAE, 0xAB, 0xAD, 0xAE, 0x8D, 0x14, 0x14,
    0xC9, 0x29, 0x27, 0x00, 0x00, 0x00, 0xAB, 0xAD, 0xAE, 0xAB, 0xAD, 0xAE,
    0x5A, 0x25, 0x5C, 0x75, 0x2A, 0x74, 0x00, 0x00, 0x00, 0xAB, 0xAD, 0xAE,
    0xAB, 0xAD, 0xAE, 0x33, 0xA5, 0x43, 0x39, 0xB5, 0x4A, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD0, 0xD2, 0xD3, 0xBD, 0xBF, 0xC0, 0xD0, 0xD2, 0xD3, 0xCF, 0xD1, 0xD2,
    0xCE, 0xD0, 0xD1, 0x00, 0x00, 0x00, 0xB9, 0xBB, 0xBC, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB7, 0xB9, 0xBA, 0x8D, 0x14, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB7, 0xB9, 0xBA,
    0x5A, 0x25, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xB7, 0xB9, 0xBA, 0x33, 0xA5, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xC3, 0xC4, 0xC5, 0x00, 0x00, 0x00, 0x9E, 0x9F, 0xA0, 0x00, 0x00, 0x00,
    0x27, 0x27, 0x29, 0x00, 0x00, 0x00, 0x27, 0x27, 0x29, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3, 0x48, 0x27, 0x27, 0x27, 0x29,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x1A, 0x47,
    0x27, 0x27, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2B, 0x97, 0x7E, 0x9B, 0x9D, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x39, 0x3A, 0x3B, 0x00, 0x00, 0x00, 0x2B, 0x2C, 0x2C, 0x00, 0x00, 0x00,
    0x39, 0x3A, 0x3B, 0x00, 0x00, 0x00, 0x39, 0x3A, 0x3B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xED, 0x53, 0x20, 0xE3, 0x48, 0x27, 0x40, 0x3A, 0x3A,
    0x40, 0x3A, 0x3A, 0x00, 0x00, 0x00, 0xC1, 0x20, 0x50, 0xA1, 0x1A, 0x47,
    0x40, 0x3A, 0x3A, 0x40, 0x3A, 0x3A, 0x00, 0x00, 0x00, 0x42, 0x7A, 0x8F,
    0x2B, 0x97, 0x7E, 0x7B, 0x7C, 0x7D, 0x7B, 0x7C, 0x7D, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4B, 0x4C, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4B, 0x4C, 0x4D, 0x00, 0x00, 0x00, 0x4B, 0x4C, 0x4D, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xED, 0x53, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4B, 0x4C, 0x4D, 0xC8, 0x2E, 0x4C, 0xC1, 0x20, 0x50, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x4B, 0x4C, 0x4D, 0x26, 0x8D, 0x91, 0x42, 0x7A, 0x8F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B, 0x4C, 0x4D, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6B, 0x6C, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6B, 0x6C, 0x6E, 0x00, 0x00, 0x00, 0x6B, 0x6C, 0x6E, 0x00, 0x00, 0x00,
    0xF6, 0x89, 0x1F, 0xF6, 0x89, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6B, 0x6C, 0x6E, 0x6B, 0x6C, 0x6E, 0xC8, 0x2E, 0x4C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x6B, 0x6C, 0x6E, 0x6B, 0x6C, 0x6E, 0x26, 0x8D, 0x91,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6A, 0x6C, 0x6E, 0x6A, 0x6C, 0x6E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
];
    

/**
 *  Led buttons color mapping
 * M_ prefixes monochromatic buttons leds
 * C_ prefixes color buttons leds
 * R_ prefixes RGB buttons leds
 */
AbletonPush.Colors = {
    off: 0x00,

    M_dim: 1,
    M_light: 4,

    C_red_dim: 1,
    C_red: 4,
    C_orange_dim: 7,
    C_orange: 10,
    C_yellow_dim: 13,
    C_yellow: 16,
    C_green_dim: 19,
    C_green: 22,

    _blinkslow: 1,
    _blinkfast: 2,

    R_grey:          0x01,
    R_white:         0x03,
    R_red_faint:     0x04,
    R_red:           0x05,
    R_red_medium:    0x06,
    R_red_dim:       0x07,
    R_orange_faint:  0x08,
    R_orange:        0x09,
    R_orange_medium: 0x0A,
    R_orange_dim:    0x0B,
    R_yellow_faint:  0x0C,
    R_yellow:        0x0D,
    R_yellow_medium: 0x0E,
    R_yellow_dim:    0x0F,
    R_apple_faint:   0x14,
    R_apple:         0x15,
    R_apple_medium:  0x16,
    R_apple_dim:     0x17,
    R_green_faint:   0x14,
    R_green:         0x15,
    R_green_medium:  0x16,
    R_green_dim:     0x17,
    R_cyan_faint:    0x24,
    R_cyan:          0x25,
    R_cyan_medium:   0x26,
    R_cyan_dim:      0x27,
    R_blue_faint:    0x2C,
    R_blue:          0x2D,
    R_blue_medium:   0x2E,
    R_blue_dim:      0x2F,
    R_purple_faint:  0x30,
    R_purple:        0x31,
    R_purple_medium: 0x32,
    R_purple_dim:    0x33,
    R_pink_faint:    0x38,
    R_pink:          0x39,
    R_pink_medium:   0x3A,
    R_pink_dim:      0x3B
};

/**
 * Text for the keys on the screen
 */
AbletonPush.Keys = [ "-",
    "1d / C",
    "8d / C#",
    "3d / D",
    "10d / D#",
    "5d / E",
    "12d / F",
    "7d / F#",
    "2d / G",
    "9d / G#",
    "4d / A",
    "11d / A#",
    "6d / B",
    "10m / Cm",
    "5m / C#m",
    "12m / Dm",
    "7m / D#m",
    "2m / Em",
    "9m / Fm",
    "4m / F#m",
    "11m / Gm",
    "6m / G#m",
    "1m / Am",
    "8m / A#m",
    "3m / Bm",
];

/**
 * Function called at MIXXX startup
 * @param {*} id
 * @param {*} debugging
 */
AbletonPush.init = function(id, debugging) {
    // Show image on Push
    if (!AbletonPush.Debug) { AbletonPush.showImage(); }

    // Screen message
    AbletonPush.displayText(1, 0, "");
    AbletonPush.displayText(2, 0, "     Ableton Pushmapping for MIXXX");
    AbletonPush.displayText(3, 0, "");
    AbletonPush.displayText(4, 0, "                                               by Replayman");

    // Set PitchBend leds to CUSTOM
    var aSysex1 = [0xF0, 0x47, 0x79, 0x15, 0x63, 0x00, 0x01, 0x04, 0xF7];
    midi.sendSysexMsg(aSysex1, aSysex1.length);
    AbletonPush.setPitchBendLeds(1);

    // Set defaults
    AbletonPush.Page="2Decks"; // Seelct the 2 decks page at startup

    if (AbletonPush.Debug) { AbletonPush.init2(); }
};

/**
 * Second part of init, after startup show
 */
AbletonPush.init2 = function() {
    // Leds startup
    _.forEach(_.filter(AbletonPush.controls, function(o) {
        return (o.defaultColor !== "");
    }), function(value) {
        midi.sendShortMsg(value.status, value.control, AbletonPush.Colors[value.defaultColor]);
    });

    // Clear screen
    AbletonPush.displayText(1, 0, "");
    AbletonPush.displayText(2, 0, "");
    AbletonPush.displayText(3, 0, "");
    AbletonPush.displayText(4, 0, "");
    
    // Create custom Component Container using components-js
    AbletonPush.LeftControls = new AbletonPush.fullDeck([1, 3], 0);
    AbletonPush.RightControls = new AbletonPush.fullDeck([2, 4], 4);
    AbletonPush.CommonControls = new AbletonPush.commonControls();

    // Show default decks on screen
    AbletonPush.displayText(3, 1, "Deck 1", 1);
    AbletonPush.displayText(3, 4, "Deck 2", 2);

    // Show knobs texts on screen
    AbletonPush.displayText(1, 0, "FX Super   Gain   Volume   Pitch  FX Super   Gain   Volume   Pitch  ")
};

/**
 * Function called at MIXXX shutdown
 */
AbletonPush.shutdown = function() {
    // Turn off all LEDs
    _.forEach(_.concat(
        _.filter(AbletonPush.controls, {"type": "Buttons"}),
        _.filter(AbletonPush.controls, {"type": "ColorButtons"}),
        _.filter(AbletonPush.controls, {"type": "RGBPads"}),
        _.filter(AbletonPush.controls, {"type": "RGBButtons"})
    ), function(value) {
        midi.sendShortMsg(value.status, value.control, 0x00);
    });

    // Clear screen
    AbletonPush.displayText(1, 0, "");
    AbletonPush.displayText(2, 0, "");
    AbletonPush.displayText(3, 0, "");
    AbletonPush.displayText(4, 0, "");
};

//=========================================
// Midi Handler function to route midi inputs to the right controls
// Will be replaced when this update is available : https://github.com/mixxxdj/mixxx/wiki/registering%20midi%20input%20handlers%20from%20javascript
//=========================================
AbletonPush.midiHandler = function(channel, control, value, status) {

    // Special routes for PitchBend to JogWheel control
    if (status === 0xE0) { // PitchBend value - control is part of value
        value = (value*128)+control;
        control = 0x00;
        // Send to jogWheel of container
        AbletonPush.CommonControls.PitchBend.inputWheel(channel, control, value, status);
        return;
    }
    if ((status === 0x90) && (control === 0x0C)) { // PitchBend touch
        AbletonPush.CommonControls.PitchBend.inputTouch(channel, control, value, status);
        return;
    }

    // RGBPads are returning 0x80 status on release, and 0x90 on push with value > 0.
    // So, change 0x80 by 0x90 to allow MIXXX understanding release reliably.
    if (status === 0x80) {
        status = 0x90;
    }

    var midiControl = _.find(AbletonPush.controls, {"status": status, "control": control});
    if (typeof midiControl === "undefined") { // In case the controller has hidden controls - or using the map to another controller
        print("Unknown control : " + channel + " / " + control + " / " + value + " / " + status);
        return;
    }
    // Test if control exists
    if (typeof AbletonPush[midiControl.zone+"Controls"][midiControl.type][control] !== "undefined") {
        // Route value to the control
        AbletonPush[midiControl.zone+"Controls"][midiControl.type][control].input(channel, control, value, status);
    }
};

/**
 * Constructor for controls that are common to all pages.
 *
 */
AbletonPush.commonControls = function() {

    this.Buttons = [];
    this.ColorButtons = [];
    this.Knobs = [];
    this.KnobsTouch = [];

    // Left column of buttons
    this.Buttons[0x03] = new components.Button({
        midi: [0xB0, 0x03],
        group: "[AutoDJ]",
        key: "enabled",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.M_light+AbletonPush.Colors._blinkslow,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x09] = new components.Button({
        midi: [0xB0, 0x09],
        group: "[AutoDJ]",
        key: "fade_now",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });

    //this.Buttons[0x77] = ;
    //this.Buttons[0x76] = ;
    //this.Buttons[0x75] = ;
    //this.Buttons[0x74] = ;

    //this.Buttons[0x5A] = ;
    //this.Buttons[0x59] = ;
    //this.Buttons[0x58] = ;
    //this.Buttons[0x57] = ;
    this.Buttons[0x56] = {};
    this.Buttons[0x56].input = function(channel, control, value, status) {
        // Switch decks to channels 3/4
        if (value === 0x7F) {
            AbletonPush.LeftControls.setCurrentDeck("[Channel3]");
            AbletonPush.RightControls.setCurrentDeck("[Channel4]");
            // Change button lights
            midi.sendShortMsg(0xB0, 0x55, AbletonPush.Colors.off);
            midi.sendShortMsg(0xB0, 0x56, AbletonPush.Colors.M_light);
            // Change deck text
            AbletonPush.displayText(3, 1, "Deck 3", 1);
            AbletonPush.displayText(3, 4, "Deck 4", 2);
            // Set jogwhell select button color
            var curDeck=AbletonPush.CommonControls.PitchBend.deck;
            midi.sendShortMsg(0x90, 0x27, (curDeck === 1) ? AbletonPush.Colors.R_white : AbletonPush.Colors.R_grey);
            midi.sendShortMsg(0x90, 0x2B, (curDeck === 2) ? AbletonPush.Colors.R_white : AbletonPush.Colors.R_grey);
        }
    };
    this.Buttons[0x55] = {};
    this.Buttons[0x55].input = function(channel, control, value, status) {
        // Switch decks to channels 1/2
        if (value === 0x7F) {
            AbletonPush.LeftControls.setCurrentDeck("[Channel1]");
            AbletonPush.RightControls.setCurrentDeck("[Channel2]");
            // Change button lights
            midi.sendShortMsg(0xB0, 0x55, AbletonPush.Colors.M_light);
            midi.sendShortMsg(0xB0, 0x56, AbletonPush.Colors.off);
            // Change deck text
            AbletonPush.displayText(3, 1, "Deck 1", 1);
            AbletonPush.displayText(3, 4, "Deck 2", 2);
            // Set jogwhell select button color
            var curDeck=AbletonPush.CommonControls.PitchBend.deck;
            midi.sendShortMsg(0x90, 0x27, (curDeck === 3) ? AbletonPush.Colors.R_white : AbletonPush.Colors.R_grey);
            midi.sendShortMsg(0x90, 0x2B, (curDeck === 4) ? AbletonPush.Colors.R_white : AbletonPush.Colors.R_grey);
        }
    };

    // Left knobs
    //this.Knobs[0x0E] = ;
    this.Knobs[0x0F] = new components.Encoder({
        midi: [0xB0, 0x0F],
        group: "[Master]",
        inKey: "headGain",
        input: function(channel, control, value, status, group) {
            this.inKey = (AbletonPush.shift) ? "gain" : "headGain";
            // Test if outkey not the same as current key and call connect if required.
            if (this.inKey != this.outKey) {
                this.disconnect();
                this.outKey = this.inKey;
                this.connect();
            }

            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc: 1, value: value, oc: 5});
        },
    });

    //this.KnobsTouch[0x0A] = ;
    this.KnobsTouch[0x09] = new components.Component({
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (AbletonPush.shift) {
                    AbletonPush.displayText(1, 1, "Master", 1);
                    AbletonPush.CommonControls.Knobs[0x0F].inKey = "gain";
                } else {
                    AbletonPush.displayText(1, 1, "Headphone", 1);
                    AbletonPush.CommonControls.Knobs[0x0F].inKey = "headGain";
                }
                AbletonPush.displayCursor({
                    hbloc: 1,
                    value: AbletonPush.CommonControls.Knobs[0x0F].inGetValue(), oc: 5
                });

            } else {
                // Reset text to old value
                AbletonPush.displayText(1, 1, "FX Super", 1);
                // Disconnect cursor
                AbletonPush.CommonControls.Knobs[0x0F].disconnect();
                AbletonPush.CommonControls.Knobs[0x0F].outKey="";
                // Refresh FX Super cursor
                AbletonPush.LeftControls.Knobs[0x47].trigger();
            }
        },
    });

    // Right knob
    this.Knobs[0x4F] = new components.Encoder({
        midi: [0xB0, 0x4F],
        group: "[Master]",
        key: "crossfader",
        outConnect: false,
        input: function(channel, control, value, status, group) {
            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc: 8, value: value, min: -1, max: 1, mid: 0});
        },
    });
    this.KnobsTouch[0x08] = new components.Component({
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (AbletonPush.shift) { // Shift pushed : reset value
                    engine.setValue("[Master]", "crossfader", 0);
                }
                AbletonPush.displayText(1, 4, "xFader", 2);
                // Connect cursor
                AbletonPush.CommonControls.Knobs[0x4F].connect();
                // Show cursor actual value
                AbletonPush.displayCursor({
                    hbloc: 8,
                    value: AbletonPush.CommonControls.Knobs[0x4F].inGetValue(), min: -1, max: 1, mid: 0
                });
            } else {
                // Reset text to old value
                AbletonPush.displayText(1, 4, "Pitch", 2);
                // Disconnect cursor
                AbletonPush.CommonControls.Knobs[0x4F].disconnect();
                // Refresh Pitch cursor
                AbletonPush.RightControls.Knobs[0x4E].trigger();
            }
        },
    });

    // Column of buttons at right of pads
    //this.Buttons[0x1C] = ;
    //this.ColorButtons[0x1D] = ;

    //this.ColorButtons[0x2B] = ;
    //this.ColorButtons[0x2A] = ;
    //this.ColorButtons[0x29] = ;
    //this.ColorButtons[0x28] = ;
    //this.ColorButtons[0x27] = ;
    //this.ColorButtons[0x26] = ;
    //this.ColorButtons[0x25] = ;
    //this.ColorButtons[0x24] = ;

    // Right (2) columns of buttons
    this.Buttons[0x72] = new components.Button({
        midi: [0xB0, 0x72],
        group: "[EqualizerRack1_[Channel1]_Effect1]",
        key: "button_parameter3",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x73] = new components.Button({
        midi: [0xB0, 0x73],
        group: "[EqualizerRack1_[Channel2]_Effect1]",
        key: "button_parameter3",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x70] = new components.Button({
        midi: [0xB0, 0x70],
        group: "[EqualizerRack1_[Channel1]_Effect1]",
        key: "button_parameter2",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x71] = new components.Button({
        midi: [0xB0, 0x71],
        group: "[EqualizerRack1_[Channel2]_Effect1]",
        key: "button_parameter2",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x6E] = new components.Button({
        midi: [0xB0, 0x6E],
        group: "[EqualizerRack1_[Channel1]_Effect1]",
        key: "button_parameter1",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });
    this.Buttons[0x6F] = new components.Button({
        midi: [0xB0, 0x6F],
        group: "[EqualizerRack1_[Channel2]_Effect1]",
        key: "button_parameter1",
        on: AbletonPush.Colors.M_light,
        off: AbletonPush.Colors.M_dim,
    });

    //this.Buttons[0x3F] = ;
    //this.Buttons[0x3E] = ;
    //this.Buttons[0x3D] = ;
    //this.Buttons[0x3C] = ;
    //this.Buttons[0x3B] = ;
    //this.Buttons[0x3A] = ;
    //this.Buttons[0x39] = ;
    //this.Buttons[0x38] = ;
    //this.Buttons[0x37] = ;
    //this.Buttons[0x36] = ;

    //this.Buttons[0x35] = ;
    //this.Buttons[0x34] = ;
    //this.Buttons[0x33] = ;
    //this.Buttons[0x32] = ;
    this.Buttons[0x31] = {};
    this.Buttons[0x31].input = function(channel, control, value, status) {
        // Shift button
        if (value === 0x7F) {
            AbletonPush.LeftControls.shift();
            AbletonPush.RightControls.shift();
            AbletonPush.shift = true;
            midi.sendShortMsg(status, control, AbletonPush.Colors.M_light); // Set button led
        } else {
            AbletonPush.LeftControls.unshift();
            AbletonPush.RightControls.unshift();
            AbletonPush.shift = false;
            midi.sendShortMsg(status, control, AbletonPush.Colors.M_dim); // Set button led
        }
    };
    //this.Buttons[0x30] = ;

    //this.Buttons[0x2C] = ;
    //this.Buttons[0x2D] = ;
    //this.Buttons[0x2E] = ;
    //this.Buttons[0x2F] = ;

    this.PitchBend = new components.JogWheelBasic({
        deck: 1, // whatever deck this jogwheel controls
        wheelResolution: 6000, // how many ticks per revolution the jogwheel has
        alpha: 1/8, // alpha-filter
        inValueScale: function(value) {
            // If Finger is <0, then this is first touch value
            if (this.Finger<0) {
                this.Finger = value;
                return 0;
            } else {
                var returnVal = value - this.Finger;
                this.Finger = value;
                return returnVal;
            }
        },
        // Add "point 0" tracking to inputTouch function to allow scratching on pitchbend
        inputTouch: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status) && this.vinylMode) {
                this.Finger = -1;
                if (! AbletonPush.shift) { // Jog if shifted
                    engine.scratchEnable(this.deck,
                        this.wheelResolution,
                        this.rpm,
                        this.alpha,
                        this.beta);
                }
            } else {
                engine.scratchDisable(this.deck);
            }
        },
    });


    /**
     * Output only controls
     *  */
    // Master Vu-Meter
    if (!AbletonPush.Debug) {
        this.VuMeter = new components.Component({
            group: "[Master]",
            outKey: "VuMeter",
            wait: 100, // milliseconds to wait between 2 updates to restrain flooding midi messages
            wait_flag: false,
            output: function(value, _group, _control) {
                if (!this.wait_flag) {
                    AbletonPush.setPitchBendLeds( Math.floor(value*24) );
                    this.wait_flag=true;
                    this.waitTimer = engine.beginTimer(this.wait, function() {
                        this.wait_flag = false;
                        this.waitTimer = 0;
                    }, true);
                }
            },
        });
    }
};

/**
 * Constructor for a deck containing full 4 columns of controls.
 *
 * @param {*} deckNumbers Array of decks values
 * @param {Number} midiShift allows to address right part, adding 4 to all midi controls.
 */
AbletonPush.fullDeck = function(deckNumbers, midiShift) {
    // Call the generic Deck constructor to setup the currentDeck and deckNumbers properties,
    // using Function.prototype.call to assign the custom Deck being constructed
    // to "this" in the context of the generic components.Deck constructor
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function/call
    components.Deck.call(this, deckNumbers);
    
    this.RGBPads = [];
    this.RGBPadsPressure = [];
    this.RGBButtons = [];
    this.ColorButtons = [];
    this.Knobs = [];
    this.KnobsTouch = [];

    // 1st row of RGB pads
    this.RGBPads[0x24 + midiShift] = new components.PlayButton({
        midi: [0x90, 0x24 + midiShift],
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_red_dim,
    });
    this.RGBPads[0x25 + midiShift] = new components.CueButton({
        midi: [0x90, 0x25 + midiShift],
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });

    this.RGBPads[0x26 + midiShift] = new components.Button({
        midi: [0x90, 0x26 + midiShift],
        key: "cue_play",
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });
    this.RGBPads[0x27 + midiShift] = new components.Button({
        midi: [0x90, 0x27 + midiShift],
        input: function(channel, control, value, status, group) {
            if (value>0 && this.group.substr(0, 8) === "[Channel") {
                AbletonPush.CommonControls.PitchBend.group = this.group;
                AbletonPush.CommonControls.PitchBend.deck = this.group.substr(8, 1);

                // Light this button and dim other channel
                this.send(AbletonPush.Colors.R_white);
                midi.sendShortMsg(this.midi[0], this.midi[1] + 2*(2-midiShift), AbletonPush.Colors.R_grey);
            }
        },
    });

    // Hotcues on rows 3 and 2
    for (var i = 1; i <= 4; i++) {
        var midiNum=0x33 + i + midiShift;
        this.RGBPads[midiNum] = new components.HotcueButton({
            midi: [0x90, midiNum],
            number: i,
            sendRGB: function(color_obj) {
                AbletonPush.setRGBPadColor(this.midi[1], color_obj.red, color_obj.green, color_obj.blue);
            }
        });
    }
    for (var i = 5; i <= 8; i++) {
        var midiNum=0x27 + i + midiShift;
        this.RGBPads[midiNum] = new components.HotcueButton({
            midi: [0x90, midiNum],
            number: i,
            sendRGB: function(color_obj) {
                AbletonPush.setRGBPadColor(this.midi[1], color_obj.red, color_obj.green, color_obj.blue);
            }
        });
    }

    // row 4 of RGB pads
    this.RGBPads[0x3C + midiShift] = new components.Component({
        midi: [0x90, 0x3C + midiShift],
        input: function(channel, control, value, _status, _group) {
            this.send((value>0) ? AbletonPush.Colors.R_red : AbletonPush.Colors.R_red_dim);
        },
    });
    this.RGBPadsPressure[0x3C + midiShift] = new components.Pot({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        midi: [0xA0, 0x3C + midiShift],
        inKey: "wheel",
        inValueScale: function(value) {
            return 0.5 - (value/((this.isShifted)?256:2048));
        }
    });
    this.RGBPads[0x3D + midiShift] = new components.Button({
        midi: [0x90, 0x3D + midiShift],
        key: "beatjump_backward",
        on: AbletonPush.Colors.R_pink,
        off: AbletonPush.Colors.R_pink_dim,
    });
    this.RGBPads[0x3E + midiShift] = new components.Button({
        midi: [0x90, 0x3E + midiShift],
        key: "beatjump_forward",
        on: AbletonPush.Colors.R_pink,
        off: AbletonPush.Colors.R_pink_dim,
    });
    this.RGBPads[0x3F + midiShift] = new components.Component({
        midi: [0x90, 0x3F + midiShift],
        input: function(channel, control, value, _status, _group) {
            this.send((value>0) ? AbletonPush.Colors.R_red : AbletonPush.Colors.R_red_dim);
        },
    });
    this.RGBPadsPressure[0x3F + midiShift] = new components.Pot({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        midi: [0xA0, 0x3F + midiShift],
        inKey: "wheel",
        inValueScale: function(value) {
            return 0.5 + (value/((this.isShifted)?256:2142)); // 2142 is the min value before tune-shifting occurs
        }
    });

    // row 5 of RGB pads
    this.RGBPads[0x44 + midiShift] = new components.Button({
        midi: [0x90, 0x44 + midiShift],
        outKey: "intro_start_enabled",
        unshift: function() {
            this.inKey = "intro_start_activate";
        },
        shift: function() {
            this.inKey = "intro_start_clear";
        },
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });
    this.RGBPads[0x45 + midiShift] = new components.Button({
        midi: [0x90, 0x45 + midiShift],
        outKey: "intro_end_enabled",
        unshift: function() {
            this.inKey = "intro_end_activate";
        },
        shift: function() {
            this.inKey = "intro_end_clear";
        },
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });
    this.RGBPads[0x46 + midiShift] = new components.Button({
        midi: [0x90, 0x46 + midiShift],
        outKey: "outro_start_enabled",
        unshift: function() {
            this.inKey = "outro_start_activate";
        },
        shift: function() {
            this.inKey = "outro_start_clear";
        },
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });
    this.RGBPads[0x47 + midiShift] = new components.Button({
        midi: [0x90, 0x47 + midiShift],
        outKey: "outro_end_enabled",
        unshift: function() {
            this.inKey = "outro_end_activate";
        },
        shift: function() {
            this.inKey = "outro_end_clear";
        },
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });

    // row 6 of RGB pads
    this.RGBPads[0x4C + midiShift] = new components.SyncButton({
        midi: [0x90, 0x4C + midiShift],
        on: AbletonPush.Colors.R_grey,
        off: AbletonPush.Colors.off,
    });
    this.RGBPads[0x4D + midiShift] = new components.Button({
        midi: [0x90, 0x4D + midiShift],
        key: "slip_enabled",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.R_grey,
        off: AbletonPush.Colors.off,
    });
    this.RGBPads[0x4E + midiShift] = new components.Button({
        midi: [0x90, 0x4E + midiShift],
        key: "quantize",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.R_grey,
        off: AbletonPush.Colors.off,
    });
    this.RGBPads[0x4F + midiShift] = new components.Button({
        midi: [0x90, 0x4F + midiShift],
        key: "keylock",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.R_grey,
        off: AbletonPush.Colors.off,
    });

    // row 7 of RGB pads
    this.RGBPads[0x54 + midiShift] = new components.Button({
        midi: [0x90, 0x54 + midiShift],
        key: "loop_in",
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_green_dim,
    });
    this.RGBPads[0x55 + midiShift] = new components.Button({
        midi: [0x90, 0x55 + midiShift],
        key: "loop_out",
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_green_dim,
    });
    this.RGBPads[0x56 + midiShift] = new components.Button({
        midi: [0x90, 0x56 + midiShift],
        key: "pfl",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.R_white,
        off: AbletonPush.Colors.off,
    });
    this.RGBPads[0x57 + midiShift] = new components.Button({
        midi: [0x90, 0x57 + midiShift],
        key: "reset_key",
        on: AbletonPush.Colors.R_yellow,
        off: AbletonPush.Colors.R_yellow_dim,
    });

    // row 8 of RGB pads
    this.RGBPads[0x5C + midiShift] = new components.Button({
        midi: [0x90, 0x5C + midiShift],
        key: "beatloop_activate",
        type: components.Button.prototype.types.toggle,
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_green_dim,
    });
    this.RGBPads[0x5D + midiShift] = new components.LoopToggleButton({
        midi: [0x90, 0x5D + midiShift],
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_green_dim,
    });
    this.RGBPads[0x5E + midiShift] = new components.Button({
        midi: [0x90, 0x5E + midiShift],
        input: function(channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                engine.setValue(this.group, "rate", 0);
                midi.sendShortMsg(status, control, AbletonPush.Colors.R_orange);
            } else {
                midi.sendShortMsg(status, control, AbletonPush.Colors.R_orange_dim);
            }
        },
    });
    this.RGBPads[0x5F + midiShift] = new components.Button({
        midi: [0x90, 0x5F + midiShift],
        key: "sync_key",
        on: AbletonPush.Colors.R_yellow,
        off: AbletonPush.Colors.R_yellow_dim,
    });

    // row 9 : RGB buttons
    this.RGBButtons[0x66 + midiShift] = new components.Button({
        midi: [0xB0, 0x66 + midiShift],
        key: "loop_halve",
        on: AbletonPush.Colors.R_green,
        off: AbletonPush.Colors.R_green_dim,
    });
    this.RGBButtons[0x67 + midiShift] = new components.Button({
        midi: [0xB0, 0x67 + midiShift],
        input: function(channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                var oldval = engine.getValue(this.group, "beatjump_size");
                if (oldval > 0.03125) { engine.setValue(this.group, "beatjump_size", oldval / 2); }
                midi.sendShortMsg(status, control, AbletonPush.Colors.R_pink);
            } else {
                midi.sendShortMsg(status, control, AbletonPush.Colors.R_pink_dim);
            }
        },
    });
    this.RGBButtons[0x68 + midiShift] = new components.Button({
        midi: [0xB0, 0x68 + midiShift],
        unshift: function() {
            this.inKey = "rate_perm_down";
        },
        shift: function() {
            this.inKey = "rate_perm_down_small";
        },
        key: "rate_perm_down",
        on: AbletonPush.Colors.R_orange,
        off: AbletonPush.Colors.R_orange_dim,
    });
    this.RGBButtons[0x69 + midiShift] = new components.Button({
        midi: [0xB0, 0x69 + midiShift],
        key: "pitch_down",
        on: AbletonPush.Colors.R_yellow,
        off: AbletonPush.Colors.R_yellow_dim,
    });

    // row 10 : Color buttons
    this.ColorButtons[0x14 + midiShift] = new components.Button({
        midi: [0xB0, 0x14 + midiShift],
        key: "loop_double",
        on: AbletonPush.Colors.C_green,
        off: AbletonPush.Colors.C_green_dim,
    });
    this.ColorButtons[0x15 + midiShift] = new components.Button({
        midi: [0xB0, 0x15 + midiShift],
        input: function(channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                var oldval = engine.getValue(this.group, "beatjump_size");
                if (oldval < 512) { engine.setValue(this.group, "beatjump_size", oldval * 2); }
                midi.sendShortMsg(status, control, AbletonPush.Colors.C_red);
            } else {
                midi.sendShortMsg(status, control, AbletonPush.Colors.C_red_dim);
            }
        },
    });
    this.ColorButtons[0x16 + midiShift] = new components.Button({
        midi: [0xB0, 0x16 + midiShift],
        unshift: function() {
            this.inKey = "rate_perm_up";
        },
        shift: function() {
            this.inKey = "rate_perm_up_small";
        },
        key: "rate_perm_up",
        on: AbletonPush.Colors.C_orange,
        off: AbletonPush.Colors.C_orange_dim,
    });
    this.ColorButtons[0x17 + midiShift] = new components.Button({
        midi: [0xB0, 0x17 + midiShift],
        key: "pitch_up",
        on: AbletonPush.Colors.C_yellow,
        off: AbletonPush.Colors.C_yellow_dim,
    });

    // Knobs
    this.Knobs[0x47 + midiShift] = new components.Encoder({
        midi: [0xB0, 0x47 + midiShift],
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        key: "super1",
        input: function(channel, control, value, status, group) {
            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc: 1+4*((parseInt(this.group.substr(26, 1))+1)%2), value: value, mid: 0.5});
        },
    });
    this.Knobs[0x48 + midiShift] = new components.Encoder({
        midi: [0xB0, 0x48 + midiShift],
        key: "pregain",
        input: function(channel, control, value, status, group) {
            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc:2+4*((parseInt(this.group.substr(8, 1))+1)%2), value:value, oc: 4});
        },
    });
    this.Knobs[0x49 + midiShift] = new components.Encoder({
        midi: [0xB0, 0x49 + midiShift],
        key: "volume",
        input: function(channel, control, value, status, group) {
            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc: 3+4*((parseInt(this.group.substr(8, 1))+1)%2), value: value});
        },
    });
    this.Knobs[0x4A + midiShift] = new components.Encoder({
        midi: [0xB0, 0x4A + midiShift],
        key: "pitch_adjust",
        input: function(channel, control, value, status, group) {
            if (value < 0x40) {
                this.inSetParameter(this.inGetParameter() + (value * AbletonPush.KnobSensitivity));
            } else {
                this.inSetParameter(this.inGetParameter() - ((128 - value) * AbletonPush.KnobSensitivity));
            }
        },
        output: function(value, _group, _control) {
            AbletonPush.displayCursor({hbloc: 4+4*((parseInt(this.group.substr(8, 1))+1)%2), value: value, min: -3, max: 3, mid: 0});
        },
    });

    // Knobs touch
    this.KnobsTouch[0x00 + midiShift] = new components.Component({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (this.isShifted) { // Shift pushed : reset value
                    script.triggerControl(this.group, "super1_set_default", 0x01);
                }
            }
        },
    });
    this.KnobsTouch[0x01 + midiShift] = new components.Component({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (this.isShifted) { // Shift pushed : reset value
                    engine.setValue(this.group, "pregain", 1);
                }
            }
        },
    });
    this.KnobsTouch[0x02 + midiShift] = new components.Component({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (this.isShifted) { // Shift pushed : reset value
                    engine.setValue(this.group, "volume", 1);
                }
            }
        },
    });
    this.KnobsTouch[0x03 + midiShift] = new components.Component({
        shift: function() { this.isShifted = true; },
        unshift: function() { this.isShifted = false; },
        input: function(channel, control, value, status, group) {
            if (value === 0x7F) {
                if (this.isShifted) { // Shift pushed : reset value
                    engine.setValue(this.group, "pitch_adjust", 0);
                }
            }
        },
    });

    /**
     *  Components used to show info on screen
     *  */
    // Track position
    this.screen_TrackPosition = new components.Component({
        outKey: "playposition",
        last_value: -1,
        last_second: -1,
        output: function(value, _group, _control) {
            // Show track time using "duration" to get track time/total
            var duration = Math.round(engine.getValue(this.group, "duration"));
            var second = Math.round(value * duration);
            if (second !== this.last_second) { // Only send sysex when value changes.
                    this.last_second = second;
                    AbletonPush.displayText(3, 1+3*((parseInt(this.group.substr(8, 1))+1)%2),
                        second + "/" + Math.round(duration), 2-(parseInt(this.group.substr(8, 1))+1)%2);
                    print("=============================="+this.group + " = "+second + " / "+midiShift);
                }

            // Show track position fader
            value=Math.floor(value*33)+1;
            if (value !== this.last_value) { // Only send sysex when value changes.
                this.last_value = value;
                var text=[];
                if (value<1) {
                    value=0;
                } else {
                    // Transform position value in text string
                    for (n=1; n<Math.ceil(value/2); n++) { text.push(0x06); }
                    text.push(3+(value+1)%2);
                }
                for (n=Math.ceil(value/2); n<17; n++) { text.push(0x06); }
                AbletonPush.displayText(3, 2+(parseInt(this.group.substr(8, 1))+1)%2, text);
            }

        },
    });
    
    this.screen_BeatLoop = new components.Component({
        outKey: "beatloop_size",
        output: function(value, _group, _control) {
            if (value<1) value="1/" + 1/value;
            AbletonPush.displayText(4, 1+2*((parseInt(this.group.substr(8, 1))+1)%2), "" + value, 1);
        },
    });
    this.screen_BeatJump = new components.Component({
        outKey: "beatjump_size",
        output: function(value, _group, _control) {
            if (value<1) value="1/" + 1/value;
            AbletonPush.displayText(4, 1+2*((parseInt(this.group.substr(8, 1))+1)%2), "" + value, 2);
        },
    });
    this.screen_BPM = new components.Component({
        outKey: "visual_bpm",
        output: function(value, _group, _control) {
            AbletonPush.displayText(4, 2+2*((parseInt(this.group.substr(8, 1))+1)%2), "" + value, 1);
        },
    });
    this.screen_Key = new components.Component({
        outKey: "key",
        output: function(value, _group, _control) {
            AbletonPush.displayText(4, 2+2*((parseInt(this.group.substr(8, 1))+1)%2), AbletonPush.Keys[value], 2);
        },
    });

    // Set the group properties of the above Components and connect their output callback functions
    // Without this, the group property for each Component would have to be specified to its
    // constructor.
    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            // "this" inside a function passed to reconnectComponents refers to the ComponentContainer
            // so "this" refers to the custom Deck object being constructed
            c.group = this.currentDeck;
        }
    });
    // when called with JavaScript's "new" keyword, a constructor function
    // implicitly returns "this"
};
// give your custom Deck all the methods of the generic Deck in the Components library
AbletonPush.fullDeck.prototype = new components.Deck();

//#####################################################################
// Functions used to control Ableton Push.
//#####################################################################

/**
 * Displaying a line of text on the screen.
 * Screen is split in 4 lines.
 * Each line is made of 68 characters.
 * TODO : If a text is more than 68 cars, make it scroll.
 *
 * @param {String, Array} Text Text to display
 * @param {Number} iLine Line number (1 to 4)
 * @param {Number} iBloc Text block number on the line (1 to 4) - 0 to display full line
 * @param {Number} iSubBloc Push text only to one half of the block (1 or 2)
 */
AbletonPush.displayText = function(iLine, iBloc, Text, iSubBloc) {
    // Test line 1 to 4
    if (isNaN(iLine) || iLine < 1 || iLine > 4) {
        iLine = 1;
        Text = "ERROR line";
    }
    // Test block 0 to 4
    if (isNaN(iBloc) || iBloc < 0 || iBloc > 4) {
        iLine = 1;
        iBloc = 0;
        Text = "ERROR block";
    }
    // Test subbloc
    if (isNaN(iSubBloc) || iSubBloc < 1 || iSubBloc > 2) {
        iSubBloc = 0;
    }

    var textLen = (iBloc === 0) ? 68 : (iSubBloc === 0) ? 17 : 8;
    // If string, test it and convert it.
    if (typeof(Text) === "string") {
        var sText = Text;
        Text = [];
        // Test if text is ASCII only
        if (!/^[\x00-\x7F]*$/.test(sText)) {
            sText="ERROR not ASCII";
        }
        // Prepare text
        if (sText.length > textLen) {
            sText=sText.slice(0, textLen);
        }
        // Convert to ASCII code array
        for (var i=0; i<sText.length; i++) {
            Text.push(Number(sText.charCodeAt(i)));
        }
    }
    for (s=Text.length; s<textLen; s++) { (s%2==1) ? Text.push(0x20): Text.unshift(0x20); } // Complete with spaces (keeping text centered)

    // Prepares Sysex message
    var aSysex = [0xF0, 0x47, 0x79, 0x15, 0x17+iLine, 0x00];
    if (iBloc === 0) {
        aSysex.push(69);
        aSysex.push(0);
    } else if (iSubBloc === 0) {
        aSysex.push(18);
        aSysex.push(17*(iBloc-1));
    } else {
        aSysex.push(9);
        aSysex.push((17*(iBloc-1))+(9*(iSubBloc-1)));
    }
    aSysex = _.concat(aSysex, Text);
    aSysex.push(0xF7);

    midi.sendSysexMsg(aSysex, aSysex.length);
};

/**
 * Displaying a cursor on the screen.
 * Be careful this function use named parameters.
 *
 * @param {Number} hbloc Text half/block number on the line (1 to 8)
 * @param {Number} value Value of the control
 * @param {Number} min value of the control (default 0)
 * @param {Number} max value of the control (default 1)
 * @param {Number} mid value of the control (default disabled)
 * @param {Number} oc Value of the control (default disabled)
 */
//AbletonPush.displayCursor = function(hbloc, value, min, max, mid, oc) {
AbletonPush.displayCursor = function(options) {
    // Set defaults
    this.min = 0;
    this.max = 1;
    this.mid = -1;
    this.oc = -1;
    // Get parameters
    _.assign(this, options);
    
    // Keep oc value if required
    if (this.oc>0) {
        var valueoc = (this.value > this.max) ? this.value - this.max : 0;
        this.value = this.value - valueoc;
        if (valueoc > this.oc - this.max) { valueoc = this.oc - this.max; }
        valueoc = valueoc / (this.oc - this.max);
        valueoc=Math.ceil(valueoc*100)/100;
    }
    // Limit to min/max/oc values - example, pitch goes beyond 3.0 in mixxx !
    if (this.value > this.max) { this.value = this.max; }
    if (this.value < this.min) { this.value = this.min; }

    // Translate to 0-1
    this.value = (this.value - this.min) / (this.max - this.min);
    this.mid = (this.mid - this.min) / (this.max - this.min);
    
    // Clean number
    var value2=Math.round(this.value*100)/100;

    // Create steps
    value2=Math.round(value2*15)+1;
    var text=[];
    // Transform position value in text string
    if ((this.mid>0) && (this.value==this.mid)) {
        text.push(0x06, 0x06, 0x06, 0x04, 0x03, 0x06, 0x06, 0x06);
    } else {
        for (n=1; n<Math.ceil(value2/2); n++) { text.push(0x06); }
        text.push(3+(value2+1)%2);
        for (n=Math.ceil(value2/2); n<8; n++) { text.push(0x06); }
    }

    // Overlap Overclock if set
    if ((this.oc>0) && (valueoc>0)) {
        valueoc = Math.floor(valueoc*15)+1;
        for (n=1; n<Math.ceil(valueoc/2); n++) text[n-1]=0x02;
        text[n-1] = (((valueoc+1)%2)==1) ? 0x02 : 0x3D;
    }
    AbletonPush.displayText(2, Math.ceil((this.hbloc/2)), text, (2-(this.hbloc%2)));
}

/**
 * Set a pad color from 8 bit RGB value
 *
 * @param {Number} Pad Pad midi note
 * @param {Number} R Red color value in 8 bit (0-255)
 * @param {Number} G Green color value in 8 bit (0-255)
 * @param {Number} B Blue color value in 8 bit (0-255)
 */
AbletonPush.setRGBPadColor = function(Pad, R, G, B) {
    var aSysex = [0xF0, 0x47, 0x79, 0x15, 0x04, 0x00, 0x08, Pad-0x24, 0x00, R/16, R%16, G/16, G%16, B/16, B%16, 0xF7];
    midi.sendSysexMsg(aSysex, aSysex.length);
}

/**
 * Set pitchbend leds level
 * 
 * @param {Number} Level Between 0 and 24
 */
AbletonPush.setPitchBendLeds = function(Level) {
    // Transform value in level
    var aSysex = [0xF0, 0x47, 0x79, 0x15, 0x64, 0x00, 0x08];
    var bits = Math.pow(2, Level)-1; // Transform level in bits sequence
    var octet = 0x00;
    for (var i=0; i<24; i++) {
        octet += (bits & 1) ? 3 << (i%3)*2 : 0; // Duplicate the lower bit from "bits" and place it in "octet"
        if ((i%3) === 2) {
            aSysex.push(octet);
            octet = 0x00;
        }
        bits = bits >> 1; // Next bit
    }
    // Creates sysex message and send it
    aSysex.push([0xF7]);
    midi.sendSysexMsg(aSysex, aSysex.length);
};

/**
 * Show image on the pads
 *
 * @param {number} pos Position for recursive call
 */
AbletonPush.showImage = function(pos) {
    var width=AbletonPush.image_MIXXX.length/8/3;
    if (typeof(pos) === "undefined") { pos=0; }

    // Parse image to set RGB leds
    for (x=0; x<8; x++) {
        for (y=0; y<8; y++) {
            // get pixel address
            pixel=((x+pos)*3)+(y*width*3);
            led=(0x5C+x)-(y*8);
            // Set led color
            AbletonPush.setRGBPadColor(led, AbletonPush.image_MIXXX[pixel], AbletonPush.image_MIXXX[pixel+1], AbletonPush.image_MIXXX[pixel+2]);
        }
    }

    // If image left to print, scroll it with a timer.
    if (pos<width-8) {
        next=pos+1;
        engine.beginTimer(100, "AbletonPush.showImage("+next+")", true);
    } else {
        // Do init2 to finish startup sequence
        engine.beginTimer(100, "AbletonPush.init2()", true);
    }
};
