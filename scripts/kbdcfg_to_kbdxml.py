#!/usr/bin/python3

import os
import re
import codecs
import ntpath
import configparser
from tkinter import *
from xml.dom import minidom
from tkinter import filedialog
from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring


def main():
    app = App()
    app.mainloop()


class App(Tk):
    """ Main class for kbdcfg_to_kbdxml, Tk root

    This is the main class of the KbdCfg_to_KbdXml converter tool Tk
    application. It is the view, being the main application window, but
    also the model, holding mapping data.

    Attributes:
        dlg_sidebar:      Sidebar widget at the left side of the application, showing loaded mappings
        dlg_mapping_info: Widget displaying info about selected mapping, at the right of dlg_sidebar
        dlg_save:         Widget which can be interacted with to choose a layouts resource file and a path to save to

        mappings:         Loaded mappings, represented as instances of the Mapping class
        selected_mapping: The currently selected mapping
        master_mapping:   The mapping on which the keyboard preset will be based on
    """

    TITLE = "Keyboard legacy files to keyboard XML converter"

    FONTS = {
        'huge': ("Helvetica", 18),
        'big': ("Helvetica", 14),
        'normal': ("Helvetica", 10),
        'small': ("Helvetica", 7)
    }

    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)

        self.wm_title(App.TITLE)

        # Mappings will be listed in the side bar
        self.dlg_sidebar = DlgSidebar(self, app=self)

        # The container contains the mapping info and the save-preset frame
        container = Frame()
        self.dlg_mapping_info = DlgMappingInfo(container, app=self)
        self.dlg_save = DlgSave(container, app=self)

        # Pack everything up
        self.dlg_sidebar.pack(side=LEFT, pady=0, fill=Y, expand=1)
        self.dlg_save.pack(side=RIGHT, fill=BOTH, expand=1, pady=5, padx=5)
        self.dlg_mapping_info.pack(pady=5, padx=5)
        container.pack(side=RIGHT, fill=BOTH, expand=1)

        # Loaded mappings (Mapping objects)
        self.mappings = []
        self.selected_mapping = None
        self.master_mapping = None

    def add_mapping(self, mapping):
        """Add a new mapping to mappings

        Add a new mapping to mappings if necessary. If there is already a
        mapping loaded with the same path, don't add it.

        :param mapping: Mapping to be added
        :return: True if mapping was added successfully, False if not.
        """

        path = mapping.path
        for i_mapping in self.mappings:
            if i_mapping.path == path:
                print("Not adding mapping. There is already a "
                      "loaded mapping with path: " + i_mapping.path)
                return False

        self.mappings.append(mapping)

        self.dlg_sidebar.update()
        self.dlg_mapping_info.update()
        self.dlg_save.update()
        return True

    def remove_mapping(self, mapping):
        """ Remove mapping / unload mapping from mappings

        :param mapping: Mapping to be removed.
        :return:
        """

        if type(mapping).__name__ != 'Mapping':
            raise TypeError("Can't remove mapping. Expected "
                            "type: Mapping. Got: " + type(mapping).__name__)

        self.mappings.remove(mapping)
        if mapping == self.master_mapping:
            self.master_mapping = None

        # If all mappings were removed, set selected mapping to None
        if len(self.mappings) == 0:
            self.selected_mapping = None

        self.dlg_sidebar.update()
        self.dlg_mapping_info.update()
        self.dlg_save.update()

    def select_mapping(self, index):
        """ Select a mapping at given index

        :param index: Mapping's index
        :return:
        """
        index = int(index)

        if index > len(self.mappings):
            raise IndexError("Can't select mapping, given index is not valid: " + str(index) +
                             ". Only " + str(len(self.mappings)) + " mappings loaded.")

        self.selected_mapping = self.mappings[index]

        self.dlg_sidebar.update()
        self.dlg_mapping_info.update()
        self.dlg_save.update()

    def set_master_mapping(self, mapping):
        if mapping not in self.mappings:
            return False
        self.master_mapping = mapping
        self.dlg_sidebar.update()
        self.dlg_mapping_info.update()

    def get_mapping_names(self):
        """
        :return: List of the names of loaded mappings
        """
        names = []
        for mapping in self.mappings:
            names.append(mapping.name)
        return names

    def save_preset(self, path, layouts_path, preset_name):
        """ Save keyboard controller preset

        :param path: Path to which the preset should be saved
        :param layouts_path: Layouts resource file paths
        :param preset_name: This is the name that Mixxx will display under preferences
        :return:
        """

        layouts = self._load_layouts(layouts_path)
        root = self.create_xml(preset_name, layouts)

        if not root:
            print("XML was not created, not saving preset...")
            return

        if os.path.isfile(path):
            print("Warning: '" + path + "' already exists.\nOverriding that file assuming that it's ok :)\n")
        print("Writing to file: " + path + "\n...")
        xmlstr = minidom.parseString(tostring(root)).toprettyxml()
        with codecs.open(path, 'w', "utf-8") as f:
            f.write(xmlstr)
        print("Saved successfully!")

    def _load_layouts(self, path):
        """ Read layouts resource file and load it in

        Extract each layout in layouts resource file and create for
        each one a KeyboardLayout object.

        :param path: Layouts resource file path
        :return: List of KeyboardLayout objects
        """

        layouts = []

        if not os.path.isfile(path):
            print("File doesn't exist: " + path)
            self.file_path = None
            return None

        print("Opening layouts file: " + path)
        with codecs.open(path, 'r', "utf-16") as f:
            self.tree = ElementTree.parse(f)

        # Get KeyboardLayoutTranslations element, which holds all
        # information and is basically the root element
        root = None
        for element in self.tree.getroot().iter():
            if element.tag == 'KeyboardLayoutTranslations':
                root = element
                break
        self.root = root

        # Retrieve layouts element
        layouts_element = root.find('layouts')

        if not layouts_element:
            print("Couldn't retrieve layout list, no <layouts> element "
                  "in loaded XML. Pleas load in an other XML file.")
            return None

        # Iterate over languages specified in layouts file
        for layout in layouts_element.iter('lang'):
            name = layout.text
            if not KeyboardLayout.validate_layout_name(name):
                print("Layout name: " + name + " is not a valid language code name. Not loading this layout.")
                continue
            layouts.append(
                KeyboardLayout(name=name, root=root)
            )

        return layouts

    def create_xml(self, preset_name, layouts, mixxx_version="2.0.1+"):
        """ Create keyboard controller preset XML based on loaded mappings

        Construct an XML document following the format for KeyboardControllerPreset. Add mapping information
        for master mapping layout and if necessary, overload key sequences for specific languages / keyboard
        layouts. That is, when the translation of a key sequence from the master mapping's layout to another
        layout doesn't match the key described in the legacy mapping file for that other layout.

        Example where master mapping's layout is en_US:

        <group name='[Master]'>
          <control action='crossfader_up'>
            <keyseq lang='en_US' scancode='36'>h</keyseq>
          </control>

          <control action='crossfader_down'>
            <keyseq lang='en_US' scancode='35'>g</keyseq>
          </control>
        </group>

        <group name='[Microphone]'>
          <control action='talkover'>
            <keyseq lang='en_US' scancode='1'>`</keyseq>

            <!-- In de_DE layout, '`' is a dead key, so it has to be overloaded -->
            <keyseq lang='de_DE' scancode='45'><</keyseq>
          </control>
        </group>

        :param preset_name: Preset name, will be visible in Mixxx -> preferences
        :param layouts: List of KeyboardLayouts objects
        :param mixxx_version: Minimal mixxx version needed for this controller preset
        :return: XML root element
        """

        mappings = self.mappings
        master_mapping = self.master_mapping

        # Check if for each mapping, there is a matching layout
        for mapping in mappings:
            mapping_locale_name = mapping.get_locale_name()
            layout_found_for_mapping = False
            for layout in layouts:
                if layout.name == mapping_locale_name:
                    layout_found_for_mapping = True
                    break
            if not layout_found_for_mapping:
                print("No layout found for '" + mapping.name +
                      "', with locale name: '" + mapping_locale_name +
                      ". Please choose another layouts file.")
                return None

        # Check if master mapping is specified
        if not master_mapping:
            print("Master mapping not specified. Please "
                  "mark one of the mappings as master mapping")
            return None

        # Retrieve layout for master mapping
        master_mapping_layout = None
        for layout in layouts:
            if layout.name == master_mapping.get_locale_name():
                master_mapping_layout = layout

        # Technically this is not possible, otherwise it would
        # have returned at the "check if for each mapping, there
        # is a matching layout" check
        assert master_mapping_layout is not None

        # A dictionary that will store the controls of master_mapping, but
        # instead of having a flat hierarchy, stored in a dictionary where:
        #     - key:    group-name
        #     - value:  list containing controls belonging to group
        master_mapping_data = {}
        for group_name in master_mapping.groups:
            controls = master_mapping.find_controls_by_group(group_name)
            master_mapping_data[group_name] = controls

        # Create XML root element
        root_element = Element('MixxxKeyboardPreset')
        root_element.set('schemaVersion', '1')
        root_element.set('mixxxVersion', mixxx_version)

        # Create info block
        info_element = SubElement(root_element, 'info')
        SubElement(info_element, 'name').text = preset_name
        SubElement(info_element, 'author').text = "KbdCfg_to_KbdXML converter"
        SubElement(info_element, 'description').text = \
            "This preset was generated based on: " + str(self.get_mapping_names())

        # Create controller block
        controller_element = SubElement(root_element, 'controller')

        # Helper function that tells whether a given string is
        # in an array or not. The check is case-insensitive
        def string_in_array_case_insensitive(string, array):
            array_uppercase = [x.upper() for x in array]
            return string.upper() in array_uppercase

        # Fill controller_element with <group> elements, fill those with
        # <control> elements, which will contain one or more <keyseq> elements
        #
        # But first, iterate over groups
        for group_name, controls in master_mapping_data.items():
            group_element = SubElement(controller_element, 'group')
            group_element.set('name', group_name)

            # Iterate over controls in this group
            for master_control in controls:
                control_element = SubElement(group_element, 'control')
                control_element.set('action', master_control.action)

                # Retrieve master control key info
                master_control_keyseq = master_control.keysequence
                master_control_split_keyseq = KeyboardKey.split_keysequence(master_control_keyseq)
                master_control_mods = list(master_control_split_keyseq)
                master_control_mods.pop()
                if string_in_array_case_insensitive("Shift", master_control_mods):
                    master_control_mod_int = KeyboardKey.MODIFIERS.SHIFT
                else:
                    master_control_mod_int = KeyboardKey.MODIFIERS.NONE
                master_control_scancode = master_mapping_layout.get_scancode(
                    master_control_keyseq,
                    master_control_mod_int
                )

                # Create <keyseq> element for master control key
                master_keyseq_element = SubElement(control_element, 'keyseq')
                master_keyseq_element.set('lang', master_mapping.get_locale_name())
                master_keyseq_element.set('scancode', str(master_control_scancode))
                master_keyseq_element.text = master_control_keyseq

                # Check if there is a control with the same group and action
                # in other mappings. If one is found, iterate over given layouts
                # and translate master key to current layout. Then compare
                # scancode and modifiers with master key info. If they are the
                # same, that means that the key sequence can be deduced and
                # doesn't have to be explicitly set for the correspondent layout
                for mapping in mappings:

                    # We don't need to check control in master mapping, because
                    # a keyseq element is already added for that one
                    if mapping == master_mapping:
                        continue

                    reference_control = mapping.find_control(
                        master_control.group,
                        master_control.action
                    )

                    # If no control is found in mapping with same group
                    # and action, it's pretty safe to say that we don't
                    # need an extra <keyseq> element :)
                    if not reference_control:
                        continue

                    # Retrieve layout for current mapping
                    reference_mapping_layout = None
                    for i_layout in layouts:
                        if i_layout.name == mapping.get_locale_name():
                            reference_mapping_layout = i_layout
                            break

                    # This is impossible. Otherwise it would already have
                    # returned at the beginning of create_xml
                    assert reference_mapping_layout is not None

                    reference_control_keyseq = reference_control.keysequence
                    reference_control_split_keyseq = KeyboardKey.split_keysequence(reference_control_keyseq)
                    reference_control_mods = list(reference_control_split_keyseq)
                    reference_control_mods.pop()
                    if string_in_array_case_insensitive("SHIFT", reference_control_mods):
                        reference_control_mod_int = KeyboardKey.MODIFIERS.SHIFT
                    else:
                        reference_control_mod_int = KeyboardKey.MODIFIERS.NONE
                    reference_control_scancode = reference_mapping_layout.get_scancode(
                        reference_control_keyseq,
                        reference_control_mod_int
                    )

                    # Check if reference modifiers are the same as master's.
                    # We check with a set() so that it's unordered.
                    # This way we make sure that Ctrl+Shift and Shift+Ctrl
                    # both return true
                    # TODO(Tomasito) Do we still need this?
                    # mods_are_equal = \
                    #     set([x.upper() for x in reference_control_mods]) == \
                    #     set([x.upper() for x in master_control_mods])

                    # Check if reference scancode is the same as master scancode
                    scancodes_are_equal = reference_control_scancode == master_control_scancode

                    # If translated master control wouldn't match reference keysequence
                    if not scancodes_are_equal:
                        overloaded_keyseq_element = SubElement(control_element, 'keyseq')
                        overloaded_keyseq_element.set('lang', mapping.get_locale_name())
                        overloaded_keyseq_element.set('scancode', str(reference_control_scancode))
                        overloaded_keyseq_element.text = reference_control_keyseq

        return root_element


