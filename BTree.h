//
// Created by raohui on 17-5-11.
//

#ifndef BTREE_MYBTEE_H
#define BTREE_MYBTEE_H
typedef struct BTreeNode {
  int keyCount;
  int *key;
  BTreeNode **chlids;
  bool isLeaf;
} BTreeNode, *PBTreeNode;
class BTree {
 public:
  BTree(int tt);
  virtual ~BTree();
  bool Insert(int key);
  bool Delete(int key);
  void Display();
 private:
  PBTreeNode root;
  int t;
  void DeleteTree(PBTreeNode pNode);
  PBTreeNode AllocateNode();
  void SplitChild(PBTreeNode pNode, int i, PBTreeNode pTreeNode);
  void InsertNotFull(PBTreeNode pNode, int key);
  void Print(PBTreeNode pNode);
  bool DeleteNotHalf(PBTreeNode pNode, int key);
  int Max(BTreeNode *pNode);
  int Min(BTreeNode *pNode);
  PBTreeNode UnionChild(PBTreeNode pParents, BTreeNode *pLeftChild, BTreeNode *pRightChild, int index);
  void DellocateNode(PBTreeNode pNode);
};

#endif //BTREE_MYBTEE_H
