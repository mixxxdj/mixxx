#!/usr/bin/python3

import os
import re
import ntpath
import configparser
from tkinter import *
from xml.dom import minidom
from tkinter import filedialog
from tkinter import simpledialog
from collections import defaultdict
from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring


# README
#
# 1. Run this file. (with PYTHON3 or higher!!)
#
# Note: Parsing cz_CH.kbd.cfg and ru_RU.kbd.cfg fails on Windows.


def main():
    app = Gui(None)
    app.mainloop()

    # TEST
    # KeyboardParser(
    #     multilang=False,
    #     legacy_file="C:/Users/jordi/Development/mixxx/mixxx/res/keyboard/de_DE.kbd.cfg",
    #     target_file="C:/Users/jordi/Development/mixxx/mixxx/scripts/test.kbd.xml",
    #     layouts_path="C:/Users/jordi/Desktop/layouts.xml"
    # )


class KeyboardParser:
    def __init__(self, legacy_folder=None, name="", layouts_path=None,
                 multilang=True, legacy_file=None, target_file=None,
                 mixxx_version="2.1.0+", author="kbdcfg_to_kbdxml.py"):

        self.legacy_folder = legacy_folder
        self.name = name
        self.layouts = []
        self.multilang = multilang
        self.legacy_file = legacy_file
        self.legacy_extension = ".kbd.cfg"
        self.target_file = target_file
        self.xml_extension = ".kbd.xml"
        self.mixxx_version = mixxx_version
        self.author = author
        self.description = "This preset was generated using kbdcfg_to_kbdxml.py"

        if not target_file:
            raise ValueError("Given target file is empty, please provide a path to save the *kbd.xml file")
        print("Preset will be saved to: " + target_file)

        self.open_layouts_file(layouts_path)

        if multilang:
            print("kbdcfg_to_kbdxml in multi-lang mode")
            if not name:
                raise ValueError("Given name is empty, please enter a valid preset name.")
            if not os.path.exists(legacy_folder):
                raise ValueError("Given legacy folder does not exist: \'" + legacy_folder + "\'")
            print("Reading legacy files from: " + legacy_folder)

            # List containing configparsers for each *.kbd.cfg file
            legacy_mappings = self.parse_multi_lang_config()

            # XML root element containing mapping info for each language
            xml = self.create_multi_lang_xml(legacy_mappings)

            # Simplify XML by getting rid of key seqs
            # shared by multiple keyboard layouts
            #
            # Example:
            # <keyseq lang="en_US">f</keyseq>
            # <keyseq lang="es_ES">f</keyseq>
            #
            # Simplified to:
            # <keyseq lang="en_US, es_ES">f</keyseq>
            #
            # Note: If a keysequence is shared by all supported languages, the lang
            #       attribute will not be added.
            simplified_xml = self.simplify_multilang_xml(xml)
            self.write_out(simplified_xml)

        else:
            print("kbdcfg_to_kbdxml in single-file mode")
            if not legacy_file:
                raise ValueError("Given legacy file is empty, please provide a path to a *.kbd.cfg file")
            if not os.path.isfile(legacy_file):
                raise FileNotFoundError("Given legacy file does not exist: " + legacy_file)

            # Configparser for legacy file
            legacy_mapping = self.parse_single_config()

            # XML root element containing mapping info
            xml = self.create_xml(legacy_mapping)

            self.write_out(xml)

    def open_layouts_file(self, path):
        if not path:
            print("Layouts file is not given, key scancodes won't be set.")
            return

        if not os.path.isfile(path):
            print("Layouts file doesn't exist: " + path + "\nKey scancodes won't be set")
            return

        print("Parsing layouts from " + path + "...")
        with open(path, 'rt') as f:
            tree = ElementTree.parse(f)

        # Get KeyboardLayoutTranslations element, which holds all
        # information and is basically the root element
        root = None
        for element in tree.getroot().iter():
            if element.tag == 'KeyboardLayoutTranslations':
                root = element
                break

        # Retrieve layouts element
        layouts_element = root.find('layouts')

        if not layouts_element:
            print("Couldn't retrieve layout list, no <layouts> element "
                  "in loaded XML. Pleas load in an other XML file.")
            return

        for layout in layouts_element.iter('lang'):
            name = layout.text
            if not KeyboardLayout.validate_layout_name(name):
                print("Layout name: " + name + " is not a valid language code name. Not loading this layout.")
                continue
            self.layouts.append(
                KeyboardLayout(name=name, root=root)
            )

    def parse_single_config(self):
        legacy_file = self.legacy_file
        legacy_extension = self.legacy_extension

        file_name = ntpath.basename(legacy_file)
        base_name = file_name[:-len(legacy_extension)]

        if KeyboardLayout.validate_layout_name(base_name):
            print("Parsing " + legacy_file + "...")
            parser = configparser.ConfigParser(allow_no_value=True)
            parser.optionxform = str
            parser.read(legacy_file)
            return parser
        else:
            print("Skipping " + legacy_file +
                  ". The filename did not pass the name validation test.")

    def parse_multi_lang_config(self):
        legacy_folder = self.legacy_folder
        legacy_extension = self.legacy_extension

        # Keys will hold the language code, values will hold config parsers
        # for the config file corresponding to that language code
        multi_lang_mappings = {}

        # Iterate over all files in legacy_folder
        for file_name in sorted(os.listdir(legacy_folder)):
            if file_name.endswith(legacy_extension):
                full_path = legacy_folder + '/' + file_name
                base_name = file_name[:-len(legacy_extension)]

                # Parse file and store it in multi_lang_mappings if filename
                # ends with kbd.cfg and starts with a valid language code
                if KeyboardLayout.validate_layout_name(base_name):
                    print("Parsing " + file_name + "...")
                    parser = configparser.ConfigParser(allow_no_value=True)
                    parser.optionxform = str
                    parser.read(full_path)
                    multi_lang_mappings[base_name] = parser
                else:
                    print("Skipping " + file_name +
                          ". The filename did not pass the name validation test.")

        return multi_lang_mappings

    def _create_empty_xml(self):
        elements = {}

        # Create XML root element
        root = Element('MixxxKeyboardPreset')
        root.set('schemaVersion', '1')
        root.set('mixxxVersion', self.mixxx_version)
        elements['root'] = root

        # Create info block
        info = SubElement(root, 'info')
        SubElement(info, 'name').text = self.name
        SubElement(info, 'author').text = self.author
        SubElement(info, 'description').text = self.description
        elements['info'] = info

        # Create controller block
        elements['controller'] = SubElement(root, 'controller')

        return elements

    def create_xml(self, parser):
        if not type(parser):
            raise TypeError("Expected configparser, but got a " + type(parser).__name__)

        # Retrieve base name of file (hence, language code)
        file_name = ntpath.basename(self.legacy_file)
        print(file_name)
        lang = file_name[:-len(self.legacy_extension)]

        # Create empty preset file and store dictionary holding basic elements
        elements = self._create_empty_xml()
        controller = elements['controller']
        root = elements['root']

        groups = parser.sections()

        # Iterate over all groups specified in the legacy file
        for group in groups:
            group_element = SubElement(controller, 'group')
            group_element.set('name', group)

            # Iterate over all controls
            for control in parser.items(group):
                stripped_control = control[0].split()
                action = stripped_control[0]
                try:
                    keyseq = stripped_control[1]
                except:
                    keyseq = ""

                # Retrieve modifiers
                modifiers = KeyboardParser.get_modifiers(keyseq)

                # Retrieve scancode
                layout = self.get_layout_with_name(lang)
                modifier_int = KeyboardKey.MODIFIERS.SHIFT if modifiers == {"SHIFT"} else KeyboardKey.MODIFIERS.NONE
                scancode = layout.get_scancode(keyseq=keyseq, modifier=modifier_int) if layout else\
                    "TODO: Set scancode (layout was not found for '" + lang + "')"

                # Create control element node inside of group block
                control_element = SubElement(group_element, 'control')
                control_element.set('action', action)

                # Create keyseq element node inside of group block
                keyseq_element = SubElement(control_element, 'keyseq')
                keyseq_element.set('lang', lang)
                keyseq_element.set('scancode', str(scancode))
                keyseq_element.text = keyseq

        return root

    MODIFIERS = ["SHIFT", "CTRL", "ALT", "META"]

    @staticmethod
    def get_modifiers(keyseq):
        modifiers = set()
        split_keyseq = keyseq.split('+')
        for key in split_keyseq:
            key = key.strip().upper()
            if key in KeyboardParser.MODIFIERS:
                modifiers.add(key)
        return modifiers

    def create_multi_lang_xml(self, mappings):
        if not type(mappings) is dict:
            raise TypeError("Expected dictionary, but got a " + type(mappings).__name__)

        elements = self._create_empty_xml()
        controller = elements['controller']
        root = elements['root']

        # Create keyboard layouts block
        keyboard_layouts = SubElement(controller, 'keyboard-layouts')
        for layout in sorted(mappings.items()):
            SubElement(keyboard_layouts, 'lang').text = layout[0]

        # Retrieve which groups and actions are defined and store them in
        # actions dictionary, where:
        #   key   = group name
        #   value = set, containing every action corresponding to the key group
        actions = {}

        # Iterate over all keyboard layouts and their mappings
        print("Reading data...")
        for mapping in mappings.items():
            parser = mapping[1]
            groups_in_this_mapping = parser.sections()

            for group in groups_in_this_mapping:
                if group not in actions:
                    actions[group] = set()

                # Retrieve all actions in this group
                actions_in_this_group = set()
                for control in parser.items(group):
                    stripped_action = control[0].split()[0]
                    actions_in_this_group.add(stripped_action)

                # Add all actions to the actions set, if not already there
                actions[group].update(actions_in_this_group)

        # Add group blocks
        print("Adding groups...")
        for group in sorted(actions):
            group_element = SubElement(controller, 'group')
            group_element.set('name', '[' + group + ']')
            print("\t" + '[' + group + ']')

            # Iterate over actions in this group
            for action in sorted(actions[group]):
                # Add control element node to current group node
                control_element = SubElement(group_element, 'control')
                control_element.set('action', action)

                # Iterate over all keyboard layouts and find if
                # this action is defined
                for mapping in mappings.items():
                    parser = mapping[1]

                    # TODO(Tomasito) Don't build logic on exception catching!
                    try:
                        parser.items(group)
                    except configparser.NoSectionError:
                        continue

                    # Iterate through all actions in the group and see
                    # if the current action is defined in the current
                    # parser. (hence, the current keyboard layout)
                    for action_in_this_group in parser.items(group):
                        stripped_action_in_this_group = action_in_this_group[0].split()
                        try:
                            this_keyseq = stripped_action_in_this_group[1]
                        except:
                            this_keyseq = ""

                        if action == action_in_this_group[0].split()[0]:
                            keyseq_element = SubElement(control_element, 'keyseq')
                            keyseq_element.set('lang', mapping[0])
                            keyseq_element.text = this_keyseq

        return root

    @staticmethod
    def simplify_multilang_xml(xml):
        print("Simplifying XML")

        # Retrieve which keyboard layouts are supported
        supported_layouts = []
        supported_layouts_element = xml. \
            find('controller'). \
            find('keyboard-layouts')
        for lang in supported_layouts_element.iter('lang'):
            supported_layouts.append(lang.text)

        # Iterate over all group blocks
        for group in xml.iter('group'):
            for control in group.iter('control'):

                # Defaultdict with the default value being a list. Keyseqs will store
                # information about all keyseqs in this particular control where:
                #    key   = keysequence string
                #    value = list, listing all kbd layouts thich this keyseq targets
                keyseqs = defaultdict(list)

                # Storing keyseq elements in list so that we can remove them later
                keyseq_elements = list()

                # Fill keyseqs
                for keyseq_element in control.iter('keyseq'):
                    lang = keyseq_element.get('lang')
                    keyseqs[keyseq_element.text].append(lang)
                    keyseq_elements.append(keyseq_element)

                # Remove all keyseq elements
                for keyseq_element in keyseq_elements:
                    control.remove(keyseq_element)

                # Refill control block with keyseq elements
                for keyseq, langs in keyseqs.items():
                    # Number of languages that share this keyseq
                    n_lang = len(langs)
                    assert n_lang >= 1

                    keyseq_element = Element('keyseq')
                    keyseq_element.text = keyseq

                    if n_lang == 1:
                        keyseq_element.set('lang', langs[0])
                    else:
                        shared_by_all_langs = True
                        for lang in supported_layouts:
                            if lang not in langs:
                                shared_by_all_langs = False
                                break
                        if not shared_by_all_langs:
                            keyseq_element.set('lang', ', '.join(langs))
                    control.append(keyseq_element)
        return xml

    def write_out(self, root):
        full_path = self.target_file
        if os.path.isfile(full_path):
            print("Warning: '" + full_path + "' already exists.\nOverriding that file assuming that it's ok :)\n")
        print("Writing to file: " + full_path + "\n...")
        xmlstr = minidom.parseString(tostring(root)).toprettyxml()
        with open(full_path, "w") as f:
            f.write(xmlstr)
        print("Saved successfully!")

    def get_layout_with_name(self, name):
        for layout in self.layouts:
            if layout.name == name:
                return layout
        return None


