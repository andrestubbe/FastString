# FastString Roadmap 🗺️

**Vision:** To provide a zero-allocation, SIMD-accelerated replacement for java.lang.String optimized for modern data streams.

## 🟢 Short-term (v0.2.x)
- [ ] **Native Regex Engine**: Pattern matching directly on UTF-8 bytes without conversion.
- [ ] **Fuzzy Matching**: Levenshtein distance and Jaro-Winkler native implementations.
- [ ] **Expanded Case Logic**: Locale-aware upper/lower case conversion (SIMD).

## 🟡 Mid-term (v0.5.x)
- [ ] **ICU Integration**: Full Unicode support via internal native ICU linking.
- [ ] **Search Indexing**: Fast substring searching via Boyer-Moore or Horspool.
- [ ] **Template Engine**: High-speed string interpolation for high-frequency logs.

## 🔴 Long-term (v1.0.x)
- [ ] **Direct Network Integration**: Bindings for Netty/FastIO to process packets as FastStrings.
- [ ] **Cross-Platform Parity**: Linux and macOS native support.

---
**Part of the FastJava Ecosystem** — *Making the JVM faster.*
