# BTree

# dir_meta_tree

- class DirBTree : btree for `struct dir_meta_obj`

    // public interface of DirBTree
     	
    void init();
    bool add_new_file(
        struct file_descriptor fdes,
        File_Nat_Entry_ID_Type mobj_id );
    bool del_file(
        struct file_descriptor fdes );
    File_Nat_Entry_ID_Type get_file_meta_obj_id(
        struct file_descriptor fdes );
    void display();

- `struct dir_meta_obj` : 4KB obj ; R/W by `meta_block_area` with its `obj_id`
	- this is also the node of DirBTree

    #define Dir_Node_Half 2
    #define Dir_Node_Degree ( Dir_Node_Half * 2 + 1 )
     	
    struct dir_meta_obj{
        struct file_descriptor fdes[ Dir_Node_Degree ];        // 4 B * degree
        File_Nat_Entry_ID_Type mobj_id [Dir_Node_Degree ];     // 4 B * degree
        Dir_Nat_entry_ID_Type  cobj_id [ Dir_Node_Degree + 1]; // 2 B * degree
        uint32_t is_leaf;                                      // 4 B
        dir_meta_number_t fcount;                              // 4 B
    }// size = 10 * degree + 20
     
    struct file_descriptor fdes{
        uint32_t fhash;
    }// 4 B

- test stub : function supported by `meta_block_area_t* dir_meta_area`

    // global var for test 
    
    #define uint16_size 65536
    dir_meta_number_t dir_obj_id_table[ uint16_size ];
    char dir_obj_table_use[ uint16_size ];
    struct dir_meta_obj* dir_obj_table[ uint16_size ];
     
    // function
    
    void dir_meta_init();	// obly for test
    // public interface of meta_block_area_t* dir_meta_mba
    Dir_Nat_entry_ID_Type dir_meta_alloc_obj_id();
    void dir_meta_dealloc_obj_id(
        Dir_Nat_Entry_ID_Type dobj_id );
    void dir_meta_write_by_obj_id(
        Dir_Nat_Entry_ID_Type dobj_id,
        struct dir_meta_obj* dobj );
    struct dir_meta_obj* dir_meta_read_by_obj_id(
        Dir_Nat_Entry_ID_Type dobj_id );
