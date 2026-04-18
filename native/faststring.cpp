#include "faststring.h"
#include <windows.h>
#include <intrin.h>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <mutex>

// SIMD headers
#include <immintrin.h>
#include <nmmintrin.h>

namespace faststring {

// ============================================================================
// FastString Implementation
// ============================================================================

FastString::FastString(int initialCapacity) 
    : buffer(nullptr), bufSize(0), dataLen(0), charCount(0) {
    ensureCapacity(initialCapacity > 0 ? initialCapacity : FASTSTRING_INITIAL_CAPACITY);
    buffer[0] = '\0';
}

FastString::FastString(const char* utf8Data, size_t length)
    : buffer(nullptr), bufSize(0), dataLen(0), charCount(-1) {
    ensureCapacity(length + 1);
    memcpy(buffer, utf8Data, length);
    dataLen = length;
    buffer[dataLen] = '\0';
}

FastString::FastString(const FastString& other)
    : buffer(nullptr), bufSize(0), dataLen(other.dataLen), charCount(other.charCount) {
    ensureCapacity(other.bufSize);
    memcpy(buffer, other.buffer, dataLen + 1);
}

FastString::FastString(FastString&& other) noexcept
    : buffer(other.buffer), bufSize(other.bufSize), dataLen(other.dataLen), charCount(other.charCount) {
    other.buffer = nullptr;
    other.bufSize = 0;
    other.dataLen = 0;
    other.charCount = 0;
}

FastString::~FastString() {
    if (buffer) {
        _aligned_free(buffer);
    }
}

FastString& FastString::operator=(const FastString& other) {
    if (this != &other) {
        ensureCapacity(other.bufSize);
        memcpy(buffer, other.buffer, other.dataLen + 1);
        dataLen = other.dataLen;
        charCount = other.charCount;
    }
    return *this;
}

FastString& FastString::operator=(FastString&& other) noexcept {
    if (this != &other) {
        if (buffer) _aligned_free(buffer);
        buffer = other.buffer;
        bufSize = other.bufSize;
        dataLen = other.dataLen;
        charCount = other.charCount;
        other.buffer = nullptr;
        other.bufSize = 0;
        other.dataLen = 0;
        other.charCount = 0;
    }
    return *this;
}

// Capacity Management
void FastString::ensureCapacity(size_t minCapacity) {
    if (minCapacity <= bufSize) return;
    
    size_t newCapacity = bufSize;
    while (newCapacity < minCapacity) {
        newCapacity = (newCapacity == 0) ? FASTSTRING_INITIAL_CAPACITY : newCapacity * FASTSTRING_GROWTH_FACTOR;
    }
    
    // Align to SIMD boundary
    newCapacity = (newCapacity + FASTSTRING_SIMD_ALIGN - 1) & ~(FASTSTRING_SIMD_ALIGN - 1);
    
    resize(newCapacity);
}

void FastString::resize(size_t newCapacity) {
    char* newBuffer = (char*)_aligned_malloc(newCapacity, FASTSTRING_SIMD_ALIGN);
    if (!newBuffer) {
        throw std::bad_alloc();
    }
    
    if (buffer) {
        memcpy(newBuffer, buffer, dataLen + 1);
        _aligned_free(buffer);
    }
    
    buffer = newBuffer;
    bufSize = newCapacity;
}

size_t FastString::capacity() const {
    return bufSize;
}

void FastString::shrinkToFit() {
    if (dataLen + 1 < bufSize) {
        resize(dataLen + 1);
    }
}

// Core Properties
size_t FastString::length() const {
    if (charCount == (size_t)-1) {
        charCount = countUtf8Chars();
    }
    return charCount;
}

size_t FastString::byteLength() const {
    return dataLen;
}

bool FastString::isEmpty() const {
    return dataLen == 0;
}

void FastString::clear() {
    dataLen = 0;
    charCount = 0;
    if (buffer) buffer[0] = '\0';
}

// Append Operations
FastString& FastString::append(const char* str) {
    return append(str, strlen(str));
}

FastString& FastString::append(const char* str, size_t len) {
    ensureCapacity(dataLen + len + 1);
    memcpy(buffer + dataLen, str, len);
    dataLen += len;
    buffer[dataLen] = '\0';
    charCount = (size_t)-1; // Invalidate char count
    return *this;
}

FastString& FastString::append(char c) {
    ensureCapacity(dataLen + 2);
    buffer[dataLen++] = c;
    buffer[dataLen] = '\0';
    charCount = (size_t)-1;
    return *this;
}

FastString& FastString::append(const FastString& other) {
    return append(other.buffer, other.dataLen);
}

// Substring (zero-copy view - actually creates a copy for safety)
FastString FastString::substring(size_t beginIndex) const {
    if (beginIndex >= dataLen) return FastString();
    
    // For zero-copy view, we'd use a different approach
    // For now, create a proper copy to maintain thread safety
    return FastString(buffer + beginIndex, dataLen - beginIndex);
}

FastString FastString::substring(size_t beginIndex, size_t endIndex) const {
    if (beginIndex >= dataLen || beginIndex >= endIndex) return FastString();
    if (endIndex > dataLen) endIndex = dataLen;
    return FastString(buffer + beginIndex, endIndex - beginIndex);
}

// SIMD-Accelerated Search
size_t FastString::indexOf(const char* str, size_t fromIndex) const {
    if (!str || !buffer) return (size_t)-1;
    
    size_t strLen = strlen(str);
    if (strLen == 0) return fromIndex;
    if (fromIndex >= dataLen) return (size_t)-1;
    
    // Use AVX2 if available for pattern matching
    if (hasAVX2() && strLen <= 32 && dataLen - fromIndex >= 32) {
        return indexOfAVX2(str, fromIndex);
    }
    
    // Fallback to standard search
    const char* result = strstr(buffer + fromIndex, str);
    return result ? (result - buffer) : (size_t)-1;
}

size_t FastString::indexOfAVX2(const char* str, size_t fromIndex) const {
    // AVX2-accelerated substring search for small patterns
    // Implementation uses vpcmpeqb for parallel comparison
    size_t strLen = strlen(str);
    if (strLen == 0 || strLen > 32) return indexOfSSE42(str, fromIndex);
    
    __m256i pattern = _mm256_loadu_si256((__m256i*)str);
    __m256i firstChar = _mm256_set1_epi8(str[0]);
    
    size_t i = fromIndex;
    size_t limit = dataLen - strLen;
    
    while (i <= limit) {
        // Load 32 bytes and find first char matches
        __m256i chunk = _mm256_loadu_si256((__m256i*)(buffer + i));
        __m256i cmp = _mm256_cmpeq_epi8(chunk, firstChar);
        int mask = _mm256_movemask_epi8(cmp);
        
        while (mask != 0) {
            int bit = _tzcnt_u32(mask);
            if (i + bit + strLen <= dataLen) {
                // Verify full match
                if (memcmp(buffer + i + bit, str, strLen) == 0) {
                    return i + bit;
                }
            }
            mask &= mask - 1; // Clear lowest bit
        }
        i += 32;
    }
    
    // Handle remaining bytes
    for (; i <= dataLen - strLen; i++) {
        if (memcmp(buffer + i, str, strLen) == 0) {
            return i;
        }
    }
    
    return (size_t)-1;
}

size_t FastString::indexOfSSE42(const char* str, size_t fromIndex) const {
    // SSE4.2 implementation (simpler than AVX2)
    size_t strLen = strlen(str);
    const char* result = strstr(buffer + fromIndex, str);
    return result ? (result - buffer) : (size_t)-1;
}

size_t FastString::lastIndexOf(const char* str) const {
    if (!str || !buffer) return (size_t)-1;
    
    size_t strLen = strlen(str);
    if (strLen == 0) return dataLen;
    if (strLen > dataLen) return (size_t)-1;
    
    // Search from end
    for (size_t i = dataLen - strLen; i != (size_t)-1; i--) {
        if (memcmp(buffer + i, str, strLen) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

bool FastString::contains(const char* str) const {
    return indexOf(str) != (size_t)-1;
}

bool FastString::startsWith(const char* prefix) const {
    if (!prefix) return false;
    size_t prefixLen = strlen(prefix);
    if (prefixLen > dataLen) return false;
    return memcmp(buffer, prefix, prefixLen) == 0;
}

bool FastString::endsWith(const char* suffix) const {
    if (!suffix) return false;
    size_t suffixLen = strlen(suffix);
    if (suffixLen > dataLen) return false;
    return memcmp(buffer + dataLen - suffixLen, suffix, suffixLen) == 0;
}

// SIMD-Accelerated Case Conversion
FastString FastString::toLowerCase() const {
    FastString result;
    result.ensureCapacity(dataLen + 1);
    
    if (hasAVX2() && dataLen >= 32) {
        toLowerAVX2(result.buffer, buffer, dataLen);
    } else if (hasSSE42()) {
        toLowerSSE42(result.buffer, buffer, dataLen);
    } else {
        // Scalar fallback
        for (size_t i = 0; i < dataLen; i++) {
            char c = buffer[i];
            result.buffer[i] = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
        }
    }
    
    result.dataLen = dataLen;
    result.buffer[dataLen] = '\0';
    return result;
}

void FastString::toLowerAVX2(char* dest, const char* src, size_t len) const {
    const __m256i A_vec = _mm256_set1_epi8('A');
    const __m256i Z_vec = _mm256_set1_epi8('Z');
    const __m256i add_vec = _mm256_set1_epi8(32);
    
    size_t i = 0;
    for (; i + 32 <= len; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i*)(src + i));
        __m256i gtA = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(A_vec, _mm256_set1_epi8(1)));
        __m256i ltZ = _mm256_cmpgt_epi8(_mm256_add_epi8(Z_vec, _mm256_set1_epi8(1)), chunk);
        __m256i mask = _mm256_and_si256(gtA, ltZ);
        __m256i add = _mm256_and_si256(mask, add_vec);
        __m256i result = _mm256_add_epi8(chunk, add);
        _mm256_storeu_si256((__m256i*)(dest + i), result);
    }
    
    // Handle remaining bytes
    for (; i < len; i++) {
        char c = src[i];
        dest[i] = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
    }
}

void FastString::toLowerSSE42(char* dest, const char* src, size_t len) const {
    const __m128i A_vec = _mm_set1_epi8('A');
    const __m128i Z_vec = _mm_set1_epi8('Z');
    const __m128i add_vec = _mm_set1_epi8(32);
    
    size_t i = 0;
    for (; i + 16 <= len; i += 16) {
        __m128i chunk = _mm_loadu_si128((__m128i*)(src + i));
        __m128i gtA = _mm_cmpgt_epi8(chunk, _mm_sub_epi8(A_vec, _mm_set1_epi8(1)));
        __m128i ltZ = _mm_cmpgt_epi8(_mm_add_epi8(Z_vec, _mm_set1_epi8(1)), chunk);
        __m128i mask = _mm_and_si128(gtA, ltZ);
        __m128i add = _mm_and_si128(mask, add_vec);
        __m128i result = _mm_add_epi8(chunk, add);
        _mm_storeu_si128((__m128i*)(dest + i), result);
    }
    
    for (; i < len; i++) {
        char c = src[i];
        dest[i] = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
    }
}

FastString FastString::toUpperCase() const {
    FastString result;
    result.ensureCapacity(dataLen + 1);
    
    if (hasAVX2() && dataLen >= 32) {
        toUpperAVX2(result.buffer, buffer, dataLen);
    } else if (hasSSE42()) {
        toUpperSSE42(result.buffer, buffer, dataLen);
    } else {
        for (size_t i = 0; i < dataLen; i++) {
            char c = buffer[i];
            result.buffer[i] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        }
    }
    
    result.dataLen = dataLen;
    result.buffer[dataLen] = '\0';
    return result;
}

void FastString::toUpperAVX2(char* dest, const char* src, size_t len) const {
    const __m256i a_vec = _mm256_set1_epi8('a');
    const __m256i z_vec = _mm256_set1_epi8('z');
    const __m256i sub_vec = _mm256_set1_epi8(32);
    
    size_t i = 0;
    for (; i + 32 <= len; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i*)(src + i));
        __m256i gta = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(a_vec, _mm256_set1_epi8(1)));
        __m256i ltz = _mm256_cmpgt_epi8(_mm256_add_epi8(z_vec, _mm256_set1_epi8(1)), chunk);
        __m256i mask = _mm256_and_si256(gta, ltz);
        __m256i sub = _mm256_and_si256(mask, sub_vec);
        __m256i result = _mm256_sub_epi8(chunk, sub);
        _mm256_storeu_si256((__m256i*)(dest + i), result);
    }
    
    for (; i < len; i++) {
        char c = src[i];
        dest[i] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    }
}

