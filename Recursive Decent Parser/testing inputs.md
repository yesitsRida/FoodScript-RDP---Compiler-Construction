
g++ main.cpp -o parser.exe 
!! to run the program


## Valid examples

 digit age. $

taste(age). $

digit age = 18. display("Hello"). $

check(age >= 18) { display("Adult"). } otherwise { display("Minor"). } $ 
 
 repeat(digit i = 1. i <= 5. i++) { display(i). } $

`task greet() { display("Hello"). } greet(). $` — task declaration and call.


## Invalid examples (should report the actual line and unexpected/missing item)

digit age $

taste(). $

display("Hello". $
check(age >= 18) { display("Adult"). otherwise { display("Minor"). } $

repeat(digit i = 1. i <= 5 i++) { display(i). } $

digit total = 2 + . $ 
