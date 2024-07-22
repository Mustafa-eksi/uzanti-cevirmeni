use std::{process::Command, sync::{Arc, Mutex}, thread, time::Duration};
use gtk::{gio::ListStore, prelude::{CastNone, ListModelExt}, Stack, StringObject};
use gtk::glib::{idle_add, GString, ControlFlow};

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum ConverterProgram {
    ImageMagick,
    Ffmpeg,
    Pandoc
}

pub const EXT_TO_CONVERTER: [(&str, ConverterProgram); 1] = [
    ("png", ConverterProgram::ImageMagick),
];

pub const IMAGEMAGICK_EXTS: [&str; 1] = [
    "jpg"
];

pub fn ext_to_converter(ext: &str) -> Option<ConverterProgram> {
    for t in EXT_TO_CONVERTER {
        if t.0 == ext {
            return Some(t.1);
        }
    }
    None
}

/*
 * Returns ConverterProgram if every file in files has the same ConverterProgram.
 * Returns None otherwise.
 * */
pub fn get_converter_program(files: &ListStore) -> Option<ConverterProgram> {
    let mut i = 0;
    let mut current_converter: Option<ConverterProgram> = None;
    while i < files.n_items() {
        let fp = files.item(i)
            .and_downcast::<StringObject>()
            .expect("File isn't a StringObject")
            .string();
        let ext = fp.split(".").last().expect("There's no extension?");
        let files_converter = ext_to_converter(ext);
        if !current_converter.is_some() {
            current_converter = files_converter;
            continue;
        }
        if !files_converter.is_some() {
            return None;
        }
        if files_converter.unwrap() != current_converter.unwrap() {
            return None;
        }
        i += 1;
    }
    current_converter
}


pub fn convert(file: GString, cp: ConverterProgram, main_stack: Arc<Mutex<Stack>>) -> Result<(), ()> {
    let child = Command::new("magick")
        .arg(file)
        .arg("~/Desktop/deneme.jpg")
        .spawn()
        .expect("Can't spawn child");
    // TODO: https://stackoverflow.com/questions/66510406/gtk-rs-how-to-update-view-from-another-thread
   // let h = thread::spawn(move || {
   //     main_stack;
   //     let output = child.wait_with_output();
   //     idle_add(move || {
   //         main_stack;
   //         //.lock().unwrap()
   //         //    .set_visible_child_name("open_file_button");
   //         ControlFlow::Continue
   //     })
   // });
    Ok(())
}
