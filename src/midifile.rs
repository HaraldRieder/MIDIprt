//mod midifile {
    use std::fs;
    use std::io;
//    use midly::{Smf,Header};
use std::error::Error;
use nom_midi::{parser,SimpleMidiFile};
/*
    struct OkResult<'a> {
        data: /*&'a*/ Box<Vec<u8>>,
        smf: /*&'a*/ Smf<'a>,
    }
*/
/*    enum ReadResult<'a> {
        Ok(OkResult<'a>),
        Err(std::io::Error),
        SmfErr(midly::Error), // midly::Error does not implement the Error trait? Else we could use a common enum value of type Box<dyn Error>.
        None
    }*/
/*
    pub fn read(path: &str) -> ReadResult {
        let data: /* &*/Vec<u8> = match fs::read(path) {
            io::Result::Ok(d) => /*&*/d,
            io::Result::Err(error) => return ReadResult::Err(error)
        };
        let smf: /*&*/Smf = match Smf::parse(&data) {
            Result::Ok(smf) => /*&*/smf,
            Result::Err(error) => return ReadResult::SmfErr(error)
        };
        ReadResult::Ok(OkResult{smf, data})
    }
*/
/*
pub fn read0(path: &str) -> Smf {
    let data = fs::read(path).unwrap();
    Smf::parse(&data).unwrap()
}
*/
pub struct Ret {
    data: Vec<u8>,
}
/*
pub fn read(data: &Vec<u8>) -> Smf {
    let smf = Smf::parse(&data).unwrap();
    smf
}
*/
pub fn read(data: &Vec<u8>) -> Option<SimpleMidiFile> {
    match parser::parse_smf(&data) {
        Ok(result) => return Some(result.1),
        Err(e) => println!("{:?}",e)
    }
    None
}


//}
