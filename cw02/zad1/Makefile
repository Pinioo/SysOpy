RES = wyniki.txt
GEN = ./lib_main generate
SCOPY = ./sys_main copy
LCOPY = ./lib_main copy
SSORT = ./sys_main sort
LSORT = ./lib_main sort

TEST1 = test1_1
TEST1_S = 400000 1

TEST2 = test1_2
TEST2_S = 1000000 1

TEST3 = test4_1
TEST3_S = 200000 4

TEST4 = test4_2
TEST4_S = 1000000 4

TEST5 = test512_1
TEST5_S = 200000 512

TEST6 = test512_2
TEST6_S = 1000000 512

TEST7 = test1024_1
TEST7_S = 100000 1024

TEST8 = test1024_2
TEST8_S = 600000 1024

TEST9 = test4096_1
TEST9_S = 100000 4096

TEST10 = test4096_2
TEST10_S = 400000 4096

TEST11 = test8192_1
TEST11_S = 100000 8192

TEST12 = test8192_2
TEST12_S = 300000 8192

all:
	gcc lib_main.c -o lib_main
	gcc sys_main.c -o sys_main

test:
	$(GEN) test_subject 15 6
	./lib_main print test_subject 15 6
	echo "sys copying"
	$(SCOPY) test_subject test_subject_copy 15 6
	rm test_subject_copy
	echo "lib copying"
	$(LCOPY) test_subject test_subject_copy 15 6
	echo "sys sorting"
	$(SSORT) test_subject 15 6
	./sys_main print test_subject 15 6
	echo "lib sorting"
	$(LSORT) test_subject_copy 15 6
	./lib_main print test_subject_copy 15 6

times:
	$(GEN) $(TEST1) $(TEST1_S)	
	$(GEN) $(TEST2) $(TEST2_S) 
	$(GEN) $(TEST3) $(TEST3_S) 
	$(GEN) $(TEST4) $(TEST4_S) 
	$(GEN) $(TEST5) $(TEST5_S) 
	$(GEN) $(TEST6) $(TEST6_S) 
	$(GEN) $(TEST7) $(TEST7_S) 
	$(GEN) $(TEST8) $(TEST8_S) 
	$(GEN) $(TEST9) $(TEST9_S) 
	$(GEN) $(TEST10) $(TEST10_S) 
	$(GEN) $(TEST11) $(TEST11_S) 	
	$(GEN) $(TEST12) $(TEST12_S)
	echo -n "" > $(RES)
	make TEST="$(TEST1)" TEST_S="$(TEST1_S)" FILE="$(RES)" single_times
	make TEST="$(TEST2)" TEST_S="$(TEST2_S)" FILE="$(RES)" single_times
	make TEST="$(TEST3)" TEST_S="$(TEST3_S)" FILE="$(RES)" single_times
	make TEST="$(TEST4)" TEST_S="$(TEST4_S)" FILE="$(RES)" single_times
	make TEST="$(TEST5)" TEST_S="$(TEST5_S)" FILE="$(RES)" single_times
	make TEST="$(TEST6)" TEST_S="$(TEST6_S)" FILE="$(RES)" single_times
	make TEST="$(TEST7)" TEST_S="$(TEST7_S)" FILE="$(RES)" single_times
	make TEST="$(TEST8)" TEST_S="$(TEST8_S)" FILE="$(RES)" single_times
	make TEST="$(TEST9)" TEST_S="$(TEST9_S)" FILE="$(RES)" single_times
	make TEST="$(TEST10)" TEST_S="$(TEST10_S)" FILE="$(RES)" single_times
	make TEST="$(TEST11)" TEST_S="$(TEST11_S)" FILE="$(RES)" single_times
	make TEST="$(TEST12)" TEST_S="$(TEST12_S)" FILE="$(RES)" single_times

single_times:
	echo "Times for (records block_size) = $(TEST_S)" >> $(RES)
	$(SCOPY) $(TEST) $(TEST).tmp $(TEST_S) $(FILE)
	rm $(TEST).tmp
	$(LCOPY) $(TEST) $(TEST).tmp $(TEST_S) $(FILE)
	$(SSORT) $(TEST).tmp $(TEST_S) $(FILE)
	$(LSORT) $(TEST) $(TEST_S) $(FILE)
	rm $(TEST).tmp
	echo "" >> $(RES)

clean:
	rm test* sys_main lib_main
