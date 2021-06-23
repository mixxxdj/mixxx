#!/usr/bin/python

#############################################################################
#                          make_xone.py  - Ugly script which automatically
#                          creates a midi mapping for the Xone K2.
#                          Lots of this is boilerplate...
#                             -------------------
#    begin                : Fri Aug 31 18:45:00 EDT 2012
#    copyright            : (C) 2012 Owen Williams
#    email                : owilliams@mixxx.org
#############################################################################


import sys
import getopt


def help():
    print("usage: make_xone [args] outputfilename")
    print("optional args: ")
    print("          --4decks     ", end="")
    print("to generate a layout for 4 deck-enabled versions of mixxx")
    print("          --mastersync ", end="")
    print("to generate alternative layout for master_sync testing")


if len(sys.argv) < 2:
    help()
    sys.exit(1)

MASTER_SYNC_LAYOUT = False
MAX_DECKS = 2
fname = ""
DECK_ORDER = range(0, 2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["mastersync", "4decks"])
    for o, a in opts:
        if o == "--mastersync":
            # This is an alternative layout for my work on master sync
            MASTER_SYNC_LAYOUT = True
        elif o == "--4decks":
            MAX_DECKS = 4
            DECK_ORDER = (2, 0, 1, 3)
    if len(args) > 0:
        fname = args[0]
except Exception as e:
    print(str(e))
    help()
    sys.exit(1)

if fname == "":
    help()
    sys.exit(1)

try:
    fh = open(fname, "w")
except Exception as e:
    print("Error opening file for write:", str(e))
    sys.exit(1)

button_definitions = """0x34	0x35	0x36	0x37
0x58	0x59	0x5A	0x5B
0x7C	0x7d	0x7e	0x7f


0x30	0x31	0x32	0x33
0x54	0x55	0x56	0x57
0x78	0x79	0x7A	0x7b



0x2c	0x2d	0x2e	0x2f
0x50	0x51	0x52	0x53
0x74	0x75	0x76	0x77


0x28	0x29	0x2a	0x2b
0x4c	0x4d	0x4e	0x4f
0x70	0x71	0x72	0x73



0x24	0x25	0x26	0x27
0x48	0x49	0x4a	0x4b
0x6c	0x6d	0x6e	0x6f


0x20	0x21	0x22	0x23
0x44	0x45	0x46	0x47
0x68	0x69	0x6a	0x6b


0x1c	0x1d	0x1e	0x1f
0x40	0x41	0x42	0x43
0x64	0x65	0x66	0x67


0x18	0x19	0x1A	0x1b
0x3c	0x3d	0x3e	0x3f
0x60	0x61	0x62	0x63"""

controls = [
    "spinknob",
    "knoblight1",
    "knoblight2",
    "knoblight3",
    "button1",
    "button2",
    "button3",
    "button4",
]
controls.reverse()
colors = ["red", "orange", "green"]
cur_color = 0

# {'spinknob': [{'red':1,'orange':2,'green':3},
#               {'red':4,'orange':5,'green':6}, ...

midi = {}
current_control = controls.pop()
for line in button_definitions.split("\n"):
    if len(line.strip()) == 0:
        continue

    hexlist = line.split()

    if current_control not in midi:
        midi[current_control] = [{}, {}, {}, {}]

    for i, h in enumerate(hexlist):
        midi[current_control][i][colors[cur_color]] = h

    cur_color = (cur_color + 1) % len(colors)
    if cur_color == 0:
        try:
            current_control = controls.pop()
        except Exception:
            break

# ok now we have a mapping between control and hex key

# now we need midi CC's -- these just increment simply

cc_controls = ["spinknob", "eq1", "eq2", "eq3", "slider"]
cc_code = 0
midi_cc = {}
for cc in cc_controls:
    midi_cc[cc] = []
    for i in range(0, 4):
        midi_cc[cc].append("0x%x" % cc_code)
        cc_code += 1


# now we need a mapping of control and controlobject

# controls = ['spinknob', 'knoblight1', 'knoblight2', 'knoblight3', 'button1',
#             'button2', 'button3', 'button4']