class DlgMappingInfo(Frame):
    """ Widget that displays information about selected mapping

    This widget shows information about the current selected mapping and provides
    a knob to the user to mark the mapping as master mapping.
    """

    def __init__(self, *args, app, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        self.grid_columnconfigure(0, pad=30)

        self.title = Label(self, text="[title]", font=App.FONTS['huge'])
        self.path = Label(self, text="[path]", font=App.FONTS['small'])
        self.language = Label(self, text="[lang]", font=App.FONTS['normal'])
        self.country = Label(self, text="[country]", font=App.FONTS['normal'])
        self.groups = Label(self, text="[groups]", font=App.FONTS['normal'])
        self.controls = Label(self, text="[controls]", font=App.FONTS['normal'])
        self.status = Label(self, text="[status]", font=App.FONTS['normal'])
        self.master_button = Button(self, command=self._make_master_command,
                                    text="Make master", font=App.FONTS['normal'])

        # Pack everything up
        self.title.grid(row=0, column=0, sticky='w')
        self.path.grid(row=1, column=0, sticky='w', pady=(0, 14))

        Label(self, text="Language code (ISO 639)", font=App.FONTS['normal']).grid(row=2, column=0, sticky='w')
        self.language.grid(row=2, column=1, sticky='w')

        Label(self, text="Country code (ISO 3166)", font=App.FONTS['normal']).grid(row=3, column=0, sticky='w')
        self.country.grid(row=3, column=1, sticky='w')

        Label(self, text="Groups", font=App.FONTS['normal']).grid(row=4, column=0, sticky='w')
        self.groups.grid(row=4, column=1, sticky='w')

        Label(self, text="Controls", font=App.FONTS['normal']).grid(row=5, column=0, sticky='w')
        self.controls.grid(row=5, column=1, sticky='w')

        self.status.grid(row=6, column=0, sticky='w')
        self.master_button.grid(row=6, column=1, sticky='w')

        self.reset()

    def update(self):
        """
        Update mapping information based on current selected mapping in App
        """

        mapping = self.app.selected_mapping
        if not mapping:
            self.reset()
            return

        name = mapping.name
        self.title.configure(text=name)
        self.path.configure(text=mapping.path)

        lang_code = Mapping.get_lang_from_name(name)
        country_code = Mapping.get_country_from_name(name)
        self.language.configure(text=lang_code)
        self.country.configure(text=country_code)

        n_groups = len(mapping.groups)
        n_controls = len(mapping.controls)
        self.groups.configure(text=str(n_groups))
        self.controls.configure(text=str(n_controls))

        master = self.app.master_mapping
        is_master = mapping == master
        if is_master:
            state = "This mapping is master."
            self.master_button.configure(state=DISABLED)
        else:
            self.master_button.configure(state=ACTIVE)
            self.master_button.configure(state=ACTIVE)
            if not master:
                state = "There is no master mapping set."
            else:
                state = "This mapping complements " + master.name
        self.status.configure(text=state)

    def reset(self):
        self.title.configure(text="-")
        self.path.configure(text="-")
        self.language.configure(text="-")
        self.country.configure(text="-")
        self.groups.configure(text="-")
        self.controls.configure(text="-")
        self.status.configure(text="-")
        self.master_button.configure(state=DISABLED)

    def _make_master_command(self):
        set_successfully = self.app.set_master_mapping(
            self.app.selected_mapping
        )
        if not set_successfully:
            return


class DlgSave(Frame):
    """
    Save keyboard controller preset widget. The user can browse for a layouts resource
    file, select a path to save the kbd.xml file to and set a preset name
    """

    def __init__(self, *args, app, **kwargs):
        Frame.__init__(self, *args, bd=4, relief=RAISED, **kwargs)
        self.app = app

        body = Frame(self)
        Grid.columnconfigure(body, 0, weight=1)
        Grid.columnconfigure(body, 1, weight=1, pad=10)

        Label(self, text='Export', font=App.FONTS['big']).pack(pady=10, padx=10)

        for x in range(0, 4):
            Grid.rowconfigure(body, x, pad=5)

        Label(body, text="Layouts").grid(row=0, column=0, sticky='w')
        self.layouts_path = StringVar()
        Entry(body, textvariable=self.layouts_path).grid(row=0, column=1, sticky='w')
        Button(body, text="Browse", command=self._browse_layouts_file_command).grid(row=0, column=2, sticky='e')

        Label(body, text="Save preset to").grid(row=1, column=0, sticky='w')
        self.save_preset_to_path = StringVar()
        Entry(body, textvariable=self.save_preset_to_path).grid(row=1, column=1, sticky='w')
        Button(body, text="Browse", command=self._browse_saveto_command).grid(row=1, column=2, sticky='e')

        Label(body, text="Preset name").grid(row=2, column=0, sticky='w')
        self.preset_name = StringVar()
        Entry(body, textvariable=self.preset_name).grid(row=2, column=1, sticky='w')

        body.pack(padx=6)

        self.start_button = Button(self, text="Start", state=DISABLED, command=self._start_button_command)
        self.start_button.pack(side=BOTTOM, pady=10, padx=10, fill=X)

    def update(self):
        if len(self.app.mappings) < 1:
            self.start_button.configure(state=DISABLED)
        else:
            self.start_button.configure(state=ACTIVE)

    def _browse_layouts_file_command(self):
        path = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a layouts xml file"
        )
        if not path:
            return None
        self.layouts_path.set(path)

    def _browse_saveto_command(self):
        try:
            path = filedialog.asksaveasfilename(
                defaultextension=".kbd.xml",
                title="Where to save the kbd.xml file")
        except AttributeError:
            return
        self.save_preset_to_path.set(path)

    def _start_button_command(self):
        self.app.save_preset(
            self.save_preset_to_path.get(),
            self.layouts_path.get(),
            self.preset_name.get()
        )


