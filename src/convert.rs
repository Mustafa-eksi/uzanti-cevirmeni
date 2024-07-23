use std::{fmt::format, process::{Child, Command}, sync::{mpsc::Sender, Arc, Mutex}, thread, time::Duration};
use fragile::Fragile;
use gtk::{gio::ListStore, glib::idle_add_once, prelude::{Cast, CastNone, ListModelExt}, Stack, StringObject};
use gtk::glib::{idle_add, GString, ControlFlow};

use crate::ConvertionUI;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum ConverterProgram {
    ImageMagick,
    Ffmpeg,
    Pandoc
}

pub const EXT_TO_CONVERTER: [(&str, ConverterProgram); 25] = [
// IMAGEMAGICK
    ("png", ConverterProgram::ImageMagick),
    ("gif", ConverterProgram::ImageMagick),
    ("webp", ConverterProgram::ImageMagick),
    ("jpg", ConverterProgram::ImageMagick),
    ("jpeg", ConverterProgram::ImageMagick),
    ("svg", ConverterProgram::ImageMagick),
    ("xcf", ConverterProgram::ImageMagick),
// FFMPEG
    ("webm", ConverterProgram::Ffmpeg),
    ("gif", ConverterProgram::Ffmpeg),
    ("mp4", ConverterProgram::Ffmpeg),
    ("mp3", ConverterProgram::Ffmpeg),
    ("opus", ConverterProgram::Ffmpeg),
    ("avi", ConverterProgram::Ffmpeg),
    ("mov", ConverterProgram::Ffmpeg),
// PANDOC
    ("epub", ConverterProgram::Pandoc),
    ("html", ConverterProgram::Pandoc),
    ("json", ConverterProgram::Pandoc),
    ("tex", ConverterProgram::Pandoc),
    ("md", ConverterProgram::Pandoc),
    ("odt", ConverterProgram::Pandoc),
    ("odf", ConverterProgram::Pandoc),
    ("pdf", ConverterProgram::Pandoc),
    ("docx", ConverterProgram::Pandoc),
    ("doc", ConverterProgram::Pandoc),
    ("txt", ConverterProgram::Pandoc),
];

pub const IMAGEMAGICK_EXTS: [&str; 7] = [
    "png", 
    "gif", 
    "webp", 
    "jpg", 
    "jpeg", 
    "svg", 
    "xcf", 
];

pub const FFMPEG_EXTS: [&str; 7] = [
    "webm", 
    "gif", 
    "mp4", 
    "mp3", 
    "opus", 
    "avi", 
    "mov", 
];

pub const PANDOC_EXTS: [&str; 11] = [
    "epub", 
    "html", 
    "json", 
    "tex", 
    "md", 
    "odt", 
    "odf", 
    "pdf", 
    "docx", 
    "doc", 
    "txt", 
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
        println!("{fp}");
        let ext = fp.split(".").last().expect("There's no extension?");
        let files_converter = ext_to_converter(ext);
        if !files_converter.is_some() {
            return None;
        }
        if !current_converter.is_some() {
            current_converter = files_converter;
            continue;
        }
        if files_converter.unwrap() != current_converter.unwrap() {
            return None;
        }
        i += 1;
    }
    current_converter
}

pub enum ConvertionCom {
    Ongoing(usize),
    Success,
    Failure
}

pub fn convert(file: GString, output_folder: String, cp: ConverterProgram) -> std::process::Child {
    let (_folder, filename) = file.rsplit_once("/").expect("This is not a valid path");
    let output_path = format!("{output_folder}/{filename}");
    match cp {
        ConverterProgram::ImageMagick => Command::new("magick")
            .arg(file.clone())
            .arg(output_path)
            .spawn()
            .expect("Can't spawn child (imagemagick)"),
        ConverterProgram::Ffmpeg => Command::new("ffmpeg")
            .arg("-i")
            .arg(file.clone())
            .arg(output_path)
            .spawn()
            .expect("Can't spawn child (ffmpeg)"),
        ConverterProgram::Pandoc => Command::new("pandoc")
            .arg(file.clone())
            .arg("-o")
            .arg(output_path)
            .spawn()
            .expect("Can't spawn child (pandoc)")
    }
}

pub fn convert_multiple(files: Fragile<Arc<ListStore>>, output_folder: String, cp: ConverterProgram, ui: Fragile<Arc<ConvertionUI>>) {
    let mut i = 0;
    let ls = files.get();
    let mut children: Vec<std::process::Child> = Vec::new();
    while i < ls.n_items() {
        children.push(convert(
            ls.item(i).unwrap().downcast::<StringObject>().unwrap().string(),
            output_folder.clone(),
            cp));
        i += 1;
    }
    std::thread::spawn(move || {
        for mut ch in children {
            let _ = ch.wait();
        }
        idle_add_once(move || {
            ui.get().finished();
        });
    });
}
