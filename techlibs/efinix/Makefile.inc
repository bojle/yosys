
OBJS += techlibs/efinix/synth_efinix.o
OBJS += techlibs/efinix/efinix_fixcarry.o
OBJS += techlibs/efinix/efx_mult_bypass.o

$(eval $(call add_share_file,share/efinix,techlibs/efinix/cells_map.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/arith_map.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/cells_sim.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/brams_map.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/gbuf_map.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/mul_map.v))
$(eval $(call add_share_file,share/efinix,techlibs/efinix/brams.txt))