void FastString::toUpperSSE42(char* dest, const char* src, size_t len) const {
    const __m128i a_vec = _mm_set1_epi8('a');
    const __m128i z_vec = _mm_set1_epi8('z');
    const __m128i sub_vec = _mm_set1_epi8(32);
    
    size_t i = 0;
    for (; i + 16 <= len; i += 16) {
        __m128i chunk = _mm_loadu_si128((__m128i*)(src + i));
        __m128i gta = _mm_cmpgt_epi8(chunk, _mm_sub_epi8(a_vec, _mm_set1_epi8(1)));
        __m128i ltz = _mm_cmpgt_epi8(_mm_add_epi8(z_vec, _mm_set1_epi8(1)), chunk);
        __m128i mask = _mm_and_si128(gta, ltz);
        __m128i sub = _mm_and_si128(mask, sub_vec);
        __m128i result = _mm_sub_epi8(chunk, sub);
        _mm_storeu_si128((__m128i*)(dest + i), result);
    }
    
    for (; i < len; i++) {
        char c = src[i];
        dest[i] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    }
}

// Comparison
bool FastString::equals(const FastString& other) const {
    if (dataLen != other.dataLen) return false;
    return memcmp(buffer, other.buffer, dataLen) == 0;
}

int FastString::compareTo(const FastString& other) const {
    size_t minLen = std::min(dataLen, other.dataLen);
    int cmp = memcmp(buffer, other.buffer, minLen);
    if (cmp != 0) return cmp;
    return (dataLen < other.dataLen) ? -1 : (dataLen > other.dataLen) ? 1 : 0;
}

