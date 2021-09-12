#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
// disable opening of cmd in windows

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
use human_panic::setup_panic;
use std::io::prelude::*;
use std::net::SocketAddr;
use std::sync::{Arc, Mutex};

#[cfg(feature = "notify-rust")]
use notify_rust::Notification;

pub mod network;
pub mod utility;
use crate::network::Packet;
use crate::utility::{car_cdr, get_address, get_username, Message};

fn main() {
    setup_panic!(human_panic::Metadata {
        name: env!("CARGO_PKG_NAME").into(),
        version: env!("CARGO_PKG_VERSION").into(),
        authors: "phnixir (phoenix_ir_) <ayitsmephoenix@airmail.cc>".into(),
        homepage: "https://github.com/Spixa/openSIMP".into(),
    });

    /* start gui initialization */
    let (s, r) = app::channel::<Message>();
    let app = app::App::default();

    let widget_theme = WidgetTheme::new(ThemeType::Dark);
    widget_theme.apply();

    let mut wind = Window::default()
        .with_size(730, 570)
        .with_label("Rust openSIMP client")
        .center_screen();

    wind.set_icon(Some(
        image::PngImage::from_data(include_bytes!("rustlogo.png")).unwrap(),
    ));

    // shared textbox arcmutex
    let textbox = Arc::new(Mutex::new(
        TextDisplay::default().with_size(700, 500).center_of(&wind),
    ));

    // init textbox
    {
        let mut textbox = textbox.lock().unwrap();

        // these three lines just push the textbox up a bit so that
        // the buttons dont look dumb, and yes, this is a case of
        // ｔｈｅ ｍａｇｉｃ ｎｕｍｂｅｒ
        let txbx_x = textbox.x();
        let txbx_y = textbox.y();
        textbox.set_pos(txbx_x, txbx_y - 10);

        textbox.set_buffer(TextBuffer::default());
    }

    /* start button definition */
    let mut toggle = ToggleButton::default()
        .with_size(100, 30)
        .with_label("autoscroll +");
    {
        let textbox = textbox.lock().unwrap();
        toggle = toggle.below_of(&*textbox, 5);
        toggle.toggle(true);
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

    toggle.set_callback(move |_| {
        s.send(Message::Redraw);
    });

    goto_newest.set_callback(move |_| {
        s.send(Message::GoToNewest);
    });

    send_button.set_callback(move |_| {
        s.send(Message::SendMessage);
    });
    /* end button definition */
    let textbox_ref = textbox.clone();

    /* start menu definition */
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
    /* end menu definition */

    // basic event handler for the text input,
    // on pressing enter it sends teh SendMesssage
    // signal, just like the send button
    message.handle(move |_, event| match event {
        Event::KeyDown => {
            if app::event_key() == Key::Enter {
                s.send(Message::SendMessage);
            }
            true
        }
        _ => false,
    });

    // insert welcome message into textbox, seperated from the initialization
    // for easier editing in the future
    {
        let textbox = textbox.lock().unwrap();
        let welcome_msg = concat!(
            "[Info]: Welcome! do \".help\" for help with all client and server commands\n",
            "[Info]: If you encounter crashes relaunch using the terminal (or cmd in windows)\n",
            "[Info]: and reproduce the crash, then please follow the instructions that you see\n",
            "[Info]: on the terminal (or cmd).\n"
        );
        textbox.insert(welcome_msg);
    }

    wind.show();
    /* end gui intialization */

    // NOTE: function from src/utility.rs
    let (ip_address, port) = get_address();
    let raw_tcpstream = match network::connect(SocketAddr::new(ip_address, port)) {
        Ok(stream) => stream,
        Err(err) => {
            let alert = concat!(
                "Oopsie woopsie! UwU we made a fucky wucky!! a wittle fucko boingo!\n",
                "The code monkeys at our headquarters are working VEWY HAWD to fix this!\n",
                "(no they arent, connection to the server failed, press \"Close\" to get game-ended)\n",
                "run using the terminal (or cmd on windows) to see the error when this happens",
            );
            dialog::alert_default(alert);
            println!("error occured while trying to connect: {}", err);
            std::process::exit(-1);
        }
    };

    // because of "https://doc.rust-lang.org/stable/std/net/struct.TcpStream.html#method.try_clone"
    // you can use a tcpstream in multiple threads without needing an arcmutex...
    let mut writer_tcpstream = raw_tcpstream.try_clone().unwrap();
    let mut reader_tcpstream = raw_tcpstream.try_clone().unwrap();

    // NOTE: function from src/utility.rs
    let uname = get_username();
    wind.set_label(format!("Rust openSIMP client - {}", uname).as_str());

    // autofocus input box on launch
    message.take_focus().unwrap();

    s.send(Message::Login);

    std::thread::spawn(move || loop {
        let mut buf: Vec<u8> = vec![0; 4096];
        let bytes_read = reader_tcpstream.read(&mut buf).unwrap();
        buf.truncate(bytes_read);
        if bytes_read == 0 {
            let dialog_text = concat!(
                "Connection error occured!\n",
                "Press close to shutdown gracefully or ignore to continue\n",
                "(It's recommended to close and relaunch)\n",
                "\n",
                "Note: If this dialog keeps coming up it (in most cases) means\n",
                "either your internet connection cut off, server went down, you\n",
                "executed /suicide or you were kicked, and if you did execute\n",
                "/suicide, why? theres the \".leave\" client command and it\n",
                "doesn't cause this error, rather, it leaves gracefully\n"
            );
            match dialog::choice_default(dialog_text, "Ignore", "Close", "") {
                0 => continue,
                1 => std::process::exit(0),
                _ => panic!("THIS SHOULD BE UNREACHABLE!!!! crash and burn right this second..."),
            }
        }

        let packet = network::parse_packet(&buf);
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

                #[cfg(feature = "notify-rust")]
                {
                    Notification::new()
                        .summary(format!("Message from {}", &username).as_str())
                        .body(format!("{}: {}", &username, &message).as_str())
                        .show()
                        .unwrap();
                }
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

                #[cfg(feature = "notify-rust")]
                {
                    Notification::new()
                        .summary("Someone joined!")
                        .body(&notif)
                        .show()
                        .unwrap();
                }
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

                #[cfg(feature = "notify-rust")]
                {
                    Notification::new()
                        .summary("Someone left!")
                        .body(&notif)
                        .show()
                        .unwrap();
                }
            }
            Packet::CommandReplyPacket(reply) => {
                let text = format!("[Server]: {}", reply.trim());
                {
                    let mut textbox = textbox_ref.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.set_insert_position(txbxlen);
                    textbox.insert(&text);
                    textbox.insert("\n");
                    s.send(Message::NewestIfToggled);
                }
            }
            Packet::ServerDmPacket(dm) => {
                let text = format!("[Server (DM)]: {}", dm.trim());
                {
                    let mut textbox = textbox_ref.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.set_insert_position(txbxlen);
                    textbox.insert(&text);
                    textbox.insert("\n");
                    s.send(Message::NewestIfToggled);
                }
            }
        };
    });

    /* begin clientside flags */
    let mut uwuify = false;
    /* end clientside flags */

    // NOTE: the Message enum is defined in "src/utility.rs"
    // this is the handler for all self-defined events for
    // the program, they can be sent by callbacks from buttons
    // or from another thread.
    while app.wait() {
        match r.recv() {
            Some(Message::ChangeTheme(themetype)) => {
                let widget_theme = WidgetTheme::new(themetype);
                widget_theme.apply();
                widget_theme.apply();
            }
            Some(Message::Login) => {
                writer_tcpstream
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
                } else if message.value().trim() == ".help" {
                    {
                        let textbox = textbox.lock().unwrap();
                        let concatter = concat!(
                            "[Info]:\n",
                            "",
                            "Client commands: .help .op .uwuify .leave\n",
                            "Server commands: do /help\n",
                            "Types of system messages: Info (from the client), Server (responses from the server)\n",
                        );
                        textbox.insert(concatter);
                    }
                    message.set_value("");
                } else if message.value().trim() == ".op" {
                    {
                        let textbox = textbox.lock().unwrap();
                        textbox.insert("[Info]: Requested operator\n");
                    }
                    writer_tcpstream
                        //.write_all(&format!("4").into_bytes())
                        .write_all(&"4".to_string().into_bytes())
                        .unwrap();
                    message.set_value("");
                } else if message.value().trim() == ".uwuify" {
                    if !uwuify {
                        uwuify = true;
                        {
                            let textbox = textbox.lock().unwrap();
                            textbox.insert("[Info]: uwuifier ON!\n");
                        }
                    } else {
                        uwuify = false;
                        {
                            let textbox = textbox.lock().unwrap();
                            textbox.insert("[Info]: uwuifier OFF!\n");
                        }
                    }
                    message.set_value("");
                } else if message.value().trim() == ".leave" {
                    {
                        let textbox = textbox.lock().unwrap();
                        textbox.insert("[Info]: disconnecting...\n");
                    }
                    s.send(Message::Quit);
                } else if message.value().starts_with('/') {
                    let strmsg = message.value().clone();
                    let (_, command) = car_cdr(strmsg.as_str());
                    {
                        let textbox = textbox.lock().unwrap();
                        textbox
                            .insert(format!("[Info]: Sending command \"{}\" \n", command).as_str());
                    }
                    writer_tcpstream
                        .write_all(&format!("5{}", command).into_bytes())
                        .unwrap();
                    message.set_value("");
                } else {
                    let mut string = message.value().replace('\n', "");
                    if uwuify {
                        string = string.replace(&['r', 'l'][..], "w");
                    }
                    {
                        let textbox = textbox.lock().unwrap();
                        textbox.insert(format!("{}: {}\n", uname, string).as_str());
                    }
                    {
                        writer_tcpstream
                            .write_all(&format!("0{}", string).into_bytes())
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
                // this "toggle" variable is the autoscroll button
                if toggle.is_toggled() {
                    let mut textbox = textbox.lock().unwrap();
                    let txbxlen = textbox.buffer().unwrap().length();
                    textbox.scroll(txbxlen, 0);
                }
            }
            Some(Message::Redraw) => {
                // force redrawing of anything, in this case
                // the autoscroll button uses "s.send(Message::Redraw)"
                // as a callback to change its own label which is handled here
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
