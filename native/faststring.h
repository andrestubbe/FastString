/**
 * @file faststring.h
 * @brief FastString JNI Header - SIMD-accelerated string operations for Java
 *
 * @details High-performance mutable UTF-8 string implementation with:
 * - AVX2 SIMD acceleration for search and case conversion
 * - SSE4.2 fallback for older CPUs
 * - Zero-copy substring views
 * - Automatic memory management with growth
 *
 * @par Features
 * - Mutable operations: append, insert, replace
 * - Search: indexOf, lastIndexOf, contains (SIMD optimized)
 * - Case conversion: toLowerCase, toUpperCase (SIMD optimized)
 * - Comparison: equals, compareTo, hashCode
 * - UTF-8 support: codePointAt, proper character counting
 *
 * @par SIMD Optimization
 * - indexOf: ~6x faster with AVX2
 * - case conversion: ~8x faster with AVX2
 * - Auto-detection at runtime with scalar fallback
 *
 * @par Performance Tips
 * - Use ensureCapacity() when size is known
 * - toLowerCaseInPlace() avoids allocation
 * - substring() is zero-copy (shares buffer)
 *
 * @author FastJava Team
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef FASTSTRING_H
#define FASTSTRING_H

#include <jni.h>
#include <cstdint>
#include <cstddef>

/** @defgroup Constants Version and Configuration
 *  @brief Library version and compile-time settings
 *  @{ */

#define FASTSTRING_VERSION_MAJOR 1   /**< Major version */
#define FASTSTRING_VERSION_MINOR 0   /**< Minor version */
#define FASTSTRING_VERSION_PATCH 0   /**< Patch version */

#define FASTSTRING_SIMD_ALIGN 32     /**< AVX2 alignment requirement (32 bytes) */
#define FASTSTRING_INITIAL_CAPACITY 16  /**< Default buffer capacity */
#define FASTSTRING_GROWTH_FACTOR 2   /**< Buffer growth multiplier */

/** @} */

namespace faststring {

/** @defgroup CoreClass FastString Core Class
 *  @brief Main mutable string implementation
 *  @{ */

/**
 * @class FastString
 * @brief Mutable UTF-8 string with SIMD-accelerated operations
 * @details High-performance string class optimized for:
 * - Frequent modifications (append, replace)
 * - Search operations (indexOf, contains)
 * - Case conversion (toLowerCase, toUpperCase)
 *
 * @par Memory Management
 * - Automatic buffer growth (2x strategy)
 * - Manual shrinkToFit() to release excess capacity
 * - Copy-on-write for substring views
 *
 * @par Thread Safety
 * - NOT thread-safe - external synchronization required
 * - Each FastString instance is independent
 */
class FastString {
public:
    // Constructors
    FastString(int initialCapacity = FASTSTRING_INITIAL_CAPACITY);
    FastString(const char* utf8Data, size_t length);
    FastString(const FastString& other);
    FastString(FastString&& other) noexcept;
    ~FastString();
    
    // Assignment
    FastString& operator=(const FastString& other);
    FastString& operator=(FastString&& other) noexcept;
    
    // Core properties
    size_t length() const;           // UTF-8 code points
    size_t byteLength() const;         // Raw bytes
    bool isEmpty() const;
    void clear();
    
    // Buffer management
    void ensureCapacity(size_t minCapacity);
    size_t capacity() const;
    void shrinkToFit();
    
    // Append operations (mutable)
    FastString& append(const char* str);
    FastString& append(const char* str, size_t len);
    FastString& append(char c);
    FastString& append(const FastString& other);
    
    // Substring (zero-copy view)
    FastString substring(size_t beginIndex) const;
    FastString substring(size_t beginIndex, size_t endIndex) const;
    
    // Search operations (SIMD accelerated)
    size_t indexOf(const char* str, size_t fromIndex = 0) const;
    size_t lastIndexOf(const char* str) const;
    bool contains(const char* str) const;
    bool startsWith(const char* prefix) const;
    bool endsWith(const char* suffix) const;
    
    // Case conversion (SIMD accelerated)
    FastString toLowerCase() const;
    FastString toUpperCase() const;
    void toLowerCaseInPlace();
    void toUpperCaseInPlace();
    
