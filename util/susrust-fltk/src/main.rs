use fltk::{app, frame::Frame, text::{TextDisplay, TextBuffer}, button::{ToggleButton, Button}, prelude::*, window::Window, input::Input};
use fltk::{enums::{Event, Key}, dialog};
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
}

fn inc_frame(frame: &mut Frame, val: &mut i32, step: i32) {
    *val += step;
    frame.set_label(&val.to_string());
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
    let mut wind = Window::default().with_size(730, 565).with_label("Rust openSIMP debug utility").center_screen();
    let mut textbox = TextDisplay::default().with_size(700, 500).with_label("Log").center_of(&wind);
    textbox.set_pos(textbox.x()/* - 45*/, textbox.y() - 10);
    textbox.set_buffer(TextBuffer::default());
    //let mut frame = Frame::default().with_size(40, 20).with_label("0");
    let mut toggle = ToggleButton::default().with_size(100, 30).with_label("autoscroll -").below_of(&textbox, 5);
    let mut goto_newest = Button::default().with_size(60, 30).with_label("newest").right_of(&toggle, 5);
    let mut message = Input::default().with_size(485, 30).right_of(&goto_newest, 5);
    let mut send_button = Button::default().with_size(40, 30).with_label("send").right_of(&message, 5);

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
    
    let mut val = 0;

    wind.show();

    let mut uname;
    loop {
        uname = dialog::input(0, 0, "Please type in a username", "");
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

    std::thread::spawn(move || loop {
        //app::sleep(0.5);
        //s.send(Message::Increment(1));
    });

    while app.wait() {
        match r.recv() {
            Some(Message::SendMessage) => {
                textbox.set_insert_position(textbox.buffer().unwrap().length()); // put cursor at the end before insertion

                if message.value().is_empty() {
                    dialog::alert(0, 0, "Cannot send an empty message!");
                } else {
                    textbox.insert(format!("{}: {}\n", uname, message.value()).as_str());
                    message.set_value("");
                    val += 1;
                }

                if toggle.is_toggled() {
                    textbox.scroll(val - 1, 0);
                }
            },
            Some(Message::GoToNewest) => {
                textbox.scroll(val - 1, 0);
            },
            Some(Message::Redraw) => {
                if toggle.is_toggled() {
                    toggle.set_label("autoscroll +");
                } else if !toggle.is_toggled() {
                    toggle.set_label("autoscroll -");
                }
            }
            _ => {
                continue;
            },
        }
    }
}
