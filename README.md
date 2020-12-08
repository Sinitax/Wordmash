# Wordmash

Wordmash is a console application written in c++ used for creating pronouncable words with english speech patterns. It accomplishes this by analyzing existing english texts and creating a probability table based on n preceding letters. In this program, n is reffered to as the order of approximation. After generating such a table with 'chain_gen.exe', using an input dictionary and an order of marcov chains, one can generate an arbitrary length of text using 'lang_gen.exe'. By default input data for 'chain_gen.exe' must contain words exclusively in lower case english alphabet, although this may be changed through modifying the range in the 'validChar' and 'validChars' functions. If too little data is used to populate the table of probabilites it can occur that a given sequence of letters has no known successor. The program will then choose at random from the known map of letters and output a warning to console.


## References

[Information Theory by Claude Shanon](http://www.mast.queensu.ca/~math474/shannon1948.pdf)    
[Simplified Explanation](https://www.youtube.com/watch?v=3pRR8OK4UfE&t=181s)   
[Simple English Words Dictionary (alpha)](https://github.com/dwyl/english-words/blob/master/words_alpha.txt)