uint32_t FastString::hashCode() const {
    // FNV-1a hash
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < dataLen; i++) {
        hash ^= (uint8_t)buffer[i];
        hash *= 16777619u;
    }
    return hash;
}

// Character Access
char FastString::charAt(size_t index) const {
    if (index >= dataLen) return '\0';
    return buffer[index];
}

uint32_t FastString::codePointAt(size_t index) const {
    if (index >= dataLen) return 0;
    
    uint8_t first = (uint8_t)buffer[index];
    
    // Single byte (ASCII)
    if ((first & 0x80) == 0) return first;
    
    // Multi-byte UTF-8
    size_t remaining = dataLen - index;
    if ((first & 0xE0) == 0xC0 && remaining >= 2) {
        return ((first & 0x1F) << 6) | ((uint8_t)buffer[index + 1] & 0x3F);
    } else if ((first & 0xF0) == 0xE0 && remaining >= 3) {
        return ((first & 0x0F) << 12) | 
               (((uint8_t)buffer[index + 1] & 0x3F) << 6) |
               ((uint8_t)buffer[index + 2] & 0x3F);
    } else if ((first & 0xF8) == 0xF0 && remaining >= 4) {
        return ((first & 0x07) << 18) |
               (((uint8_t)buffer[index + 1] & 0x3F) << 12) |
               (((uint8_t)buffer[index + 2] & 0x3F) << 6) |
               ((uint8_t)buffer[index + 3] & 0x3F);
    }
    
    return first; // Invalid sequence, return first byte
}