##################################################
# Actual stuff you can edit!                     #
##################################################

# Note that EQ control objects are munged in code to produce the correct
# control objects as of Mixxx 2.0.

cc_mapping = {
    "spinknob": ["XoneK2.encoderJog", "<Script-Binding/>"],
    "eq1": ["filterHigh", "<normal/>"],
    "eq2": ["filterMid", "<normal/>"],
    "eq3": ["filterLow", "<normal/>"],
    "slider": ["volume", "<normal/>"],
}

button_mapping = {
    "spinknob": ["beatsync", "<button/>"],
    "knoblight1": ["filterHighKill", "<normal/>"],
    "knoblight2": ["filterMidKill", "<normal/>"],
    "knoblight3": ["filterLowKill", "<normal/>"],
    "button1": {
        "red": ["pfl", "<normal/>"],
        "orange": ["beatloop_4", "<button/>"],
        "green": ["hotcue_1_activate", "<button/>"],
    },
    "button2": {
        "red": ["keylock", "<normal/>"],
        "orange": ["loop_double", "<button/>"],
        "green": ["hotcue_2_activate", "<button/>"],
    },
    "button3": {
        "red": ["cue_default", "<button/>"],
        "orange": ["loop_halve", "<button/>"],
        "green": ["hotcue_3_activate", "<button/>"],
    },
    "button4": {
        "red": ["play", "<normal/>"],
        "orange": ["reloop_exit", "<button/>"],
        "green": ["hotcue_4_activate", "<button/>"],
    },
}

if MASTER_SYNC_LAYOUT:
    button_mapping = {
        "spinknob": ["XoneK2.encoderButton", "<Script-Binding/>"],
        "knoblight1": ["keylock", "<button/>"],
        "knoblight2": ["quantize", "<normal/>"],
        "knoblight3": ["filterLowKill", "<normal/>"],
        "button1": {
            "red": ["pfl", "<button/>"],
            "orange": ["beatloop_4", "<button/>"],
            "green": ["hotcue_1_activate", "<button/>"],
        },
        "button2": {
            "red": ["sync_enabled", "<button/>"],
            "orange": ["loop_double", "<button/>"],
            "green": ["hotcue_2_activate", "<button/>"],
        },
        "button3": {
            "red": ["cue_default", "<button/>"],
            "orange": ["loop_halve", "<button/>"],
            "green": ["hotcue_3_activate", "<button/>"],
        },
        "button4": {
            "red": ["play", "<button/>"],
            "orange": ["reloop_exit", "<button/>"],
            "green": ["hotcue_4_activate", "<button/>"],
        },
    }


light_mapping = {  # 'spinknob':'jog',
    "knoblight1": "filterHighKill",
    "knoblight2": "filterMidKill",
    "knoblight3": "filterLowKill",
    "button1": {
        "red": "pfl",
        "orange": "beatloop_4",
        "green": "hotcue_1_enabled",
    },
    "button2": {
        "red": "keylock",
        "orange": "loop_double",
        "green": "hotcue_2_enabled",
    },
    "button3": {
        "red": "cue_default",
        "orange": "loop_halve",
        "green": "hotcue_3_enabled",
    },
    "button4": {
        "red": "play",
        "orange": "loop_enabled",
        "green": "hotcue_4_enabled",
    },
}

if MASTER_SYNC_LAYOUT:
    light_mapping = {  # 'spinknob':'jog',
        "knoblight1": "keylock",
        "knoblight2": "quantize",
        "knoblight3": "filterLowKill",
        "button1": {
            "red": "pfl",
            "orange": "beatloop_4",
            "green": "hotcue_1_enabled",
        },
        "button2": {
            "red": "sync_enabled",
            "orange": "loop_double",
            "green": "hotcue_2_enabled",
        },
        "button3": {
            "red": "cue_default",
            "orange": "loop_halve",
            "green": "hotcue_3_enabled",
        },
        "button4": {
            "red": "play",
            "orange": "loop_enabled",
            "green": "hotcue_4_enabled",
        },
    }


