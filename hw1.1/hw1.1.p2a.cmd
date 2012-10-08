# Command: 
find ~ 2>/dev/null | wc -l

Description="
`find ~` list all kinds of files (include directories) 
under ~ (home directory), 2>/dev/null redirects all 
errors to /dev/null, which means supressing them, 
wc -l means count the number of newline characters in 
stdin. In the result of find ~, each file occupies a 
line, so the count of lines is just the number of files.
"
