// node_obj_address_table
// Entry_ID_Type :  Dir_Nat_Entry_ID_Type
// Entry_content :  node_obj_inner_address 16b = 4b + 9b + 2b + 1b
#include<iostream>

//#define DEBUG

typedef int File_Nat_Entry_ID_Type;
typedef int dir_meta_number_t;
typedef dir_meta_number_t Dir_Nat_Entry_ID_Type;
typedef unsigned int uint32_t;

// global vars:
// meta_block_area_t* dir_meta_mba;
//      struct dir_meta_obj;
        void dir_meta_init();
        dir_meta_number_t dir_meta_alloc_obj_id();
        void dir_meta_dealloc_obj_id(dir_meta_number_t dobj_id);
        void dir_meta_write_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id, struct dir_meta_obj * obj);
        struct dir_meta_obj* dir_meta_read_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id);
// meta_block_area_t* file_meta_mba;
//      struct file_meta_obj;
//      void file_meta_init();
//      file_meta_number_t file_meta_alloc_obj_id();
//      void file_meta_write_by_obj_id();
//      void file_meta_read_by_obj_id();

struct file_descriptor{
    uint32_t fhash;
};

#define Dir_Leaf_Node_Half_Degree   2
#define Dir_Leaf_Node_Degree        ( Dir_Leaf_Node_Half_Degree * 2 )

// 4KB
// fdes[]    =    { 0, 1, 2 ... n }
//                 /  /  / ... / \
// cobj_id[] =  { 0, 1, 2 ... n, n+1 }
struct dir_meta_obj{    // node
    struct file_descriptor  fdes       [ Dir_Leaf_Node_Degree + 1];        // 400 * 4 B = 1600 B 
    File_Nat_Entry_ID_Type  mobj_id    [ Dir_Leaf_Node_Degree + 1];        // 400 * 4 B = 1600 B
    Dir_Nat_Entry_ID_Type   cobj_id    [ Dir_Leaf_Node_Degree + 1 + 1 ];    // 401 * 2 B =  802 B
    Dir_Nat_Entry_ID_Type   father_id;                                  // 2 B
    uint32_t                is_leaf;                                    // 4 B
    dir_meta_number_t       fcount;                                     // 4 B
};// 1600 + 1600 + 802 + 2 + 4 = 4008 < 4096

class DirBTree{
    public:
    DirBTree();
    virtual ~DirBTree();

    void init();
    bool add_new_file(struct file_descriptor fdes, File_Nat_Entry_ID_Type mobj_id);
    bool del_file(struct file_descriptor fdes);
    // bool modify_file(struct file_descriptor fdes, File_Nat_Entry_ID_Type n_mobj_id);
    File_Nat_Entry_ID_Type get_file_meta_obj_id(struct file_descriptor fdes);
    void display();

    private:
    int root_dir_node_id;
    int node_degree;
    void insert_not_full(struct dir_meta_obj* dobj, 
        Dir_Nat_Entry_ID_Type dobj_id,
        struct file_descriptor fdes, 
        File_Nat_Entry_ID_Type mobj_id);
    void split_child(struct dir_meta_obj* dobj_parent,
        Dir_Nat_Entry_ID_Type dobj_parent_id,
        int index,
        struct dir_meta_obj* dobj_child,
        Dir_Nat_Entry_ID_Type dobj_child_id);
    void print_dir_node(struct dir_meta_obj* dobj,
        Dir_Nat_Entry_ID_Type dobj_id,
        int print_child);
    void delete_not_half(struct dir_meta_obj* dobj,
        Dir_Nat_Entry_ID_Type dobj_id,
        struct file_descriptor fdes);
    void union_child(struct dir_meta_obj* obj_parent, Dir_Nat_Entry_ID_Type obj_parent_id,
        struct dir_meta_obj* obj_cur,  Dir_Nat_Entry_ID_Type obj_cur_id,    // cur_id   == parent->cobj_id[ index ]
        struct dir_meta_obj* obj_righ, Dir_Nat_Entry_ID_Type obj_right_id,  // right_id == parent-<cobj_id[ index+1 ]
        int index);
};

