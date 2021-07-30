use fltk::{
    app,
    button::{Button, ToggleButton},
    input::Input,
    prelude::*,
    text::{TextBuffer, TextDisplay},
    window::Window,
};
use fltk::{
    dialog,
    enums::{Event, FrameType, Key, Shortcut},
    image, menu,
};
use fltk_theme::{ThemeType, WidgetTheme};
use std::io::prelude::*;
use std::net::SocketAddr;
use std::sync::{Arc, Mutex};
use trust_dns_resolver::config::*;
use trust_dns_resolver::Resolver;
use notify_rust::Notification;

pub mod network {
    use std::io;
    use std::net::{SocketAddr, TcpStream};

    pub fn connect(socket_addr: SocketAddr) -> io::Result<TcpStream> {
        let stream = TcpStream::connect(socket_addr)?;

        Ok(stream)
    }
}

#[derive(Debug, Copy, Clone)]
pub enum Message {
    Login,
    SendMessage,
    GoToNewest,
    NewestIfToggled,
    Redraw,
    Quit,
    ChangeTheme(ThemeType),
}

#[derive(Debug, Clone)]
pub enum Packet {
    // username, message
    MessagePacket(Arc<String>, Arc<String>),
    // ip + has connected line
    JoinPacket(Arc<String>),
    // username + disconnected line
    DisconnectPacket(Arc<String>),
}

fn center() -> (i32, i32) {
    (
        (app::screen_size().0 / 2.0) as i32,
        (app::screen_size().1 / 2.0) as i32,
    )
}

fn parse_packet(buf: &[u8]) -> Packet {
    // i barely have any idea how this works
    // please excuse my c programming background
    let mut last_point = 0;
    let mut ifpasses = 0;

    let mut strvec: Vec<String> = vec![];
    for (i, val) in buf.iter().enumerate() {
        if *val == 1u8 {
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
            }
        }
    }

    let res = match strvec[0].as_str() {
        "0" => Packet::MessagePacket(Arc::new(strvec[2].clone()), Arc::new(strvec[1].clone())),
        "1" => Packet::JoinPacket(Arc::new(strvec[1].clone())),
        "2" => Packet::DisconnectPacket(Arc::new(strvec[1].clone())),
        _ => panic!("please screenshot this and open an issue"),
    };

    res
}

