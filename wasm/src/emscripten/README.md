# Emscripten

```shell
emsdk_env.bat
emcc src/lib.c src/sha256.c -o dist/cap_wasm.js -sEXPORTED_FUNCTIONS=["_solve_pow"] -sEXPORTED_RUNTIME_METHODS=["cwrap"] -sMODULARIZE -sEXPORT_ES6 -sASSERTIONS=0 -sENVIRONMENT=web -O2

```
