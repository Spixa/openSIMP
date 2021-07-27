#ifndef CHATHANDLER_H
#define CHATHANDLER_H

#include "ServerObject.h"
class ChatHandler : public ServerObject {
public:
    ChatHandler();
    void start();
    void update(Property& property);

    simp::updater_void checkMessage(); 
    simp::updater_void let_property(Property& prop);

    Property* sharedString;
};

#endif