# these aren't worth automating
master_knobs = """            <control>
                <group>[Playlist]</group>
                <key>XoneK2.rightBottomKnob</key>
                <status>0xBF</status>
                <midino>0x15</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
            <control>
                <group>[Playlist]</group>
                <key>LoadSelectedIntoFirstStopped</key>
                <status>0x9F</status>
                <midino>0x0E</midino>
                <options>
                    <normal/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.leftBottomKnob</key>
                <status>0xBF</status>
                <midino>0x14</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.shift_on</key>
                <status>0x9F</status>
                <midino>0xF</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.shift_on</key>
                <status>0x8F</status>
                <midino>0xF</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>"""

if MASTER_SYNC_LAYOUT:
    master_knobs = """            <control>
                <group>[Playlist]</group>
                <key>XoneK2.rightBottomKnob</key>
                <status>0xBF</status>
                <midino>0x15</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
             <control>
                <group>[Playlist]</group>
                <key>LoadSelectedIntoFirstStopped</key>
                <status>0x9F</status>
                <midino>0x0E</midino>
                <options>
                    <normal/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.leftBottomKnob</key>
                <status>0xBF</status>
                <midino>0x14</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.shift_on</key>
                <status>0x9F</status>
                <midino>0xF</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>
            <control>
                <group>[Master]</group>
                <key>XoneK2.shift_on</key>
                <status>0x8F</status>
                <midino>0xF</midino>
                <options>
                    <Script-Binding/>
                </options>
            </control>"""


xml = []
xml.append(
    """<?xml version='1.0' encoding='utf-8'?>
<!-- This file automatically generated by make_xone.py. -->
<MixxxControllerPreset mixxxVersion="" schemaVersion="1">
    <info>
        <name>Allen&amp;Heath Xone K2</name>
        <author>Owen Williams</author>
        <description>For this mapping to work:
- Set Xone:K2 midi channel to 16;
- Set Xone:K2 Latching Layers state to "Switch Matrix" (state 2).
(See product manual for details.)</description>
    </info>
    <controller id="XONE:K2">
        <scriptfiles>
            <file filename="Xone-K2-scripts.js" functionprefix="XoneK2"/>
        </scriptfiles>
        <controls>"""
)


xml.append("<!-- CC Controls (knobs and sliders) -->")

# for i, deck in enumerate(DECK_ORDER):
#    xml.append("""            <control>
#                <group>[Channel%i]</group>
#                <key>XoneK2.encoderJog%i</key>
#                <status>0xBF</status>
#                <midino>%s</midino>
#                <options>
#                    <Script-Binding/>
#                </options>
#            </control>""" % (deck+1, deck+1, midi_cc['spinknob'][i]))


########################################
########################################
########################################
# ok back to boilerplate...


def get_group_name(channel, key):
    """Optionally munge group name if an EQ"""
    if "filter" in key:
        return "[EqualizerRack1_[Channel%d]_Effect1]" % channel
    return "[Channel%d]" % channel


def get_key_name(key):
    """Optionally munge key name if an EQ"""
    if key == "filterLow":
        return "parameter1"
    if key == "filterMid":
        return "parameter2"
    if key == "filterHigh":
        return "parameter3"
    if key == "filterLowKill":
        return "button_parameter1"
    if key == "filterMidKill":
        return "button_parameter2"
    if key == "filterLowKill":
        return "button_parameter3"
    return key


