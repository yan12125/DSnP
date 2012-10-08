# Command: 
nl -n ln hw1.1.p2.log | sed 's/ /:/' | grep -v '   Darr (nothing)' | grep 'Output Transition Time' | sort -nt: -k3 > hw1.1.p2b.log

Description="
nl -n ln        nl adds line number to each line, where -n ln assigns 
                line number format, which means numbers will be justified left
sed 's/ /:/'    replaces the first blank with a colon in each line
grep -v         extract all lines DOES NOT match the pattern, while
grep            extract all lines match the pattern
sort -n         means compare as numbers
     -t:        means use : symbol as field separation character
     -k3        means sort by the 3nd field
Lastly, redirect all outputs to file hw1.1.p2b.log
"
