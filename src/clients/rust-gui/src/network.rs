use std::io;
use std::net::{SocketAddr, TcpStream};
use std::sync::Arc;

pub fn connect(socket_addr: SocketAddr) -> io::Result<TcpStream> {
    let stream = TcpStream::connect(socket_addr)?;

    Ok(stream)
}

#[derive(Debug, Clone)]
pub enum Packet {
    // username, message
    MessagePacket(Arc<String>, Arc<String>),
    // ip + has connected line
    JoinPacket(Arc<String>),
    // username + disconnected line
    DisconnectPacket(Arc<String>),
    // command result
    CommandReplyPacket(Arc<String>),
    // dm from server
    ServerDmPacket(Arc<String>),
}

pub fn parse_packet(buf: &[u8]) -> Packet {
    // i barely have any idea how this works
    // please excuse my c programming background
    let mut last_point = 0;
    let mut ifpasses = 0;

    let mut strvec: Vec<String> = vec![];
    for (i, val) in buf.iter().enumerate() {
        if *val == 1_u8 {
            strvec.push(
                std::str::from_utf8(&buf[last_point..i])
                    .unwrap()
                    .to_string(),
            );
            last_point = i + 1;
            ifpasses += 1;

            #[allow(clippy::if_same_then_else)]
            if (&strvec[0] == "0") && (ifpasses == 2) {
                strvec.push(
                    std::str::from_utf8(&buf[last_point..buf.len()])
                        .unwrap()
                        .to_string(),
                );
            } else if (&strvec[0] == "1") && (ifpasses == 1) {
                strvec.push(
                    std::str::from_utf8(&buf[last_point..buf.len()])
                        .unwrap()
                        .to_string(),
                );
            } else if (&strvec[0] == "2") && (ifpasses == 1) {
                strvec.push(
                    std::str::from_utf8(&buf[last_point..buf.len()])
                        .unwrap()
                        .to_string(),
                );
            } else if (&strvec[0] == "5") && (ifpasses == 1) {
                strvec.push(
                    std::str::from_utf8(&buf[last_point..buf.len()])
                        .unwrap()
                        .to_string(),
                );
            } else if (&strvec[0] == "6") && (ifpasses == 1) {
                strvec.push(
                    std::str::from_utf8(&buf[last_point..buf.len()])
                        .unwrap()
                        .to_string(),
                );
            }
        }
    }

    let res = match strvec[0].as_str() {
        "0" => Packet::MessagePacket(Arc::new(strvec[2].clone()), Arc::new(strvec[1].clone())),
        "1" => Packet::JoinPacket(Arc::new(strvec[1].clone())),
        "2" => Packet::DisconnectPacket(Arc::new(strvec[1].clone())),
        "5" => Packet::CommandReplyPacket(Arc::new(strvec[1].clone())),
        "6" => Packet::ServerDmPacket(Arc::new(strvec[1].clone())),
        _else => Packet::MessagePacket(
            Arc::new(format!("illegal packet reply starter ({})", _else)),
            Arc::new("???".to_string()),
        ),
    };

    res
}
