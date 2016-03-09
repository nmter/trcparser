db_src=naive_db.hpp test_naive_db.cpp
msr_gendb_src=naive_db.hpp msr_gen_blk_db.cpp
run_trace=naive_db.hpp run_trace.cpp
CC=g++
all: msr_gen_blk_db run_trace

msr_gen_blk_db:
	$(CC) -g $(msr_gendb_src) -o $@

run_trace:
	$(CC) -g $(run_trace) -o $@

TEST_DB:
	$(CC) -g $(db_src) -o $@

clean:
	rm -rf TEST_DB msr_gen_blk_db run_trace
