# Emscripten

```shell
emsdk_env.bat
emcc src/lib.c src/sha256.c -o dist/cap_wasm_bg.js -s EXPORTED_FUNCTIONS=["_solve_pow"] -s EXPORTED_RUNTIME_METHODS=["cwrap","ccall"]

```
