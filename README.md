


<div align="center">
  <p>
    <h1>
      <br />
      EasyBonsai<sup>2</sup>
    </h1>
    <h4>A crossplatform compiler/translator/macro to make writing bonsai code easier!</h4>
  </p>
  <p>
    <a href="https://github.com/Git-Curve/EasyBonsai2/blob/master/LICENSE">
      <img src="https://img.shields.io/github/license/Git-Curve/EasyBonsai2.svg?style=flat-square" alt="License" />
    </a>
  </p>
</div>

## Index
- [Introduction](#introduction)
- [Dependencies](#dependencies)
- [Install](#install)
- [Features](#features)
- [Usage](#usage)
- [Example](#example)

# Introduction
When writing [Bonsai Code](https://bonsai.pinyto.de/assembler/) its often hard to structure the code, because Bonsai is missing things like labels, or instructions like "mov", "and", "or", "cmp", which further increases the codes size. EasyBonsai however adds those features! (See [Features](#features))

Also other than its predecessor EasyBonsai2 has a web-implementation, which allows you to use this in the web too!

# Dependencies
- [rang](https://github.com/agauniyal/rang) [Terminal Colors]
- [emscripten](https://emscripten.org/) [*Only for WebBonsai*] 
# Install
<b>Important: </b>
- Please compile with C++17 or above

Windows/Linux:
- Compile the project with an compiler of your choice

Web:
- Compile with emscripten
  ```em++ main.cpp Compiler.cpp ArgParser.cpp -o easybonsai.js -std=c++17 -D BONSAI_WEB --bind```

# Features
## Labels
Labels are important for structure and are implemented as following in EasyBonsai
```nasm
; The name of a label can be everything, no restrictions to special characters or similar.
lbl.main:
; code...
; somewhere else in code
goto lbl.main
; or with a jmp
jmp .lbl.main ; The dot is important here!
```
## Relative Jmps
Sometimes its easier to have jmps that jump to a relative address instead of a static one.
```nasm
0: ; code...
1: jmp +5
2: ;...
3: ;...
4: ;...
5: ;...
6: hlt ; jmp goes here.
```
## Custom Instructions

### Mov
Mov is used to copy the value of a variable to another, however this cannot be done without any helper-variables in Bonsai, this is why the mov instructions needs a third parameter,  the helper variable (*Only if the program was ran without the -ah flag, which will automatically determine the address of the helper-variable*)
```nasm
; Variable named "1" will be asgined the value of variable named "2", "3" will be
; used as a buffer and is expected to be 0, after successfull allocation 
; the helper value will be set to 0 again.
mov 1, 2 # 3
; spaces are ignored, so it can also be written like this for example
mov 1,2#3
; or if compiled with `-ah` flag
mov 1, 2 ; Variable 3 still needs to be initialized but will be auto filled in.
; code...
; or set a variable to 0!
mov 1, NULL ; "1" will now be 0
```
### And
The and instruction checks wether the value of two given variables is greater than 0.
```nasm
; Assuming "1" = 3 and "2" = 5
and 1, 2  ; Both variables hold a value greater than 0
hlt     ; Programm stops
jmp 0     ; If one or both of the variables holds a value equal to 0
      ; the program will enter an endless loop
```
### Or
The or instruction checks wether the value of one of two given variables is greater than 0.
```nasm
; Assuming "1" = 3 and "2" = 0
or 1, 2   ; One of the variables holds a value greater than 0
hlt     ; Programm stops
jmp 0     ; If both of the variables holds a value equal to 0
      ; the program will enter an endless loop
```
### CMP, JE, JL, JG
To compare two variables you can use cmp. JE is used to jmp somewhere when the two variables are equal, jl if x is less than y, and jg if x is greater than y
```nasm
; After execution:
; "2" will be 1 if "0">"1"
; "3" will be 1 if "0"<"1"
; "4" will be 1 if "0"=="1"
cmp 0, 1
je .equal
jl .less
jg .greater
greater:
inc 2
hlt
less:
inc 3
hlt
equal:
inc 4
hlt
```

# Usage 
- Compile/Translate your code
  ```sh
  ./EasyBonsai.exe -i codefile.extension -o outputfile.extension
  # Or just displaying the output in your console
  ./EasyBonsai.exe -i codefile.extension
  # Further information on flags like -ah is presented here:
  ./EasyBonsai.exe -h
  ```

# Example
To see how "powerful" (or rather how much simpler) EasyBonsai is I have this example program:
```nasm
start:
cmp 0, 1
je .equal
jl .less
jg .greater
greater:
dec 0
goto start
less:
inc 0
goto start
equal:
hlt
```
All it does is increment/decrement 0 until its equal to 1.
While this program only has 12 lines in EasyBonsai, it has 282 in Bonsai.
<details closed>

```nasm
jmp 1
jmp 22
jmp 13
jmp 16
jmp 19
jmp 6
dec 0
jmp 1
jmp 9
inc 0
jmp 1
jmp 12
hlt
tst 4
jmp 12
jmp 3
tst 3
jmp 4
jmp 9
tst 3
jmp 6
jmp 5
jmp 53
jmp 68
tst 0
jmp 27
jmp 30
tst 1
jmp 33
jmp 41
tst 1
jmp 36
jmp 47
dec 0
dec 1
jmp 24
jmp 83
jmp 98
jmp 173
jmp 178
jmp 2
jmp 113
jmp 128
jmp 183
jmp 188
inc 3
jmp 2
jmp 143
jmp 158
jmp 193
jmp 198
inc 4
jmp 2
jmp 203
jmp 208
tst 0
jmp 58
jmp 62
inc 2
inc 3
dec 0
jmp 55
tst 2
jmp 65
jmp 23
dec 2
inc 0
jmp 62
jmp 213
jmp 218
tst 1
jmp 73
jmp 77
inc 2
inc 4
dec 1
jmp 70
tst 2
jmp 80
jmp 24
dec 2
inc 1
jmp 77
jmp 223
jmp 228
tst 3
jmp 88
jmp 92
inc 2
inc 0
dec 3
jmp 85
tst 2
jmp 95
jmp 37
dec 2
inc 3
jmp 92
jmp 233
jmp 238
tst 4
jmp 103
jmp 107
inc 2
inc 1
dec 4
jmp 100
tst 2
jmp 110
jmp 38
dec 2
inc 4
jmp 107
jmp 243
jmp 248
tst 3
jmp 118
jmp 122
inc 2
inc 0
dec 3
jmp 115
tst 2
jmp 125
jmp 42
dec 2
inc 3
jmp 122
jmp 253
jmp 258
tst 4
jmp 133
jmp 137
inc 2
inc 1
dec 4
jmp 130
tst 2
jmp 140
jmp 43
dec 2
inc 4
jmp 137
jmp 263
jmp 268
tst 3
jmp 148
jmp 152
inc 2
inc 0
dec 3
jmp 145
tst 2
jmp 155
jmp 48
dec 2
inc 3
jmp 152
jmp 273
jmp 278
tst 4
jmp 163
jmp 167
inc 2
inc 1
dec 4
jmp 160
tst 2
jmp 170
jmp 49
dec 2
inc 4
jmp 167
tst 3
jmp 176
jmp 39
dec 3
jmp 173
tst 4
jmp 181
jmp 40
dec 4
jmp 178
tst 3
jmp 186
jmp 44
dec 3
jmp 183
tst 4
jmp 191
jmp 45
dec 4
jmp 188
tst 3
jmp 196
jmp 50
dec 3
jmp 193
tst 4
jmp 201
jmp 51
dec 4
jmp 198
tst 2
jmp 206
jmp 54
dec 2
jmp 203
tst 3
jmp 211
jmp 55
dec 3
jmp 208
tst 2
jmp 216
jmp 69
dec 2
jmp 213
tst 4
jmp 221
jmp 70
dec 4
jmp 218
tst 2
jmp 226
jmp 84
dec 2
jmp 223
tst 0
jmp 231
jmp 85
dec 0
jmp 228
tst 2
jmp 236
jmp 99
dec 2
jmp 233
tst 1
jmp 241
jmp 100
dec 1
jmp 238
tst 2
jmp 246
jmp 114
dec 2
jmp 243
tst 0
jmp 251
jmp 115
dec 0
jmp 248
tst 2
jmp 256
jmp 129
dec 2
jmp 253
tst 1
jmp 261
jmp 130
dec 1
jmp 258
tst 2
jmp 266
jmp 144
dec 2
jmp 263
tst 0
jmp 271
jmp 145
dec 0
jmp 268
tst 2
jmp 276
jmp 159
dec 2
jmp 273
tst 1
jmp 281
jmp 160
dec 1
jmp 278
```

</details>


# TODO
- [x] ~~Greater/Less than~~
- [x] ~~Allow custom implementations~~