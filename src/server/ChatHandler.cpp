#include "ChatHandler.h"

ChatHandler::ChatHandler() : ServerObject() {
    setName("Chat Handler Module");
    sharedString = new Property("");

    steadySetup();
}

void ChatHandler::start() {

}

void ChatHandler::update(Property& property) {
    let_property(property);
    checkMessage();


}


simp::updater_void ChatHandler::checkMessage() {
    if (getProperty("checking_message").asString() == "shit") {
        std::cout << "This word is bad. i think it is bad. plz stop say it it make me sad plz dont.\n";
    } 

    setProperty("checking_message",Property(""));
    return simp::updater_void::success_return;
}

simp::updater_void ChatHandler::let_property(Property& prop) {
    if (!prop.isValid()) {
        issueNewServerObjectError("Property is invalid.");
        failObject();
        return simp::updater_void::fail_return;
    }
    
    setProperty("checking_message",prop);


    return simp::updater_void::success_return;
}