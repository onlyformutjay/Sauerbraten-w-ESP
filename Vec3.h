#pragma once
#include <math.h>
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vec3() {}

    Vec3 operator+ (Vec3& a) {
        return Vec3(x + a.x, y + a.y, z + a.z);
    }

    Vec3 operator- (Vec3& a) {
        return Vec3(x - a.x, y - a.y, z - a.z);
    }

    Vec3 operator/ (Vec3& a) {
        return Vec3(x / a.x, y / a.y, z / a.z);
    }

    Vec3 operator* (Vec3& a) {
        return Vec3(x * a.x, y * a.y, z * a.z);
    }

    float hypo3() {
        return sqrt(((x * x) + (y * y) + (z * z)));
    }
};