class Gui(Tk):
    TITLE = "kbd.cfg to kbd.xml converter"

    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)
        self.wm_title(Gui.TITLE)

        Button(self,
               text="Single file conversion",
               command=lambda: SingleFileConversionDialog(self)).pack(padx=5, pady=5, fill=BOTH, expand=1)

        Button(self,
               text="Multi-lang conversion",
               command=lambda: MultiLangConversionDialog(self)).pack(padx=5, pady=5, fill=BOTH, expand=1)


class SingleFileConversionDialog(simpledialog.Dialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.legacy_file = None
        self.target_file = None
        self.layouts_file = None

    def body(self, master):
        self.wm_title("Single file conversion")

        Label(master, text="Transform one single .kbd.cfg file to one kbd.xml file. \n"
                           "Please note that tha given filename must follow the QLocale \n"
                           "name format: 'language[_country]', where:\n"
                           "- language is a lowercase, two-letter, ISO 639 language code\n"
                           "- country is an uppercase, two- or three-letter, ISO 3166 country code", justify=LEFT,
              bg='#388E3C', fg='white').pack()

        body = Frame(master)
        Grid.columnconfigure(body, 0, weight=1)
        Grid.columnconfigure(body, 1, weight=1)
        for i in range(0, 3):
            Grid.rowconfigure(body, i, weight=1)

        Label(body, text="Legacy file:").grid(row=0, sticky='w')
        Label(body, text="Save to:").grid(row=1, sticky='w')
        Label(body, text="Layouts file:").grid(row=2, sticky='w')

        self.legacy_file = Entry(body)
        self.target_file = Entry(body)
        self.layouts_file = Entry(body)

        self.legacy_file.grid(row=0, column=1, sticky='nsew')
        self.target_file.grid(row=1, column=1, sticky='nsew')
        self.layouts_file.grid(row=2, column=1, sticky='nsew')

        Button(body, text="Browse", command=lambda: self._browse_legacy_file()).\
            grid(row=0, column=2, sticky='nsew', padx=3)
        Button(body, text="Browse", command=lambda: self._browse_target_file()).\
            grid(row=1, column=2, sticky='nsew', padx=3)
        Button(body, text="Browse", command=lambda: self._browse_layouts_file()).\
            grid(row=2, column=2, sticky='nsew', padx=3)

        body.pack(fill=BOTH, expand=1, pady=10)

        # Set first focus
        return self.legacy_file

    def _browse_legacy_file(self):
        legacy_file = filedialog.askopenfilename(
            filetypes=(("Keyboard config files", ".kbd.cfg"), ("All Files", "*")),
            title="Choose a legacy kbd.cfg file"
        )
        if not legacy_file:
            return None

        self.legacy_file.delete(0, END)
        self.legacy_file.insert(0, legacy_file)

    def _browse_target_file(self):
        try:
            target_file = filedialog.asksaveasfilename(
                defaultextension=".kbd.xml",
                title="Where to save the kbd.xml file")
        except AttributeError:
            return

        self.target_file.delete(0, END)
        self.target_file.insert(0, target_file)

    def _browse_layouts_file(self):
        layouts_file = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a layouts xml file"
        )
        if not layouts_file:
            return None

        self.layouts_file.delete(0, END)
        self.layouts_file.insert(0, layouts_file)

    def apply(self):
        KeyboardParser(
            multilang=False,
            legacy_file=self.legacy_file.get(),
            target_file=self.target_file.get(),
            layouts_path=self.layouts_file.get()
        )


