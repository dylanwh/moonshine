  .section ".rodata"
  .globl moon_boot_embed
  .type moon_boot_embed, @object
moon_boot_embed:
  .incbin "lua/boot.lua"
  .byte 0
  .size moon_boot_embed, .-moon_boot_embed
