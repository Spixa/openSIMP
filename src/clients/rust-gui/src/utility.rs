use fltk::dialog;
use fltk_theme::ThemeType;
use std::net::IpAddr;
use trust_dns_resolver::config::*;
use trust_dns_resolver::Resolver;

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

// credit: https://stackoverflow.com/questions/30811107/how-do-i-get-the-first-character-out-of-a-string#comment83958722_48482196
pub fn car_cdr(s: &str) -> (&str, &str) {
    match s.chars().next() {
        Some(c) => s.split_at(c.len_utf8()),
        None => s.split_at(0),
    }
}

pub fn get_username() -> String {
    let uname;
    loop {
        uname = dialog::input_default("Please type in a username", "");
        match uname {
            Some(ref name) => {
                if !name.is_empty() {
                    break;
                }
                std::process::exit(0);
            }
            None => std::process::exit(0),
        }
    }
    uname.unwrap()
}

pub fn get_password() -> String {
    let passwd;
    loop {
        passwd = dialog::input_default("Please type in a password", "");
        match passwd {
            Some(ref pass) => {
                if !pass.is_empty() {
                    break;
                }
                std::process::exit(0);
            }
            None => std::process::exit(0),
        }
    }
    passwd.unwrap()
}

pub fn get_address() -> (IpAddr, u16) {
    let inner_url;
    loop {
        inner_url = dialog::input_default(
            concat!(
                "Please enter a url/ip for the server with port seperated by ':'\n",
                "(ui may freeze while resolving urls)",
            ),
            "",
        );
        match inner_url {
            Some(ref inner_url) => {
                if !inner_url.is_empty() {
                    break;
                }
                std::process::exit(0)
            }
            None => std::process::exit(0),
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

    (response /* the ip */, port)
}
