use std::borrow::BorrowMut;
use std::sync::{Arc, Mutex};
use std::cell::RefCell;
use gtk::gio::builders::ListStoreBuilder;
use gtk::gio::{Cancellable, ListModel};
use gtk::glib::{GString, SignalHandlerId};
use gtk::Window;
use gtk::{Label, ListItem, ListView, NoSelection, SignalListItemFactory, Stack, StringList, StringObject, Box, gio::File};
use gtk::{prelude::*, Button, FileDialog, FileFilter, gio::ListStore, Builder};
use gtk::{glib, glib::clone, Application, ApplicationWindow, glib::object::ObjectExt};

const APP_ID: &str = "me.mustafaeksi.uzanti-cevirmeni";

fn main() -> glib::ExitCode {
    // Create a new application
    let app = Application::builder().application_id(APP_ID).build();

    // Connect to "activate" signal of `app`
    app.connect_activate(build_ui);

    // Run the application
    app.run()
}

struct FileList {
    string_list: Arc<ListStore>,
    model: Arc<NoSelection>,
    factory: SignalListItemFactory,
    signals: Arc<Mutex<Vec<(ListItem, SignalHandlerId)>>>,
    listview: ListView,
}

impl FileList {
    fn new(lv: ListView) -> FileList {
        let mut fl = FileList {
            listview: lv,
            string_list: Arc::new(ListStore::new::<StringObject>()),
            model: Arc::new(NoSelection::new(None::<StringList>)),
            factory: SignalListItemFactory::new(),
            signals: Arc::new(Mutex::new(Vec::new())),
        };
        fl.model.set_model(Some(fl.string_list.as_ref()));
        fl.factory.connect_setup(move |_, obj| {
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

        fl.factory.connect_bind(clone!(
            #[strong(rename_to = sl)] fl.string_list,
            #[strong(rename_to = signals)] fl.signals,
            move |_, obj| {
            let item = obj
                .downcast_ref::<ListItem>()
                .expect("Needs to be ListItem");
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
            let sid = delbut.connect_clicked(clone!(
            #[strong] sl,
            #[strong] item,
            move |_| {
                let item_index = item.position();
                //println!("I'm deleting {item_index}!!!");
                sl.remove(item_index);
            }));
            let mut sig = signals.lock().unwrap();
            let index = item.position() as usize;
            if index >= sig.len() {
                sig.push((item.clone(), sid));
            } else {
                let bxo = sig[index].0
                    .downcast_ref::<ListItem>()
                    .expect("Needs to be ListItem")
                    .child()
                    .and_downcast::<Box>();
                if let Some(bx) = bxo { 
                    let delbut2 = bx.last_child()
                        .expect("Can't get last child")
                        .downcast::<Button>()
                        .expect("Can't downcast to Button");
                    delbut2.disconnect(sig.remove(index).1);
                }
                sig.insert(index, (item.clone(), sid));
            }
        }));

        fl.listview.set_factory(Some(&fl.factory));
        fl.listview.set_model(Some(fl.model.as_ref()));
        return fl;
    }

    fn add_file(&self, path: String) {
        println!("file: {path}");
        self.string_list.append(&StringObject::new(&path));
    }
}

fn build_ui(app: &Application) {
    let b = Builder::from_file("./ui.ui");
    let button = b.object::<Button>("open_file_button")
        .expect("Couldn't find button");

    let lm = ListStore::new::<FileFilter>();
    let ff = FileFilter::new();
    ff.add_suffix("png");
    lm.append(&ff);
    let cc = FileDialog::builder()
        .filters(&lm)
        .build();

    let main_stack = Arc::new(b.object::<Stack>("main_stack").expect("Stack not found"));

    let file_list = b.object::<ListView>("file_list").expect("file_list not found");
    let fl: Arc<FileList> = Arc::new(FileList::new(file_list));

    // Create a window and set the title
    let window = Arc::new(b.object::<Window>("main_window")
        .expect("Couldn't find window"));

    button.connect_clicked(clone!(
    #[strong] window,
    #[strong] main_stack,
    #[strong] fl,
    move |_| {
        let cancel = Cancellable::new();
        println!("Pressed button!");
        cc.open_multiple(Some(window.as_ref()), Some(&cancel), clone!(
            #[strong] main_stack,
            #[strong] fl,
            move |res| {
                let files = res.expect("Opening file dialog failed");
                let mut i = 0;
                while i < files.n_items() {
                    let b = files.item(i).expect("Couldn't iterate through files");
                    let file = b
                        .downcast_ref::<File>()
                        .expect("Couldn't downcast to File");
                    main_stack.set_visible_child_name("file_list");
                    fl.add_file(String::from(file.path().expect("Can't get path").to_str().unwrap()));
                    i += 1;
                }
            }
        ));
    }));

    // Present window
    window.set_application(Some(app));
    window.present();
}
