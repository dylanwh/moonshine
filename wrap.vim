" Vim syntax file
" Language:     Wrap (Lua)
" Maintainer:   Dylan William Hardison <dylan@hardison.net>
" Last Change:  11-Sep-2007.
" Remark:       DSL for wrapping C code.

if exists("b:current_syntax")
    finish
endif

syn include @cTop syntax/c.vim
syn include @cTop2 after/syntax/c.vim

syn keyword wrapStatement require
syn keyword wrapSpecial method meta include finish class print
syn keyword wrapMagic boxed default line

syn region wrapString start=/'/ end=/'/
syn region wrapString start=/"/ end=/"/
syn region wrapCode matchgroup=wrapSpecial start=/\[\[/ end=/\]\]/ keepend contains=@cTop,@cTop2

syn match wrapName /'\w\+'/


hi def link wrapStatement Statement
hi def link wrapSpecial Special
hi def link wrapMagic   Type
hi def link wrapName    Identifier
hi def link wrapString  String
hi def link wrapCode None

let b:current_syntax = "wrap"
