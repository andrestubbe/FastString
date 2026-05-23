# FastString — High-performance SIMD UTF-8 String for Java v0.1.0 [ALPHA] - v0.1.0
**Mutable, zero-allocation UTF-8 string implementation with SIMD-accelerated operations. Bypasses Java String overhead for elite performance.**

[![Status](https://img.shields.io/badge/status-v0.1.0-brightgreen.svg)](https://github.com/andrestubbe/FastString/releases/tag/v0.1.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-ready-green.svg)](https://jitpack.io/#andrestubbe)

---

**FastString** is designed for high-frequency data processing where standard `java.lang.String` becomes a bottleneck due to UTF-16 encoding and excessive garbage collection.

```java
// Quick Start — Example
import faststring.FastString;

public class Demo {
    public static void main(String[] args) {
        // FastString works directly with UTF-8 bytes
        FastString s = new FastString("High Performance!");
        s.toUpperCase(); // Native SIMD accelerated (AVX2)
        System.out.println(s);
    }
}
```

---

## Table of Contents
- [Features](#features)
- [Installation](#installation)
- [License](#license)
- [Related Projects](#related-projects)

---

## Features
- **⚡ UTF-8 Native**: No conversion overhead between network/file bytes and the JVM.
- **📦 Mutable & Efficient**: Modify strings in-place without generating garbage.
- **🚀 SIMD Accelerated**: AVX2/SSE optimized for searching, case-conversion, and validation.
- **🛠️ Zero Allocation**: Designed for 120 FPS UI and high-throughput backend services.

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
    <!-- FastString Library -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>faststring</artifactId>
        <version>v0.1.0</version>
    </dependency>

    <!-- FastCore (Required Native Loader) -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>v0.1.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (via JitPack)
```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:faststring:v0.1.0'
    implementation 'com.github.andrestubbe:fastcore:v0.1.0'
}
```

### Option 3: Direct Download (No Build Tool)
Download the latest JARs directly to add them to your classpath:

1. 📦 **[faststring-v0.1.0.jar](https://github.com/andrestubbe/FastString/releases/download/v0.1.0/faststring-v0.1.0.jar)** (The Core Library)
2. ⚙️ **[fastcore-v0.1.0.jar](https://github.com/andrestubbe/FastCore/releases/download/v0.1.0/fastcore-v0.1.0.jar)** (The Mandatory Native Loader)

> [!IMPORTANT]
> All JARs must be in your classpath for the native JNI calls to function correctly.


## License
MIT License — See [LICENSE](LICENSE) for details.

---

## Related Projects
- [FastCore](https://github.com/andrestubbe/FastCore) — Native Library Loader
- [FastBytes](https://github.com/andrestubbe/FastBytes) — SIMD-accelerated Byte Operations
- [FastJSON](https://github.com/andrestubbe/FastJSON) — High-speed JSON parsing

---
**Made with ⚡ by Andre Stubbe**

