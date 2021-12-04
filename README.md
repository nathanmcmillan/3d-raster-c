# Scroll & Sigil

A 3D adventure game with a strong focus on modding and user crafted content.

[Visit the website here.](https://scrollandsigil.com)

# Compiling

## Windows Clang Test

```
> clang src/*.c -Isrc -I%SDL%\include -Wall -Wextra -Werror -pedantic -std=c11 -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -l%SDL%\lib\x64\SDL2 -o SCRLSIGL.exe
```

## Windows Clang Debug

```
> clang -g -fsanitize=address src/*.c -Isrc -I%SDL%\include -Wall -Wextra -Werror -pedantic -std=c11 -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -l%SDL%\lib\x64\SDL2 -o SCRLSIGL.exe
```

## Windows Clang Release

```
> clang -O2 src/*.c -Isrc -I%SDL%\include -Wno-deprecated-declarations -std=c11 -l%SDL%\lib\x64\SDL2 -o SCRLSIGL.exe
```

## MSVC Debug

```
> cl src/*.c /W4 /WX /wd4996 /link /out:scroll-and-sigil.exe
```
