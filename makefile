all:btree_test btree_test_
	
btree_test:BTree.h BTree.cpp main.cpp
	g++ -o btree_test BTree.h BTree.cpp main.cpp -std=c++0x

btree_test_:BTree_.h BTree_.cpp main_.cpp
	g++ -o btree_test_ BTree_.h BTree_.cpp main_.cpp -std=c++0x
