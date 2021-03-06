<div align="center">
  <p>
    <h1>
      <br/>
      EasyBonsai<sup>3</sup>
    </h1>
    <h4>A project that aims to simplify and improve Bonsai</h4>
  </p>
  <p>
    <a href="https://github.com/Curve/EasyBonsai3/blob/master/LICENSE">
      <img src="https://img.shields.io/github/license/Curve/EasyBonsai3.svg?style=for-the-badge" alt="License" />
    </a>
    <a href="https://github.com/Curve/EasyBonsai3/actions?query=workflow%3A%22Build+and+release+on+push%22">
      <img src="https://img.shields.io/github/workflow/status/Curve/EasyBonsai3/Build%20and%20release%20on%20push?style=for-the-badge" alt="Builds" />
    </a>
  </p>
</div>

*Changes to EasyBonsai2: Heavily improved code, which also results in faster compile-times!*

# Introduction
EasyBonsai is a extension for [Bonsai](https://bonsai.pinyto.de/assembler/)  which adds more advanced instructions and is also compileable to Bonsai.
# Install
## Use the latest binaries
From [here](https://github.com/Curve/EasyBonsai3/releases/tag/latest), or [try it on the web](https://curve.github.io/EasyBonsai3/)!

## Or compile it yourself
<b>Important: </b>
- Please compile with C++17 or above

Windows/Linux:
- Compile the project with a compiler of your choice

Web:
- Compile with emscripten
  ```em++ main.cpp -D BONSAI_WEB -std=c++17 --bind```  
# Index
- [Introduction](#introduction)
- [Install](#install)
	- [Use the latest binaries](#use-the-latest-binaries)
	- [Or compile it yourself](#or-compile-it-yourself)
- [Index](#index)
- [Features](#features)
	- [Relative Jmps](#relative-jmps)
	- [Compare](#compare)
	- [Mov](#mov)
	- [And / Or](#and--or)
	- [Labels / Goto](#labels--goto)
	- [Variable Naming](#variable-naming)
	- [Add / Sub](#add--sub)
	- [Inc / Dec](#inc--dec)
	- [Functions](#functions)
- [Usage](#usage)
- [Examples](#examples)
	- [Exponentiate Function](#exponentiate-function)
# Features
### Relative Jmps
Allows you to jump relatively (This will skip Reg-Instructions aswell as Label-Definitions)
```nasm
0: jmp +2
1: jmp 0
2: hlt ; <-- jmps here
```
### Compare
Assembler-like compare instruction, that will compare two memory-cells.
```nasm
0: cmp 0, 1
1: je 10 ; will jump to line 10 if [0] == [1]
1: jne 10 ; will jump to line 10 if [0] != [1]
2: jl 20 ; will jump to line 20 if [0] < [1]
3: jg 30 ; will jump to line 30 if [0] > [1]
```
JE/JNE/JL/JG do not support relative jumping.

### Mov
Allows you to copy the content of one memory-cell into another, also allows you to just set one memory-cell to zero.
```nasm
0: mov 0, 1 ; [0] is now equal to [1]
1: mov 1, NULL ; [1] is now 0.
```
### And / Or
Allows you to perform a simple and/or operation on two memory-cells.
```nasm
0: and 0, 1
1: jmp 10 ; will jump to line 10 if [0] and [1] are greater than zero.
2: jmp 20 ; will jump to line 10 if either [0] or [1] are zero.
3: or 2, 3
4: jmp 30 ; will jump to line 30 if [2] or [3] are greater than zero.
5: jmp 40 ; will jump to line 40 if [2] and [3] are zero.
```
### Labels / Goto
Simple labels.
```nasm
0: goto end ; will jump to line 2
1: jmp .end ; will jump to line 2, the dot is important!
2: end: hlt
```
```nasm
0: goto end
1: jmp .end
2: end:
3: hlt ; works exactly as the code example before.
```
### Variable Naming
Allows you to give memory-cells names.
```nasm
0: reg TEST, 0
1: reg TEST2 ; Since we didn't provide a memory cell that should be used the compiler will automatically assign one to it.
2: inc TEST ; will increase memory-cell [0]
3: dec TEST ; will decrease memory-cell [0]
4: jmp TEST ; this will **not** work, because jmp instructions cant access memory-cells 
5: hlt
```
### Add / Sub
Allows you to perform simple addition and subtraction with memory-cells.
```nasm
0: add 0, 1 ; will add the value of memory-cell [1] to memory-cell [0]
1: hlt
```
```nasm
0: sub 0, 1 ; will subtract the value of memory-cell[1] from memory-cell [0]
1: hlt
```
### Inc / Dec
Allows you to increase a memory-cell by a given value
```nasm
0: inc 0, 10 ; will increase memory-cell [0] by 10
0: dec 1, 5  ; will decrease memory-cell [1] by 5
```
### Functions
#### Declaration
A function is delcared like this:
```nasm
(fun functionName(parameter1, parameter2):
; code
)
```
#### Calling & Parameters
Parameters can be accessed by their name inside of the function body.
- Example
	```nasm
	(fun functionName(parameter1, parameter2):
	dec parameter1
	add parameter1, parameter2
	; code...
	)
	```
Parameters are passed by **REFERENCE**. It is recommended to always have variables that you can save the function parameters into if you plan on modifying them.

To call a function the `call` instruction is used.
- Example
	```nasm
	(fun function():
	; code...
	)
	
	call function
	; code...
	```
To pass parameters to the function call the `push` variable is used.
- Example
	```nasm
	(fun function(param1, param2):
	; code...
	)
	push a
	push b
	call function 
	```

#### Return
To return a function the `ret` instruction is used.
- Example
	```nasm
	(fun functionName(param1, param2):
	inc param1
	ret
	)
	```
You can also return a value
- Example
	```nasm
	(fun functionName(param1, param2):
	inc param1
	ret param1
	)
	```
The return value will be saved inside of the **eax** register, you can access this register by simply using it like a variable.
-	Example
	```nasm
	(fun functionName(param1, param2):
	inc param1
	ret param1
	)
	push a
	push b
	call functionName
	mov someOtherVariable, eax ; eax will hold the return value!
	```
#### Tips
It is recommended to use *relative jumps* inside of functions. Using Labels inside of the function will most likely not work as it will often lead to label-redefinition.
### Int
The `int` instruction is **only useable in the cli version of easybonsai**.
What it does is pause the program (wait for the user to press the return key) and print all current registers.

# Usage
- Compile your code
	```bash
	> ./EasyBonsai3-Linux --input input.bon
	> ./EasyBonsai3-Linux --input input.bon --output output.bon
	> ./EasyBonsai3-Linux --input input.bon --output output.bon --usedVars 1
	> ./EasyBonsai3-Linux --input input.bon --output output.bon --usedVars 1,2,3
	```
	- Example Output:
		```bash
		[02:16:38] [EasyBonsai3] Using input file: input.txt
		[02:16:38] [EasyBonsai3] Using output file: output.bon
		[02:16:38] [EasyBonsai3] User defined variables:
		[02:16:38] [Debug] Detected Labels: { [equal,8], [greater,2], [less,5], [start,0] }
		[02:16:38] [Debug] Detected Address-Macros: {  }
		[02:16:38] [Debug] Detected Used-Addresses: { 0, 1 }
		[02:16:38] [Debug] Setting Help-Register to [$0: 2]
		[02:16:38] [Debug] Setting Compare-Registers to [$1: 3] and [$2: 4]
		[02:16:38] [EasyBonsai3] Additional defined registers: 3, 4, 2
		[02:16:38] [EasyBonsai3] Compilation finished in 104ms!
		```
	*usedVars* will tell the compiler to not use the provided vars as registers, you will only need to define this, if you don't use those memory-cells in your code and don't want them to be used.

- Run your code  
	*Why? Because running the code in the web version is often times slower, and for bigger code you may want faster execution times*
	```bash
	> ./EasyBonsai3-Linux --run output.bon
	> ./EasyBonsai3-Linux --run outbut.bon --setVars 0:0,1:100
	```
	*setVars* usage: `register:value`, it will set the provided register to the value provided before running the code.
	- Example Output:
		```bash
		[19:56:25] [EasyBonsai3] Running output.bon
		[19:56:26] [Debug] Setting Register $0 to 0
		[19:56:26] [Debug] Setting Register $1 to 100
		[19:56:26] [EasyBonsai3] Execution finished in 262ms!
		[19:56:26] [EasyBonsai3] Registers after execution:
		[19:56:26] [EasyBonsai3] [$0]: 100
		[19:56:26] [EasyBonsai3] [$1]: 100
		[19:56:26] [EasyBonsai3] [$2]: 0
		[19:56:26] [EasyBonsai3] [$3]: 0
		[19:56:26] [EasyBonsai3] [$4]: 1
		```

# Examples
### Exponentiate Function
A small example that demonstrates most of the EasyBonsai-Extensions to provide a function which can be used to exponentiate a given number with a given exponent.

```nasm
reg number, 0
reg exponent, 1

reg NumBuffer, 2
reg FacBuffer, 3

reg Result, 4

reg Num2Buffer, 5
reg ExpoBuffer, 6
reg ONumBuffer, 7

(fun multiply(num, fac):
mov FacBuffer, fac
mov NumBuffer, NULL
tst FacBuffer
jmp +2
ret NumBuffer
dec FacBuffer
add NumBuffer, num
jmp -5
)
(fun expo(num, expon):
mov ExpoBuffer, expon
dec ExpoBuffer
mov ONumBuffer, num
mov Num2Buffer, num
tst ExpoBuffer
jmp +2
ret Num2Buffer
push Num2Buffer
push ONumBuffer
call multiply
mov Num2Buffer, eax
dec ExpoBuffer
jmp -8
)

push number
push exponent
call expo
mov Result, eax
hlt
```

This example code has 43 lines, after compilation it has a total of 263 lines.