class MultiLangConversionDialog(simpledialog.Dialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.legacy_folder = None
        self.target_file = None
        self.layouts_file = None
        self.preset_name = None

    def body(self, master):
        self.wm_title("Multi-language conversion")

        Label(master, text="Convert multiple kbd.cfg files into one kbd.xml file. Please \n"
                           "note that only the files will be converted of which the filename \n"
                           "follows the QLocale name format: 'language[_country]', where:\n"
                           "- language is a lowercase, two-letter, ISO 639 language code\n"
                           "- country is an uppercase, two- or three-letter, ISO 3166 country code",
              justify=LEFT, bg='#388E3C', fg='white').pack()

        body = Frame(master)
        Grid.columnconfigure(body, 0, weight=1)
        Grid.columnconfigure(body, 1, weight=1)
        for i in range(0, 4):
            Grid.rowconfigure(body, i, weight=1)

        Label(body, text="Legacy folder:").grid(row=0, sticky='w')
        Label(body, text="Save to:").grid(row=1, sticky='w')
        Label(body, text="Layouts file:").grid(row=2, sticky='w')
        Label(body, text="Preset name:").grid(row=3, sticky='w')

        self.legacy_folder = Entry(body)
        self.target_file = Entry(body)
        self.preset_name = Entry(body)
        self.layouts_file = Entry(body)

        self.legacy_folder.grid(row=0, column=1, sticky='nsew')
        self.target_file.grid(row=1, column=1, sticky='nsew')
        self.layouts_file.grid(row=2, column=1, sticky='nsew')
        self.preset_name.grid(row=3, column=1, sticky='nsew')

        Button(body, text="Browse", command=lambda: self._browse_legacy_folder()).grid(
            row=0, column=2, sticky='nsew', padx=3)
        Button(body, text="Browse", command=lambda: self._browse_target_file()).grid(
            row=1, column=2, sticky='nsew', padx=3)
        Button(body, text="Browse", command=lambda: self._browse_layouts_file()).grid(
            row=2, column=2, sticky='nsew', padx=3)

        body.pack(fill=BOTH, expand=1, pady=10)

        # Set first focus
        return self.legacy_folder

    def _browse_legacy_folder(self):
        legacy_folder = filedialog.askdirectory(
            title="Choose a folder containing kbd.cfg files")
        if not legacy_folder:
            return
        if not self.preset_name.get():
            folder_name = os.path.basename(
                os.path.normpath(legacy_folder)
            )
            self.preset_name.insert(0, folder_name)
        self.legacy_folder.delete(0, END)
        self.legacy_folder.insert(0, legacy_folder)

    def _browse_target_file(self):
        try:
            target_file = filedialog.asksaveasfilename(
                defaultextension=".kbd.xml",
                title="Where to save the kbd.xml file")
        except AttributeError:
            return
        self.target_file.delete(0, END)
        self.target_file.insert(0, target_file)

    def _browse_layouts_file(self):
        layouts_file = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a layouts xml file"
        )
        if not layouts_file:
            return None
        self.layouts_file.delete(0, END)
        self.layouts_file.insert(0, layouts_file)

    def apply(self):
        KeyboardParser(
            multilang=True,
            legacy_folder=self.legacy_folder.get(),
            target_file=self.target_file.get(),
            name=self.preset_name.get(),
            layouts_path=self.layouts_file.get()
        )


