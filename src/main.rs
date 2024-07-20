use std::sync::{Arc, Mutex};
use gtk::gio::builders::ListStoreBuilder;
use gtk::gio::{Cancellable, ListModel};
use gtk::{Label, ListItem, ListView, NoSelection, SignalListItemFactory, Stack, StringList, StringObject};
use gtk::{prelude::*, Button, FileDialog, FileFilter, gio::ListStore, Builder};
use gtk::{glib, glib::clone, Application, ApplicationWindow};

const APP_ID: &str = "me.mustafaeksi.uzanti-cevirmeni";

fn main() -> glib::ExitCode {
    // Create a new application
    let app = Application::builder().application_id(APP_ID).build();

    // Connect to "activate" signal of `app`
    app.connect_activate(build_ui);

    // Run the application
    app.run()
}

fn build_ui(app: &Application) {
    let button = Button::builder()
        .label("Open file")
        .halign(gtk::Align::Center)
        .valign(gtk::Align::Center)
        .build();
    let lm = ListStore::new::<FileFilter>();
    let ff = FileFilter::new();
    ff.add_suffix("png");
    lm.append(&ff);
    let cc = FileDialog::builder()
        .filters(&lm)
        .build();

    let main_stack = Stack::new();
    main_stack.add_child(&button);

    let factory = SignalListItemFactory::new();
    factory.connect_setup(|_, obj| {
        let lbl = Label::new(None);
        obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .set_child(Some(&lbl));
    });
    factory.connect_bind(|_, obj| {
        let pathobj = obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .item()
            .and_downcast::<StringObject>()
            .expect("The item has to be an `IntegerObject`.");
        let label = obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .child()
            .and_downcast::<Label>()
            .expect("The child has to be a `Label`.");
        label.set_text(&pathobj.string());
    });
    let modellist = Arc::new(StringList::new(&[]));
    let ms = Arc::new(main_stack.clone());
    let model = NoSelection::new(Some((*modellist).clone()));

    let file_list = ListView::builder()
        .factory(&factory)
        .model(&model)
        .build();

    main_stack.add_named(&file_list, Some("file_list"));

    // Create a window and set the title
    let window: ApplicationWindow = ApplicationWindow::builder()
        .application(app)
        .title("Uzanti Cevirmeni")
        .default_width(500)
        .default_height(600)
        .child(&main_stack)
        .build();
    let warc: Arc<ApplicationWindow> = Arc::new(window.clone());
    button.connect_clicked(move |button| {
        let w = warc.clone();
        let s = ms.clone();
        let sl = modellist.clone();
        button.set_label("Clicked!");
        let cancel = Cancellable::new();
        cc.open(Some(w.as_ref()), Some(&cancel), move |res| {
            match res {
                Ok(file) => {
                    println!("file: {}", file.path().unwrap().into_os_string().into_string().unwrap());
                    s.set_visible_child_name("file_list");
                    sl.append(&file.path().unwrap().into_os_string().into_string().unwrap());
                },
                Error => {
                    println!("Error!!!!!");
                }
            }
        })
    });

    // Present window
    window.present();
}
