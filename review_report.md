# Review Report: Polar Pipeline Refactor

## 1) Non-negotiable contracts

*   **Angle/phase contract:**
    *   **Status:** **Verified**.
    *   `Units::PhaseTurnsUQ16_16` is consistently used as `uint32_t` with high 16 bits as turns.
    *   `PolarUtils::cartesianCoords` and `polarCoords` correctly handle promotion/demotion.
    *   `RotationTransform`, `VortexTransform` use full-resolution addition.
    *   `KaleidoscopeTransform` correctly handles phase folding and mandala multiplication (wrapping).
*   **Noise semantics:**
    *   **Status:** **Verified**.
    *   `CartesianNoiseLayers` normalize to `NoiseNormU16` (0..65535).
    *   `PolarPipeline` samples and maps to 8-bit palette index correctly.
*   **Signal semantics:**
    *   **Status:** **Verified**.
    *   `PhaseVelAngleUnitsQ16_16` is treated as `AngleTurns16` per second.
    *   Presets use values like 1500 (~0.02 turns/sec) which is consistent with this scale (where 1.0 Q16.16 = 1/65536 turns/sec).

## 2) Presets and parameter scaling

**Risk: High (Broken Presets)**
Several presets use `Waveforms::ConstantAccelerationWaveform` with large integer values (500, 1500, 2000, 2500) inside `Sine` or `Pulse` waveforms.
Since `LinearSignal(<int>)` promotes to Q16.16 (e.g., 500 -> 500.0), these result in massive acceleration values (e.g., 1000.0 units/sec²), causing `RadialScaleTransform` to saturate instantly.

*   **`CRTRipple`**: `Sine(..., 500)` -> Amplitude 500.0. **Fixed**.
*   **`StarburstPulse`**: `Pulse(..., 2000)` -> Amplitude 2000.0. **Fixed**.
*   **`RippleRing`**: `Sine(..., 1500)` -> Amplitude 1500.0. **Fixed**.
*   **`StutterPulse`**: `Pulse(..., 2500)` -> Amplitude 2500.0. **Fixed**.

**Verified Presets (Safe):**
*   `BarrelTunnel`: `Lens(1/64)`, `Radial(1/32)`. Safe.
*   `NoiseWarpFlame`: `NoiseWarp(1/20)`, `Bend(1/100)`. Safe.
*   `TiledMirrorMandala`: Safe.
*   `LiquidMarble`: Safe.
*   `HeatShimmer`: Safe.
*   `SpiralGalaxy`: Safe.
*   `ElectricTunnel`: Safe.
*   `RainShear`: Safe.
*   `FractalTileBloom`: Safe.
*   `CurlFlowSmoke`: Safe.
*   `PerspectiveDepth`: Safe (after fix).
*   `PosterizedRings`: Safe.
*   `JitteredTiles`: Safe (after fix).
*   `TimeStutter`: Safe.

## 3) Transform math correctness and UB/overflow audit

### High Risk (Fixed)
*   **`PerspectiveWarpTransform`**:
    *   **Issue:** `denom` calculation `1.0 + ky` could approach 0, causing division by zero or overflow in `scale_q16`.
    *   **Fix Applied:** Added clamping to `denom` to ensure a minimum magnitude (approx +/- 0.001).

### Medium Risk (Fixed)
*   **`TileJitterTransform`**:
    *   **Issue:** `uint32_t tileIdxX = (static_cast<uint32_t>(x) / tileX)`. For negative `x`, this cast caused a jump to `UINT_MAX`, creating a seam at `x=0`.
    *   **Fix Applied:** Implemented signed floor division for tile indices.

### Medium Risk (Fixed)
*   **`Waveforms::Pulse`**:
    *   **Issue:** The logic `(saw < QUARTER) ? saw << 1 : (MAX - saw)` creates a discontinuity at `QUARTER` (0.25).
    *   **Fix Applied:** Updated logic to use continuous triangle wave logic.

### Low Risk
*   **Implementation-defined Shifts**: Right shift of negative signed values is used in multiple transforms. This is standard on target platforms but technically implementation-defined in older C++ standards.
*   **`BendTransform`**: Safe due to existing clamp logic.

## 4) Builder + pipeline construction correctness

*   **Status:** **Verified**.
*   `PolarPipelineBuilder` correctly enforces single build, ensures final polar domain, and handles names.

## 5) Include hygiene

*   **Status:** **Verified**.
*   Includes appear correct. `KaleidoscopeTransform` is self-contained. `PresetPicker` includes `FastLED.h`.

---

## Actions Taken

1.  **Fixed `PerspectiveWarpTransform.cpp`**: Added safety clamps.
2.  **Fixed `TileJitterTransform.cpp`**: Implemented signed floor division.
3.  **Fixed `Waveforms.h`**:
    *   Added `ConstantAccelerationWaveformRaw` to allow passing raw Q16.16 values directly.
    *   Fixed `Pulse` waveform logic to be continuous.
4.  **Fixed `Presets.cpp`**: Updated broken presets (`CRTRipple`, `StarburstPulse`, `RippleRing`, `StutterPulse`) to use `ConstantAccelerationWaveformRaw` for small fractional amplitudes.
