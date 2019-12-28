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
    pub chunk_length: u32,
    pub number_tracks: u16,
    pub smpte: u16,			/* >= 0 if SMPTE format */
    pub ticks_per_beat: u16,
    pub ticks_per_frame: u16,
}

impl MidiFileHeader {
    /// Creates a new MIDI file 1.0 header (6 bytes chunk length) with still invalid file type.
    fn new() -> MidiFileHeader {
        MidiFileHeader{
            midi_file_type: MidiFileType::Invalid,
            chunk_length: 6,
            number_tracks: 0,
            smpte: 0, // frames per second, usually 24,25,29 or 30
            ticks_per_beat: 0, // if smpte is zero
            ticks_per_frame: 0 // if smpte is not zero
        }
    }
}

fn to_u16(u0: u8, u1: u8) -> u16 {
    (u0 as u16) << 8 | u1 as u16
}

fn to_u32(u0: u8, u1: u8, u2: u8, u3: u8) -> u32 {
    (to_u16(u0, u1) as u32) << 16 | to_u16(u2, u3) as u32
}

/// Reads only the first 3 data fields of a MIDI file. I.e. all that occurs in a 1.0 header with 6 data bytes.
pub fn read_midi_file_header(data: &Vec<u8>) -> MidiFileHeader {
    let division: u16 = 0;
    let mut header = MidiFileHeader::new();

    if data.len() < 14 || &data[0..4] != b"MThd" {
        /* no correct MIDI file format */
        return header
    }
    header.chunk_length = to_u32(data[4], data[5], data[6], data[7]);
    header.midi_file_type = match to_u16(data[8], data[9]) {
        0 => MidiFileType::SingleTrack,
        1 => MidiFileType::Parallel,
        2 => MidiFileType::Sequential,
        _ => MidiFileType::Invalid
    };
    header.number_tracks = to_u16(data[10], data[11]);
    let division = to_u16(data[12], data[13]);
    if division & 0x8000 != 0
    {
        /* SMPTE format */
        header.smpte           = -((division >> 8) as i8) as u16;
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

#[cfg(test)]
mod tests {
    use crate::midi::{
        read_midi_file_header,
        MidiFileType
    };

    #[test]
    fn read_header_too_short() {
        let raw = vec![0x4D, 0x53, 0x68, 0x64, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::Invalid);
        assert_eq!(header.chunk_length, 6);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 0);
        assert_eq!(header.number_tracks, 0);
    }


    #[test]
    fn read_header_no_MThd() {
        let raw = vec![0x4D, 0x53, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x02, 0xE0];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::Invalid);
        assert_eq!(header.chunk_length, 6);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 0);
        assert_eq!(header.number_tracks, 0);
    }

    #[test]
    fn read_header_midi_file_type_0() {
        let raw = vec![0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x02, 0xE0];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::SingleTrack);
        assert_eq!(header.chunk_length, 6);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 736);
        assert_eq!(header.number_tracks, 1);
    }

    #[test]
    fn read_header_midi_file_type_1_with_4_tracks() {
        let raw = vec![0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x04, 0x01, 0xE0];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::Parallel);
        assert_eq!(header.chunk_length, 256);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 480);
        assert_eq!(header.number_tracks, 4);
    }

    #[test]
    fn read_header_midi_file_type_2_with_256_tracks() {
        let raw = vec![0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x02, 0x01, 0x00, 0x01, 0xE0];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::Sequential);
        assert_eq!(header.chunk_length, 6);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 480);
        assert_eq!(header.number_tracks, 256);
    }

    #[test]
    fn read_header_midi_file_type_invalid() {
        let raw = vec![0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x04, 0x01, 0xE0];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::Invalid);
        assert_eq!(header.chunk_length, 256);
        assert_eq!(header.smpte, 0);
        assert_eq!(header.ticks_per_frame, 0);
        assert_eq!(header.ticks_per_beat, 480);
        assert_eq!(header.number_tracks, 4);
    }

    /// Test data has been derived from https://github.com/craigsapp/binasc/wiki/smpte
    /// example 1.
    #[test]
    fn read_header_midi_file_type_0_smpte() {
        let raw = vec![0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0xE7, 0x28];
        let header = read_midi_file_header(&raw);
        assert_eq!(header.midi_file_type, MidiFileType::SingleTrack);
        assert_eq!(header.chunk_length, 6);
        assert_eq!(header.smpte, 25);
        assert_eq!(header.ticks_per_frame, 40);
        assert_eq!(header.ticks_per_beat, 0);
        assert_eq!(header.number_tracks, 1);

    }
}
