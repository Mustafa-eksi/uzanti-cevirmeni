use std::sync::{Arc, Mutex};
use gtk::gio::{Cancellable, File};
use gtk::{NoSelection, SignalListItemFactory, StringList, StringObject};
use gtk::{
    prelude::*, Button, FileDialog, FileFilter, gio::ListStore, Builder,
    Application, DropDown, Window, ListView, Stack, Box, Label, ListItem
};
use gtk::glib::{clone, ExitCode};

mod file_list;
use file_list::FileList;

mod convert;
use convert::{get_converter_program, ConverterProgram, IMAGEMAGICK_EXTS};

const APP_ID: &str = "me.mustafaeksi.uzanti-cevirmeni";

fn main() -> ExitCode {
    // Create a new application
    let app = Application::builder().application_id(APP_ID).build();

    // Connect to "activate" signal of `app`
    app.connect_activate(build_ui);

    // Run the application
    app.run()
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
    let fl: Arc<FileList> = Arc::new(FileList::new(file_list, main_stack.clone()));

    let dd = Arc::new(b.object::<DropDown>("convert_dropdown").expect("Can't get dropdown"));
    let dropdown_factory = SignalListItemFactory::new();
    dropdown_factory.connect_setup(|_, obj| {
        let lbl = Label::new(None);
        obj
            .downcast_ref::<ListItem>()
            .expect("Needs to be ListItem")
            .set_child(Some(&lbl));
    });

    dropdown_factory.connect_bind(|_, obj| {
        let list_item = obj.downcast_ref::<ListItem>()
            .expect("Needs to be a ListItem");
        let item_str = list_item.item()
            .expect("There needs to be an item")
            .downcast::<StringObject>()
            .expect("Needs to be a StringObject")
            .string();
        let lbl = list_item.child()
            .expect("List item doesn't have a child")
            .downcast::<Label>()
            .expect("Needs to be a Label");
        lbl.set_text(&item_str);
    });
    dd.set_factory(Some(&dropdown_factory));

    let convert_button = b.object::<Button>("convert_button")
        .expect("convert_button doesn't exist");

    let dropdown_stringlist = Arc::new(ListStore::new::<StringObject>());
    let dropdown_model = NoSelection::new(None::<ListStore>);
    dropdown_model.set_model(Some(dropdown_stringlist.as_ref()));
    dd.set_model(Some(&dropdown_model));

    let cp: Arc<Mutex<Option<ConverterProgram>>> = Arc::new(Mutex::new(None));

    convert_button.connect_clicked(clone!(
    #[strong(rename_to = files)] fl.string_list,
    #[strong] cp,
    move |_| {
        println!("{:?}", cp.lock().unwrap().unwrap());
    }));

    // Create a window and set the title
    let window = Arc::new(b.object::<Window>("main_window")
        .expect("Couldn't find window"));

    button.connect_clicked(clone!(
    #[strong] window,
    #[strong] main_stack,
    #[strong] fl,
    #[strong] dropdown_stringlist,
    #[strong] cp,
    move |_| {
        let cancel = Cancellable::new();
        println!("Pressed button!");
        cc.open_multiple(Some(window.as_ref()), Some(&cancel), clone!(
            #[strong] main_stack,
            #[strong] fl,
            #[strong] dropdown_stringlist,
            #[strong] cp,
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
                let ccp = get_converter_program(fl.string_list.as_ref()).expect("You should be seeing an alert dialog but i'm lazy af");
                let mut mcp = cp.lock().unwrap();
                *mcp = Some(ccp);
                match ccp {
                    ConverterProgram::ImageMagick => {
                        println!("Everything went well...");
                        for ext in IMAGEMAGICK_EXTS {
                            dropdown_stringlist.append(&StringObject::new(ext));
                        }
                    }
                    // TODO: Implement these (nuts)
                    ConverterProgram::Ffmpeg => {}
                    ConverterProgram::Pandoc => {}
                }
            }
        ));
    }));

    // Present window
    window.set_application(Some(app));
    window.present();
}