    // Comparison
    bool equals(const FastString& other) const;
    int compareTo(const FastString& other) const;
    uint32_t hashCode() const;
    
    // Character access
    char charAt(size_t index) const;           // Byte at position
    uint32_t codePointAt(size_t index) const;  // Full UTF-8 code point
    
    // Modification
    FastString& replace(char oldChar, char newChar);
    FastString& replace(const char* target, const char* replacement);
    FastString& trim();
    FastString& reverse();
    
    // Raw access
    const char* data() const;
    char* mutableData();
    const uint8_t* bytes() const;
    uint8_t* mutableBytes();
    
    // Java interop
    jstring toJString(JNIEnv* env) const;
    static FastString fromJString(JNIEnv* env, jstring str);
    
    // SIMD utilities
    /**
     * @brief Check for SSE4.2 support
     * @return true if SSE4.2 available on this CPU
     */
    static bool hasSSE42();
    
    /**
     * @brief Check for AVX2 support
     * @return true if AVX2 available on this CPU
     */
    static bool hasAVX2();
    
    /**
     * @brief Check for AVX-512 support
     * @return true if AVX-512 available on this CPU
     */
    static bool hasAVX512();
    
    /**
     * @brief Override SIMD level selection
     * @param level 0=AUTO, 1=AVX2, 2=SSE4, 3=NONE (scalar)
     * @note Useful for testing performance of different implementations
     */
    void setSimdLevel(int level);
    
private:
    char* buffer;           /**< UTF-8 byte buffer (may have slack) */
    size_t bufSize;         /**< Total allocated buffer size */
    size_t dataLen;         /**< Current string length in bytes */
    mutable size_t charCount; /**< Cached UTF-8 code point count (-1 = dirty) */
    int simdOverride;       /**< SIMD override: 0=AUTO, 1=AVX2, 2=SSE4, 3=NONE */
    
    // Internal helpers
    void grow(size_t minCapacity);         /**< Grow buffer to at least minCapacity */
    void resize(size_t newCapacity);     /**< Reallocate to exact capacity */
    size_t countUtf8Chars() const;         /**< Count UTF-8 code points */
    static size_t utf8CharLength(uint8_t firstByte); /**< Get UTF-8 char length from first byte */
    
    // SIMD implementations
    size_t indexOfSSE42(const char* str, size_t fromIndex) const;  /**< SSE4.2 indexOf */
    size_t indexOfAVX2(const char* str, size_t fromIndex) const;   /**< AVX2 indexOf */
    void toLowerSSE42(char* dest, const char* src, size_t len) const; /**< SSE4.2 toLowerCase */
    void toLowerAVX2(char* dest, const char* src, size_t len) const;  /**< AVX2 toLowerCase */
    void toUpperSSE42(char* dest, const char* src, size_t len) const; /**< SSE4.2 toUpperCase */
    void toUpperAVX2(char* dest, const char* src, size_t len) const;  /**< AVX2 toUpperCase */
};

/** @} */

/** @defgroup Registry JNI Handle Registry
 *  @brief Global handle management for JNI interop
 *  @details Maps jlong handles to FastString instances for safe
 *           JNI memory management without pointer exposure.
 *  @{ */

class FastStringRegistry {
public:
    /**
     * @brief Register a FastString and get handle
     * @param str FastString pointer to register
     * @return Unique handle for this string
     */
    static jlong registerString(FastString* str);
    
    /**
     * @brief Look up FastString from handle
     * @param handle Handle obtained from registerString
     * @return FastString pointer or nullptr if invalid
     */
    static FastString* getString(jlong handle);
    
    /**
     * @brief Unregister and delete a string
     * @param handle Handle to unregister
     */
    static void unregisterString(jlong handle);
    
    /**
     * @brief Clean up all registered strings
     * @note Call on library unload to prevent memory leaks
     */
    static void cleanup();
};

/** @} */

} // namespace faststring