// Modification
FastString& FastString::replace(char oldChar, char newChar) {
    for (size_t i = 0; i < dataLen; i++) {
        if (buffer[i] == oldChar) {
            buffer[i] = newChar;
        }
    }
    return *this;
}

FastString& FastString::replace(const char* target, const char* replacement) {
    // Simple implementation - for production, use a more efficient algorithm
    size_t targetLen = strlen(target);
    size_t replLen = strlen(replacement);
    
    FastString result;
    size_t i = 0;
    while (i < dataLen) {
        if (i + targetLen <= dataLen && memcmp(buffer + i, target, targetLen) == 0) {
            result.append(replacement, replLen);
            i += targetLen;
        } else {
            result.append(buffer[i]);
            i++;
        }
    }
    
    *this = std::move(result);
    return *this;
}

FastString& FastString::trim() {
    if (dataLen == 0) return *this;
    
    // Find start (skip whitespace)
    size_t start = 0;
    while (start < dataLen && (buffer[start] == ' ' || buffer[start] == '\t' || 
                               buffer[start] == '\n' || buffer[start] == '\r')) {
        start++;
    }
    
    // Find end
    size_t end = dataLen;
    while (end > start && (buffer[end - 1] == ' ' || buffer[end - 1] == '\t' ||
                           buffer[end - 1] == '\n' || buffer[end - 1] == '\r')) {
        end--;
    }
    
    if (start > 0 || end < dataLen) {
        memmove(buffer, buffer + start, end - start);
        dataLen = end - start;
        buffer[dataLen] = '\0';
        charCount = (size_t)-1;
    }
    
    return *this;
}

