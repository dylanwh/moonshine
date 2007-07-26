  .section ".rodata"
  .globl lua_embed_boot
  .type lua_embed_boot, @object
lua_embed_boot:
  .incbin "lua/boot.lua"
  .byte 0
  .size lua_embed_boot, .-lua_embed_boot
