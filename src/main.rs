use std::sync::{Arc, Mutex};
use gtk::gio::builders::ListStoreBuilder;
use gtk::gio::{Cancellable, ListModel};
use gtk::{Label, ListItem, ListView, NoSelection, SignalListItemFactory, Stack, StringList, StringObject, Box, gio::File};
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

    let main_stack = Arc::new(Stack::new());
    main_stack.add_child(&button);

    let modellist = Arc::new(StringList::new(&[]));
    let model = Arc::new(NoSelection::new(Some((*modellist).clone())));
    let factory = SignalListItemFactory::new();
    factory.connect_setup(move |_, obj| {
        let bx = Box::builder()
            .orientation(gtk::Orientation::Horizontal)
            .spacing(5)
            .build();
        let lbl = Label::new(None);
        bx.append(&lbl);
        let delbut = Button::builder()
            .label("-")
            .build();
        bx.append(&delbut);
        obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .set_child(Some(&bx));
    });

    factory.connect_bind(clone!(
        #[strong] modellist,
        move |_, obj| {
        let item = obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem");
        let item_index = item.position();
        let pathobj = item.item()
            .and_downcast::<StringObject>()
            .expect("The item has to be an `IntegerObject`.");
        let bx = obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .child()
            .and_downcast::<Box>()
            .expect("The child has to be a `Box`.");
        let label = bx.first_child()
            .expect("Can't get first child")
            .downcast::<Label>()
            .expect("Can't downcast to Label");
        label.set_text(&pathobj.string());

        let delbut = bx.last_child()
            .expect("Can't get last child")
            .downcast::<Button>()
            .expect("Can't downcast to Button");
        delbut.connect_clicked(clone!(
        #[strong] modellist,
        move |_| {
            modellist.remove(item_index);
        }));
    }));

    let file_list = ListView::builder()
        .factory(&factory)
        .model(model.as_ref())
        .build();

    main_stack.add_named(&file_list, Some("file_list"));

    // Create a window and set the title
    let window: ApplicationWindow = ApplicationWindow::builder()
        .application(app)
        .title("Uzanti Cevirmeni")
        .default_width(500)
        .default_height(600)
        .child(main_stack.as_ref())
        .build();
    let warc: Arc<ApplicationWindow> = Arc::new(window.clone());
    button.connect_clicked(move |button| {
        let w = warc.clone();
        let s = main_stack.clone();
        let sl = modellist.clone();
        button.set_label("Clicked!");
        let cancel = Cancellable::new();
        cc.open_multiple(Some(w.as_ref()), Some(&cancel), clone!(
            move |res| {
                let files = res.expect("Opening file dialog failed");
                let mut i = 0;
                while i < files.n_items() {
                    let b = files.item(i).expect("Couldn't iterate through files");
                    let file = b
                        .downcast_ref::<File>()
                        .expect("Couldn't downcast to File");
                    println!("file: {}", file.path().unwrap().into_os_string().into_string().unwrap());
                    s.set_visible_child_name("file_list");
                    sl.append(&file.path().unwrap().into_os_string().into_string().unwrap());
                    i += 1;
                }
            }
        ));
    });

    // Present window
    window.present();
}