class KeyboardKey:
    """ Class representing one physical character key as seen here:
    https://en.wikipedia.org/wiki/Keyboard_layout#/media/File:ISO_keyboard_(105)_QWERTY_UK.svg

    This class contains information about which character is bound
    to this keyboard key, also for different modifiers"""

    class MODIFIERS:
        """ Modifier enum, following the Qt::KeyboardModifier standard, found here:
        http://doc.qt.io/qt-5/qt.html#KeyboardModifier-enum """
        NONE, SHIFT, CTRL, ALT, META, KEYPAD = range(1, 7)

        @staticmethod
        def is_valid_modifier(modifier):
            return modifier in range(1, 3)

    class KeyChar:
        def __init__(self, modifier, char):
            self.modifier = modifier
            self.char = char

    def __init__(self, scancode):
        self.scancode = scancode

        # A list containing KeyChar objects. For example, for the a key, it will contain:
        #   - One KeyChar with modifier == NONE and char = 'a'
        #   - One KeyChar with modifier == SHIFT and char = 'A'
        self._key_chars = []

    def get_char(self, modifier, verbose=True):
        if not KeyboardKey.MODIFIERS.is_valid_modifier(modifier):
            if verbose:
                print("Given modifier: " + modifier + " is not valid.")
            return

        for key_char in self._key_chars:
            if key_char.modifier == modifier:
                return key_char

        if verbose:
            print("Keyboard key with scancode '" + str(self.scancode) +
                  "' has not a key char set with modifier: " + str(modifier))
        return None

    def set_key_char(self, modifier, char):
        if not KeyboardKey.MODIFIERS.is_valid_modifier(modifier):
            print("Given modifier: " + modifier + " is not valid.")
            return

        # Check if there is already a KeyChar with the given modifier
        key_char = self.get_char(modifier=modifier, verbose=False)

        if key_char:
            key_char.char = char
        else:
            self._key_chars.append(KeyboardKey.KeyChar(
                modifier=modifier,
                char=char
            ))


