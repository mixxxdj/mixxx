use cxx_qt;
use cxx_qt_lib;

#[cxx_qt::bridge]
pub mod rekordbox {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "RustQt" {
        #[qobject]
        #[qml_element]
        #[qproperty(i32, number)]
        #[qproperty(QString, string)]
        type Rekordbox = super::RekordboxRust;
    }

    unsafe extern "RustQt" {
        // Declare the invocable methods we want to expose on the QObject
        #[qinvokable]
        fn increment_number(self: Pin<&mut Rekordbox>);
    }
}

use core::pin::Pin;
use cxx_qt_lib::QString;

/// The Rust struct for the QObject
#[derive(Default)]
pub struct RekordboxRust {
    number: i32,
    string: QString,
}

impl rekordbox::Rekordbox {
    /// Increment the number Q_PROPERTY
    pub fn increment_number(self: Pin<&mut Self>) {
        let previous = *self.number();
        self.set_number(previous + 1);
    }
}
