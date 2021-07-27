#include "ChatHandler.h"

ChatHandler::ChatHandler() : ServerObject() {
    setName("Chat Handler Module");
    sharedMessage = new Property("message");
    setProperty("checking_message",sharedMessage);

    steadySetup();
}

void ChatHandler::start() {

}

void ChatHandler::update(Property& property) {
    let_property(prop);
}


simp::updater_void ChatHandler::checkMessage() {
    if (sharedMessage.asString() == "shit") {
        std::cout << "shits hot.\n";
    }
}

simp::updater_void ChatHandler::let_property(Property& prop) {
    if (!property.isValid()) {
        issueNewServerObjectError("Property is invalid.");
        failObject();
        return;
    }
    
    *sharedMessage = prop; 
}