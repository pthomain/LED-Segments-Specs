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

#ifndef UMBRELLA_EFFECT_CONFIG_H
#define UMBRELLA_EFFECT_CONFIG_H

#include "crgb.h"
#include "UmbrellaLayoutConfig.h"
#include "effects/linear/chaos/ChaosEffect.h"
#include "effects/linear/fractal/FractalEffect.h"
#include "effects/linear/noise/NoiseEffect.h"
#include "effects/linear/rose/RoseEffect.h"
#include "effects/linear/swirl/SwirlEffect.h"
#include "effects/linear/slide/SlideEffect.h"
#include "engine/render/renderable/TypedRenderable.h"
#include "engine/displayspec/config/LayoutConfig.h"

using namespace LEDSegments;

static RenderablesAndMirrors<CRGB> umbrellaEffectSelector(uint16_t layoutId) {
    if (layoutId == LEDS_IN_SPOKE) {
        return {
            {
                {SwirlEffect::factory, 2},
                {NoiseEffect::factory, 2},
                // {SlideEffect::factory, 1},
                // {GradientEffect::factory, 1},
                {ChaosEffect::factory, 1},
                {FractalEffect::factory, 1},
                {RoseEffect::factory, 1},
            },
            allMirrors<CRGB>
        };
    }
    //SPOKES_IN_WHOLE
    return {
        {
            {SwirlEffect::factory, 1},
            {ChaosEffect::factory, 1},
            {FractalEffect::factory, 1},
            {RoseEffect::factory, 1},
        },
        [](RenderableFactoryRef<CRGB> effectFactory) {
            return undividedMirrors<CRGB>(effectFactory);
        }
    };
}

#endif //UMBRELLA_EFFECT_CONFIG_H
