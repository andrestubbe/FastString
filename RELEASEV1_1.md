FastString v1.1 - Performance Evolution Roadmap
=================================================

**Status:** Planned | **Target:** Q2 2026 | **Branch:** `main` (v1.1-dev)

The next evolution of FastString - integrating production-grade optimizations 
for enterprise workloads and high-frequency processing.

---

🚀 Phase 1: simdutf Integration (Priority: HIGH)
--------------------------------------------------

**What:** Google's simdutf library integration for UTF-8 processing

**Impact:**
- UTF-8 validation: 3-6 GB/s (vs 500 MB/s current)
- UTF-8 → UTF-16 conversion: 2-4 GB/s (vs 200 MB/s current)
- toString(): 5-10× faster

**Implementation:**
```cpp
// New JNI methods
validateUtf8SIMD(long address, int length)
utf8ToUtf16SIMD(long address, int length, char[] out)
```

**Java API:**
```java
// Automatic - uses simdutf when available
FastString fs = new FastString(largeText);
String java = fs.toString();  // simdutf path

// Validation
if (fs.isValidUtf8()) { /* ... */ }  // 5-10× faster
```

**Reference:** https://github.com/simdutf/simdutf

---

⚡ Phase 2: Arena Allocator (Priority: HIGH)
--------------------------------------------

**What:** Thread-local bump-pointer allocator for zero-GC string workloads

**Impact:**
- 5-20× faster for many small strings
- 0% GC pressure in arena scope
- Deterministic performance (no malloc/free jitter)
- Batch deallocation (arena.reset())

**Java API:**
```java
// Create 1MB arena
FastStringArena arena = new FastStringArena(1024 * 1024);

// Create strings (bump-pointer allocation)
for (int i = 0; i < 100000; i++) {
    FastString fs = arena.create(logLinePrefix + i);
    process(fs);
}

// Free all at once (no individual GC)
arena.reset();
```

**Use Cases:**
- Log aggregation (100k+ lines/sec)
- CSV/JSON parsing pipelines
- Microservices request/response handling
- Real-time data processing

---

🔥 Phase 3: Small String Optimization (SSO) (Priority: MEDIUM)
--------------------------------------------------------------

**What:** Stack storage for strings < 64 chars, heap for larger

**Impact:**
- No heap allocation for small strings (85% of typical workloads)
- Cache-friendly stack storage
- Zero JNI overhead for small operations

**Implementation:**
```cpp
class FastString {
    union {
        char* heapBuffer;      // For strings >= 64 chars
        char smallBuffer[64]; // Stack storage for small strings
    };
    bool isSmall;
    uint8_t smallLen;
};
```

**Threshold:** 63 chars (64 incl. null terminator)

---

💎 Phase 4: JNI Critical Sections Expansion (Priority: MEDIUM)
---------------------------------------------------------------

**What:** Extend JNI Critical Sections to more methods

**Current (v1.0):**
- getBytesFast()

**v1.1 Additions:**
- appendFast(byte[] data)
- copyFast(FastString src, int offset, int len)
- toByteArrayFast()

**Impact:** 2-4× reduction in JNI overhead for bulk operations

---

🧩 Phase 5: Advanced SIMD Search (Priority: MEDIUM)
------------------------------------------------------

**What:** Bit-masking SIMD search (from simdjson paper)

**New Methods:**
```java
// Find all occurrences (SIMD parallel)
int[] findAll(String pattern);  

// Split with SIMD delimiter scanning
FastString[] splitFast(char delimiter);

// SIMD-accelerated regex for simple patterns
boolean matchesSimple(String pattern);  // Limited regex subset
```

**Impact:**
- indexOf(): Additional 2-3× speedup
- split(): 10× faster for CSV/TSV parsing

---

🛠️ Phase 6: Java 22+ Panama Support (Priority: LOW)
-----------------------------------------------------

**What:** MemorySegment-based zero-copy slices (optional module)

**API:**
```java
// Java 22+ only
MemorySegment segment = fs.asMemorySegment();
FastString slice = FastString.fromSegment(segment.slice(100, 50));
```

**Benefits:**
- True zero-copy slicing without JNI
- FFI-friendly for Project Panama
- Future-proof API

---

📊 Expected Performance Gains (v1.1 vs v1.0)
===========================================

| Operation | v1.0 | v1.1 (Projected) | Gain |
|-----------|------|------------------|------|
| toString() | 500 MB/s | 3-6 GB/s | **6-12×** |
| UTF-8 validation | 200 MB/s | 3-6 GB/s | **15-30×** |
| Small string create | Heap | Stack | **5-10×** |
| Arena bulk ops | N/A | 5-20× baseline | **New** |
| indexOf() | 2 GB/s | 4-6 GB/s | **2-3×** |
| Memory (batch) | GC-heavy | Zero GC | **∞** |

---

🏗️ Architecture Changes
========================

```
FastString v1.1 Architecture
├── Core (v1.0 compatible)
│   ├── Heap-based strings
│   ├── SIMD operations
│   └── JNI bridge
│
├── simdutf Module (optional)
│   ├── UTF-8 validation
│   ├── UTF-8 → UTF-16 conversion
│   └── Transcoding
│
├── Arena Module (optional)
│   ├── Thread-local bump allocator
│   ├── FastStringArena API
│   └── Batch lifecycle management
│
└── Panama Module (Java 22+, optional)
    ├── MemorySegment integration
    └── Zero-copy FFI
```

---

📦 Build System Updates
=======================

**CMake Integration:**
```cmake
# simdutf as submodule
git submodule add https://github.com/simdutf/simdutf.git

# Build options
option(FASTSTRING_SIMDUTF "Enable simdutf integration" ON)
option(FASTSTRING_ARENA "Enable arena allocator" ON)
```

**Maven Profiles:**
```xml
<profile>
    <id>simdutf</id>
    <activation>
        <property><name>simdutf</name></property>
    </activation>
    <dependencies>
        <!-- Native library with simdutf -->
    </dependencies>
</profile>
```

---

🎯 Release Checklist
====================

- [ ] simdutf C++ integration complete
- [ ] Arena allocator implementation
- [ ] SSO (Small String Optimization)
- [ ] Extended JNI Critical Sections
- [ ] SIMD search improvements
- [ ] Java 22 Panama module (optional)
- [ ] Benchmark suite expansion
- [ ] Documentation updates
- [ ] Migration guide (v1.0 → v1.1)
- [ ] Performance regression tests

---

💡 Design Principles
====================

1. **Backward Compatibility:** v1.1 is drop-in replacement for v1.0
2. **Optional Features:** All new modules are opt-in
3. **Zero Overhead:** Unused features have no runtime cost
4. **Auto-Detection:** Best implementation chosen automatically

---

🚦 Decision Matrix
==================

| Workload Type | Recommended Config |
|---------------|-------------------|
| Logging (100k+ lines/sec) | Arena + SSO |
| JSON/CSV parsing | simdutf + Arena |
| UI text processing | SSO only |
| ETL pipelines | Full stack (simdutf+Arena+SSO) |
| Game development | Arena + SIMD search |

---

**Made with ⚡ and faster Strings by the same guy who still can't sleep**

---

*This is a living document. Features may be added, removed, or reprioritized 
based on community feedback and real-world testing.*