extern "C" {

/** @defgroup JNI JNI Function Declarations
 *  @brief Java Native Interface exports
 *  @{ */

/** @defgroup JNI_Construction Construction
 *  @brief Create and destroy FastString instances
 *  @{ */
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeCreate(JNIEnv* env, jobject obj, jint capacity);
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromString(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromBytes(JNIEnv* env, jobject obj, jbyteArray bytes, jint offset, jint length);
JNIEXPORT void JNICALL Java_faststring_FastString_nativeDestroy(JNIEnv* env, jobject obj, jlong handle);

/** @} */

/** @defgroup JNI_Core Core Operations
 *  @brief Basic property access
 *  @{ */
JNIEXPORT jint JNICALL Java_faststring_FastString_length(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL Java_faststring_FastString_byteLength(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL Java_faststring_FastString_isEmpty(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_faststring_FastString_clear(JNIEnv* env, jobject obj);

// Append
JNIEXPORT jobject JNICALL Java_faststring_FastString_append__Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jobject JNICALL Java_faststring_FastString_append__C(JNIEnv* env, jobject obj, jchar c);
JNIEXPORT jobject JNICALL Java_faststring_FastString_append___3B(JNIEnv* env, jobject obj, jbyteArray bytes);
JNIEXPORT jobject JNICALL Java_faststring_FastString_append__Lfaststring_FastString_2(JNIEnv* env, jobject obj, jobject other);

// Substring
JNIEXPORT jobject JNICALL Java_faststring_FastString_substring__I(JNIEnv* env, jobject obj, jint beginIndex);
JNIEXPORT jobject JNICALL Java_faststring_FastString_substring__II(JNIEnv* env, jobject obj, jint beginIndex, jint endIndex);

// Search
JNIEXPORT jint JNICALL Java_faststring_FastString_indexOf__Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jint JNICALL Java_faststring_FastString_indexOf__Ljava_lang_String_2I(JNIEnv* env, jobject obj, jstring str, jint fromIndex);
JNIEXPORT jint JNICALL Java_faststring_FastString_lastIndexOf(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jboolean JNICALL Java_faststring_FastString_contains(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jboolean JNICALL Java_faststring_FastString_startsWith(JNIEnv* env, jobject obj, jstring prefix);
JNIEXPORT jboolean JNICALL Java_faststring_FastString_endsWith(JNIEnv* env, jobject obj, jstring suffix);

// Case conversion
JNIEXPORT jobject JNICALL Java_faststring_FastString_toLowerCase(JNIEnv* env, jobject obj);
JNIEXPORT jobject JNICALL Java_faststring_FastString_toUpperCase(JNIEnv* env, jobject obj);

// Comparison
JNIEXPORT jboolean JNICALL Java_faststring_FastString_equals(JNIEnv* env, jobject obj, jobject other);
JNIEXPORT jint JNICALL Java_faststring_FastString_compareTo(JNIEnv* env, jobject obj, jobject other);
JNIEXPORT jint JNICALL Java_faststring_FastString_hashCode(JNIEnv* env, jobject obj);

// Character access
JNIEXPORT jchar JNICALL Java_faststring_FastString_charAt(JNIEnv* env, jobject obj, jint index);
JNIEXPORT jint JNICALL Java_faststring_FastString_codePointAt(JNIEnv* env, jobject obj, jint index);

// Modification
JNIEXPORT jobject JNICALL Java_faststring_FastString_replace__CC(JNIEnv* env, jobject obj, jchar oldChar, jchar newChar);
JNIEXPORT jobject JNICALL Java_faststring_FastString_replace__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring target, jstring replacement);
JNIEXPORT jobject JNICALL Java_faststring_FastString_trim(JNIEnv* env, jobject obj);
JNIEXPORT jobject JNICALL Java_faststring_FastString_reverse(JNIEnv* env, jobject obj);

// Buffer management
JNIEXPORT void JNICALL Java_faststring_FastString_ensureCapacity(JNIEnv* env, jobject obj, jint minCapacity);
JNIEXPORT jbyteArray JNICALL Java_faststring_FastString_getBytes(JNIEnv* env, jobject obj);
JNIEXPORT jstring JNICALL Java_faststring_FastString_nativeToString(JNIEnv* env, jobject obj, jlong handle);

/** @} */

/** @} */

}

#endif // FASTSTRING_H
