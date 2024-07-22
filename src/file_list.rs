use std::sync::{Arc, Mutex};
use gtk::{
    prelude::*, Button, gio::ListStore,
    Label, ListItem, ListView, NoSelection,
    SignalListItemFactory, Stack, StringList,
    StringObject, Box
};
use gtk::glib::{clone, object::ObjectExt, SignalHandlerId};

pub struct FileList {
    pub string_list: Arc<ListStore>,
    model: Arc<NoSelection>,
    factory: SignalListItemFactory,
    signals: Arc<Mutex<Vec<(ListItem, SignalHandlerId)>>>,
    listview: ListView,
}

impl FileList {
    pub fn new(lv: ListView, main_stack: Arc<Stack>) -> FileList {
        let fl = FileList {
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
                .hexpand(true)
                .build();
            let lbl = Label::new(None);
            bx.append(&lbl);

            let delbut = Button::builder()
                .label("-")
                .halign(gtk::Align::End)
                .hexpand(true)
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
            let bx = item
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
            #[strong] main_stack,
            #[strong(rename_to = sigs)] signals,
            move |_| {
                let item_index = item.position();
                sl.remove(item_index);
                if sl.n_items() == 0 {
                    main_stack.set_visible_child_name("open_file_button");
                    sigs.lock().unwrap().clear();
                }
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

    pub fn add_file(&self, path: String) {
        self.string_list.append(&StringObject::new(&path));
    }
}
