use std::sync::{Arc, Mutex, mpsc};
use fragile::Fragile;
use gtk::gio::{Cancellable, File, ListModel};
use gtk::{NoSelection, SignalListItemFactory, StringList, StringObject};
use gtk::{
    prelude::*, Button, FileDialog, FileFilter, gio::ListStore, Builder,
    Application, DropDown, Window, ListView, Stack, Box, Label, ListItem,
    AlertDialog,
};
use gtk::glib::{clone, ExitCode, timeout_add, ControlFlow};

mod file_list;
use file_list::FileList;

mod convert;
use convert::{convert_multiple, get_converter_program, ConverterProgram, ConvertionCom, EXT_TO_CONVERTER, FFMPEG_EXTS, IMAGEMAGICK_EXTS, PANDOC_EXTS};

const APP_ID: &str = "me.mustafaeksi.uzanti-cevirmeni";

fn main() -> ExitCode {
    // Create a new application
    let app = Application::builder().application_id(APP_ID).build();
    // Connect to "activate" signal of `app`
    app.connect_activate(build_ui);
    // Run the application
    app.run()
}

struct ConvertionUI {
    fl: Arc<FileList>,
    dd: Arc<DropDown>,
    dropdown_factory: SignalListItemFactory,
    output_button: Button,
    output_file_dialog: FileDialog,
    cp: Arc<Mutex<Option<ConverterProgram>>>,
    convert_button: Button, 
    dropdown_stringlist: Arc<ListStore>,  
    dropdown_model: NoSelection,
    output_folder: Arc<Mutex<String>>,
    stack: Arc<Stack>,
}

