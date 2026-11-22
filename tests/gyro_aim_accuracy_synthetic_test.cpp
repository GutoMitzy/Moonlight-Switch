#include "GyroAimPointer.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>

int main()
{
    std::mt19937 rng(1221);
    std::normal_distribution<float> noise(0.0f, 0.004f);

    int hits = 0;
    float errorTotal = 0.0f;
    constexpr int trials = 1000;
    constexpr float tolerance = 0.01f;

    for (int trial = 0; trial < trials; ++trial)
    {
        GyroAimPointer pointer;
        float x = 0.5f;
        float y = 0.5f;
        const int axis = trial % 2;
        const float rate = 0.009f + 0.0024f * (trial % 100);

        for (int sample = 0; sample < 200; ++sample)
        {
            float gx = noise(rng);
            float gy = noise(rng);
            const float gz = noise(rng);
            if (axis == 0)
                gy += rate;
            else
                gx += rate;
            assert(pointer.update(gx, gy, gz, 0.005f, 1.0f, x, y));
        }

        const float expected = 0.5f - std::tan(rate) * 0.89f;
        const float measured = axis == 0 ? x : y;
        const float error = std::fabs(measured - expected);
        errorTotal += error;
        hits += error <= tolerance;
    }

    std::printf("Synthetic gyro target hits: %d/%d (mean error %.6f)\\n",
                hits, trials, errorTotal / trials);
    assert(hits >= 990);
}
