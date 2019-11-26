with import <nixpkgs> {};
runCommand "build" {
  buildInputs = [ gcc ];
}
  ''
g++ -Wall -Wextra -O2 -std=c++17 ${./.}/main.cpp -o $out
''
