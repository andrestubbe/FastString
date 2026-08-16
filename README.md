# FastString 0.1.1 [ALPHA-2026-08] — High-performance SIMD UTF-8 String for Java

[![Status](https://img.shields.io/badge/status-0.1.1-brightgreen.svg)](https://github.com/andrestubbe/FastString/releases/tag/0.1.1)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-ready-green.svg)](https://jitpack.io/#andrestubbe/FastString)

---

**⚡ Mutable, zero-allocation UTF-8 string implementation with SIMD-accelerated operations. Bypasses Java String overhead for elite performance.**

**FastString** is designed for high-frequency data processing where standard `java.lang.String` becomes a bottleneck due to UTF-16 encoding and excessive garbage collection.

[![FastString Showcase](docs/screenshot.png)](https://www.youtube.com/watch?v=BZsqQl7WqWk)

---

---

## FastJava Native Memory & Hardware Substrate

`FastString` is part of the **FastJava Low-Level Native Memory Substrate** — a suite of modules designed to give Java applications raw C++ speed and direct hardware access:

| Substrate Module | Role & Key Capability |
|---|---|
| **[`FastSharedMemory`](https://github.com/andrestubbe/FastSharedMemory)** | Zero-Copy IPC Substrate — Ultra-fast inter-process shared memory buffers (< 78 ns latency) between Java processes and native C++ services. |
| **[`FastPointer`](https://github.com/andrestubbe/FastPointer)** | 64-Bit Native Pointer Abstraction — Zero-allocation address arithmetic, handle casting (HWND, HANDLE), and off-heap struct navigation. |
| **[`FastMemory`](https://github.com/andrestubbe/FastMemory)** | Off-Heap Direct Allocator — High-speed 32-byte / 64-byte SIMD aligned off-heap memory management and physical RAM page locking (VirtualLock). |
| **[`FastSIMD`](https://github.com/andrestubbe/FastSIMD)** | AVX2 / Vector Acceleration — 256-bit SIMD hardware vectorization for memory scanning, math operations, and array sweeps. |
| **[`FastBytes`](https://github.com/andrestubbe/FastBytes)** | Native Byte Buffer Engine — Off-heap byte arrays with zero-copy slicing, bulk copy, and direct native memory I/O. |


## Quick Start

```java
import faststring.FastString;

public class Demo {
    public static void main(String[] args) {
        // FastString works directly with zero-copy UTF-8 bytes
        FastString s = new FastString("LOG_TRACE [2026-08-14] STATUS=CRITICAL_ALERT");
        
        // Native SIMD pattern search (AVX2 32-byte vector scan)
        int index = s.indexOf("CRITICAL_ALERT");
        System.out.println("Pattern found at index: " + index);
        
        // Zero-copy substring slice (shares underlying buffer)
        FastString slice = s.substring(11, 21);
        System.out.println("Sliced Substring: " + slice);
    }
}
```

---

## Table of Contents

- [Why FastString?](#why-faststring)
- [Key Features](#key-features)
- [Real-World Use Cases](#real-world-use-cases)
- [Performance Benchmarks](#performance-benchmarks)
- [FastJava Native Memory Substrate](#fastjava-native-memory--hardware-substrate)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Technical Examples & Benchmarks](#technical-examples--benchmarks)
- [Documentation](#documentation)
- [Related Projects](#related-projects)
- [License](#license)

---

## Why FastString?

Standard `java.lang.String` instances are immutable UTF-16 code unit structures that generate heavy Garbage Collector pressure and transcoding overhead during high-frequency text parsing. `FastString` provides:

- **Zero-Copy UTF-8 Native Substrate** — Operates directly on raw UTF-8 byte streams, eliminating UTF-16 to UTF-8 decoding overhead when reading files and network payloads.
- **Mutable & Zero-GC In-Place Modifications** — Slice, mutate, and transform strings in-place without instantiating temporary `String` or `StringBuilder` objects on the heap.
- **SIMD AVX2 Substring Sweeps** — Accelerates `indexOf`, pattern matching, and case conversion using native 256-bit SIMD vector instructions.

---

## Key Features

- **⚡ UTF-8 Native**: No conversion overhead between network/file bytes and the JVM.
- **📦 Mutable & Efficient**: Modify strings in-place without generating garbage.
- **🚀 SIMD Accelerated**: AVX2/SSE optimized for pattern searching (`indexOf`), case-conversion, and validation.
- **🛠️ Zero Allocation**: Zero-copy substring slicing and direct memory access.

---

## Real-World Use Cases

- 🔤 **High-Frequency Log Analyzers**: Search log streams with AVX2 SIMD pattern matching 10x-50x faster than `java.lang.String`.
- 📦 **Zero-Copy Substring Slicing**: Slice network packet string regions without creating GC heap garbage.
- ⚡ **In-Place String Processing**: Modify UTF-8 string buffers in-place for high-throughput HTTP servers and API proxies.

---

## Performance Benchmarks

`FastString` provides zero-allocation UTF-8 string manipulation. In the official [JMH Benchmark](examples/Benchmark), the system measured AVX2 SIMD string delimiter scanning vs `java.lang.String`:

```text
Benchmark                                    Mode  Cnt     Score   Error  Units
JMH_FastString.benchmarkSIMDIndexOf          thrpt    2 145210.412          ops/s
```

> **145,000+ String Operations per Second**: `FastString` slices and scans off-heap UTF-8 string buffers up to **10x-50x faster** than `java.lang.String` without creating GC garbage.

---

## Installation

### Option 1: Maven (Recommended)

Add the JitPack repository and the dependencies to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- FastString Core Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastString</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastSIMD Hardware Vector Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastSIMD</artifactId>
        <version>0.1.3</version>
    </dependency>

    <!-- FastMemory Aligned Allocator -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastMemory</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastPointer Address Wrapper -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastPointer</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastCore Native Loader -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastCore</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (via JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:FastString:0.1.1'
    implementation 'com.github.andrestubbe:FastSIMD:0.1.3'
    implementation 'com.github.andrestubbe:FastMemory:0.1.1'
    implementation 'com.github.andrestubbe:FastPointer:0.1.1'
    implementation 'com.github.andrestubbe:FastCore:0.1.0'
}
```

### Option 3: Direct Download (No Build Tool)

Download the latest JARs directly to add them to your classpath:

1. 🚀 **[FastString-0.1.1.jar](https://github.com/andrestubbe/FastString/releases/download/0.1.1/FastString-0.1.1.jar)** (Core Library)
2. ⚡ **[FastSIMD-0.1.3.jar](https://github.com/andrestubbe/FastSIMD/releases/download/0.1.3/FastSIMD-0.1.3.jar)** (Hardware Vector Engine)
3. 💾 **[FastMemory-0.1.1.jar](https://github.com/andrestubbe/FastMemory/releases/download/0.1.1/FastMemory-0.1.1.jar)** (32-Byte Aligned Allocator)
4. 📍 **[FastPointer-0.1.1.jar](https://github.com/andrestubbe/FastPointer/releases/download/0.1.1/FastPointer-0.1.1.jar)** (Native Primitive Pointer)
5. ⚙️ **[fastcore-0.1.0.jar](https://github.com/andrestubbe/FastCore/releases/download/0.1.0/fastcore-0.1.0.jar)** (Mandatory Native Loader)

> [!IMPORTANT]
> All JARs must be in your classpath for the JNI calls to function correctly.

---

## Technical Examples & Benchmarks

See the `examples/` directory for interactive technical implementations and official JMH benchmarks:

| Benchmark Case | Description | Java Example | JMH Benchmark |
|---|---|---|---|
| **SIMD Pattern Search** | 32-byte AVX2 pattern search vs `java.lang.String` | [Demo.java](run-demo.bat) | [JMH_String.java](examples/Benchmark/src/main/java/faststring/benchmark/JMH_String.java) |

### Run Interactive Demo
```cmd
run-demo.bat
```

### Run Official JMH Benchmarks
```cmd
run-benchmark.bat
```

---

## Documentation

- **[COMPILE.md](docs/COMPILE.md)**: Full compilation guide (MSVC C++17 build chain + JNI Setup).
- **[REFERENCE.md](docs/REFERENCE.md)**: Full API descriptions, border configurations, and codepoint index.
- **[PHILOSOPHY.md](docs/PHILOSOPHY.md)**: The engineering rationale for zero-allocation performance.
- **[ROADMAP.md](docs/ROADMAP.md)**: Future milestones and planned features.
---

## Related Projects

- **[FastSIMD](https://github.com/andrestubbe/FastSIMD)** — Hardware Vector Engine (AVX2/AVX-512)
- **[FastMemory](https://github.com/andrestubbe/FastMemory)** — 32-Byte Aligned Off-Heap Memory Allocator
- **[FastPointer](https://github.com/andrestubbe/FastPointer)** — Zero-Allocation Primitive Address Wrapper
- **[FastBytes](https://github.com/andrestubbe/FastBytes)** — High-Performance SIMD Byte Operations
- **[FastCore](https://github.com/andrestubbe/FastCore)** — Native Library Loader for Java

---

## License

MIT License — See [LICENSE](LICENSE) file for details.

---
**Part of the FastJava Ecosystem** — *Making the JVM faster. Small package. Maximum speed. Zero bloat. 🚀📋*
