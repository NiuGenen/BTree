//
// Created by raohui on 17-5-11.
//

#include <iostream>
#include "BTree.h"
BTree::BTree() : root(nullptr), t(2) {

}
BTree::~BTree() {
  DeleteTree(root);
  delete root;
}
bool BTree::Insert(int key) {
  PBTreeNode tempNode = root;
  if (nullptr == tempNode) {
    tempNode = AllocateNode();
    tempNode->isLeaf = true;
    tempNode->keyCount = 0;
    root = tempNode;
  }

  if (tempNode->keyCount == 2 * t - 1) {
    PBTreeNode newNode = AllocateNode();
    root = newNode;
    newNode->isLeaf = false;
    newNode->keyCount = 0;
    newNode->chlids[1] = tempNode;
    SplitChild(newNode, 1, tempNode);
    InsertNotFull(newNode, key);
  } else {
    InsertNotFull(tempNode, key);
  }
  return false;
}

void BTree::DeleteTree(PBTreeNode pNode) {
  if (pNode->isLeaf) {
    delete[]pNode->key;
    delete[]pNode->chlids;
  } else {
    for (int i = 1; i <= pNode->keyCount + 1; ++i) {
      DeleteTree(pNode->chlids[i]);
      delete pNode->chlids[i];
    }

    delete[]pNode->key;
    delete[]pNode->chlids;
  }
}
PBTreeNode BTree::AllocateNode() {
  PBTreeNode result = new BTreeNode;
  result->key = new int[2 * t];
  result->chlids = new PBTreeNode[2 * t + 1];
  for (int i = 0; i < 2 * t; ++i) {
    result->key[i] = 0;
    result->chlids[i] = nullptr;
  }
  result->chlids[2 * t] = nullptr;
  return result;
}
void BTree::SplitChild(PBTreeNode pParents, int index, PBTreeNode pChild) {
  PBTreeNode pNewChild = AllocateNode();
  pNewChild->isLeaf = pChild->isLeaf;
  pNewChild->keyCount = t - 1;

  for (int j = 1; j < t; ++j) {
    pNewChild->key[j] = pChild->key[j + t];
  }
  if (!pChild->isLeaf) {
    for (int j = 1; j <= t; ++j) {
      pNewChild->chlids[j] = pChild->chlids[j + t];
    }
  }

  pChild->keyCount = t - 1;
  for (int j = pParents->keyCount + 1; j > index; j--) {
    pParents->chlids[j + 1] = pParents->chlids[j];
  }
  pParents->chlids[index + 1] = pNewChild;

  for (int j = pParents->keyCount; j >= index; j--) {
    pParents->key[j + 1] = pParents->key[j];
  }
  pParents->key[index] = pChild->key[t];
  pParents->keyCount++;
}
void BTree::InsertNotFull(PBTreeNode pNode, int key) {
  int i = pNode->keyCount;
  if (pNode->isLeaf) {
    while (i >= 1 && key < pNode->key[i]) {
      pNode->key[i + 1] = pNode->key[i];
      i--;
    }
    pNode->key[i + 1] = key;
    pNode->keyCount++;
  } else {
    while (i >= 1 && key < pNode->key[i]) {
      i--;
    }
    i++;

    if (pNode->chlids[i]->keyCount == 2 * t - 1) {
      SplitChild(pNode, i, pNode->chlids[i]);
      if (key > pNode->key[i]) {
        i++;
      }
    }

    InsertNotFull(pNode->chlids[i], key);
  }
}
void BTree::Display() {
  Print(root);
}

