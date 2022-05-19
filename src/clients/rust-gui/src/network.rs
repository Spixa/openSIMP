use rand::rngs::OsRng;
use rsa::{PaddingScheme, PublicKey, RsaPrivateKey, RsaPublicKey};
use std::io::{self, Read};
use std::net::{SocketAddr, TcpStream};
use std::str::FromStr;
use std::sync::mpsc::Sender;
use std::sync::Arc;
use std::thread;
use std::time::Instant;

use aes_gcm::aead::{Aead, NewAead};
use aes_gcm::{Aes256Gcm, Key, Nonce};
use rand::{thread_rng, Rng};

const NONCE_LENGTH: usize = 12;

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

pub fn dehex(hex: Vec<u8>) -> Vec<u8> {
    let mut vec: Vec<u8> = vec![0; hex.len() / 2];
    faster_hex::hex_decode(&hex[..], &mut vec[..]).unwrap();

    vec
}

pub fn aes_encrypt(key: &[u8], plaintext: &[u8]) -> Vec<u8> {
    let cipher = Aes256Gcm::new(Key::from_slice(key));
    let nonce_rnd = thread_rng().gen::<[u8; NONCE_LENGTH]>(); // nonce is the same as IV
    let nonce = Nonce::from_slice(&nonce_rnd);
    let encrypt_msg = cipher.encrypt(nonce, plaintext).unwrap();
    let mut ciphertext = Vec::new();
    ciphertext.extend_from_slice(&nonce_rnd);
    ciphertext.extend(encrypt_msg);

    ciphertext
}

pub fn aes_decrypt(key: &[u8], ciphertext: &[u8]) -> Vec<u8> {
    if ciphertext.len() <= NONCE_LENGTH {
        panic!("msg had invalid ciphertext, panicking for now, if this happens a lot please report it!");
        //NOTE: panic!
    }

    let cipher = Aes256Gcm::new(Key::from_slice(key));
    let nonce_rnd = &ciphertext[..NONCE_LENGTH];
    let nonce = Nonce::from_slice(nonce_rnd);

    //plaintext
    cipher.decrypt(nonce, &ciphertext[NONCE_LENGTH..]).unwrap()
}

pub fn make_rsa_keypair(tx: Sender<(RsaPrivateKey, RsaPublicKey)>) {
    println!("spawned rsa gen");
    thread::spawn(move || {
        let now = Instant::now();

        let mut rng = OsRng;
        let bits = 4096;
        let priv_key =
            RsaPrivateKey::new(&mut rng, bits).expect("failed to generate private rsa key");
        let pub_key = RsaPublicKey::from(&priv_key);

        println!(
            "keypair generation done, took {} seconds.",
            now.elapsed().as_secs(),
        );

        tx.send((priv_key, pub_key)).unwrap();
    });
}

pub fn make_rsa_pubkey(n: &str, e: &str) -> RsaPublicKey {
    let n = rsa::BigUint::from_str(&n[..n.len() - 1]).unwrap();
    let e = rsa::BigUint::from_str(e).unwrap();

    RsaPublicKey::new(n, e).unwrap()
}

pub fn encrypt_with_rsa(pubkey: RsaPublicKey, plaintext: &[u8]) -> Vec<u8> {
    let mut rng = OsRng;
    pubkey
        .encrypt(&mut rng, PaddingScheme::new_pkcs1v15_encrypt(), plaintext)
        .expect("failed to encrypt")
}

pub fn decrypt_with_rsa(privkey: RsaPrivateKey, ciphertext: &[u8]) -> Vec<u8> {
    privkey
        .decrypt(PaddingScheme::new_pkcs1v15_encrypt(), ciphertext)
        .expect("failed to decrypt")
}

pub fn read_packet(buf: &mut Vec<u8>, tcpstream: &mut TcpStream) -> Option<()> {
    let bytes_read = tcpstream.read(buf).unwrap();
    buf.truncate(bytes_read);
    if bytes_read == 0 {
        None
    } else {
        Some(())
    }
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
