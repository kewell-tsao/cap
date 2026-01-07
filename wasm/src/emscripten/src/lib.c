#include <stdint.h>
#include <emscripten/emscripten.h>
#include "sha256.h"

uint64_t EMSCRIPTEN_KEEPALIVE solve_pow(const char * salt, const char * target) {
    return 0;
}
