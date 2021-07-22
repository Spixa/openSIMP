#ifndef PACKETTYPE_H
#define PACKETTYPE_H

enum class PacketType {
	MessagePacket = 0,
	IdentifyPacket = 1,
	JoinPacket = 2,
	LeavePacket = 3
};

enum class MessageType {
	NotMessageType,
	BroadcastMessage,
	ChatMessage,
	CommandMessage,
	DirectMessage
};

#endif