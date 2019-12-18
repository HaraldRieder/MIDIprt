/// The different formats an SMF file can be.
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub enum MidiFileType {
    Invalid,
    SingleTrack,
    Parallel,
    Sequential,
}

/// A MIDI file header.
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct MidiFileHeader {
    pub midi_file_type: MidiFileType,
    pub number_tracks: u16,
    pub smpte: u16,			/* >= 0 if SMPTE format */
    pub ticks_per_beat: u16,
    pub ticks_per_frame: u16,
}

impl MidiFileHeader {
    fn new() -> MidiFileHeader {
        MidiFileHeader{
            midi_file_type: MidiFileType::Invalid,
            number_tracks: 0,
            smpte: 0,
            ticks_per_beat: 0,
            ticks_per_frame: 0
        }
    }
}

fn to_u16(array: &[u8; 2]) -> u16 {
    (array[0] as u16) << 8 + (array[1] as u16)
}

pub fn read_midi_file_header(data: &Vec<u8>) -> MidiFileHeader {
    let division: u16 = 0;
    let mut header = MidiFileHeader::new();

    /* evaluate header chunk */
    if data.split(4) != b"MThd" {
        /* no correct MIDI file format */
        header
    }

    /* continue evaluating header chunk: */
    header.midi_file_type = match to_u16(*data[8..10]) {
        0 => MidiFileType::SingleTrack,
        1 => MidiFileType::Parallel,
        2 => MidiFileType::Sequential,
        _ => MidiFileType::Invalid
    };
    header.number_tracks = to_u16(*data[10..12]);
    let division = to_u16(*data[12..14]);
    if division & 0x1000
    {
        /* SMPTE format */
        header.smpte           = (-division) >> 8 ;
        header.ticks_per_frame = division & 0xff ;
        header.ticks_per_beat  = 0 ;
    }
    else
    {
        /* non-SMPTE format */
        header.smpte           = 0 ;
        header.ticks_per_frame = 0 ;
        header.ticks_per_beat  = division ;
    }
    header
}

