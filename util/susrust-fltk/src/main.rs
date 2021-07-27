use fltk::{app, frame::Frame, text::{TextDisplay, TextBuffer}, button::{ToggleButton, Button}, prelude::*, window::Window, input::Input};
use fltk::{enums::{Event, Key, FrameType, Shortcut}, dialog, image, menu};
use fltk_theme::{WidgetTheme, ThemeType};
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

#[derive(Debug, Copy, Clone)]
pub enum Message {
    SendMessage,
    GoToNewest,
    Redraw,
    Quit,
    ChangeTheme(ThemeType),
}

fn center() -> (i32, i32) {
    (
        (app::screen_size().0 / 2.0) as i32,
        (app::screen_size().1 / 2.0) as i32,
    )
}

fn main() {
    let (s, r) = app::channel::<Message>();

    let app = app::App::default();
    let mut widget_theme = WidgetTheme::new(ThemeType::Dark);
    if std::env::var("AERO").is_ok() {
        widget_theme = WidgetTheme::new(ThemeType::Aero);
    }
    let widget_theme = widget_theme;
    widget_theme.apply();
    let mut wind = Window::default().with_size(730, 570).with_label("Rust openSIMP client").center_screen();
    wind.set_icon(Some(image::PngImage::from_data(include_bytes!("rustlogo.png")).unwrap()));

    let mut textbox = TextDisplay::default().with_size(700, 500).center_of(&wind);
    textbox.set_pos(textbox.x()/* - 45*/, textbox.y() - 10);
    textbox.set_buffer(TextBuffer::default());
    //let mut frame = Frame::default().with_size(40, 20).with_label("0");
    let mut toggle = ToggleButton::default().with_size(100, 30).with_label("autoscroll -").below_of(&textbox, 5);
    let mut goto_newest = Button::default().with_size(60, 30).with_label("newest").right_of(&toggle, 5);
    let mut message = Input::default().with_size(485, 30).right_of(&goto_newest, 5);
    let mut send_button = Button::default().with_size(40, 30).with_label("send").right_of(&message, 5);

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
        "Themes/High contrast\t",
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
        },
        _ => false,
    });

    wind.show();

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
            },
            None => continue,
        }
    }
    let uname = uname.unwrap();
    wind.set_label(format!("Rust openSIMP client - {}", uname).as_str());

    message.take_focus().unwrap();

    std::thread::spawn(move || loop {
        //app::sleep(0.5);
        //s.send(Message::Increment(1));
    });

    while app.wait() {
        match r.recv() {
            Some(Message::ChangeTheme(themetype)) => {
                let widget_theme = WidgetTheme::new(themetype);
                widget_theme.apply();
                widget_theme.apply();
            },
            Some(Message::SendMessage) => {
                textbox.set_insert_position(textbox.buffer().unwrap().length()); // put cursor at the end before insertion

                if message.value().is_empty() {
                    dialog::alert(center().0, center().1, "Cannot send an empty message!");
                } else {
                    textbox.insert(format!("{}: {}\n", uname, message.value()).as_str());
                    message.set_value("");
                }

                if toggle.is_toggled() {
                    textbox.scroll(textbox.buffer().unwrap().length(), 0);
                }
            },
            Some(Message::GoToNewest) => {
                textbox.scroll(textbox.buffer().unwrap().length(), 0);
            },
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
            _ => {
                continue;
            },
        }
    }
}