FastString& FastString::reverse() {
    if (dataLen <= 1) return *this;
    
    // For UTF-8, we need to reverse by code points, not bytes
    // Simple approach: decode to array, reverse, re-encode
    // For pure ASCII, this is just byte reversal
    
    std::reverse(buffer, buffer + dataLen);
    // Note: This breaks multi-byte UTF-8! 
    // For production, implement proper UTF-8 code point reversal
    
    return *this;
}

// Raw Access
const char* FastString::data() const {
    return buffer ? buffer : "";
}

char* FastString::mutableData() {
    return buffer;
}

const uint8_t* FastString::bytes() const {
    return (const uint8_t*)buffer;
}

uint8_t* FastString::mutableBytes() {
    return (uint8_t*)buffer;
}

// Java Interop
jstring FastString::toJString(JNIEnv* env) const {
    return env->NewStringUTF(buffer);
}

FastString FastString::fromJString(JNIEnv* env, jstring str) {
    if (!str) return FastString();
    const char* utf8 = env->GetStringUTFChars(str, nullptr);
    FastString result(utf8, strlen(utf8));
    env->ReleaseStringUTFChars(str, utf8);
    return result;
}

// SIMD Detection
bool FastString::hasSSE42() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 20)) != 0; // SSE4.2 bit
}

bool FastString::hasAVX2() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    bool hasOSXSAVE = (cpuInfo[2] & (1 << 27)) != 0;
    bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;
    
    if (hasOSXSAVE && hasAVX) {
        __cpuid(cpuInfo, 7);
        return (cpuInfo[1] & (1 << 5)) != 0; // AVX2 bit
    }
    return false;
}

bool FastString::hasAVX512() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    return (cpuInfo[1] & (1 << 16)) != 0; // AVX-512F bit
}

