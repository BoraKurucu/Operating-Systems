all: schedule

schedule: schedule.c
	gcc -g -Wall -o schedule -pthread schedule.c -lrt

clean:
	rm -fr *~ schedule
