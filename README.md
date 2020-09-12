<div align="center">
  <p>
    <h1>
      <br/>
      EasyBonsai<sup>3</sup>
    </h1>
    <h4>A project that aims to simplify and improve Bonsai</h4>
  </p>
  <p>
    <a href="https://github.com/Git-Curve/EasyBonsai2/blob/master/LICENSE">
      <img src="https://img.shields.io/github/license/Git-Curve/EasyBonsai2.svg?style=flat-square" alt="License" />
    </a>
  </p>
</div>

*Changes to EasyBonsai2: Heavily improved code, which also results in faster compile-times!*

# Introduction
EasyBonsai is a extension for [Bonsai](https://bonsai.pinyto.de/assembler/)  which adds more advanced instructions and is also compileable to Bonsai.
# Install
<b>Important: </b>
- Please compile with C++17 or above

Windows/Linux:
- Compile the project with an compiler of your choice

Web:
- Compile with emscripten
  ```em++ main.cpp -D BONSAI_WEB -std=c++17 -Wno-narrowing --bind```  
# Features
- [Realtive Jumps](#relative-jmps)
- [Compare (cmp, je, jg, jl)](#compare)
- [Mov](#mov)
- [And, Or](#and--or)
- [Goto (+ Labels)](#labels--goto)
- [Variable naming](#variable-naming)
# Examples
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
2: jl 20 ; will jump to line 20 if [0] < [1]
3: jg 30 ; will jump to line 30 if [0] > [1]
```
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
1: inc TEST ; will increase memory-cell [0]
2: dec TEST ; will decrease memory-cell [0]
3: jmp TEST ; this will **not** work, because jmp instructions cant access memory-cells 
3: hlt
```
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
