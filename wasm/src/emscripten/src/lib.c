#include "sha256.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// 将十六进制字符串解析为字节数组
static size_t parse_hex_target(const char *target, uint8_t *out_buf) {
  size_t len = strlen(target);
  char *padded;
  if (len % 2 != 0) {
    // 补充一个 '0'（Rust 代码是补到末尾）
    padded = malloc(len + 2);
    strcpy(padded, target);
    padded[len] = '0';
    padded[len + 1] = '\0';
    len++;
  } else {
    padded = strdup(target);
  }

  for (size_t i = 0; i < len; i += 2) {
    char byte_str[3] = {padded[i], padded[i + 1], 0};
    out_buf[i / 2] = (uint8_t)strtoul(byte_str, NULL, 16);
  }

  free(padded);
  return len / 2;
}

// 将 u64 转化为 ASCII 数字字符串
static size_t write_u64_to_buffer(uint64_t value, uint8_t *buffer) {
  if (value == 0) {
    buffer[0] = '0';
    return 1;
  }
  size_t len = 0;
  uint64_t temp = value;

  while (temp > 0) {
    len++;
    temp /= 10;
  }

  for (size_t i = len; i > 0; i--) {
    buffer[i - 1] = (value % 10) + '0';
    value /= 10;
  }

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
    if ((hash[full_bytes] & mask) != (target_bytes[full_bytes] & mask)) {
      return 0;
    }
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
  size_t target_bytes_len = parse_hex_target(target, target_bytes);
  size_t target_bits = strlen(target) * 4;

  uint8_t nonce_buffer[20];
  uint8_t hash_result[SHA256_BLOCK_SIZE];

  for (uint64_t nonce = 0; nonce < UINT64_MAX; nonce++) {
    size_t nonce_len = write_u64_to_buffer(nonce, nonce_buffer);

    // SHA256 计算
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, salt_bytes, salt_len);
    sha256_update(&ctx, nonce_buffer, nonce_len);
    sha256_final(&ctx, hash_result);

    if (hash_matches_target(hash_result, target_bytes, target_bits)) {
      return nonce;
    }
  }

  fprintf(stderr, "Solution not found!\n");
  exit(1);
}
