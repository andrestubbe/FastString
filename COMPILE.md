# Building FastString

Guide for developers building FastString from source.

## Prerequisites

- **JDK 17+** — [Download](https://adoptium.net/)
- **Maven 3.9+** — [Download](https://maven.apache.org/download.cgi)
- **Visual Studio 2019 or 2022** — [Download](https://visualstudio.microsoft.com/downloads/)
  - Required: "Desktop development with C++" workload
  - Required: Windows 10/11 SDK

## Quick Build

```bash
# Clone repository
git clone https://github.com/andrestubbe/faststring.git
cd faststring

# Build native DLL + Java
compile.bat
mvn clean package
```

## Build Commands

| Command | Purpose |
|---------|---------|
| `compile.bat` | Build native DLL (requires Visual Studio) |
| `mvn clean compile` | Compile Java only |
| `mvn clean package` | Build JAR with DLL |
| `mvn clean package -DskipTests` | Fast build |
| `mvn exec:java` | Run benchmarks |

## Testing

```bash
# Run all tests
mvn test

# Run benchmarks only
mvn exec:java

# Run specific test
mvn test -Dtest=FastStringTest
```

## Native Code Structure

```
native/
├── faststring.cpp          # Main JNI implementation
├── faststring.h            # JNI header with SIMD declarations
```

## Troubleshooting

**"Cannot find cl.exe"** — Run in "Developer Command Prompt for VS 2019/2022"

**"UnsatisfiedLinkError: no faststring in java.library.path"** — Run `compile.bat` first

**"AVX2 not supported"** — Code falls back to SSE4.2 automatically

## Release Process

Releases are published via JitPack when tags are pushed:

```bash
# 1. Update version in pom.xml
# 2. Commit all changes
git add .
git commit -m "Prepare v1.0.0"

# 3. Create and push tag
git tag -a v1.0.0 -m "FastString 1.0.0"
git push origin v1.0.0
```

JitPack will automatically build and publish:
```xml
<dependency>
    <groupId>com.github.andrestubbe</groupId>
    <artifactId>faststring</artifactId>
    <version>v1.0.0</version>
</dependency>
```
