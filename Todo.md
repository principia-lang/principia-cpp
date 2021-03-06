# To do list

/// TODO: Use alloca when we can

/// TODO: Background thread to analyse the performance oprofile style
/// TODO: Background thread to dynamically optimize code

/// TODO: Find a contrived example where the validity of the mutual recursion depends on an unsolved problem.
///       f x ↦ (≔(≔ if (↦(≔ complicated_function_1 x)) (↦(≔ g x)) (↦x))
///       g x ↦ (≔(≔ if (↦(≔ complicated_function_2 x)) (↦(≔ f x)) (↦x))
/// Then add a variable to force the closure to be dynamic.
/// TODO: Find a contrived example where a partially filled closure might still be executed.
/// TODO: Proof that finding an executable closure storing order is equivalent to the halting problem.

/// TODO: McCarthy 91 function

// M n ↦ (≔(≔ if (≔ greater n 100) (↦(≔ sub n 10)) (↦(≔ M (≔ M (≔ add n 11))))))


/// TODO: Add proof language

/*

Hoare Logic:

f a ↦ b
	Theorems A(a) about a
	Derivations / proofs
	Theorems B(b) about b

Theorems Y(y) about y
x ≔ f y
	Verify A(y) ⊆ Y(y)
	Therefore B(x)

How do we do derivations?

IDEA: We can use abstract functions, i.e. functions without implementations
 to represent (non-constructive) theorems.
This function should generally return just the input variables,
however, it may return new values. This would be an existence proof!

How do we do axioms?

IDEA: We omit the derivations from the function.

This gives four kinds of functions

- Without implementation and without derivation. (no-op)
- With implementation and without derivation. (unproven construction)
- Without implementation and with derivation. (abstract/pure/nonconstructive proof)
- With implementation and with derivation. (plain function with proof)

*/


// Boolean edges as truth values
// Theorems as 

// Implement boolean values as:
// True a b ↦ a
// False a b ↦ b
// Then the following are equivalent:
// a ≔ if condition then else
// a ≔ condition then else

// IDEA: False = the non-halting function, true = everything else


/*

TODO: Namespaces and other sugar.

Namespaces = named scopes!

f a b ↦ r
	x …
	y …

then allows the global scope to access the constants:

f.x
f.y


Simmilarly, one can define a namespace:

f ↦
	x …
	y …

TODO: importing namespaces:

import int Principia.Integers

int.add a b


TODO: process docstrings:

ignore “uncommented piece of code”

doc symbol “Documentation”


TODO: Reflection

a ≔ parse “some piece of source code”

doc parse “Returns a closure representing the parsed source code, all unbound variables are arguments, all exported symbols are returns.”

c ≔ link a b

doc link “Go trough all arguments of the closure a and link them to variables in scope b. Unlinked symbols remain.”

rets ≔ eval f args

doc eval “Execute f with arguments args and return the results in rets.”


TODO: Multiple implementations (algorithms) of the same function

May not differ in the function specification. Must contain full proofs. May differ in runtime resource usage.

The compiler is free to change between different implementations of a function as it sees fit (optimizing). The programmer can provide hints to direct this choice.

Alternative: Functions have a unique implementation, but one can proof equivalences, which the compiler will then use.

Suppose we have two functions, safediv and unsafediv, where the later is faster, but has the additional precondition that m ≠ 0. If the compiler can deduce statically or runtime that always m ≠ 0, then it may move to the unsafediv.


TODO: Types‽ What do we mean we we say “pre n : integer”?
That there exists functions +, -, ×, |·|, =, ≠, <, ≤, >, ≥, etc… doing the ‘expected’ thing.

They can be implemented as unspecified propositions: IsInteger(n). Dependent types can be implemented as more complex variants: IsMatrix(A, ℂ, 2, 4) to state that A is a complex valued 2 × 4 matrix.


TODO: Performance characteristics:

slowdiv n m ↦ q r
	complexity.time slowdiv log(n) + log(m) 
	complexity.memory slowdiv 2 * m


TODO: Compiler hints

intdiv n m ↦ q r
	hint intdiv hint.Inline
	hint intdiv hint.Tailrecurse
	force intdiv force.Inline


TODO: Something like the Common Lisp Object System and "The Art of the Metaobject Protocol"


TODO: Allow syntax modifications in language:

Lexer -> Preprocessor -> Parser
Lexer: Chunk source into identifiers
Preprocessor: resolve scoping and references
Parser: 


[parse rule:  #1 + #2 ↦ (. ≔ plus #1 #2) ]
[parse rule:  if #1 then #2 else #3  ↦ (≔(≔if #1 (↦ #2) (↦#3))) ]
[parse rule:  /#1/ ↦ ( . ≔  regexp_parse("#3") ) ]
etc…

