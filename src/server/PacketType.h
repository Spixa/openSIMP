#pragma once

enum class PacketType {
	MessagePacket = 0,
	IdentifyPacket = 1,
	JoinPacket = 2,
	LeavePacket = 3
};