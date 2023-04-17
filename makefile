all:
	gcc -o ipkcpc src/ipkcpc.c src/ipkcpUtils.c
	gcc -o ipkcpd src/ipkcpd.c src/ipkcpUtils.c -lm