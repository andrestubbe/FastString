# FastString — High-performance SIMD UTF-8 String for Java v0.1.0

**Mutable, zero-allocation UTF-8 string implementation with SIMD-accelerated operations.**

[![Build](https://img.shields.io/github/actions/workflow/status/andrestubbe/FastString/maven.yml?branch=main)](https://github.com/andrestubbe/FastString/actions)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![JitPack](https://jitpack.io/v/andrestubbe/FastString.svg)](https://jitpack.io/#andrestubbe/FastString)

---

FastString replaces standard Java `String` for high-frequency processing, bypassing UTF-16 overhead and garbage collection pressure.

```java
// Quick Start — Example
import faststring.FastString;

public class Demo {
    public static void main(String[] args) {
        FastString s = new FastString("Hello FastJava!");
        s.toUpperCase(); // Native SIMD accelerated
        System.out.println(s);
    }
}
```

---

## Installation

FastString requires `FastCore` for native library loading.

### Maven (JitPack)
```xml
<dependencies>
    <dependency>
        <groupId>io.github.andrestubbe</groupId>
        <artifactId>faststring</artifactId>
        <version>0.1.0</version>
    </dependency>
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

---

## License
MIT License — See [LICENSE](LICENSE) for details.

---
**Made with ⚡ by Andre Stubbe**
