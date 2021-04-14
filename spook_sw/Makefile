D=2
all:
	gcc spook_masked/clyde_masked.c spook_masked/prng.c spook_masked/utils_masking.c spook_ref/primitives.c spook_ref/shadow_32bit.c -I spook_masked/ -I spook_ref main.c -DD=$D -DROUNDREDUCED -o implem -O3
clean:
	rm implem

