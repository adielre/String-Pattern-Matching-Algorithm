# String-Pattern-Matching-Algorithm

The program enable simple search for pattern in a given input text. It performed by implementaion of a dictionary-matching algorithm that locates elements of a finite set of strings (the "dictionary") within an input text. It matches all patterns "at once", so the complexity of the algorithm is linear in the length of the patterns + the length of the searched text + the number of output matches.

## Files
* `slist.c`<br/>
This file represent a implementation of a simple generic single linked list.(the headers appears in <b>slist.h</b>).
The methods of linked list(init, append.. etc) apply to construct the FSM(Finite-state machine) that composed of set of spesific words.

* `pattern-matching.c`<br/>
This file represent the main algoritm, that matches multiple patterns simultaneously, by first constructing a Deterministic Finite Automaton (DFA) representing the patterns set, and then, with this DFA on its disposal, processing the text in a single pass. (total o(n) efficiency).(the headers appears in <b>pattern-matching.h</b>).
