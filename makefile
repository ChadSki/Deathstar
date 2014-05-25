CC=gcc

deathstar_make: ZZTDeathstar.c ZZTTagClasses.c main.c
	$(CC) -std=c99 ZZTTagClasses.c ZZTDeathstar.c main.c -o deathstar