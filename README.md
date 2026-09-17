# FastString 0.1.1 [ALPHA-2026-08] — High-performance SIMD UTF-8 String for Java

[![Status](https://img.shields.io/badge/status-0.1.1-brightgreen.svg)](https://github.com/andrestubbe/FastString/releases/tag/0.1.1)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-ready-green.svg)](https://jitpack.io/#andrestubbe/FastString)

---

**⚡ Mutable, zero-allocation UTF-8 string implementation with SIMD-accelerated operations. Bypasses Java String overhead for elite performance.**

**FastString** is designed for high-frequency data processing where standard `java.lang.String` becomes a bottleneck due to UTF-16 encoding and excessive garbage collection.

[**Watch the Demo**](https://www.youtube.com/watch?v=BZsqQl7WqWk) | Watch JMH Benchmark (YouTube)

[![FastString Showcase](docs/screenshot.png)](https://www.youtube.com/watch?v=BZsqQl7WqWk)

---

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

- [Quick Start](#quick-start)
- [Why FastString?](#why-faststring)
- [Key Features](#key-features)
- [Real-World Use Cases](#real-world-use-cases)
- [Performance Benchmarks](#performance-benchmarks)
- [API Quick Reference](#api-quick-reference)
- [Technical Demos & Benchmarks](#technical-demos--benchmarks)
- [Installation](#installation)
- [Documentation](#documentation)
- [Related Projects](#related-projects)
- [License](#license)

---

## Why FastString?

Standard `java.lang.String` instances are immutable UTF-16 code unit structures that generate heavy Garbage Collector pressure and transcoding overhead during high-frequency text parsing. `FastString` provides:

- **Zero-Copy UTF-8 Native Substrate** — Operates directly on raw UTF-8 byte streams, eliminating UTF-16 to UTF-8 decoding overhead when reading files and network payloads.
- **Mutable & Zero-GC In-Place Modifications** — Slice, mutate, and transform strings in-place without instantiating temporary `String` or `StringBuilder` objects on the heap.
- **SIMD AVX2 Substring Sweeps** — Accelerates `indexOf`, pattern matching, and case conversion using native 256-bit SIMD vector instructions.

FastString replaces immutable UTF-16 heap objects with high-throughput native UTF-8 string buffers:

| Feature | Standard `java.lang.String` | Apache Commons `StringUtils` | FastString |
|:---|:---|:---|:---|
| **Encoding Format** | UTF-16 / Compact Latin-1 | UTF-16 / String wrappers | **Raw Native UTF-8 Bytes** |
| **Substring Slicing** | Copies underlying byte array| Allocates new String | **Zero-Copy Buffer View (0 Alloc)**|
| **Pattern Search (`indexOf`)**| Scalar character loop | Scalar character loop | **AVX2 SIMD (32 Bytes / Cycle)** |
| **Mutations (Append/Case)** | Allocates new String object | Allocates new String | **In-Place Vectorized Mutation** |
| **Garbage Collector Churn** | High object creation | High object creation | **0 Heap Allocations (Off-Heap)** |
| **Dependencies** | JDK standard lib | External JAR (~600 KB) | **Pure Java 17+ backed by FastCore** |

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
Benchmark.testFastStringIndexOf              thrpt    3 145210.412          ops/s
```

> **145,000+ String Operations per Second**: `FastString` slices and scans off-heap UTF-8 string buffers up to **10x-50x faster** than `java.lang.String` without creating GC garbage.

---

## API Quick Reference

| Method | Return Type | Description | Docs |
|---|---|---|---|
| `new FastString(String str)` | `FastString` | Creates a mutable UTF-8 FastString from a Java string. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `indexOf(String str)` | `int` | SIMD AVX2-accelerated 256-bit vector scan for substring. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `substring(int begin, int end)` | `FastString` | Zero-copy substring slice sharing the underlying buffer. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `append(String str)` | `FastString` | Appends text in-place without JVM heap reallocation. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `toLowerCase()` | `FastString` | Converts characters to lowercase via SIMD vectorization. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `toUpperCase()` | `FastString` | Converts characters to uppercase via SIMD vectorization. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `getBytesFast()` | `byte[]` | Direct UTF-8 byte retrieval via JNI Critical Sections. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |
| `dispose()` | `void` | Deterministically releases native memory backing storage. | [Reference](docs/REFERENCE.md#2-class-faststringfaststring) |

---

## Technical Demos & Benchmarks

| Case | Java Example | Launcher | Description |
|---|---|---|---|
| **SIMD Pattern Search & Slicing** | [Demo.java](examples/Demo/src/main/java/faststring/Demo.java) | `run-demo.bat` | Interactive demo demonstrating AVX2 pattern search, zero-copy slicing, and throughput comparison against `java.lang.String`. |
| **JMH Microbenchmark Suite** | [Benchmark.java](examples/Benchmark/src/main/java/faststring/benchmark/Benchmark.java) | `run-benchmark.bat` | Official JMH throughput benchmark measuring SIMD `indexOf()` vs standard JDK string search. |

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
**Part of the FastJava Ecosystem** — *Making the JVM faster.* 🚀
