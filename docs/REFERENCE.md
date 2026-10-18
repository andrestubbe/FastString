# FastString Reference Manual

FastString is the high-performance, mutable UTF-8 string and SIMD pattern scanning substrate of the FastJava ecosystem.

---

## 1. Core Vocabulary

* **UTF-8 Native Substrate**: Operates directly on raw UTF-8 byte streams, avoiding UTF-16 decoding overhead.
* **In-Place Mutation**: Modifies and extends text in pre-allocated off-heap or native buffers without heap garbage.
* **SIMD Scanning**: Employs AVX2 (256-bit) and SSE4.2 (128-bit) vector instructions for string delimiter sweeps and pattern searches.
* **Zero-Copy Slicing**: Substrings share the parent string buffer, avoiding memory duplication.

---

## 2. Class: aststring.FastString

### Constructors
- public FastString(String str)  
  Creates a FastString initialized with the content of a standard Java string.
- public FastString(int initialCapacity)  
  Allocates an empty FastString with a pre-configured byte capacity.
- public FastString(byte[] bytes, int offset, int length)  
  Creates a FastString directly from an existing UTF-8 byte array.

### Inspection & Character Access
- public int length()  
  Returns the character count (Unicode code units).
- public int byteLength()  
  Returns the total UTF-8 byte count of the buffer.
- public boolean isEmpty()  
  Returns 	rue if the string contains zero characters.
- public char charAt(int index)  
  Returns the UTF-16 character at the specified index.
- public int codePointAt(int index)  
  Returns the 32-bit Unicode code point at the specified character index.

### Search & Pattern Matching (SIMD Accelerated)
- public int indexOf(String str)  
  Scans for the first occurrence of the target pattern using AVX2 vector sweeps.
- public int indexOf(String str, int fromIndex)  
  Scans for pattern starting from a specific character index.
- public int lastIndexOf(String str)  
  Scans backwards for the last occurrence of the target pattern.
- public boolean contains(String str)  
  Returns 	rue if the target sequence is found within the string.
- public boolean startsWith(String prefix)  
  Tests if this string begins with the specified prefix.
- public boolean endsWith(String suffix)  
  Tests if this string ends with the specified suffix.

### Modification & Appends (Zero-Allocation)
- public FastString append(String str)  
  Appends text to the mutable buffer in-place.
- public FastString append(FastString other)  
  Appends another FastString buffer.
- public FastString appendBatch(String s1, String s2)  
  Appends multiple strings in a single JNI call to minimize boundary overhead.
- public FastString replace(String target, String replacement)  
  Replaces occurrences of target with replacement in-place.
- public FastString trim()  
  Removes leading and trailing whitespace.
- public FastString reverse()  
  Reverses the character sequence in-place.
- public void clear()  
  Resets the buffer length to zero without deallocating backing storage.

### Slicing & Casing
- public FastString substring(int beginIndex)  
  Returns a zero-copy substring starting at eginIndex.
- public FastString substring(int beginIndex, int endIndex)  
  Returns a zero-copy substring slice between eginIndex and endIndex.
- public FastString toLowerCase()  
  Converts characters to lowercase using SIMD acceleration.
- public FastString toUpperCase()  
  Converts characters to uppercase using SIMD acceleration.

### Memory & Interop
- public byte[] getBytes()  
  Returns the UTF-8 encoded byte array.
- public byte[] getBytesFast()  
  Returns UTF-8 bytes using direct JNI Critical Sections for maximum throughput.
- public void dispose()  
  Explicitly frees the underlying native buffer ahead of garbage collection.

---

## 3. Class: `faststring.FastUTF`

Hardware- and memory-optimized UTF-8 utilities for platform-independent string processing, JSON escaping, and BOM stripping.

### Static Methods

| Method Signature | Return Type | Description |
|:---|:---|:---|
| `toBytes(CharSequence sequence)` | `byte[]` | Encodes sequence to pure UTF-8 bytes without default codepage fallback. Returns `EMPTY_BYTES` for null/empty input. |
| `toString(byte[] bytes)` | `String` | Null-safe decoding of full UTF-8 byte buffer into Java String. |
| `toString(byte[] bytes, int offset, int length)` | `String` | Range- and bounds-checked UTF-8 decoding. |
| `stripBom(byte[] bytes)` | `byte[]` | Detects and strips leading UTF-8 Byte Order Mark (`0xEF, 0xBB, 0xBF`). |
| `escapeJson(CharSequence text)` | `String` | RFC 8259-compliant JSON escaping preserving multilingual UTF-8 characters without `\uXXXX` expansion. |
| `isPureAscii(byte[] bytes, int offset, int length)` | `boolean` | Verifies whether byte range is pure 7-bit ASCII (`0x00`-`0x7F`) to skip multibyte decoding. |

---

## 4. Platform & Hardware Vectorization

| Feature | Requirement | Description |
|---|---|---|
| **AVX2 Engine** | x86_64 CPU with AVX2 support | 32-byte 256-bit SIMD vector search & casing |
| **SSE4.2 Engine** | x86_64 CPU with SSE4.2 support | 16-byte 128-bit vector fallback |
| **Scalar Engine** | Any CPU | Standard fallback for non-vectorized hardware |
| **Platform** | Windows 10/11 (x64) | Native C++17 JNI runtime (`faststring.dll`) |

---

**Part of the FastJava Ecosystem** — *Making the JVM faster.* 🚀
