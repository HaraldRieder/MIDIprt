use clap::{Arg, App};
use std::fs;

#[path = "../midiheader.rs"]
mod midiheader;

fn main() {
    let matches = App::new("readmidifile")
        .about("A test program to try out features of the midly crate")
        .arg(Arg::with_name("file").required(true).help("A standard MIDI file"))
        .get_matches();
    let file = matches.value_of("file").unwrap();
    let data = fs::read(file).unwrap();
    //let smf = midifile::read(&data);
    let header = midiheader::read_midi_file_header(&data);
    println!("{:?}", header);
}
