# Future Integration: conservation-matrix-c

## Current State
C implementation of conservation laws for ternary agent systems {-1, 0, +1}. Tracks energy, surprise, and population conservation across ternary transformations.

## Integration Opportunities

### With ternary-cell (On-Device Conservation Enforcement)
The C port enforces conservation laws on edge devices. Every cell tick must pass through the conservation matrix check: energy in = energy out. On ESP32, this is a lightweight matrix multiply — no heap allocation, no floating point. The conservation matrix is precomputed and stored in ROM.

### With conservation-verify-c
`conservation-matrix-c` implements the laws; `conservation-verify-c` verifies they hold. Together on edge devices: the matrix enforces conservation in the hot path, the verifier periodically audits in the background. If the verifier finds a violation, it triggers a system reset.

### With ternary-science
The Rust `ternary-science` crate validates conservation laws experimentally. The C port implements those laws for deployment. Cross-language validation: run the same conservation test in Rust (development) and C (production) and verify identical results.

## Potential in Mature Systems
In room-as-codespace, conservation enforcement is the physics engine. Every room, on every hardware tier, runs the same conservation checks. The C port targets ESP32/Jetson where Rust is too heavy. Cross-language room operations: Rust for cloud, C for edge, same conservation guarantees.

## Cross-Pollination Ideas
- Precomputed conservation matrices in ROM for zero-RAM enforcement
- Cross-validation between Rust and C implementations for correctness assurance
- Conservation matrix as a room fingerprint — each room type has a characteristic matrix

## Dependencies for Next Steps
- FFI bindings for Rust ternary-cell integration
- ROM layout optimization for ESP32 deployment
- Cross-validation test suite with Rust counterpart