impl ConvertionUI {
    pub fn new(b: Builder, main_stack: Arc<Stack>, window: Arc<Window>) -> Arc<Self> {
        let file_list = b.object::<ListView>("file_list").expect("file_list not found");
        let cui = Arc::new(ConvertionUI{
            fl: Arc::new(FileList::new(file_list, main_stack.clone())),
            dd: Arc::new(b.object::<DropDown>("convert_dropdown").expect("Can't get dropdown")),
            dropdown_factory: SignalListItemFactory::new(),
            output_button: b.object::<Button>("select_output_folder").unwrap(),
            output_file_dialog: FileDialog::new(),
            cp: Arc::new(Mutex::new(None)),
            convert_button: b.object::<Button>("convert_button")
                .expect("convert_button doesn't exist"),
            dropdown_stringlist: Arc::new(ListStore::new::<StringObject>()),
            dropdown_model: NoSelection::new(None::<ListStore>),
            output_folder: Arc::new(Mutex::new(String::new())),
            stack: main_stack.clone(),
        });

        cui.dropdown_factory.connect_setup(|_, obj| {
            let lbl = Label::new(None);
            obj
                .downcast_ref::<ListItem>()
                .expect("Needs to be ListItem")
                .set_child(Some(&lbl));
        });

        cui.dropdown_factory.connect_bind(|_, obj| {
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
        cui.dd.set_factory(Some(&cui.dropdown_factory));

        cui.dropdown_model.set_model(Some(cui.dropdown_stringlist.as_ref()));
        cui.dd.set_model(Some(&cui.dropdown_model));

        let frag = Fragile::new(cui.clone());
        cui.convert_button.connect_clicked(clone!(
        #[strong(rename_to = files)] cui.fl.string_list,
        #[strong(rename_to = cp)] cui.cp,
        #[strong] frag,
        #[strong(rename_to = output_folder)] cui.output_folder,
        #[strong] main_stack,
        move |_| {
            println!("{:?}", cp.lock().unwrap().unwrap());
            let out = output_folder.lock().unwrap();
            convert_multiple(
                Fragile::new(files.clone()),
                out.clone(),
                cp.lock().unwrap().expect("No convertion program"),
                frag.clone());
            main_stack.set_visible_child_name("loading_page");
        }));
        cui.output_button.connect_clicked(clone!(
        #[strong] window,
        #[strong(rename_to = output_folder)] cui.output_folder,
        #[strong(rename_to = ofd)] cui.output_file_dialog,
        move |_| {
            let cancel = Cancellable::new();
            ofd.select_folder(Some(window.as_ref()), Some(&cancel), clone!(
            #[strong] output_folder,
            move |res| {
                let file = res.unwrap();
                let mut out = output_folder.lock().unwrap();
                *out = String::from(file.path().expect("empty folder path").to_str().unwrap());
            }));
        }));
        cui
    }

    pub fn open_files(&self, files: ListModel) -> Result<(), ()> {
        let mut i = 0;
        self.fl.clear();
        while i < files.n_items() {
            let b = files.item(i).expect("Couldn't iterate through files");
            let file = b
                .downcast_ref::<File>()
                .expect("Couldn't downcast to File");
            self.fl.add_file(String::from(file.path().expect("Can't get path").to_str().unwrap()));
            i += 1;
        }
        let conprog = get_converter_program(self.fl.string_list.as_ref());
        if !conprog.is_some() {
            return Err(());
        }
        let ccp = conprog.unwrap();
        let mut mcp = self.cp.lock().unwrap();
        *mcp = Some(ccp);
        match ccp {
            ConverterProgram::ImageMagick => {
                for ext in IMAGEMAGICK_EXTS {
                    self.dropdown_stringlist.append(&StringObject::new(ext));
                }
            }
            // TODO: Implement these (nuts)
            ConverterProgram::Ffmpeg => {
                for ext in FFMPEG_EXTS {
                    self.dropdown_stringlist.append(&StringObject::new(ext));
                }
            }
            ConverterProgram::Pandoc => {
                for ext in PANDOC_EXTS {
                    self.dropdown_stringlist.append(&StringObject::new(ext));
                }
            }
        }
        Ok(())
    }

    pub fn finished(&self) {
        self.stack.set_visible_child_name("open_file_button");
        self.fl.clear();
        self.output_folder.lock().unwrap().clear();
        *self.cp.lock().unwrap() = None;
        self.dropdown_stringlist.remove_all();
    }
}

fn build_ui(app: &Application) {
    let b = Builder::from_file("./ui.ui");
    let button = b.object::<Button>("open_file_button")
        .expect("Couldn't find button");

    let lm = ListStore::new::<FileFilter>();
    let ff = FileFilter::new();
    // TODO: add support for seperate file filters
    let mut suffixes = String::new();
    for (suf, _program) in EXT_TO_CONVERTER {
        if suf != "" {
            suffixes = format!("{suffixes} *.{suf}");
        }
    }
    ff.add_suffix(&suffixes);
    lm.append(&ff);
    let cc = FileDialog::builder()
        .filters(&lm)
        .build();

    let main_stack = Arc::new(b.object::<Stack>("main_stack").expect("Stack not found"));

    // Create a window and set the title
    let window = Arc::new(b.object::<Window>("main_window")
        .expect("Couldn't find window"));

    let cui = Arc::new(Mutex::new(ConvertionUI::new(b, main_stack.clone(), window.clone())));

    button.connect_clicked(clone!(
    #[strong] window,
    #[strong] main_stack,
    #[strong] cui,
    move |_| {
        let cancel = Cancellable::new();
        println!("Pressed button!");
        cc.open_multiple(Some(window.as_ref()), Some(&cancel), clone!(
            #[strong] main_stack,
            #[strong] window,
            #[strong] cui,
            move |res| {
                if res.is_err() {
                    return;
                }
                let r = cui.lock().unwrap().open_files(res.unwrap());
                match r {
                    Ok(_) => main_stack.set_visible_child_name("file_list"),
                    Err(_) => {
                        let alert = AlertDialog::builder()
                            .message("Couldn't find suitable converter program")
                            .buttons(["Ok"])
                            .build();
                        alert.choose(Some(window.as_ref()), Some(&Cancellable::new()), move |_| {});
                    }
                }
            }
        ));
    }));

    // Present window
    window.set_application(Some(app));
    window.present();
}