class DlgSidebar(Frame):
    """
    Sidebar widget, that shows a list of loaded mappings which can be clicked on to
    select a mapping. Also it provides two buttons, one for adding a new mapping, which
    will popup a filedialog to search a *.kbd.cfg file and one for removing a mappoing,
    that will remove the currently selected mapping.
    """

    def __init__(self, *args, app, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        buttons = Frame(self)
        Button(buttons, command=self._add_command, text="Add").pack(side=LEFT, fill=X, expand=1)
        self.remove_button = Button(buttons, command=self._remove_command, text="Remove", state=DISABLED)
        self.remove_button.pack(side=LEFT, fill=X, expand=1)
        buttons.pack(fill=BOTH)

        self.listbox = Listbox(self)
        self.listbox.pack(fill=BOTH, expand=1)

        self.listbox.bind('<<ListboxSelect>>', self._on_item_selected)

    def update(self):
        """
        Update items in list based on loaded mappings in App
        """

        selected_mapping = self.app.selected_mapping
        previous_selection_index = int(self.listbox.curselection()[0]) if selected_mapping else -1

        self.reset()
        for mapping in self.app.mappings:
            name = mapping.name
            if mapping == self.app.master_mapping:
                name += ' *'
            self.listbox.insert(END, name)

        # If none of the mappings is selected, disable remove button
        if selected_mapping:
            self.remove_button.configure(state=ACTIVE)
        else:
            self.remove_button.configure(state=DISABLED)

        # Check if there is a selected mapping in the base app. If so, make sure it's
        # highlighted here in the listbox. If there is no mapping selected yet, select
        # the first one of the item and generate <<ListboxSelect>> event so that app
        # can deal with the other views.
        selected_mapping_still_exists = selected_mapping in self.app.mappings

        if selected_mapping and selected_mapping_still_exists:
            i = self.app.mappings.index(selected_mapping)
        else:
            n_mappings = self.listbox.size()
            previous_selection_index_is_valid = 0 <= previous_selection_index < n_mappings

            if previous_selection_index_is_valid:
                i = previous_selection_index
            else:
                i = self.listbox.size() - 1 if previous_selection_index == self.listbox.size() else 0

        self.listbox.select_set(i)
        self.listbox.activate(i)
        if not selected_mapping or not selected_mapping_still_exists:
            self.listbox.event_generate("<<ListboxSelect>>")

    def reset(self):
        self.listbox.delete(0, END)

    def _add_command(self):
        paths_string = filedialog.askopenfilenames(
            filetypes=(("Keyboard config files", "kbd.cfg"), ("All Files", "*")),
            title="Add a legacy mapping file"
        )

        # Paths_string is just one long string. Split them up and make a list
        paths = self.tk.splitlist(paths_string)

        for path in paths:
            if not path:
                continue

            if not os.path.isfile(path):
                print("File doesn't exist: " + path)
                continue

            self.app.add_mapping(Mapping(path))

    def _remove_command(self):
        self.app.remove_mapping(
            self.app.selected_mapping
        )

    def _on_item_selected(self, event):
        del event
        self.listbox.focus_set()
        try:
            index = int(self.listbox.curselection()[0])
        except IndexError:
            return
        self.app.select_mapping(index)


class Mapping:
    """ Contains mapping info for specific language

    There is one Mapping object per legacy keyboard mapping file (*.kbd.cfg file)
    """

    LEGACY_EXTENSION = ".kbd.cfg"
    NAME_VALIDATION_PATTERN = re.compile("^[a-z]{2}_[A-Z]{2,3}$")

    @staticmethod
    # Check if name has the correct format, described here: http://doc.qt.io/qt-4.8/qlocale.html#QLocale-2
    # Name has to start with a two letter ISO 639-1 language code ...
    # ... followed by an underscore ...
    # ... followed by a three letter uppercase ISO 3166 country code
    #
    # Note: This validation is not full-prove: qw_ZXC will pass.
    def validate_name(name):
        return Mapping.NAME_VALIDATION_PATTERN.match(name)

    # Get filename without path and extension. If given path has not the
    # correct legacy extension (.kbd.xfg), returns an empty string
    @staticmethod
    def get_file_name_from_path(path):
        if not path.endswith(Mapping.LEGACY_EXTENSION):
            print("Given file has not a kbd.cfg extension: " + path)
            return ""

        file_name = ntpath.basename(path)
        return file_name[:-len(Mapping.LEGACY_EXTENSION)]

    @staticmethod
    def get_lang_from_name(name):
        return name.split('_')[0]

    @staticmethod
    def get_country_from_name(name):
        return name.split('_')[1]

    @staticmethod
    def validate_group_name(name):
        return name.startswith('[') and name.endswith(']')

    def __init__(self, path):
        self.path = ""
        self.name = ""
        self.lang = ""
        self.country = ""

        # List containing all controls, instances of the Control class
        self.controls = []

        # List containing names of groups in this mapping
        self.groups = []
        self.load(path)

    def load(self, path):
        """ Load legacy keyboard mapping file

        Parse legacy file and load controls into memory.
        :param path: Path to *.kbd.cfg file
        """

        parser = configparser.ConfigParser(allow_no_value=True, delimiters=' ')
        parser.optionxform = str
        parser.read_file(codecs.open(path, "r", "utf8"))

        # Set name if valid. If not, return
        name = Mapping.get_file_name_from_path(path)
        if not Mapping.validate_name(name):
            print("Name is not valid: " + name + ", abort loading...")
            return False
        self.name = name

        # Set country and language
        self.lang = Mapping.get_lang_from_name(self.name)
        self.country = Mapping.get_country_from_name(self.name)

        # [ and ] are stripped out by ConfigParser, re-add them
        self.groups = ['[{0}]'.format(x) for x in parser.sections()]

        for section in parser.sections():
            group = '[{0}]'.format(section)

            for action, keyseq in parser.items(section):
                self.controls.append(Control(
                    group=group,
                    action=action,
                    keysequence=keyseq
                ))

        # Set path only if loading went well
        self.path = path

    def find_control(self, group, action):
        if not Mapping.validate_group_name(group):
            print("Given group name is not valid: " + group)
            return None

        found_controls = []
        for control in self.controls:
            if control.action == action and control.group == group:
                found_controls.append(control)

        n_found_controls = len(found_controls)

        if n_found_controls == 1:
            return found_controls[0]
        elif n_found_controls < 1:
            print("Control not found with group: " + group + " and action '" + action + "'")
            return None
        else:
            print("Multiple controls (" + str(n_found_controls) + ") + found with group: " +
                  group + " and action '" + action + "'. Returning last one...")
            return found_controls[-1]

    # Returns set of controls that belong to a given group
    def find_controls_by_group(self, group):
        controls = set()
        for control in self.controls:
            if control.group == group:
                controls.add(control)
        return controls

    def get_locale_name(self):
        return self.lang + "_" + self.country


class Control:
    def __init__(self, group='', action='', keysequence=''):
        self.group = group
        self.action = action
        self.keysequence = keysequence


class KeyboardKey:
    """
    Class representing one physical character key as seen here:
    https://en.wikipedia.org/wiki/Keyboard_layout#/media/File:ISO_keyboard_(105)_QWERTY_UK.svg

    This class contains information about which character is bound
    to this keyboard key, also for different modifiers

    Attributes:
        scancode: positional code of key, that is the same for each keyboard layout
        _key_chars: characters bound to this key for different keyboard layouts
    """

    # Create regular expression to split key sequence on each
    # plus sign without splitting on the last plus sign.
    # So that the last plus sign in "Shift++" is not lost
    KEYSEQUENCE_SPLIT_PATTERN = re.compile("\+(?!$)")

    # Split given key sequence and returns list, split on
    # KEYSEQUENCE_SPLIT_PATTERN regular expression
    @staticmethod
    def split_keysequence(keyseq):
        return KeyboardKey.KEYSEQUENCE_SPLIT_PATTERN.split(keyseq)

    class MODIFIERS:
        """ Modifier enum, following the Qt::KeyboardModifier standard, found here:
        http://doc.qt.io/qt-5/qt.html#KeyboardModifier-enum """
        NONE, SHIFT, CTRL, ALT, META, KEYPAD = range(1, 7)

        @staticmethod
        def is_valid_modifier(modifier):
            return modifier in range(1, 7)

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

    def get_key_char(self, modifier, verbose=True):
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
        key_char = self.get_key_char(modifier=modifier, verbose=False)

        if key_char:
            key_char.char = char
        else:
            self._key_chars.append(KeyboardKey.KeyChar(
                modifier=modifier,
                char=char
            ))


class KeyboardLayout:
    """
    This class represent a keyboard layout, for example: en_US. It holds info about which
    characters are bound to specific keys, with or without modifiers (Shift). For each layout
    in the layouts resource file, there will be one KeyboardLayout object.

    Attributes:
        name: Keyboard layout name
        _data: Mapping data for this keyboard layout, contains instances of KeyboardKey
    """

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
        self.name = name
        self._data = []

        # Parse XML and store it into self._data
        self.parse_xml(root)

    def parse_xml(self, root):
        """
        Read XML and load in all character information whose language code is
        the same as the name of this KeyboardLayout

        :param root: Layouts resource file XML root
        """

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
                        char=char.text
                    )

    def find(self, scancode):
        for key in self._data:
            if key.scancode == scancode:
                return key
        return None

    def update_key(self, scancode, modifier, char, verbose=False):
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
        split_keyseq = KeyboardKey.split_keysequence(keyseq)
        char = split_keyseq[-1] if split_keyseq else ""

        # Make sure that the character is a lower-case character if shift is not pressed and
        # is an upper-case character when shift is pressed so that it can be found in _data
        char = char.upper() if modifier == KeyboardKey.MODIFIERS.SHIFT else char.lower()

        scancodes = []
        for key in self._data:
            key_char = key.get_key_char(modifier)
            if key_char is None:
                scancode = "There was no key-char found for keyseq '" + keyseq + "' with modifier '" + str(modifier)
                continue
            if key_char.char == char:
                scancodes.append(key.scancode)

        scancodes_found = len(scancodes)
        if scancodes_found == 1:
            scancode = scancodes[0]
        elif scancodes_found > 1:
            scancode = "TODO: Set scancode for " + char + "' for layout: '" + self.name + "'" + \
                       " (please choose between one of these: " + str(scancodes) + ")"
        elif scancodes_found == 0:
            scancode = "TODO: No scancode found in " + self.name + " for character: '" + char + "'"
        else:
            scancode = "TODO: Set scancode for '" + char + "' for layout: '" + self.name + "'"

        # Check if this key is universal (for example: F-keys or Space)
        if KeyboardLayout.is_universal_key(char):
            scancode = "universal_key"

        return scancode

    @staticmethod
    def is_universal_key(key):
        """
        Check if given key is universal or not. Universal
        keys are keys that are the same for each KeyboardLayout.

        :param key: Key to be checked for universality
        :return: True if it is universal, False if it's not
        """
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
