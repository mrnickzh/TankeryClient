#include <emscripten/websocket.h>
#include <cstdio>
#include "Socket.hpp"
#include "PacketHelper.hpp"
#include "Packets/HandShakePacket.hpp"

EMSCRIPTEN_WEBSOCKET_T ws = 0;

EM_BOOL on_open(int type, const EmscriptenWebSocketOpenEvent *e, void *userData) {
    HandShakePacket pkt;
    pkt.name = "playername";
    SocketClient::sendPacket(&pkt);
    return EM_TRUE;
}

EM_BOOL on_message(int type, const EmscriptenWebSocketMessageEvent *e, void *userData) {
    std::vector<uint8_t> vec(e->data, e->data + e->numBytes);
    PacketHelper::decodePacket(vec);
    return EM_TRUE;
}

void SocketClient::connect() {
    EmscriptenWebSocketCreateAttributes attr = {
            .url = "ws://localhost:20201",
            .protocols = NULL,
            .createOnMainThread = EM_FALSE
    };

    ws = emscripten_websocket_new(&attr);

    emscripten_websocket_set_onopen_callback(ws, NULL, on_open);
    emscripten_websocket_set_onmessage_callback(ws, NULL, on_message);
}

void SocketClient::sendPacket(Packet* packet) {
    std::vector<uint8_t> data = PacketHelper::encodePacket(packet);
    emscripten_websocket_send_binary(ws, data.data(), static_cast<uint32_t>(data.size()));
}