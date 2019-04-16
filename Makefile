all:
		gcc client.c -o client
		gcc monitor.c -o monitor
		gcc aws.c -o aws
		gcc serverA.c -o serverA
		gcc serverB.c -o serverB
		gcc serverC.c -o serverC -lm

serverA:
		./serverA

serverB:
		./serverB

serverC:
		./serverC

aws:
		./aws

monitor:
		./monitor