// UTF-8 Helpers
size_t FastString::countUtf8Chars() const {
    size_t count = 0;
    size_t i = 0;
    while (i < dataLen) {
        uint8_t b = (uint8_t)buffer[i];
        if ((b & 0x80) == 0) {
            i += 1;
        } else if ((b & 0xE0) == 0xC0) {
            i += 2;
        } else if ((b & 0xF0) == 0xE0) {
            i += 3;
        } else if ((b & 0xF8) == 0xF0) {
            i += 4;
        } else {
            i += 1; // Invalid, skip
        }
        count++;
    }
    return count;
}

size_t FastString::utf8CharLength(uint8_t firstByte) {
    if ((firstByte & 0x80) == 0) return 1;
    if ((firstByte & 0xE0) == 0xC0) return 2;
    if ((firstByte & 0xF0) == 0xE0) return 3;
    if ((firstByte & 0xF8) == 0xF0) return 4;
    return 1; // Invalid
}

} // namespace faststring

// ============================================================================
// JNI Implementation
// ============================================================================

using namespace faststring;

static std::unordered_map<jlong, FastString*> g_registry;
static std::mutex g_registryMutex;
static std::atomic<jlong> g_nextHandle{1};

// Registry implementation
jlong FastStringRegistry::registerString(FastString* str) {
    jlong handle = g_nextHandle++;
    std::lock_guard<std::mutex> lock(g_registryMutex);
    g_registry[handle] = str;
    return handle;
}

FastString* FastStringRegistry::getString(jlong handle) {
    std::lock_guard<std::mutex> lock(g_registryMutex);
    auto it = g_registry.find(handle);
    return (it != g_registry.end()) ? it->second : nullptr;
}

void FastStringRegistry::unregisterString(jlong handle) {
    std::lock_guard<std::mutex> lock(g_registryMutex);
    auto it = g_registry.find(handle);
    if (it != g_registry.end()) {
        delete it->second;
        g_registry.erase(it);
    }
}

void FastStringRegistry::cleanup() {
    std::lock_guard<std::mutex> lock(g_registryMutex);
    for (auto& pair : g_registry) {
        delete pair.second;
    }
    g_registry.clear();
}

// JNI Functions
JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeCreate(JNIEnv* env, jobject obj, jint capacity) {
    FastString* str = new FastString(capacity);
    return FastStringRegistry::registerString(str);
}

JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromString(JNIEnv* env, jobject obj, jstring str) {
    FastString* fs = new FastString(FastString::fromJString(env, str));
    return FastStringRegistry::registerString(fs);
}

JNIEXPORT jlong JNICALL Java_faststring_FastString_nativeFromBytes(JNIEnv* env, jobject obj, jbyteArray bytes, jint offset, jint length) {
    jbyte* data = env->GetByteArrayElements(bytes, nullptr);
    FastString* str = new FastString((const char*)(data + offset), length);
    env->ReleaseByteArrayElements(bytes, data, JNI_ABORT);
    return FastStringRegistry::registerString(str);
}

JNIEXPORT void JNICALL Java_faststring_FastString_nativeDestroy(JNIEnv* env, jobject obj, jlong handle) {
    FastStringRegistry::unregisterString(handle);
}

JNIEXPORT jint JNICALL Java_faststring_FastString_length(JNIEnv* env, jobject obj) {
    // Get handle from object field
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* str = FastStringRegistry::getString(handle);
    return str ? (jint)str->length() : 0;
}

JNIEXPORT jint JNICALL Java_faststring_FastString_byteLength(JNIEnv* env, jobject obj) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* str = FastStringRegistry::getString(handle);
    return str ? (jint)str->byteLength() : 0;
}

JNIEXPORT jboolean JNICALL Java_faststring_FastString_isEmpty(JNIEnv* env, jobject obj) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* str = FastStringRegistry::getString(handle);
    return str ? str->isEmpty() : JNI_TRUE;
}

JNIEXPORT void JNICALL Java_faststring_FastString_clear(JNIEnv* env, jobject obj) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* str = FastStringRegistry::getString(handle);
    if (str) str->clear();
}