# cc controls (no latching needed)
for i, channel in enumerate(DECK_ORDER):
    for cc in cc_mapping:
        xml.append(
            """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0xBF</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
            % (
                get_group_name(channel + 1, cc_mapping[cc][0]),
                get_key_name(cc_mapping[cc][0]),
                midi_cc[cc][i],
                cc_mapping[cc][1],
            )
        )

# Spin Knob buttons (no latching needed)
for i, channel in enumerate(DECK_ORDER):
    xml.append(
        """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x9F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
        % (
            get_group_name(channel + 1, button_mapping["spinknob"][0]),
            get_key_name(button_mapping["spinknob"][0]),
            midi["spinknob"][i]["red"],
            button_mapping["spinknob"][1],
        )
    )
    xml.append(
        """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x8F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
        % (
            get_group_name(channel + 1, button_mapping["spinknob"][0]),
            get_key_name(button_mapping["spinknob"][0]),
            midi["spinknob"][i]["red"],
            button_mapping["spinknob"][1],
        )
    )

xml.append("<!-- Upper Buttons -->")
# knoblight buttons (no latching)
for j, channel in enumerate(DECK_ORDER):
    for knob in ["knoblight%i" % i for i in range(1, 4)]:
        xml.append(
            """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x9F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
            % (
                get_group_name(channel + 1, button_mapping[knob][0]),
                get_key_name(button_mapping[knob][0]),
                midi[knob][j]["red"],
                button_mapping[knob][1],
            )
        )
        xml.append(
            """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x8F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
            % (
                get_group_name(channel + 1, button_mapping[knob][0]),
                get_key_name(button_mapping[knob][0]),
                midi[knob][j]["red"],
                button_mapping[knob][1],
            )
        )

xml.append("<!-- Lower Button Grid -->")

# buttons
for j, channel in enumerate(DECK_ORDER):
    for latch in ["red", "orange", "green"]:
        for button in ["button%i" % i for i in range(1, 5)]:
            xml.append(
                """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x9F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
                % (
                    get_group_name(
                        channel + 1, button_mapping[button][latch][0]
                    ),
                    get_key_name(button_mapping[button][latch][0]),
                    midi[button][j][latch],
                    button_mapping[button][latch][1],
                )
            )
            xml.append(
                """            <control>
                <group>%s</group>
                <key>%s</key>
                <status>0x8F</status>
                <midino>%s</midino>
                <options>
                    %s
                </options>
            </control>"""
                % (
                    get_group_name(
                        channel + 1, button_mapping[button][latch][0]
                    ),
                    get_key_name(button_mapping[button][latch][0]),
                    midi[button][j][latch],
                    button_mapping[button][latch][1],
                )
            )

xml.append("<!-- Special Case Knobs / buttons -->")
# a couple custom entries:
xml.append(master_knobs)


# done with controls

xml.append(
    """        </controls>
        <outputs>"""
)

# ok now the lights
if "spinknob" in light_mapping:
    for i, channel in enumerate(DECK_ORDER):
        xml.append(
            """            <output>
                    <group>%s</group>
                    <key>%s</key>
                    <status>0x9F</status>
                    <midino>%s</midino>
                    <minimum>0.5</minimum>
                </output>"""
            % (
                get_group_name(channel + 1, light_mapping["spinknob"]),
                get_key_name(light_mapping["spinknob"]),
                midi["spinknob"][i]["red"],
            )
        )


xml.append("<!-- Knob lights -->")
# knoblight buttons (no latching)
for j, channel in enumerate(DECK_ORDER):
    for knob in ["knoblight%i" % i for i in range(1, 4)]:
        xml.append(
            """            <output>
                <group>%s</group>
                <key>%s</key>
                <status>0x9F</status>
                <midino>%s</midino>
                <minimum>0.5</minimum>
            </output>"""
            % (
                get_group_name(channel + 1, light_mapping[knob]),
                get_key_name(light_mapping[knob]),
                midi[knob][j]["red"],
            )
        )

xml.append("<!-- Button lights -->")
# buttons (latched)
for j, channel in enumerate(DECK_ORDER):
    for latch in ["red", "orange", "green"]:
        for button in ["button%i" % i for i in range(1, 5)]:
            xml.append(
                """            <output>
                <group>%s</group>
                <key>%s</key>
                <status>0x9F</status>
                <midino>%s</midino>
                <minimum>0.5</minimum>
            </output>"""
                % (
                    get_group_name(channel + 1, light_mapping[button][latch]),
                    get_key_name(light_mapping[button][latch]),
                    midi[button][j][latch],
                )
            )

xml.append(
    """        </outputs>
    </controller>
</MixxxControllerPreset>"""
)

fh.writelines("\n".join(xml))
fh.close()
