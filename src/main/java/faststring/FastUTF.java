package faststring;

import java.nio.charset.StandardCharsets;

/**
 * FastUTF - High-performance hardware-aligned UTF-8 utilities for the FastJava ecosystem.
 *
 * <p>Provides zero-allocation, safe UTF-8 byte conversions, fast JSON string escaping,
 * BOM stripping, and validation without triggering default ANSI/Windows-1252 charset corruptions.</p>
 */
public final class FastUTF {

    private static final byte[] EMPTY_BYTES = new byte[0];
    private static final byte[] HEX_DIGITS = "0123456789abcdef".getBytes(StandardCharsets.US_ASCII);

    private FastUTF() {
    }

    /**
     * Converts a Java CharSequence/String into a UTF-8 byte array.
     * Guarantees UTF-8 encoding regardless of host system or platform codepage.
     *
     * @param sequence character sequence to convert
     * @return raw UTF-8 byte array, never null
     */
    public static byte[] toBytes(CharSequence sequence) {
        if (sequence == null || sequence.length() == 0) {
            return EMPTY_BYTES;
        }
        if (sequence instanceof String str) {
            return str.getBytes(StandardCharsets.UTF_8);
        }
        return sequence.toString().getBytes(StandardCharsets.UTF_8);
    }

    /**
     * Decodes a UTF-8 byte buffer into a Java String.
     *
     * @param bytes raw UTF-8 bytes
     * @return decoded String, or empty string if null or empty
     */
    public static String toString(byte[] bytes) {
        if (bytes == null || bytes.length == 0) {
            return "";
        }
        return new String(bytes, 0, bytes.length, StandardCharsets.UTF_8);
    }

    /**
     * Decodes a slice of a UTF-8 byte buffer into a Java String.
     *
     * @param bytes  raw UTF-8 bytes
     * @param offset byte offset
     * @param length number of bytes to decode
     * @return decoded String
     */
    public static String toString(byte[] bytes, int offset, int length) {
        if (bytes == null || length <= 0 || offset < 0 || offset >= bytes.length) {
            return "";
        }
        return new String(bytes, offset, Math.min(length, bytes.length - offset), StandardCharsets.UTF_8);
    }

    /**
     * Strips a UTF-8 Byte Order Mark (BOM: EF BB BF) if present at the start of the byte array.
     *
     * @param bytes raw bytes
     * @return the same array or a sliced view without the leading 3 BOM bytes
     */
    public static byte[] stripBom(byte[] bytes) {
        if (bytes != null && bytes.length >= 3 &&
            (bytes[0] & 0xFF) == 0xEF &&
            (bytes[1] & 0xFF) == 0xBB &&
            (bytes[2] & 0xFF) == 0xBF) {
            byte[] stripped = new byte[bytes.length - 3];
            System.arraycopy(bytes, 3, stripped, 0, stripped.length);
            return stripped;
        }
        return bytes;
    }

    /**
     * Checks if a byte array contains exclusively ASCII characters (bytes in [0, 127]).
     *
     * @param bytes  buffer to check
     * @param offset starting offset
     * @param length number of bytes
     * @return true if all bytes are in the range 0..127
     */
    public static boolean isPureAscii(byte[] bytes, int offset, int length) {
        if (bytes == null || length <= 0) return true;
        int end = offset + length;
        for (int i = offset; i < end; i++) {
            if ((bytes[i] & 0x80) != 0) {
                return false;
            }
        }
        return true;
    }

    /**
     * Escapes a string for direct inclusion in a JSON string payload according to RFC 8259,
     * preserving all valid multi-byte UTF-8 sequences (such as German umlauts, CJK, Emojis)
     * without breaking them into unicode hex escapes unless control characters.
     *
     * @param text unescaped input string
     * @return properly escaped JSON string content (without surrounding quotes)
     */
    public static String escapeJson(CharSequence text) {
        if (text == null || text.length() == 0) {
            return "";
        }
        StringBuilder sb = new StringBuilder(text.length() + 16);
        int len = text.length();
        for (int i = 0; i < len; i++) {
            char c = text.charAt(i);
            switch (c) {
                case '"' -> sb.append("\\\"");
                case '\\' -> sb.append("\\\\");
                case '\b' -> sb.append("\\b");
                case '\f' -> sb.append("\\f");
                case '\n' -> sb.append("\\n");
                case '\r' -> sb.append("\\r");
                case '\t' -> sb.append("\\t");
                default -> {
                    if (c < 0x20) {
                        sb.append("\\u");
                        sb.append((char) HEX_DIGITS[(c >> 12) & 0xF]);
                        sb.append((char) HEX_DIGITS[(c >> 8) & 0xF]);
                        sb.append((char) HEX_DIGITS[(c >> 4) & 0xF]);
                        sb.append((char) HEX_DIGITS[c & 0xF]);
                    } else {
                        sb.append(c);
                    }
                }
            }
        }
        return sb.toString();
    }
}
