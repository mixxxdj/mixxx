import os.path
from tkinter import *
from tkinter import filedialog
from xml.etree import ElementTree


def main():
    app = KeyboardLayoutEditor()
    app.mainloop()


class KeyboardLayoutEditor(Tk):
    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)
        self.wm_title("Keyboard layouts editor")

        self.mainframe = MainFrame(self, app=self)
        self.sidebarframe = SideBarFrame(self.mainframe, app=self)
        self.workspaceframe = WorkspaceFrame(self.mainframe)
        self.dlgkeyboard = DlgKeyboard(self.workspaceframe, app=self)
        self.pack()

        # File name of current XML file
        self.file_name = None

        # Element tree of current XML file
        self.tree = None

        # List of instances of KeyboardLayout
        self.layouts = []

        # The layout that is currently selected and displayed
        self.selected_layout = None

    def pack(self):
        self.sidebarframe.pack(side=LEFT)
        self.workspaceframe.pack()
        self.mainframe.pack()
        self.dlgkeyboard.pack()

    def open_file(self):
        self.file_name = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a keyboard layout XML file"
        )

        if self.file_name == ():
            return

        if not os.path.isfile(self.file_name):
            print("File doesn't exist: " + self.file_name)
            self.file_name = None
            return

        print("Opening file: " + self.file_name)
        with open(self.file_name, 'rt') as f:
            self.tree = ElementTree.parse(f)

        # Get KeyboardLayoutTranslations element, which holds all
        # information and is basically the root element
        root = None
        for element in self.tree.getroot().iter():
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
            self.layouts.append(
                KeyboardLayout(name=name, root=root)
            )

        # Update layout list in side bar
        self.sidebarframe.update_layout_list(self.layouts)

    def select_layout(self, layout_name):
        for layout in self.layouts:
            if layout.name == layout_name:
                self.dlgkeyboard.update_keys(layout)
                self.selected_layout = layout
                self.dlgkeyboard.keyboard_layout = layout
                print("Selected layout: " + layout_name)
                return
        print("No such layout: " + layout_name)


class MainFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)

        # Setup main menu bar
        self.menubar = Menu(self)
        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="File", menu=menu)
        menu.add_command(label="New")
        menu.add_command(label="Open", command=app.open_file)
        self.master.config(menu=self.menubar)


class SideBarFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        Button(self, text="Add").pack()
        Button(self, text="Remove").pack()

        self.listbox = Listbox(self)
        self.listbox.pack()

        self.listbox.bind('<<ListboxSelect>>', self.on_listbox_item_selected)

    def update_layout_list(self, layouts):
        for layout in layouts:
            self.listbox.insert(END, layout.name)

        if self.listbox.size() > 0:
            self.listbox.select_set(0)
            self.listbox.event_generate("<<ListboxSelect>>")

    def on_listbox_item_selected(self, e):
        widget = e.widget
        index = int(widget.curselection()[0])
        layout_name = widget.get(index)
        self.app.select_layout(layout_name)


class KeyboardLayout:
    def __init__(self, name="", root=None):
        # Keyboard layout name
        # TODO(Tomasito) Validate layout name as described here: http://doc.qt.io/qt-4.8/qlocale.html#QLocale-2
        self.name = name

        # Dictionary containing layout data, where:
        #   key   = scan code (as described here http://www.barcodeman.com/altek/mule/kb102.gif)
        #   value = character bound to this key
        self._data = {}

        # Parse XML and store it into self.data
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
                    self._data[int(scancode)] = char.text

    def find(self, scancode):
        data = self._data
        if scancode in data:
            return self._data[scancode]
        else:
            return None

    def update_key(self, scancode, char):
        self._data[int(scancode)] = char



class WorkspaceFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)


class DlgKeyboard(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.keys = []
        self.app = app

        # Numeric keys
        row_1 = Frame(self)
        for i in range(1, 14):
            key = DlgKeyboardKey(row_1, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_1.grid(row=1, column=1, sticky='we')

        # Character keys (qwertyuiop[] on en_US)
        row_2 = Frame(self)
        DlgKeyboardKey(row_2, scancode=16, width=4, char="Tab", state=DISABLED)
        for i in range(17, 29):
            key = DlgKeyboardKey(row_2, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_2.grid(row=2, column=1, sticky='we')

        # Character keys (asdfghjkl;'\ on en_US)
        row_3 = Frame(self)
        DlgKeyboardKey(row_3, scancode=16, width=6, char="Caps Lock", state=DISABLED)
        for i in range(31, 43):
            key = DlgKeyboardKey(row_3, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_3.grid(row=3, column=1, sticky='we')

        # Character keys (\zxcvbnm,./ on en_US)
        row_4 = Frame(self)
        DlgKeyboardKey(row_4, scancode=16, char="Shift", state=DISABLED)
        for i in range(45, 56):
            key = DlgKeyboardKey(row_4, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_4.grid(row=4, column=1, sticky='we')

    def update_keys(self, keyboardlayout):
        keys = self.keys
        for key in keys:
            char = keyboardlayout.find(key.scancode)
            key.set_char(char)


class DlgKeyboardKey(Button):
    COLORS = {
        "background_color": "#434A55",
        "active_background_color": "#656D79",
        "background_color_key_set": "#8FC238",
        "active_background_color_key_set": "#A3D553",
        "disabled_color": "#CCD1DA"
    }

    def __init__(self, *args, scancode=None, width=2, char=None, dlg_keyboard=None, **kwargs):
        Button.__init__(self, *args, width=width, height=2, command=self.set_listening, **kwargs)
        self.set_char(char)
        self.pack(side=LEFT)
        if not scancode:
            print("Warning: no scancode given for this DlgKeyboardKey")
        self.scancode = scancode
        self.dlg_keyboard = dlg_keyboard
        self.bind("<KeyPress>", self.on_key_press)

    def set_char(self, char):
        if not char:
            char = ""
        self.config(text=char)
        self.update_button_colors(bool(char))

    def set_listening(self):
        self.focus_set()

    def update_button_colors(self, key_set):
        if self['state'] == DISABLED:
            self.configure(bg="#CCD1DA")
            return

        if key_set:
            self.configure(
                fg="black",
                activeforeground="black",
                bg=DlgKeyboardKey.COLORS["background_color_key_set"],
                activebackground=self.COLORS["active_background_color_key_set"]
            )
        else:
            self.configure(
                fg="white",
                activeforeground="white",
                bg=DlgKeyboardKey.COLORS["background_color"],
                activebackground=self.COLORS["active_background_color"]
            )

    def on_key_press(self, e):
        char = e.char

        if not char.strip():
            return

        # Check for DELETE key
        if e.keysym_num == 65535:
            self.set_char("")
        else:
            self.set_char(char)

        scancode = self.scancode
        if not scancode:
            print("This key was not assigned a scancode and the key: " +
                  char + "can not be saved to the current keyboard layout")
            self.set_char("")
            return

        app = self.dlg_keyboard.app
        layout = app.selected_layout
        if not layout:
            print("Layout is None, not setting key")
            self.set_char("")
            return

        set_successfully = layout.update_key(scancode, char)

        # If the keyboard layout was successfully updated, move focus to next key
        if set_successfully:
            self.tk_focusNext().focus_set()


main()
