//  SPDX-License-Identifier: GPL-3.0-or-later
//  Copyright (C) 2023 Pierre Thomain

/*
 * This file is part of LED Segments.
 *
 * LED Segments is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LED Segments is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LED Segments. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ARDUINO
#include <iostream>
#include <vector>
#include "polar/pipeline/signals/Modulation.h"

// Mock FastLED functions if not available in host build
#include <cmath>
#include <cstdlib>

// Simple mock for sin16: maps 0..65535 to one sine cycle scaled to +/- 32767
int16_t sin16(uint16_t theta) {
    double angle = (double) theta * (2.0 * 3.14159265358979323846) / 65536.0;
    return (int16_t) (std::sin(angle) * 32767.0);
}

// Simple mock for inoise16: deterministic pseudo-random
uint16_t inoise16(uint32_t x) {
    // A very simple hash for deterministic testing
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return (uint16_t) (x & 0xFFFF);
}

using namespace LEDSegments;

void runModulationTest() {
    std::cout << "Running Modulation Regression Test..." << std::endl;

    // 1. Setup Modulations
    // Phase velocity is expressed in turns per second (Q16.16); these constants verify deterministic outputs.

    FracQ16_16 velocity = FracQ16_16::fromRaw(66); // Slow rotation
    FracQ16_16 amplitude = FracQ16_16::fromRaw(10000);
    FracQ16_16 offset = FracQ16_16::fromRaw(5000);

    auto sineWave = Sine(ConstantPhaseVelocity(velocity), Constant(amplitude), Constant(offset));
    auto pulseWave = Pulse(ConstantPhaseVelocity(velocity), Constant(amplitude), Constant(offset));

    // Noise is harder to predict without exact inoise16, but we check determinism.
    auto noiseWave = Noise(ConstantPhaseVelocity(velocity), Constant(amplitude), Constant(offset));

    std::vector<TimeMillis> timestamps = {0, 17, 33, 50, 100, 200, 500, 1000, 2000, 5000};

    std::cout << "Timestamp, Sine, Pulse, Noise" << std::endl;
    for (TimeMillis t: timestamps) {
        FracQ16_16 s = sineWave(t);
        FracQ16_16 p = pulseWave(t);
        FracQ16_16 n = noiseWave(t);

        std::cout << t << ", "
                << s.asRaw() << ", "
                << p.asRaw() << ", "
                << n.asRaw() << std::endl;
    }
    std::cout << "Test Complete." << std::endl;
}

int main() {
    runModulationTest();
    return 0;
}

#else
#error "ModulationSelfTest.cpp is host-only and must not be compiled for embedded builds."
#endif
