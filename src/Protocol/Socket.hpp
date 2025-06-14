#pragma once
#include <emscripten/websocket.h>
#include "Packet.hpp"

class SocketClient {
public:
    static void sendPacket(Packet* packet);
    static void connect();
};