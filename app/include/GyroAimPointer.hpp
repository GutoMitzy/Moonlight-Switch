#pragma once

#include <algorithm>
#include <cmath>

class GyroAimPointer {
public:
    void reset() {
        orientation = {1.0f, 0.0f, 0.0f, 0.0f};
        filteredRate = {};
        measuredGravity = {};
        referenceGravity = {};
        hasMeasuredGravity = false;
        hasReferenceGravity = false;
        moving = false;
        stationarySeconds = 0.0f;
        displayOffsetX = 0.0f;
        displayOffsetY = 0.0f;
        displayedX = 0.5f;
        displayedY = 0.5f;
    }

    void setAcceleration(float x, float y, float z) {
        const Vec3 sample{x, y, z};
        const float magnitude = length(sample);
        if (!std::isfinite(magnitude) || magnitude < 8.5f ||
            magnitude > 11.5f)
        {
            hasMeasuredGravity = false;
            return;
        }
        measuredGravity = sample / magnitude;
        hasMeasuredGravity = true;
        if (!hasReferenceGravity) {
            referenceGravity = measuredGravity;
            hasReferenceGravity = true;
        }
    }

    bool update(float gx, float gy, float gz, float dt, float zoom,
                float& normalizedX, float& normalizedY,
                bool hardwareAtRest = false) {
        if (!std::isfinite(gx) || !std::isfinite(gy) ||
            !std::isfinite(gz) || !std::isfinite(dt) ||
            !std::isfinite(zoom))
            return false;
        if (dt <= 0.0f || dt > 0.08f) {
            filteredRate = {};
            moving = false;
            stationarySeconds = 0.0f;
            return false;
        }

        const Vec3 angularRate{gx, gy, gz};
        const float angularSpeed = length(angularRate);
        if (angularSpeed > 20.0f)
            return false;

        // Allow fine aiming on the first sample. Only attenuate very low
        // angular speeds, where stationary IMU noise dominates.
        constexpr float noiseFloor = 0.012f;
        constexpr float fullResponse = 0.036f;
        const float weight = std::clamp(
            (angularSpeed - noiseFloor) / (fullResponse - noiseFloor),
            0.0f, 1.0f);
        // A HID-confirmed resting controller must not drift because of
        // residual sensor bias. Ignore low-speed residuals only while at rest.
        moving = weight > 0.0f &&
                 !(hardwareAtRest && angularSpeed < 0.05f);
        const Vec3 target = angularRate * (moving ? weight : 0.0f);

        // Fast intentional turns receive the raw sample to avoid adding
        // filter delay. Micro-adjustments receive a short low-pass filter.
        constexpr float smoothingCeiling = 0.12f;
        const float smoothing = std::clamp(
            (smoothingCeiling - angularSpeed) / smoothingCeiling,
            0.0f, 1.0f);
        const float alpha = -std::expm1(-dt * 45.0f);
        const Vec3 smoothed = filteredRate + (target - filteredRate) * alpha;
        filteredRate = smoothed * smoothing + target * (1.0f - smoothing);

        if (moving) {
            stationarySeconds = 0.0f;
            orientation = normalized(multiply(orientation,
                                  fromRate(filteredRate, dt)));
        } else {
            filteredRate = {};
            stationarySeconds += dt;
            if (stationarySeconds > 0.3f &&
                hasMeasuredGravity && hasReferenceGravity)
            {
                const Vec3 worldGravity =
                    rotate(orientation, measuredGravity);
                const Vec3 error = cross(worldGravity, referenceGravity);
                const float errorLength = length(error);
                if (errorLength > 1.0e-6f && errorLength < 0.5f) {
                    const Quat adjustment =
                        axisAngle(error / errorLength,
                                  std::min(errorLength * dt * 0.65f,
                                           0.015f));
                    orientation = normalized(multiply(adjustment,
                                                     orientation));
                }
            }
        }

        const Vec3 aim = rotate(orientation, {0.0f, 0.0f, 1.0f});
        const float depth = std::max(0.20f, aim.z);
        constexpr float baseFocal = 0.89f;
        const float focal = baseFocal * std::clamp(zoom, 0.6f, 1.8f);
        const float rawX = 0.5f - aim.x / depth * focal + displayOffsetX;
        const float rawY = 0.5f + aim.y / depth * focal + displayOffsetY;
        if (!moving) {
            displayOffsetX += displayedX - rawX;
            displayOffsetY += displayedY - rawY;
            normalizedX = displayedX;
            normalizedY = displayedY;
        } else {
            displayedX = std::clamp(rawX, 0.0f, 1.0f);
            displayedY = std::clamp(rawY, 0.0f, 1.0f);
            normalizedX = displayedX;
            normalizedY = displayedY;
        }
        return true;
    }

private:
    struct Vec3 {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        Vec3 operator+(const Vec3& rhs) const {
            return {x + rhs.x, y + rhs.y, z + rhs.z};
        }
        Vec3 operator-(const Vec3& rhs) const {
            return {x - rhs.x, y - rhs.y, z - rhs.z};
        }
        Vec3 operator*(float scale) const {
            return {x * scale, y * scale, z * scale};
        }
        Vec3 operator/(float scale) const {
            return {x / scale, y / scale, z / scale};
        }
    };
    struct Quat {
        float w = 1.0f, x = 0.0f, y = 0.0f, z = 0.0f;
    };

    static float length(const Vec3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x};
    }
    static Quat multiply(const Quat& a, const Quat& b) {
        return {
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w
        };
    }
    static Quat normalized(const Quat& q) {
        const float len = std::sqrt(q.w*q.w + q.x*q.x +
                                    q.y*q.y + q.z*q.z);
        return {q.w/len, q.x/len, q.y/len, q.z/len};
    }
    static Quat axisAngle(const Vec3& axis, float angle) {
        const float half = angle * 0.5f;
        const float sine = std::sin(half);
        return {std::cos(half), axis.x*sine, axis.y*sine, axis.z*sine};
    }
    static Quat fromRate(const Vec3& rate, float dt) {
        const float speed = length(rate);
        return speed < 1.0e-7f
            ? Quat{} : axisAngle(rate / speed, speed * dt);
    }
    static Vec3 rotate(const Quat& q, const Vec3& v) {
        const Vec3 qv{q.x, q.y, q.z};
        const Vec3 t = cross(qv, v) * 2.0f;
        return v + t*q.w + cross(qv, t);
    }

    Quat orientation;
    Vec3 filteredRate;
    Vec3 measuredGravity;
    Vec3 referenceGravity;
    bool hasMeasuredGravity = false;
    bool hasReferenceGravity = false;
    bool moving = false;
    float stationarySeconds = 0.0f;
    float displayOffsetX = 0.0f;
    float displayOffsetY = 0.0f;
    float displayedX = 0.5f;
    float displayedY = 0.5f;
};
