#ifndef FASTSTRING_H
#define FASTSTRING_H

#include <jni.h>
#include <cstdint>
#include <cstddef>

// Version info
#define FASTSTRING_VERSION_MAJOR 1
#define FASTSTRING_VERSION_MINOR 0
#define FASTSTRING_VERSION_PATCH 0

// SIMD alignment for AVX2 (32 bytes)
#define FASTSTRING_SIMD_ALIGN 32

// Initial capacity and growth factor
#define FASTSTRING_INITIAL_CAPACITY 16
#define FASTSTRING_GROWTH_FACTOR 2

namespace faststring {

/**
 * FastString C++ implementation class.
 * Mutable UTF-8 string with SIMD-accelerated operations.
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
    const char* data() const { return buffer; }  // Raw buffer access
    
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
    static bool hasSSE42();
    static bool hasAVX2();
    static bool hasAVX512();
    void setSimdLevel(int level);  // 0=AUTO, 1=AVX2, 2=SSE4, 3=NONE
    
private:
    char* buffer;           // UTF-8 byte buffer
    size_t bufSize;         // Total buffer size
    size_t dataLen;         // Current data length in bytes
    size_t charCount;       // Cached UTF-8 character count (-1 if dirty)
    int simdOverride;       // SIMD level override (0=AUTO, 1=AVX2, 2=SSE4, 3=NONE)
    
    // Internal helpers
    void grow(size_t minCapacity);
    void resize(size_t newCapacity);
    size_t countUtf8Chars() const;
    static size_t utf8CharLength(uint8_t firstByte);
    
    // SIMD implementations
    size_t indexOfSSE42(const char* str, size_t fromIndex) const;
    size_t indexOfAVX2(const char* str, size_t fromIndex) const;
    void toLowerSSE42(char* dest, const char* src, size_t len) const;
    void toLowerAVX2(char* dest, const char* src, size_t len) const;
    void toUpperSSE42(char* dest, const char* src, size_t len) const;
    void toUpperAVX2(char* dest, const char* src, size_t len) const;
};

// Global handle map for JNI
class FastStringRegistry {
public:
    static jlong registerString(FastString* str);
    static FastString* getString(jlong handle);
    static void unregisterString(jlong handle);
    static void cleanup();
};

} // namespace faststring

// JNI function declarations
extern "C" {

// Construction
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeCreate(JNIEnv* env, jobject obj, jint capacity);
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromString(JNIEnv* env, jobject obj, jstring str);
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromBytes(JNIEnv* env, jobject obj, jbyteArray bytes, jint offset, jint length);
JNIEXPORT void JNICALL Java_faststring_FastString_nativeDestroy(JNIEnv* env, jobject obj, jlong handle);

// Core operations
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

} // extern "C"

#endif // FASTSTRING_H