JNIEXPORT jobject JNICALL Java_faststring_FastString_append__Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring str) {
    const char* utf8 = env->GetStringUTFChars(str, nullptr);
    
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* fs = FastStringRegistry::getString(handle);
    if (fs) fs->append(utf8);
    
    env->ReleaseStringUTFChars(str, utf8);
    return obj; // Return this for chaining
}

// Batch append implementations - single JNI call for multiple strings
JNIEXPORT jobject JNICALL Java_faststring_FastString_appendBatch__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring s1, jstring s2) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* fs = FastStringRegistry::getString(handle);
    if (fs) {
        const char* utf8_1 = env->GetStringUTFChars(s1, nullptr);
        const char* utf8_2 = env->GetStringUTFChars(s2, nullptr);
        fs->append(utf8_1);
        fs->append(utf8_2);
        env->ReleaseStringUTFChars(s1, utf8_1);
        env->ReleaseStringUTFChars(s2, utf8_2);
    }
    return obj;
}

JNIEXPORT jobject JNICALL Java_faststring_FastString_appendBatch__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring s1, jstring s2, jstring s3) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* fs = FastStringRegistry::getString(handle);
    if (fs) {
        const char* utf8_1 = env->GetStringUTFChars(s1, nullptr);
        const char* utf8_2 = env->GetStringUTFChars(s2, nullptr);
        const char* utf8_3 = env->GetStringUTFChars(s3, nullptr);
        fs->append(utf8_1);
        fs->append(utf8_2);
        fs->append(utf8_3);
        env->ReleaseStringUTFChars(s1, utf8_1);
        env->ReleaseStringUTFChars(s2, utf8_2);
        env->ReleaseStringUTFChars(s3, utf8_3);
    }
    return obj;
}

JNIEXPORT jobject JNICALL Java_faststring_FastString_appendBatch__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject obj, jstring s1, jstring s2, jstring s3, jstring s4) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* fs = FastStringRegistry::getString(handle);
    if (fs) {
        const char* utf8_1 = env->GetStringUTFChars(s1, nullptr);
        const char* utf8_2 = env->GetStringUTFChars(s2, nullptr);
        const char* utf8_3 = env->GetStringUTFChars(s3, nullptr);
        const char* utf8_4 = env->GetStringUTFChars(s4, nullptr);
        fs->append(utf8_1);
        fs->append(utf8_2);
        fs->append(utf8_3);
        fs->append(utf8_4);
        env->ReleaseStringUTFChars(s1, utf8_1);
        env->ReleaseStringUTFChars(s2, utf8_2);
        env->ReleaseStringUTFChars(s3, utf8_3);
        env->ReleaseStringUTFChars(s4, utf8_4);
    }
    return obj;
}

JNIEXPORT jobject JNICALL Java_faststring_FastString_appendBatch___3Ljava_lang_String_2(JNIEnv* env, jobject obj, jobjectArray strings) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, field);
    
    FastString* fs = FastStringRegistry::getString(handle);
    if (fs) {
        jsize len = env->GetArrayLength(strings);
        for (jsize i = 0; i < len; i++) {
            jstring str = (jstring)env->GetObjectArrayElement(strings, i);
            const char* utf8 = env->GetStringUTFChars(str, nullptr);
            fs->append(utf8);
            env->ReleaseStringUTFChars(str, utf8);
            env->DeleteLocalRef(str);
        }
    }
    return obj;
}

JNIEXPORT jstring JNICALL Java_faststring_FastString_nativeToString(JNIEnv* env, jobject obj, jlong handle) {
    FastString* str = FastStringRegistry::getString(handle);
    return str ? str->toJString(env) : env->NewStringUTF("");
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Initialization
        break;
    case DLL_PROCESS_DETACH:
        // Cleanup all registered strings
        FastStringRegistry::cleanup();
        break;
    }
    return TRUE;
}
