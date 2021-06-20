set nocompatible
set backspace=0
set nobackup
set nowritebackup
set novisualbell
set noerrorbells

set viminfo=
set ruler
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set showmatch

set nohlsearch
set incsearch
set ignorecase
set smartcase
set showfulltag

filetype plugin indent on
syntax on

set nowrap
set expandtab
set shiftwidth=4
set softtabstop=4

"set noautoindent
"set nosmartindent
"set nocindent
"set indentexpr=

"au FileType pl,pm setlocal comments=
au! BufRead,BufNewFile *.pgc set filetype=esqlc
autocmd BufNewFile,BufRead /home/quixadhal/dw/lib/* set filetype=lpc
autocmd BufNewFile,BufRead /home/quixadhal/sky/lib/* set filetype=lpc

"set statusline=%F%m%r%h%w\ [TYPE=%Y\ %{&ff}]\ [%l/%L\ (%p%%)]

"set nofoldenable
"set fdm=indent
"nnoremap <space> za

"set cursorline

set pastetoggle=<f5>

"Mediawiki files
autocmd BufRead,BufNewFile *.mediawiki set filetype=Wikipedia
"autocmd BufRead,BufNewFile *.wikipedia.org* setfiletype Wikipedia

"Set tab expansion off for SQL files, since tabs are a delimiter
autocmd BufRead,BufNewFile *.sql set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab
"autocmd BufRead,BufNewFile HACKLOG.* set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab
autocmd BufRead,BufNewFile *akefile set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

:command! -nargs=1 -range SuperRetab <line1>,<line2>s/\v%(^ *)@<= {<args>}/\t/g

set enc=utf-8
set t_Co=256
set t_ut=
colorscheme codedark
set encoding=utf-8
set fileencodings=utf-8

autocmd BufNewFile,BufRead *_css.php set filetype=css
autocmd BufNewFile,BufRead *_js.php set filetype=javascript