void BTree::Print(PBTreeNode pNode) {
  if (pNode->isLeaf) {
    std::cout << "leafNode keyCout = " << pNode->keyCount << " key list:";
    for (int i = 1; i <= pNode->keyCount - 1; ++i) {
      std::cout << pNode->key[i] << ",";
    }
    std::cout << pNode->key[pNode->keyCount] << "\n";
  } else {
    for (int i = 1; i <= pNode->keyCount + 1; ++i) {
      Print(pNode->chlids[i]);
    }
    std::cout << "innerNode keyCout = " << pNode->keyCount << "key list:";
    for (int i = 1; i <= pNode->keyCount - 1; ++i) {
      std::cout << pNode->key[i] << ",";
    }
    std::cout << pNode->key[pNode->keyCount] << "\n";
  }

}
bool BTree::Delete(int key) {
  return DeleteNotHalf(root, key);
}
bool BTree::DeleteNotHalf(PBTreeNode pNode, int key) {
  int i = 1;
  while (i <= pNode->keyCount && key > pNode->key[i]) {
    i++;
  }

  if (pNode->isLeaf) {
    if (i <= pNode->keyCount && key == pNode->key[i]) {
      for (int j = i; j < pNode->keyCount; ++j) {
        pNode->key[j] = pNode->key[j + 1];
      }
      pNode->keyCount--;
      return true;
    } else {
      printf("Don't find the key:%d\n", key);
      return false;
    }
  } else if (i <= pNode->keyCount && key == pNode->key[i]) {

    if (pNode->chlids[i]->keyCount >= t) {

      key = Max(pNode->chlids[i]);
      pNode->key[i] = key;

      return DeleteNotHalf(pNode->chlids[i], key);
    } else if (pNode->chlids[i + 1]->keyCount >= t) {
      key = Min(pNode->chlids[i + 1]);
      pNode->key[i] = key;

      return DeleteNotHalf(pNode->chlids[i + 1], key);

    } else {
      PBTreeNode UnionNode = UnionChild(pNode, pNode->chlids[i], pNode->chlids[i + 1], i);
      return DeleteNotHalf(UnionNode, key);
    }
  } else if (pNode->chlids[i]->keyCount == t - 1) {
    if (i > 1 && pNode->chlids[i - 1]->keyCount >= t) {
      PBTreeNode pPreNode = pNode->chlids[i - 1];
      PBTreeNode pMidNode = pNode->chlids[i];
      int PreNodeKeyCount = pPreNode->keyCount;
      int MidNodeKeyCount = pMidNode->keyCount;

      for (int j = MidNodeKeyCount; j >= 1; j--) {
        pMidNode->key[j + 1] = pMidNode->key[j];
      }
      pMidNode->key[1] = pNode->key[i - 1];
      pMidNode->keyCount++;

      for (int j = MidNodeKeyCount + 1; j >= 1; j--) {
        pMidNode->chlids[j + 1] = pMidNode->chlids[j];
      }
      pMidNode->chlids[1] = pPreNode->chlids[PreNodeKeyCount + 1];
      pNode->key[i - 1] = pPreNode->key[PreNodeKeyCount];

      pPreNode->key[PreNodeKeyCount] = 0;
      pPreNode->chlids[PreNodeKeyCount + 1] = nullptr;
      pPreNode->keyCount--;

      return DeleteNotHalf(pMidNode, key);

    } else if (i <= pNode->keyCount && pNode->chlids[i + 1]->keyCount >= t) {
      PBTreeNode pMidNode = pNode->chlids[i];
      PBTreeNode pNextNode = pNode->chlids[i + 1];
      int MidNodeKeyCount = pMidNode->keyCount;
      int NextNodeKeyCount = pNextNode->keyCount;

      pMidNode->key[MidNodeKeyCount + 1] = pNode->key[i];
      pMidNode->chlids[MidNodeKeyCount + 2] = pNextNode->chlids[1];
      pMidNode->keyCount++;
      pNode->key[i] = pNextNode->key[1];
      for (int j = 1; j < NextNodeKeyCount; ++j) {
        pNextNode->key[j] = pNextNode->key[j + 1];
      }
      for (int j = 1; j <= NextNodeKeyCount; ++j) {
        pNextNode->chlids[j] = pNextNode->chlids[j + 1];
      }
      pNextNode->keyCount--;
      return DeleteNotHalf(pMidNode, key);
    } else {
      if (i > pNode->keyCount) {
        i--;
      }
      PBTreeNode UnionNode = UnionChild(pNode, pNode->chlids[i], pNode->chlids[i + 1], i);
      return DeleteNotHalf(UnionNode, key);
    }

  }
  return DeleteNotHalf(pNode->chlids[i], key);
}
int BTree::Max(PBTreeNode pNode) {
  if (!pNode->isLeaf) {
    pNode = pNode->chlids[pNode->keyCount + 1];
  }
  return pNode->key[pNode->keyCount];
}
int BTree::Min(PBTreeNode pNode) {
  if (!pNode->isLeaf) {
    pNode = pNode->chlids[1];
  }
  return pNode->key[1];
}
PBTreeNode BTree::UnionChild(PBTreeNode pParents, PBTreeNode pLeftChild, PBTreeNode pRightChild, int index) {
  int LChlidKeyCount = pLeftChild->keyCount;
  int RChlidKeyCount = pRightChild->keyCount;
  for (int i = 1; i <= RChlidKeyCount; ++i) {
    pLeftChild->key[LChlidKeyCount + 1 + i] = pRightChild->key[i];
  }
  pLeftChild->key[LChlidKeyCount + 1] = pParents->key[index];

  for (int j = 1; j <= RChlidKeyCount + 1; ++j) {
    pLeftChild->chlids[LChlidKeyCount + 1 + j] = pRightChild->chlids[j];
  }
  pLeftChild->keyCount = pLeftChild->keyCount + pRightChild->keyCount + 1;

  for (int k = index; k < pParents->keyCount; ++k) {
    pParents->key[k] = pParents->key[k + 1];
  }

  for (int l = index; l <= pParents->keyCount; ++l) {
    pParents->chlids[l] = pParents->chlids[l + 1];
  }

  pParents->key[pParents->keyCount] = 0;
  pParents->chlids[pParents->keyCount + 1] = nullptr;
  pParents->keyCount--;

  DellocateNode(pRightChild);

  if (pParents->keyCount == 0) {
    DellocateNode(root);
    root = pLeftChild;
  }

  return pLeftChild;
}
void BTree::DellocateNode(PBTreeNode pNode) {
  delete[] pNode->key;
  delete[] pNode->chlids;
  delete pNode;
}
