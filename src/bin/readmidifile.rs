use clap::{Arg, App};

#[path = "../midifile.rs"]
mod midifile;

fn main() {
    let matches = App::new("readmidifile")
        .about("A test program to try out features of the midly crate")
        .arg(Arg::with_name("file").required(true).help("A standard MIDI file"))
        .get_matches();

    let smf = midifile::read("bla.mid");
    println!("Hello, world!");

}
