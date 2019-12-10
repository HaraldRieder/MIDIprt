//mod midifile {
    use std::fs;
    use std::io;
    use midly::{Smf};

    enum ReadResult<T> {
        Ok(T),
        Err(std::io::Error),
        SmfErr(midly::Error) // midly::Error does not implement the Error trait? Else we could use a common enum value of type Box<dyn Error>.
    }

    pub fn read<'a>(path: &'a str) -> ReadResult<&Smf> {
        let data: &'a Vec<u8> = match fs::read(path) {
            io::Result::Ok(data) => &data,
            io::Result::Err(error) => return ReadResult::Err(error)
        };
        let smf: Smf = match Smf::parse(&data) {
            Result::Ok(smf) => smf,
            Result::Err(error) => return ReadResult::SmfErr(error)
        };
        ReadResult::Ok(&smf) // use a Box?
    }
//}
