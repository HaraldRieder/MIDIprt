type Time = i32;
type Value = u8;
type Channel = u4;

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct NoteEvent {
    pub time: Time,        /* absolute time of event (element) */
    pub reference: i16,    /* pointer to corresponding note on/off event */
    pub dynamic: Value,    /* note on dynamic or note off dynamic */
    pub note: Value,       /* bit7 = 0 note off; bit7 = 1 note on */
    pub channel: Channel,  /* MIDI channel */
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct TrackInfo {
    pub channels: HashSet<Channel>, /* contains non-empty channels */
    pub min_note: Value, /* lowest note of the track  */
    pub max_note: Value, /* highest note of the track  */
    /*** MIDI meta events: ***/
    pub sequence_number: i32,    /* of format 0 or 1 file, number of sequence in format 2 file, or -1 */ // TODO Rust Option?
    pub text: String,            /* only the 1. text event found */
    pub copyright: String,       /* should occur in 1. track only */
    pub track_name: String,      /* or sequence name if in format 0 file or in the 1. track */
    pub instrument_name: String, /* per track, possible at different times ? */
    pub lyric: String,           /* only the 1. lyric event found */
    pub marker: String,          /* only the 1. text marker found */
    pub cue_point: String,       /* only the 1. description found */
    pub program_name: String,    /* only the 1. program name found */
    pub device_name: String,     /* device to be used for track */
    pub tempo: u32,              /* in microseconds, only the 1. found, or -1 */ // TODO Rust Option?
    /* SMPTE offset not supported here */
    /* time signature: */
    pub numerator: Value,        /* e.g. 6 for 6/8, or -1 */ // TODO Rust Option?
    pub denominator: Value,      /* power of 2 already expanded, e.g. 3 in file => 2^3 = 8 here, or -1 */ // TODO Rust Option?
    /* metronome clicks and ??? not supported */
    /* key signature */
    pub key: Option<u4>,         /* already chromatic: 0..11, 0 is C */
    pub minor: Option<bool>,     /* true if minor key */
}
