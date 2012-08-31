//
// AccountProperty.hpp
//

#pragma once

enum AccountProperty {
    REVISION =      0x0,
    PUBLIC_KEY =    0x1,
    LOGIN =         0x2,
    NAME =          0xA3,
    TRIP =          0xA4,
    MODEL_NAME =    0xA5,
    IP_ADDRESS =    0xF0,
    UDP_PORT =      0xF1,
};

struct PlayerPosition {
    PlayerPosition() : x(0), y(0), z(0), theta(0) {}
    PlayerPosition(float x_, float y_, float z_, float theta_) : x(x_), y(y_), z(z_), theta(theta_) {}
    float x, y, z, theta;
};
