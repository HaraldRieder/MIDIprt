extern crate gtk;
extern crate glib;
extern crate gio;
use gtk::prelude::*;
use gio::prelude::*;
use gtk::{ButtonsType, DialogFlags, MessageType, MessageDialog, Window, Menu, MenuBar, MenuItem, Label};
use std::rc::Rc;
use std::cell::RefCell;
use std::collections::HashMap;
use std::env::args;

const MFP_TITLE: &str = "Mad Harry's MIDI File Printer";
const MFP_DEFAULT_PROFILE: &str = "MIDI_PRT.MI$";
const MFP_APPLICATION_ID: &str = "Rieder.Wolfgang.Harald.MIDIprt";

// TODO wieder weg
// make moving clones into closures more convenient
macro_rules! clone {
    (@param _) => ( _ );
    (@param $x:ident) => ( $x );
    ($($n:ident),+ => move || $body:expr) => (
        {
            $( let $n = $n.clone(); )+
            move || $body
        }
    );
    ($($n:ident),+ => move |$($p:tt),+| $body:expr) => (
        {
            $( let $n = $n.clone(); )+
            move |$(clone!(@param $p),)+| $body
        }
    );
}

// TODO wieder weg?
// upgrade weak reference or return
#[macro_export]
macro_rules! upgrade_weak {
    ($x:ident, $r:expr) => {{
        match $x.upgrade() {
            Some(o) => o,
            None => return $r,
        }
    }};
    ($x:ident) => {
        upgrade_weak!($x, ())
    };
}

// TODO MFP Fenster
fn create_sub_window(
    application: &gtk::Application,
    title: &str,
    main_window_entry: &gtk::Entry,
    id: usize,
    windows: &Rc<RefCell<HashMap<usize, glib::WeakRef<gtk::Window>>>>,
) {
    let window = gtk::Window::new(gtk::WindowType::Toplevel);

    application.add_window(&window);

    window.set_title(title);
    window.set_default_size(400, 200);

    window.connect_delete_event(clone!(windows => move |_, _| {
        windows.borrow_mut().remove(&id);
        Inhibit(false)
    }));

    let button = gtk::Button::new_with_label(&format!("Notify main window with id {}!", id));
    button.connect_clicked(clone!(main_window_entry => move |_| {
        // When the button is clicked, let's write it on the main window's entry!
        main_window_entry.get_buffer().set_text(&format!("sub window {} clicked", id));
    }));
    window.add(&button);

    window.show_all();
    // Once the new window has been created, we put it into our hashmap so we can update its
    // title when needed.
    windows.borrow_mut().insert(id, window.downgrade());
}

fn create_main_window(application: &gtk::Application) -> gtk::ApplicationWindow {
    let window = gtk::ApplicationWindow::new(application);
    window.set_title(MFP_TITLE);
    window.set_default_size(400, 200); // TODO from saved settings
    window.set_position(gtk::WindowPosition::Center); // TODO from saved settings
    window.show_all();
    window
}

// TODO kann wieder weg
fn generate_new_id(windows: &HashMap<usize, glib::WeakRef<gtk::Window>>) -> usize {
    let mut id = 0;
    // As long as the id is already there, we just continue to increment.
    while windows.get(&id).is_some() {
        id += 1;
    }
    id
}

fn build_ui(application: &gtk::Application) {
    let windows: Rc<RefCell<HashMap<usize, glib::WeakRef<gtk::Window>>>> =
        Rc::new(RefCell::new(HashMap::new()));
    let window = create_main_window(application);

    // Why not changing all sub-windows' title at once?
    let windows_title_entry = gtk::Entry::new();
    windows_title_entry.set_placeholder_text(Some("Update all sub-windows' title"));
    windows_title_entry.connect_changed(clone!(windows => move |windows_title_entry| {
        // When the entry's text is updated, we update the title of every sub windows.
        let text = windows_title_entry.get_buffer().get_text();
        for window in windows.borrow().values() {
            if let Some(w) = window.upgrade() {
                w.set_title(&text)
            }
        }
    }));

    let entry = gtk::Entry::new();
    entry.set_editable(false);
    entry.set_placeholder_text(Some("Events notification will be sent here"));

    // Now let's create a button to create a looooot of new windows!
    let button = gtk::Button::new_with_label("Create new window");
    let application_weak = application.downgrade();
    button.connect_clicked(clone!(windows_title_entry, entry => move |_| {
        let application = upgrade_weak!(application_weak);
        let new_id = generate_new_id(&windows.borrow());
        create_sub_window(&application,
                          &windows_title_entry.get_buffer().get_text(),
                          &entry,
                          new_id,
                          &windows);
    }));

    let v_box = gtk::Box::new(gtk::Orientation::Vertical, 10/*TODO spacing*/);
    let menubar = gtk::MenuBar::new();
    let file = MenuItem::new_with_label("File");
    let file_label = Label::new(Some("File")); // warum noch ein Label????
    let quit = MenuItem::new_with_label("Quit");
    let file_box = gtk::Box::new(gtk::Orientation::Horizontal, 0);
    //file_box.pack_start(&file_image, false, false, 0);
    file_box.pack_start(&file_label, true, true, 0);
    let file_item = MenuItem::new();
    file_item.add(&file_box);
    let menu = Menu::new();
    menu.append(&file_item);
    menubar.append(&file);


    let toolbar = gtk::Toolbar::new();
    let label = gtk::Label::new(Some("page:"));
    //toolbar.add(&label); // Gtk-CRITICAL **: 09:55:27.832: gtk_toolbar_insert: assertion 'GTK_IS_TOOL_ITEM (item)' failed

    //v_box.pack_start(&toolbar, false, false, 0);
    v_box.pack_start(&menubar, false, false, 0);
    window.add(&v_box);

    // Now we add a layout so we can put all widgets into it.
    let layout = gtk::Box::new(gtk::Orientation::Vertical, 5);
    layout.add(&windows_title_entry);
    layout.add(&button);
    layout.add(&entry);
//    window.add(&layout);

    window.set_focus(Some(&button));

    // Then we show everything.
    window.show_all();
}

fn main() {
    let application = gtk::Application::new(Some(MFP_APPLICATION_ID), Default::default())
        .expect("GTK initialization failed...");
    application.connect_activate(|app| {
        build_ui(app);
    });
    application.run(&args().collect::<Vec<_>>());
}