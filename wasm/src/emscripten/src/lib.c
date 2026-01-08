#include "sha256.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// 把单个十六进制字符转换为数值（0-15）
static inline uint8_t hex_char_to_val(char c) {
  if (c >= '0' && c <= '9')
    return (uint8_t)(c - '0');
  if (c >= 'a' && c <= 'f')
    return (uint8_t)(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (uint8_t)(c - 'A' + 10);
  return 0;
}

// 将十六进制字符串解析为字节数组
static size_t parse_hex_target(const char *target, uint8_t *out_buf) {
  size_t len = strlen(target);
  size_t byte_count = (len + 1) / 2;
  size_t i = 0, j = 0;
  if (len & 1) {
    out_buf[j++] = hex_char_to_val(target[i++]) << 4;
  }
  while (i < len) {
    uint8_t high = hex_char_to_val(target[i++]);
    uint8_t low = hex_char_to_val(target[i++]);
    out_buf[j++] = (high << 4) | low;
  }
  return byte_count;
}

// 将 u64 转化为 ASCII 数字字符串
static inline size_t write_u64_to_buffer(uint64_t v, uint8_t *buf) {
  char tmp[20];
  char *p = tmp + sizeof(tmp);
  size_t len = 0;
  do {
    *--p = (v % 10) + '0';
    v /= 10;
    len++;
  } while (v);
  memcpy(buf, p, len);
  return len;
}

// 校验哈希是否满足目标
static int hash_matches_target(const uint8_t *hash, const uint8_t *target_bytes,
                               size_t target_bits) {
  size_t full_bytes = target_bits / 8;
  size_t remaining_bits = target_bits % 8;

  if (memcmp(hash, target_bytes, full_bytes) != 0) {
    return 0;
  }

  if (remaining_bits > 0 && full_bytes < 32) { // SHA256 输出 32 字节
    uint8_t mask = 0xFF << (8 - remaining_bits);
    return (hash[full_bytes] & mask) == (target_bytes[full_bytes] & mask);
  }

  return 1;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
uint64_t solve_pow(const char *salt, const char *target) {
  uint8_t salt_bytes[256];
  size_t salt_len = strlen(salt);
  memcpy(salt_bytes, salt, salt_len);

  uint8_t target_bytes[64];
  parse_hex_target(target, target_bytes);
  size_t target_bits = strlen(target) * 4;

  uint8_t nonce_buffer[20];
  uint8_t hash_result[SHA256_BLOCK_SIZE];

  // 预计算盐部分哈希状态
  SHA256_CTX base_ctx;
  sha256_init(&base_ctx);
  sha256_update(&base_ctx, salt_bytes, salt_len);

  SHA256_CTX ctx;

  for (uint64_t nonce = 0; nonce < UINT64_MAX; nonce++) {
    size_t nonce_len = write_u64_to_buffer(nonce, nonce_buffer);

    // 使用已准备好的盐初值状态
    ctx = base_ctx;
    sha256_update(&ctx, nonce_buffer, nonce_len);
    sha256_final(&ctx, hash_result);

    if (hash_matches_target(hash_result, target_bytes, target_bits)) {
      return nonce;
    }
  }

  fprintf(stderr, "Solution not found!\n");
  exit(1);
}
