# Changelog

## [0.1.2] - 2026-10-19

### Added
- **`faststring.FastUTF` Utility Class**:
  - Memory- and hardware-optimized UTF-8 encoder/decoder eliminating JVM/OS default codepage (Windows-1252 ANSI) corruption hazards.
  - `FastUTF.toBytes(CharSequence)`: Reliable UTF-8 encoding with static zero-allocation `EMPTY_BYTES` fallback.
  - `FastUTF.toString(byte[])` & `FastUTF.toString(byte[], offset, length)`: Null- and range-safe UTF-8 string decoding.
  - `FastUTF.stripBom(byte[])`: Automatic detection and removal of leading UTF-8 Byte Order Marks (`0xEF, 0xBB, 0xBF`).
  - `FastUTF.escapeJson(CharSequence)`: RFC 8259-compliant JSON string escaping preserving multibyte sequences (German umlauts, CJK, Emojis) without bloated `\uXXXX` expansion.
  - `FastUTF.isPureAscii(byte[], offset, length)`: Fast-path 7-bit ASCII verification skipping multibyte decoding.

---

## [0.1.1] - 2026-08-30

### Changed
- Standardized FastJava ecosystem metadata, build scripts, and multi-architecture documentation.

---

## [0.1.0] - 2026-05-23

### Added
- Initial release
- Standardized FastJava ecosystem module