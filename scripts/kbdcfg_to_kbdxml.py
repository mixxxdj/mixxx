#!/usr/bin/python3

import os
import re
import ntpath
import configparser
from tkinter import *
from xml.dom import minidom
from tkinter import filedialog
from collections import defaultdict
from xml.etree import ElementTree as ET

# README
#
# 1. Change the following string to the absolute path of the directory
#    containing the *kbd.cfg files:
KBD_CFG_FILES_DIRECTORY = "/home/tomasito/Development/Mixxx/res/keyboard"
#
#    Note: Only files with a valid language language code as filename will be
#          parsed. For example: en_US.kbd.cfg, es_ES.kbd.cfg or de_CH.kbd.cfg
#
# 2. Change the following string to the absolute path of the directory to
#    you want to save the generated keyboard preset:
TARGET_DIRECTORY = "/home/tomasito/Development/Mixxx/scripts/"
#
# 3. Change the following string to the preset name. Name this whatever you
#    like:
PRESET_NAME = "Default-mapping"
#
#    Note: The preset name will be the name you will see when the preset is
#          listed in Mixxx under preferences -> controllers -> keyboard ->
#          presets.
#
#          The preset name will be also reflected in the filename. If your
#          preset name is "foo", the generated file will be:
#          {TARGET_DIRECTORY}/"foo.kbd.xml"
#
# 4. Run this file. (with PYTHON3 or higher!!)


def main():
    app = Gui(None)
    app.mainloop()


class KeyboardParser:
    NAME_VALIDATION_PATTERN = re.compile("^[a-z]{2}_[A-Z]{2,3}$")

    @staticmethod
    # Check if name has the correct format, described here: http://doc.qt.io/qt-4.8/qlocale.html#QLocale-2
    # Name has to start with a two letter ISO 639-1 language code ...
    # ... followed by an underscore ...
    # ... followed by a three letter uppercase ISO 3166 country code
    #
    # Note: This validation is not full-prove: qw_ZXC will pass.
    def validate_name(name):
        return KeyboardParser.NAME_VALIDATION_PATTERN.match(name)

    def __init__(self, legacy_folder=None, name="",
                 multilang=True, legacy_file=None, target_file=None,
                 mixxx_version="2.1.0+", author="kbdcfg_to_kbdxml.py"):

        self.legacy_folder = legacy_folder
        self.name = name
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

    def parse_single_config(self):
        legacy_file = self.legacy_file
        legacy_extension = self.legacy_extension

        file_name = ntpath.basename(legacy_file)
        base_name = file_name[:-len(legacy_extension)]

        if KeyboardParser.validate_name(base_name):
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
                if KeyboardParser.validate_name(base_name):
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
        root = ET.Element('MixxxKeyboardPreset')
        root.set('schemaVersion', '1')
        root.set('mixxxVersion', self.mixxx_version)
        elements['root'] = root

        # Create info block
        info = ET.SubElement(root, 'info')
        ET.SubElement(info, 'name').text = self.name
        ET.SubElement(info, 'author').text = self.author
        ET.SubElement(info, 'description').text = self.description
        elements['info'] = info

        # Create controller block
        elements['controller'] = ET.SubElement(root, 'controller')

        return elements

    def create_xml(self, parser):
        if not type(parser):
            raise TypeError("Expected configparser, but got a " + type(parser).__name__)

        # Retrieve base name of file (hence, language code)
        file_name = ntpath.basename(self.legacy_file)
        base_name = file_name[:-len(self.legacy_extension)]

        # Create empty preset file and store dictionary holding basic elements
        elements = self._create_empty_xml()
        controller = elements['controller']
        root = elements['root']

        groups = parser.sections()

        # Iterate over all groups specified in the legacy file
        for group in groups:
            group_element = ET.SubElement(controller, 'group')
            group_element.set('name', group)

            # Iterate over all controls
            for control in parser.items(group):
                stripped_control = control[0].split()
                action = stripped_control[0]
                try:
                    keyseq = stripped_control[1]
                except:
                    keyseq = ""

                # Create control element node inside of group block
                control_element = ET.SubElement(group_element, 'control')
                control_element.set('action', action)

                # Create keyseq element node inside of group block
                keyseq_element = ET.SubElement(control_element, 'keyseq')
                keyseq_element.set('lang', base_name)
                keyseq_element.text = keyseq

        return root

    def create_multi_lang_xml(self, mappings):
        if not type(mappings) is dict:
            raise TypeError("Expected dictionary, but got a " + type(mappings).__name__)

        elements = self._create_empty_xml()
        controller = elements['controller']
        root = elements['root']

        # Create keyboard layouts block
        keyboard_layouts = ET.SubElement(controller, 'keyboard-layouts')
        for layout in sorted(mappings.items()):
            ET.SubElement(keyboard_layouts, 'lang').text = layout[0]

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
                if group not in actions: actions[group] = set()

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
            group_element = ET.SubElement(controller, 'group')
            group_element.set('name', '[' + group + ']')
            print("\t" + '[' + group + ']')

            # Iterate over actions in this group
            for action in sorted(actions[group]):
                # Add control element node to current group node
                control_element = ET.SubElement(group_element, 'control')
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
                        this_action = stripped_action_in_this_group[0]
                        this_keyseq = None

                        try:
                            this_keyseq = stripped_action_in_this_group[1]
                        except:
                            this_keyseq = ""

                        if action == action_in_this_group[0].split()[0]:
                            keyseq_element = ET.SubElement(control_element, 'keyseq')
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
                    keyseq = keyseq_element.text
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

                    keyseq_element = ET.Element('keyseq')
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
        xmlstr = minidom.parseString(ET.tostring(root)).toprettyxml()
        with open(full_path, "w") as f:
            f.write(xmlstr)
        print("Saved successfully!")


class Gui(Tk):
    TITLE = "kbd.cfg to kbd.xml converter"

    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)
        self.wm_title(Gui.TITLE)

        Button(self,
               text="Convert single kbd.cfg file to single kbd.xml file",
               command=lambda: self.open_single_dialog()).pack()

        Button(self,
               text="Convert multiple kbd.cfg files to single multi-lang kbd.xml file",
               command=lambda: self.open_multi_dialog()).pack()

    @staticmethod
    def open_single_dialog():
        legacy_file = filedialog.askopenfilename(
            filetypes=(("Keyboard config files", ".kbd.cfg"), ("All Files", "*")),
            title="Choose a legacy kbd.cfg file"
        )
        target_file = filedialog.asksaveasfile(
            mode='w',
            defaultextension=".kbd.xml", title="Where to save the kbd.xml file").name
        KeyboardParser(multilang=False, legacy_file=legacy_file, target_file=target_file)

    @staticmethod
    def open_multi_dialog():
        legacy_folder = filedialog.askdirectory(
            title="Choose a folder containing kbd.cfg files")
        target_file = filedialog.asksaveasfile(
            mode='w',
            defaultextension=".kbd.xml", title="Where to save the kbd.xml file").name
        KeyboardParser(multilang=True, legacy_folder=legacy_folder, target_file=target_file, name=target_file)


main()
