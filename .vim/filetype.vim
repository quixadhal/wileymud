" Taken from http://en.wikipedia.org/wiki/Wikipedia:Text_editor_support#Vim
" 	Ian Tegebo <ian.tegebo@gmail.com>

augroup filetypedetect
autocmd BufRead,BufNewFile *.wiki setfiletype Wikipedia
autocmd BufRead,BufNewFile *.wikipedia.org* setfiletype Wikipedia
augroup END

