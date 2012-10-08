" Method are found on http://vim.wikia.com/wiki/Record_a_recursive_macro
" run the macro in a macro will make it run recursively

" load data file
:read hw1.1.p3.dat

" clear the q register first because line 11 will call the macro itself
qqq
" Start to recording to register q
qq
" search for Name
/Name
" delete until character U
d/U
" Join the next line
J
" delete until the character :
d/:
" delete : itself
x
" do once again
d/:
x
" run the macro recursively
@q
" stop recording
q

" call the macro
@q
" save
:w hw1.1.p3.log
