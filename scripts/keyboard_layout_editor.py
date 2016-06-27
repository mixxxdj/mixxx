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

        # self.minsize(
        #     width=1000,
        #     height=270
        # )

        self.mainframe = MainFrame(self, app=self)
        self.sidebarframe = SideBarFrame(self.mainframe)
        self.workspaceframe = WorkspaceFrame(self.mainframe)
        self.dlgkeyboard = DlgKeyboard(self.workspaceframe)
        self.pack()

        # File name of current XML file
        self.file_name = None

        # Element tree of current XML file
        self.tree = None

    def pack(self):
        self.sidebarframe.pack()
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

        if os.path.isfile(self.file_name):
            print("Opening file: " + self.file_name)
            with open(self.file_name, 'rt') as f:
                self.tree = ElementTree.parse(f)
        else:
            print("File doesn't exist: " + self.file_name)
            self.file_name = None


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
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)


class WorkspaceFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)


class DlgKeyboard(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        # Scancodes based on http://www.barcodeman.com/altek/mule/kb102.gif

        # Numeric keys
        row_1 = Frame(self)
        for i in range(1, 14):
            DlgKeyboardKey(row_1, scancode=i)
        row_1.grid(row=1, column=1, sticky='we')

        # Character keys (qwertyuiop[] on en_US)
        row_2 = Frame(self)
        DlgKeyboardKey(row_2, scancode=16, width=4, char="Tab", state=DISABLED)
        for i in range(17, 29):
            DlgKeyboardKey(row_2, scancode=i)
        row_2.grid(row=2, column=1, sticky='we')

        # Character keys (asdfghjkl;'\ on en_US)
        row_3 = Frame(self)
        DlgKeyboardKey(row_3, scancode=16, width=6, char="Caps Lock", state=DISABLED)
        for i in range(31, 43):
            DlgKeyboardKey(row_3, scancode=i)
        row_3.grid(row=3, column=1, sticky='we')

        # Character keys (\zxcvbnm,./ on en_US)
        row_4 = Frame(self)
        DlgKeyboardKey(row_4, scancode=16, char="Shift", state=DISABLED)
        for i in range(45, 56):
            DlgKeyboardKey(row_4, scancode=i)
        row_4.grid(row=4, column=1, sticky='we')


class DlgKeyboardKey(Button):
    def __init__(self, *args, scancode=None, width=2, char=None, **kwargs):
        Button.__init__(self, *args, width=width, height=2, command=self.set_listening, **kwargs)
        self.config(text=char)
        self.pack(side=LEFT)

        if not scancode:
            print("Warning: no scancode given for this DlgKeyboardKey")

        self.scancode = scancode
        self.key_set = True if char else False

        self.colors = {
            "background_color": "#434A55",
            "active_background_color": "#656D79",
            "background_color_key_set": "#8FC238",
            "active_background_color_key_set": "#A3D553",
            "disabled_color": "#CCD1DA"
        }
        self.update_button_colors(self.key_set)
        self.bind("<KeyPress>", self.on_key_press)

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
                bg=self.colors["background_color_key_set"],
                activebackground=self.colors["active_background_color_key_set"]
            )
        else:
            self.configure(
                fg="white",
                activeforeground="white",
                bg=self.colors["background_color"],
                activebackground=self.colors["active_background_color"]
            )

    def on_key_press(self, e):
        if not e.char.strip():
            return

        # Check for DELETE key
        if e.keysym_num == 65535:
            self.configure(text="")
            self.key_set = False
        else:
            self.configure(text=e.char)
            self.key_set = True

        self.update_button_colors(self.key_set)
        self.tk_focusNext().focus_set()


main()
