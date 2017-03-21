#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>

using namespace std;

// == HELPER FUNCTIONS ==

string trim(string &str, char del)
{
  size_t first = str.find_first_not_of(del);
  if (first == string::npos)
    return "";
  size_t last = str.find_last_not_of(del);
  return str.substr(first, (last-first+1));
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

// == VM TRANSLATOR ==

int lbl = 0;

// push segm index
// "setup" sets D to the value we are pushing
// setup = @index
//         D=A   - constant stops here
//         @loc  - based on segm
//         [A=M] - for heap locations
//         A=A+D
//         D=M
// code: setup
//       @0
//       A=M
//       M=D
//       @0
//       M=M+1
string handle_push(string segm, string index) {
  string setup        = "@"+index+"\nD=A\n";
  string heap         = "A=M\n";
  string setup_offset = "A=A+D\nD=M\n";
  string code         = "@0\nA=M\nM=D\n@0\nM=M+1\n";

  if (segm == "constant") {
    ; // do nothing
  }
  else if (segm == "static") {
    setup += "@16\n";
    setup += setup_offset;
  }
  else if (segm == "temp") {
    setup += "@5\n";
    setup += setup_offset;
  }
  else if (segm == "local") {
    setup += "@LCL\n";
    setup += heap + setup_offset;
  }
  else if (segm == "argument") {
    setup += "@ARG\n";
    setup += heap + setup_offset;
  }
  else if (segm == "this") {
    setup += "@THIS\n";
    setup += heap + setup_offset;
  }
  else if (segm == "that") {
    setup += "@THAT\n";
    setup += heap + setup_offset;
  }
  else if (segm == "pointer") {
    setup = ((index=="0") ? "@THIS\n" : "@THAT\n");
    setup += "D=M\n";
  }

  return setup + code;
}

// @0
// M=M-1
// A=M
// D=M
// @13
// M=D
//
// @index
// D=A
// @loc
// [A=M]
// D=A+D
// @14
// M=D
// @13
// D=M
// @14
// A=M
// M=D
string handle_pop(string segm, string index) {
  string code1 = "@0\nM=M-1\nA=M\nD=M\n@13\nM=D\n@"+index+"\nD=A\n@{}\n";
  string heap = "A=M\n";
  string code2 = "D=A+D\n@14\nM=D\n@13\nD=M\n@14\nA=M\nM=D\n";
  string loc;
  string code;

  if (segm == "static") {
    code = code1 + code2;
    loc = "16";
  }
  else if (segm == "temp") {
    code = code1 + code2;
    loc = "5";
  }
  else if (segm == "local") {
    code = code1 + heap + code2;
    loc = "LCL";
  }
  else if (segm == "argument") {
    code = code1 + heap + code2;
    loc = "ARG";
  }
  else if (segm == "this") {
    code = code1 + heap + code2;
    loc = "THIS";
  }
  else if (segm == "that") {
    code = code1 + heap + code2;
    loc = "THAT";
  }
  else if (segm == "pointer") {
    index = ((index=="0") ? "THIS" : "THAT");
    return "@0\nM=M-1\nA=M\nD=M\n@"+index+"\nM=D\n";
  }

  return code.replace(code.find("{}"), 2, loc);
}

// arithmetic commands: add, sub, neg, eq, gt, lt, and, or, not
// @0
// M=M-1
// A=M
// { ignored for unary operators
// D=M
// @0
// M=M-1
// A=M
// }
// [comp]
// { relational operators
// D=M-D - comp
// @LABELX
// D;jump
// D=0
// @LABELY
// 0;JMP
// (LABELX)
// D=-1 - "true" value I guess
// (LABELY)
// @0
// A=M
// M=D
// }
// @0
// M=M+1
string handle_arith(string op) {
  string code_unary = "@0\nM=M-1\nA=M\n";
  string code = code_unary + "D=M\n@0\nM=M-1\nA=M\n";
  string comp = "";
  stringstream ss; ss << lbl;
  stringstream ss2; ss2 << lbl+1;
  string slbl = ss.str();
  string slbl2 = ss2.str();
  string code_relation =
    "D=M-D\n@LABEL"+slbl+"\nD;[]\nD=0\n@LABEL"+slbl2+"\n0;JMP\n(LABEL"+slbl+")\n"
    "D=-1\n(LABEL"+slbl2+")\n@0\nA=M\nM=D\n";
  string relation = "";
  string code_end = "@0\nM=M+1\n";

  if (op == "add") {
    comp = "M=M+D\n";
  }
  else if (op == "sub") {
    comp = "M=M-D\n";
  }
  else if (op == "neg") {
    comp = "M=-M\n";
    code = code_unary;
  }
  else if (op == "eq") {
    relation = code_relation;
    relation.replace(relation.find("[]"), 2, "JEQ");
    lbl += 2;
  }
  else if (op == "gt") {
    relation = code_relation;
    relation.replace(relation.find("[]"), 2, "JGT");
    lbl += 2;
  }
  else if (op == "lt") {
    relation = code_relation;
    relation.replace(relation.find("[]"), 2, "JLT");
    lbl += 2;
  }
  else if (op == "and") {
    comp = "M=M&D\n";
  }
  else if (op == "or") {
    comp = "M=M|D\n";
  }
  else if (op == "not") {
    comp = "M=!M\n";
    code = code_unary;
  }

  code += comp + relation + code_end;
  return code;
}

string translate(string line) {
  size_t pos_slash;
  vector<string> tokens;
  string result;

  // Remove any comments
  pos_slash = line.find("//");
  if (pos_slash != string::npos) {
    line = line.substr(0, pos_slash);
  }

  // Trim the line
  // line = trim_whitespace(line);
  line = trim(line, ' ');
  line = trim(line, '\n');
  line = trim(line, '\r');
  if (line.empty()) return "";

  tokens = split(line, ' ');

  if (tokens[0] == "push") {
    result = handle_push(tokens[1], tokens[2]);
  }
  else if (tokens[0] == "pop") {
    result = handle_pop(tokens[1], tokens[2]);
  }
  // arithmetic commands: add, sub, neg, eq, gt, lt, and, or, not
  else if (tokens[0] == "add") {
    result = handle_arith("add");
  }
  else if (tokens[0] == "sub") {
    result = handle_arith("sub");
  }
  else if (tokens[0] == "neg") {
    result = handle_arith("neg");
  }
  else if (tokens[0] == "eq") {
    result = handle_arith("eq");
  }
  else if (tokens[0] == "gt") {
    result = handle_arith("gt");
  }
  else if (tokens[0] == "lt") {
    result = handle_arith("lt");
  }
  else if (tokens[0] == "and") {
    result = handle_arith("and");
  }
  else if (tokens[0] == "or") {
    result = handle_arith("or");
  }
  else if (tokens[0] == "not") {
    result = handle_arith("not");
  }

  return result;
}

int main(int argc, char** args) {
  string fname_in;
  string fname_out;
  string line;
  string code;

  // iterate over each .vm file, converting it to .asm
  for (int i = 1; i < argc; i++) {
    fname_in = args[i];
    fname_out = fname_in.substr(0, fname_in.find(".")) + ".asm";
    lbl = 0;

    ifstream is(fname_in.data());
    ofstream os(fname_out.data());

    while(getline(is, line)) {

      code = translate(line);
      if (code != "")
        os << code << endl;
    }
  }

  return 0;
}