fn main() {
    let (s, r) = app::channel::<Message>();

    let app = app::App::default();
    let widget_theme = WidgetTheme::new(ThemeType::HighContrast);
    widget_theme.apply();
    let mut wind = Window::default()
        .with_size(730, 570)
        .with_label("Rust openSIMP client")
        .center_screen();
    wind.set_icon(Some(
        image::PngImage::from_data(include_bytes!("rustlogo.png")).unwrap(),
    ));

    let textbox = Arc::new(Mutex::new(
        TextDisplay::default().with_size(700, 500).center_of(&wind),
    ));
    {
        let mut textbox = textbox.lock().unwrap();
        let txbx_x = textbox.x();
        let txbx_y = textbox.y();
        textbox.set_pos(txbx_x /* - 45*/, txbx_y - 10);
    }
    {
        let mut textbox = textbox.lock().unwrap();
        textbox.set_buffer(TextBuffer::default());
    }
    //let mut frame = Frame::default().with_size(40, 20).with_label("0");
    let mut toggle = ToggleButton::default()
        .with_size(100, 30)
        .with_label("autoscroll -");
    {
        let textbox = textbox.lock().unwrap();
        toggle = toggle.below_of(&*textbox, 5)
    }
    let mut goto_newest = Button::default()
        .with_size(60, 30)
        .with_label("newest")
        .right_of(&toggle, 5);
    let mut message = Input::default()
        .with_size(485, 30)
        .right_of(&goto_newest, 5);
    let mut send_button = Button::default()
        .with_size(40, 30)
        .with_label("send")
        .right_of(&message, 5);
    let textbox_ref = textbox.clone();

    let mut menu = menu::MenuBar::default().with_size(730, 25);
    menu.set_frame(FrameType::FlatBox);
    menu.add_emit(
        "Program/Quit...\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s,
        Message::Quit,
    );
    menu.add_emit(
        "Themes/Dark\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s,
        Message::ChangeTheme(ThemeType::Dark),
    );
    menu.add_emit(
        "Themes/Aero\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s,
        Message::ChangeTheme(ThemeType::Aero),
    );
    menu.add_emit(
        "Themes/H4xx0r\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s,
        Message::ChangeTheme(ThemeType::HighContrast),
    );
    menu.add_emit(
        "Themes/Aqua\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s,
        Message::ChangeTheme(ThemeType::Aqua),
    );

    toggle.set_callback(move |_| {
        s.send(Message::Redraw);
    });

    goto_newest.set_callback(move |_| {
        s.send(Message::GoToNewest);
    });

    send_button.set_callback(move |_| {
        s.send(Message::SendMessage);
    });

    message.handle(move |_, event| match event {
        Event::KeyDown => {
            if app::event_key() == Key::Enter {
                s.send(Message::SendMessage);
            }
            true
        }
        _ => false,
    });

    /*
    wind.handle(move |_, event| match event {
        Event::Focus => {
            true
        }
        Event::Unfocus => {
            true
        }
        _ => false,
    });
    */

    wind.show();

    let mut inner_url;
    loop {
        inner_url = dialog::input(
            center().0,
            center().1,
            "Please enter a url/ip for the server with port seperated by ':'\n(ui may freeze while resolving urls)",
            "",
        );
        match inner_url {
            Some(ref name) => {
                if name.is_empty() {
                    continue;
                } else {
                    break;
                }
            }
            None => continue,
        }
    }
    let port = inner_url
        .clone()
        .unwrap()
        .split(':')
        .last()
        .unwrap()
        .parse::<u16>()
        .unwrap();

    // pain
    let url = inner_url.unwrap();
    let url = url.split(':').next().unwrap();

    let resolver = Resolver::new(ResolverConfig::default(), ResolverOpts::default()).unwrap();
    let response = resolver
        .lookup_ip(url)
        .unwrap()
        .iter()
        .next()
        .expect("no address returned!");

    let mut uname;
    loop {
        uname = dialog::input(center().0, center().1, "Please type in a username", "");
        match uname {
            Some(ref name) => {
                if name.is_empty() {
                    continue;
                } else {
                    break;
                }
            }
            None => continue,
        }
    }
    let uname = uname.unwrap();
    wind.set_label(format!("Rust openSIMP client - {}", uname).as_str());

    message.take_focus().unwrap();

    let server_connection = match network::connect(SocketAddr::new(
        //IpAddr::V4(Ipv4Addr::new(127, 0, 0, 1)),
        response, port,
    )) {
        Ok(stream) => stream,
        Err(err) => {
            dialog::alert(center().0, center().1, "Oopsie woopsie! UwU we made a fucky wucky!! a wittle fucko boingo!\nThe code monkeys at our headquarters are working VEWY HAWD to fix this!\n(no they arent, connection to the server failed, press \"Close\" to die)");
            panic!("error occured while trying to connect: {}", err);
        }
    };
    let server_connection_arc = Arc::new(Mutex::new(server_connection.try_clone().unwrap()));
    let server_connection_ref = server_connection_arc;
    let mut svc_try_cloned = server_connection.try_clone().unwrap();

    s.send(Message::Login);

    std::thread::spawn(move || loop {
        loop {
            let mut buf: Vec<u8> = vec![0; 4096];
            let bytes_read = svc_try_cloned.read(&mut buf).unwrap();
            buf.truncate(bytes_read);
            if bytes_read == 0 {
                println!("connection error occured, fucking off right this second...");
                std::process::exit(0);
            }

            let packet = parse_packet(&buf);
            match packet {
                Packet::MessagePacket(username, message) => {
                    let text = format!("{}: {}\n", &username, &message);
                    {
                        let mut textbox = textbox_ref.lock().unwrap();
                        let txbxlen = textbox.buffer().unwrap().length();
                        textbox.set_insert_position(txbxlen); // put cursor at the end before insertion
                        textbox.insert(&text);
                        s.send(Message::NewestIfToggled);
                    }
                    Notification::new()
                        .summary(format!("Message from {}", &username).as_str())
                        .body(format!("{}: {}", &username, &message).as_str())
                        .show()
                        .unwrap();
                }
                Packet::JoinPacket(notif) => {
                    let text = format!("{}\n", &notif);
                    {
                        let mut textbox = textbox_ref.lock().unwrap();
                        let txbxlen = textbox.buffer().unwrap().length();
                        textbox.set_insert_position(txbxlen);
                        textbox.insert(&text);
                        s.send(Message::NewestIfToggled);
                    }
                    Notification::new()
                        .summary("Someone joined!")
                        .body(&notif)
                        .show()
                        .unwrap();
                }
                Packet::DisconnectPacket(notif) => {
                    let text = format!("{}\n", notif);
                    {
                        let mut textbox = textbox_ref.lock().unwrap();
                        let txbxlen = textbox.buffer().unwrap().length();
                        textbox.set_insert_position(txbxlen);
                        textbox.insert(&text);
                        s.send(Message::NewestIfToggled);
                    }
                    Notification::new()
                        .summary("Someone left!")
                        .body(&notif)
                        .show()
                        .unwrap();
                }
            };

            //println!("read {} bytes: {:x?}", bytes_read, buf);
        }
    });

    while app.wait() {
        match r.recv() {
            Some(Message::ChangeTheme(themetype)) => {
                let widget_theme = WidgetTheme::new(themetype);
                widget_theme.apply();
                widget_theme.apply();
            }
            Some(Message::Login) => {
                let mut server_connection = server_connection_ref.lock().unwrap();
                server_connection
                    .write_all(&format!("3{}", uname).into_bytes())
                    .unwrap();
            }
            Some(Message::SendMessage) => {
                {
                    let mut textbox = textbox.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.set_insert_position(txbxlen); // put cursor at the end before insertion
                }

                if message.value().is_empty() {
                    //dialog::alert(center().0, center().1, "Cannot send an empty message!");
                } else {
                    {
                        let textbox = textbox.lock().unwrap();
                        textbox.insert(format!("{}: {}\n", uname, message.value()).as_str());
                    }
                    {
                        let mut server_connection = server_connection_ref.lock().unwrap();
                        server_connection
                            .write_all(&format!("0{}", message.value()).into_bytes())
                            .unwrap();
                    }
                    message.set_value("");
                }

                if toggle.is_toggled() {
                    let mut textbox = textbox.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.scroll(txbxlen, 0);
                }
            }
            Some(Message::GoToNewest) => {
                let mut textbox = textbox.lock().unwrap();
                let txbxlen = textbox.buffer().unwrap().length();
                textbox.scroll(txbxlen, 0);
            }
            Some(Message::NewestIfToggled) => {
                if toggle.is_toggled() {
                    let mut textbox = textbox.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.scroll(txbxlen, 0);
                }
            }
            Some(Message::Redraw) => {
                if toggle.is_toggled() {
                    toggle.set_label("autoscroll +");
                } else if !toggle.is_toggled() {
                    toggle.set_label("autoscroll -");
                }
            }
            Some(Message::Quit) => {
                std::process::exit(0);
            }
            _else => {
                continue;
            }
        }
    }
}
