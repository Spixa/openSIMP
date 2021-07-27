use std::io::prelude::*;
use std::net::{
    IpAddr,
    Ipv4Addr,
    SocketAddr,
};

pub mod network {
    use std::io;
    use std::net::{
        TcpStream,
        SocketAddr,
    };

    pub fn connect(socket_addr: SocketAddr) -> io::Result<TcpStream> {
        let stream = TcpStream::connect(socket_addr)?;

        Ok(stream)
    }
}

fn main() {

    if let Ok(mut server_connection) = network::connect(SocketAddr::new(IpAddr::V4(Ipv4Addr::new(127, 0, 0, 1)), 37549)) {
        println!("connected");
        match std::env::args().nth(1).unwrap().as_str() {
            "receive" => {
                let mut buf: Vec<u8> = vec![0; 4096];
                let bytes_read = server_connection.read(&mut buf).unwrap();
                buf.truncate(bytes_read);

                println!("read {} bytes: {:x?}", bytes_read, buf);
            },
            "send" => {
                let string = std::env::args().nth(2).unwrap(); // for readability purposes only
                server_connection.write(&string.into_bytes()).unwrap();
            },
            "bytesend" => {
                let string = std::env::args().nth(2).unwrap(); // for readability purposes only
                println!("{:?}", hex::decode(&string).expect("failed to decode hex"));
                server_connection.write(&hex::decode(&string).expect("failed to decode hex")).unwrap();
            },
            "exploit" => {
                match std::env::args().nth(2).unwrap().as_str() {
                    "overwhelm-checker" => {
                        // alpha sus server exploit to crash the server
                        // - is intended to panic with a connection reset after server crashes
                        println!("[!] this exploit has been fixed");
                        loop {
                            server_connection.write(&"\x01".to_string().into_bytes()).unwrap();
                        }
                    },
                    _ => panic!(),
                }
            }
            _ => panic!("no args"),
        }
    } else {
        println!("failed");
    }

}
