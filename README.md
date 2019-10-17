
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
Labels are important for structure and are implementet as following in EasyBonsai
```nasm
; The name of a label can be everything, no restrictions to special characters or similar.
lbl.main:
; code...
; somewhere else in code
goto lbl.main
; or with a jmp
jmp .lbl.main ; The dot is important here!
```
## Mov
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
## And
The and instruction checks wether the value of two given variables is greater than 0.
```nasm
; Assuming "1" = 3 and "2" = 5
and 1, 2  ; Both variables hold a value greater than 0
hlt     ; Programm stops
jmp 0     ; If one or both of the variables holds a value equal to 0
      ; the program will enter an endless loop
```
## Or
The or instruction checks wether the value of one of two given variables is greater than 0.
```nasm
; Assuming "1" = 3 and "2" = 0
or 1, 2   ; One of the variables holds a value greater than 0
hlt     ; Programm stops
jmp 0     ; If both of the variables holds a value equal to 0
      ; the program will enter an endless loop
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
## CMP, JE, JL, JG
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
or 0, 1 ; if "0" or "1" > 0
goto clearThem ; clear "0"&"1" and backup "1" in "2"
goto end
end:
mov 1, 2 # 3 ; "0" and "1" were cleared, now move the backup from "2" into "1"
hlt
clearThem:
mov 2, 1#3
mov 3, NULL
mov 1, NULL
mov 0, NULL
jmp 0
```
All it does is to clear the variables "0" and "1" (if one of them has a value greater than 0) and backups variable "1" in the variable "2", then if "0" and "1" are both equal to zero, it copies the value of "2" to "1".
While this program only has 12 lines in EasyBonsai, it has 72 in Bonsai.
<details closed>

```nasm
jmp 12
jmp 7
jmp 4
jmp 4
jmp 18
hlt
jmp 7
jmp 33
jmp 48
jmp 53
jmp 58
jmp 0
tst 0
jmp 1
jmp 15
tst 1
jmp 1
jmp 2
tst 2
jmp 21
jmp 25
inc 3
inc 1
dec 2
jmp 18
tst 3
jmp 28
jmp 31
dec 3
inc 2
jmp 25
jmp 63
jmp 5
tst 1
jmp 36
jmp 40
inc 3
inc 2
dec 1
jmp 33
tst 3
jmp 43
jmp 46
dec 3
inc 1
jmp 40
jmp 68
jmp 8
tst 3
jmp 51
jmp 9
dec 3
jmp 48
tst 1
jmp 56
jmp 10
dec 1
jmp 53
tst 0
jmp 61
jmp 11
dec 0
jmp 58
tst 3
jmp 66
jmp 32
dec 3
jmp 63
tst 3
jmp 71
jmp 47
dec 3
jmp 68
```

</details>


# TODO
- [ ] improve performance <font size="1">(I make use of a lot of recursive function calling, I should reduce that.)</font>
- [x] ~~Greater/Less than~~
- [ ] Settings
- [ ] Bonsai Interpreter
- [ ] Add Lua to allow custom implementation of custom instructions