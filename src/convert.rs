use std::{process::Command, sync::{Arc, Mutex}};
use fragile::Fragile;
use gtk::{gio::ListStore, glib::idle_add_once, prelude::{Cast, CastNone, ListModelExt}, StringObject};
use gtk::glib::GString;

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
        if t.0 == ext.to_lowercase() {
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

pub fn convert(file: GString, output_file: String, cp: ConverterProgram, ext: usize) -> std::process::Child {
    let (_folder, filename) = file.rsplit_once("/").expect("This is not a valid path");
    let (realfilename, _ext) = filename.rsplit_once(".").expect("This is not a valid path");
    let output_path = format!("{output_file}/{realfilename}");
    match cp {
        ConverterProgram::ImageMagick => Command::new("magick")
            .arg(file.clone())
            .arg(format!("{output_path}.{}", IMAGEMAGICK_EXTS[ext]))
            .spawn()
            .expect("Can't spawn child (imagemagick)"),
        ConverterProgram::Ffmpeg => Command::new("ffmpeg")
            .arg("-i")
            .arg(file.clone())
            .arg(format!("{output_path}.{}", FFMPEG_EXTS[ext]))
            .spawn()
            .expect("Can't spawn child (ffmpeg)"),
        ConverterProgram::Pandoc => Command::new("pandoc")
            .arg(file.clone())
            .arg("-o")
            .arg(format!("{output_path}.{}", PANDOC_EXTS[ext]))
            .spawn()
            .expect("Can't spawn child (pandoc)")
    }
}

pub fn convert_multiple(files: Fragile<Arc<ListStore>>, output_folder: String, cp: ConverterProgram, ext: usize, ui: Fragile<Arc<ConvertionUI>>, cancel: Arc<Mutex<bool>>,
    cc: Arc<Mutex<Option<u32>>>) {
    let mut i = 0;
    let ls = files.get();
    let mut children: Vec<std::process::Child> = Vec::new();
    while i < ls.n_items() {
        children.push(convert(
            ls.item(i).unwrap().downcast::<StringObject>().unwrap().string(),
            output_folder.clone(),
            cp,
            ext));
        i += 1;
    }
    std::thread::spawn(move || {
        for mut ch in children {
            if *cancel.lock().unwrap() {
                let _ = ch.kill();
            }
            {
                let mut cchild = cc.lock().unwrap();
                *cchild = Some(ch.id());
            }
            let _ = ch.wait();
        }
        idle_add_once(move || {
            ui.get().finished();
        });
    });
}
