<template>
  <v-container>
    <script src="./EasyBonsai3.js"></script>
    <v-row dense>
      <v-col cols="3" md="2">
        <v-chip :ripple="false">Register</v-chip>
      </v-col>
      <v-col cols="12" md="4">
        <v-chip :ripple="false">Input</v-chip>
      </v-col>
      <v-col cols="12" md="6">
        <v-chip :ripple="false">Output</v-chip>
      </v-col>
    </v-row>

    <v-row dense>
      <v-col cols="3" md="2">
        <prism-editor
          class="editor"
          v-model="registers"
          :highlight="highlighter"
          :autoStyleLineNumbers="true"
          :ignoreTabKey="true"
          line-numbers
        ></prism-editor>
      </v-col>
      <v-col cols="12" md="4">
        <prism-editor
          class="editor"
          v-model="code"
          :highlight="highlighter"
          :autoStyleLineNumbers="true"
          :ignoreTabKey="true"
          line-numbers
        ></prism-editor>
      </v-col>
      <v-col cols="12" md="6">
        <prism-editor
          class="editor"
          v-model="output"
          :highlight="highlighter"
          :autoStyleLineNumbers="true"
          :ignoreTabKey="true"
          readonly
          line-numbers
        ></prism-editor>
      </v-col>

      <v-col md="5">
        <v-btn @click="compile(false)"
          ><v-icon left>mdi-hammer</v-icon>Compile</v-btn
        >
        <v-btn @click="compile(true)"><v-icon left>mdi-send</v-icon>Run</v-btn>
      </v-col>
    </v-row>
  </v-container>
</template>

<style>
.height-200 {
  height: 200px;
}

.editor {
  /* we dont use `language-` classes anymore so thats why we need to add background and text color manually */
  background: #2d2d2d;
  color: #ccc;

  /* you must provide font-family font-size line-height. Example:*/
  font-family: Fira code, Fira Mono, Consolas, Menlo, Courier, monospace;
  font-size: 14px;
  line-height: 1.5;
  padding: 5px;
}

/* optional class for removing the outline */
.prism-editor__textarea:focus {
  outline: none;
}
</style>

<script>
window["global"] = window;
const PrismEditor = require("vue-prism-editor").PrismEditor;

import "vue-prism-editor/dist/prismeditor.min.css"; // import the styles somewhere

// import highlighting library (you can use any library you want just return html string)
import { highlight, languages } from "prismjs/components/prism-core";

import "prismjs/components/prism-clike";
import "prismjs/components/prism-javascript";
import "prismjs/themes/prism-tomorrow.css"; // import syntax highlighting styles*/

languages.bonsai = {
  comment: /^^#!.+|;(?:\[(=*)\[[\s\S]*?\]\1\]|.*)/m,
  number: /\b0x[a-f\d]+\.?[a-f\d]*(?:p[+-]?\d+)?\b|\b\d+(?:\.\B|\.?\d*(?:e[+-]?\d+)?\b)|\B\.\d+(?:e[+-]?\d+)?\b/i,
  keyword: /\b(?:mov|reg|jmp|goto|inc|dec|hlt|add|sub|tst|cmp|je|jg|jl|jne|and|or|ret|push|call|int)\b/,
  symbol: /\b(?:[A-Z]+)\b/,
  function: [
    /(?!\d)\w+(?=\s*(?:[:]))/,
    /(?!\d)(?:[.])\w+(?=\s*)/,
    /\(fun .+\(([a-zA-Z0-9, ]+)\):\ */,
  ],
  punctuation: /[\[\](){},;]|\.+|:+/,
};

export default {
  components: {
    PrismEditor,
  },
  data: () => ({ code: "", output: "", registers: "" }),
  methods: {
    highlighter(code) {
      return highlight(code, languages.bonsai); // languages.<insert language> to return html with markup
    },
    compile(run) {
      const input = new Module.StringList();
      this.code.split("\n").forEach((x) => input.push_back(x));

      const compiled = Module.compile(input, new Module.UIntList());
      let result = compiled.getSuccess()
        ? "; Compilation success: \n"
        : "; Compilation failed! Error Stack: \n";

      const stack = compiled.getResult();

      for (let i = 0; stack.size() > i; i++) {
        result += stack.get(i) + "\n";
      }

      let usedRegisters = [];
      for (let i = 0; compiled.getRegisters().size() > i; i++) {
        usedRegisters.push(compiled.getRegisters().get(i));
      }
      result += "; Additional defined registers: " + usedRegisters.join(", ");
      this.output = result;

      if (run && compiled.getSuccess()) {
        const myRegisters = this.registers.split("\n");
        const userRegisters = new Module.UIntList();
        for (let i = 0; myRegisters.length > i; i++) {
          if (myRegisters[i].length > 0)
            userRegisters.push_back(Number(myRegisters[i]));
        }
        const executeResult = Module.run(stack, userRegisters);
        let execOutStr = executeResult.getSuccess()
          ? "; Compilation success: \n"
          : "; Compilation failed! Error Stack: \n";

        for (let i = 0; executeResult.getResult().size() > i; i++) {
          execOutStr += executeResult.getResult().get(i) + "\n";
        }

        this.output = execOutStr;
      }
    },
  },
};
</script>