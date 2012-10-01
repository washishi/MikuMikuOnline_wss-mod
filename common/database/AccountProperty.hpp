//
// AccountProperty.hpp
//

#pragma once

enum AccountProperty {
    REVISION =      0x0,
    PUBLIC_KEY =    0x1,
    LOGIN =         0x2,
    CHANNEL =       0x3,
    NAME =          0xA3,
//    TRIP =          0xA4,
    MODEL_NAME =    0xA5,
    TRIP =          0xA6,
    IP_ADDRESS =    0xF0,
    UDP_PORT =      0xF1,
};

struct PlayerPosition {
    PlayerPosition() : x(0), y(0), z(0), theta(0), vy(0) {}
    PlayerPosition(int16_t x_, int16_t y_, int16_t z_, uint8_t theta_, int8_t vy_)
		: x(x_), y(y_), z(z_), theta(theta_), vy(vy_) {}
    int16_t x, y, z;
    uint8_t theta;
    int8_t vy;
};
