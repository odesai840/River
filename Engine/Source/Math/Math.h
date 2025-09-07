#ifndef MATH_H
#define MATH_H

#include <cmath>

namespace RiverCore {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // Vector operations
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(float scalar) const {
        return Vec2(x / scalar, y / scalar);
    }

    // Assignment operators
    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Vector properties
    float magnitude() const {
        return sqrt(x * x + y * y);
    }

    float magnitudeSquared() const {
        return x * x + y * y;
    }

    Vec2 normalized() const {
        float mag = magnitude();
        if (mag > 0.0f) {
            return Vec2(x / mag, y / mag);
        }
        return Vec2(0.0f, 0.0f);
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 0.0f) {
            x /= mag;
            y /= mag;
        }
    }

    float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    float distance(const Vec2& other) const {
        return (*this - other).magnitude();
    }

    float distanceSquared(const Vec2& other) const {
        return (*this - other).magnitudeSquared();
    }

    // Static utility functions
    static Vec2 zero() { return Vec2(0.0f, 0.0f); }
    static Vec2 one() { return Vec2(1.0f, 1.0f); }
    static Vec2 up() { return Vec2(0.0f, 1.0f); }
    static Vec2 down() { return Vec2(0.0f, -1.0f); }
    static Vec2 left() { return Vec2(-1.0f, 0.0f); }
    static Vec2 right() { return Vec2(1.0f, 0.0f); }
};

// Allow scalar * vector multiplication
inline Vec2 operator*(float scalar, const Vec2& vec) {
    return vec * scalar;
}

}

#endif
