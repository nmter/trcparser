db_src=naive_db.hpp test_naive_db.cpp
msr_src=naive_db.hpp msr_trace_stat.cpp
CC=g++
all: trcstatMSR
trcstatMSR:
	$(CC) -g $(msr_src) -o $@

TEST_DB:
	$(CC) -g $(db_src) -o $@

clean:
	rm -rf TEST_DB trcstatMSR