class KeyboardLayout:
    NAME_VALIDATION_PATTERN = re.compile("^[a-z]{2}_[A-Z]{2,3}$")

    @staticmethod
    # Check if name has the correct format, described here: http://doc.qt.io/qt-4.8/qlocale.html#QLocale-2
    # Name has to start with a two letter ISO 639-1 language code ...
    # ... followed by an underscore ...
    # ... followed by a three letter uppercase ISO 3166 country code
    #
    # Note: This validation is not full-prove: qw_ZXC will pass.
    def validate_layout_name(name):
        return KeyboardLayout.NAME_VALIDATION_PATTERN.match(name)

    def __init__(self, name="", root=None):
        # Keyboard layout name
        self.name = name

        # Mapping data for this keyboard layout, contains instances of KeyboardKey.
        self._data = []

        # Parse XML and store it into self._data
        self.parse_xml(root)

    def parse_xml(self, root):
        for key in root.iter('key'):
            scancode = key.get('scancode')
            if not scancode:
                print(
                    "Skipping key, no scan code defined. Make sure that all key "
                    "elements have a 'scancode' attribute telling the scancode as "
                    "described here: http://www.barcodeman.com/altek/mule/kb102.gif")
                break

            for char in key.iter('char'):
                lang = char.get('lang')
                if lang == self.name:
                    modifier_attr = char.get('modifier')
                    if modifier_attr == "NONE":
                        modifier = KeyboardKey.MODIFIERS.NONE
                    elif modifier_attr == "SHIFT":
                        modifier = KeyboardKey.MODIFIERS.SHIFT
                    else:
                        modifier = -1

                    self.update_key(
                        scancode=int(scancode),
                        modifier=int(modifier),
                        char=char.text,
                        verbose=False
                    )

    def find(self, scancode):
        for key in self._data:
            if key.scancode == scancode:
                return key
        return None

    def update_key(self, scancode, modifier, char, verbose=True):
        key = self.find(scancode)
        if not key:
            if verbose:
                print("Can't update key with scancode '" + str(scancode) +
                      "'. Scancode doesnt't exist. Creating new one...")
            key = KeyboardKey(scancode)
            self._data.append(key)
        key.set_key_char(modifier=modifier, char=char)

    def get_scancodes(self):
        scancodes = set()
        for key in self._data:
            scancodes.add(key.scancode)
        return scancodes

    def get_scancode(self, keyseq, modifier):
        # Retrieve scancode
        split_keyseq = keyseq.split('+')
        char = split_keyseq[-1] if split_keyseq else ""

        # Make sure that the character is a lower-case character if shift is not pressed and
        # is an upper-case character when shift is pressed so that it can be found in _data
        char = char.upper() if modifier == KeyboardKey.MODIFIERS.SHIFT else char.lower()

        scancodes = []
        for key in self._data:
            if key.get_char(modifier).char == char:
                scancodes.append(key.scancode)
        scancode = scancodes[0] if len(scancodes) == 1 \
            else "TODO: Set scancode for " + char + " (please choose between one of these: " + str(scancodes) + ")"

        # Check if this key is universal (for example: F-keys or Space)
        if KeyboardLayout.is_universal_key(char):
            scancode = "universal_key"

        return scancode

    @staticmethod
    def is_universal_key(key):
        universal_keys = [
            "LEFT", "UP", "RIGHT", "DOWN",
            "SPACE", "RETURN", "F1", "F2",
            "F3", "F4", "F5", "F6", "F7",
            "F8", "F9", "F10", "F11", "F12"]

        if key.upper() in universal_keys:
            return True
        else:
            return False


if __name__ == '__main__':
    main()
