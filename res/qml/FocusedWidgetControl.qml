import Mixxx 0.1 as Mixxx

Mixxx.ControlProxy {

    enum WidgetKind {
        None,
        Searchbar,
        Sidebar,
        LibraryView
    }

    group: "[Library]"
    key: "focused_widget"
}
