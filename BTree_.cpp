#include <iostream>
#include "BTree_.h"

#define DEBUG

DirBTree::~DirBTree(){
}

DirBTree::DirBTree(){
}

void DirBTree::init(){
    root_dir_node_id = dir_meta_alloc_obj_id();
    struct dir_meta_obj* root = dir_meta_read_by_obj_id( root_dir_node_id );
    root->is_leaf = 1;
    dir_meta_write_by_obj_id(root_dir_node_id, root);
}

bool DirBTree::add_new_file(struct file_descriptor fdes, File_Nat_Entry_ID_Type mobj_id)
{
    struct dir_meta_obj* dobj = dir_meta_read_by_obj_id( root_dir_node_id );
    if( dobj == nullptr ){
        //throw or alloc_new_id
    }

    if( dobj-> fcount == Dir_Leaf_Node_Degree ){
        //split child
        Dir_Nat_Entry_ID_Type new_father_id = dir_meta_alloc_obj_id();
        struct dir_meta_obj* father_obj = dir_meta_read_by_obj_id( new_father_id );
        father_obj->fcount = 0;
        father_obj->is_leaf = 0;
        father_obj->cobj_id[0] = root_dir_node_id;
        split_child( father_obj, new_father_id , 0, dobj, root_dir_node_id);
        root_dir_node_id = new_father_id;
        dobj = dir_meta_read_by_obj_id( root_dir_node_id );
        insert_not_full( dobj, root_dir_node_id, fdes, mobj_id );
    }
    else{
        insert_not_full(dobj, root_dir_node_id, fdes, mobj_id );
    }

    return true;
}

// insert @fdes & @mobj_id into @dobj
// require @dobj is not full
void DirBTree::insert_not_full(struct dir_meta_obj* dobj,
        Dir_Nat_Entry_ID_Type dobj_id,
        struct file_descriptor fdes,
        File_Nat_Entry_ID_Type mobj_id)
{
    int i = dobj->fcount - 1; // last index of dobj

    if( dobj->is_leaf ){ // dobj is leaf node
        while(i >= 0 && fdes.fhash < dobj->fdes[i].fhash ){// move value to leave one place at i
            dobj->fdes[i+1] = dobj->fdes[i];
            dobj->mobj_id[i+1] = dobj->mobj_id[i];
            i--;
        }
        dobj->fdes[i+1] = fdes; // insert directly
        dobj->mobj_id[i+1] = mobj_id;
        dobj->fcount++;
        dir_meta_write_by_obj_id(dobj_id, dobj); // dump by meta_area
    }
    else{ // not leaf node
        while( i >= 0 && fdes.fhash < dobj->fdes[i].fhash ) i--;
        i++;// find @i where fdes[i-1] < fdes < fdes[i]
        // fdes should be inserted into cobj[i]

        Dir_Nat_Entry_ID_Type cobj_id = dobj->cobj_id[i];
        struct dir_meta_obj* cobj = dir_meta_read_by_obj_id( cobj_id ); // get child obj
        if( cobj == nullptr ){
            //throw
        }

        //  fdes =   i-1,  i, i+1
        //           /    /   / \
        //  cobj = i-1, [i], i+1, i+2
        if( cobj->fcount == Dir_Leaf_Node_Degree ){ // if child obj is full
            split_child(dobj, dobj_id, i, cobj, cobj_id);
            //      fdes =   i-1, 'i' , i+1,  i+2
            // =>            /    /     /     /
            //      cobj = i-1, 'i',  'i+1', i+2
            if( fdes.fhash > dobj->fdes[i].fhash ){
                i++;
            }

            cobj_id = dobj->cobj_id[i];
            cobj = dir_meta_read_by_obj_id( cobj_id ); // get new child obj
            if( cobj == nullptr ){
                //throw
            }
        }

        insert_not_full( cobj, cobj_id, fdes, mobj_id); // insert into child
    }
}

// split @dobj_parent->cobj[ index ]
//  fdes =   i-1,  i, i+1            fdes =   i-1, 'i' , i+1,  i+2
//           /    /   / \      ==>            /    /     /     /
//  cobj = i-1, [i], i+1, i+2        cobj = i-1, 'i',  'i+1', i+2
//  AND dump them
void DirBTree::split_child(struct dir_meta_obj* dobj_parent,
    Dir_Nat_Entry_ID_Type dobj_parent_id,
    int index,
    struct dir_meta_obj* dobj_child,
    Dir_Nat_Entry_ID_Type dobj_child_id)
{
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : index = " << index << std::endl;
#endif
    //assert( dobj_parent->cobj_id[index] == dobj_child_id )

#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : ready to alloc n_child" << std::endl;
#endif
    // alloc new child dobj
    Dir_Nat_Entry_ID_Type n_dobj_child_id = dir_meta_alloc_obj_id();
    struct dir_meta_obj* n_dobj_child = dir_meta_read_by_obj_id( n_dobj_child_id );
    if( n_dobj_child == nullptr ){
        //throw
    }
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : finish to alloc n_child" << std::endl;
#endif

    // child[0, half - 1] < child[ half ] < child[ half + 1, degree - 1]
    // 
    // n_child [ 0, half - 2 ] = child [ half + 1, defree - 1]
    // n_child size = half - 1
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : ready to build n_child" << std::endl;
#endif
    n_dobj_child->is_leaf = dobj_child->is_leaf;
    n_dobj_child->fcount = Dir_Leaf_Node_Half_Degree - 1;
    for(int i = 0; i < Dir_Leaf_Node_Half_Degree - 1; ++i){
        n_dobj_child->fdes[ i ] = dobj_child->fdes[ i + Dir_Leaf_Node_Half_Degree + 1 ];
        n_dobj_child->mobj_id[ i ] = dobj_child->mobj_id[ i + Dir_Leaf_Node_Half_Degree + 1 ];
    }
    if( !dobj_child->is_leaf ){
        for(int i = 0; i < Dir_Leaf_Node_Half_Degree; ++i){
            n_dobj_child->cobj_id[ i ] = dobj_child->cobj_id[ i + Dir_Leaf_Node_Half_Degree + 1 ];
        }
    }
    // child[0, half - 1]
    dobj_child->fcount = Dir_Leaf_Node_Half_Degree;
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : finish to build n_child" << std::endl;
#endif

#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : ready to move parent" << std::endl;
#endif
    // move parent->fdes [ index , fcount - 1 ] one step back
    for(int i = dobj_parent->fcount - 1 - 1; i >= index ; i--){
        dobj_parent->fdes[ i + 1 ] = dobj_parent->fdes[ i ];
        dobj_parent->mobj_id[ i + 1 ] = dobj_parent->mobj_id[ i ];
    }
    dobj_parent->fdes[index] = dobj_child->fdes[ Dir_Leaf_Node_Half_Degree ];
    dobj_parent->mobj_id[index] = dobj_child->mobj_id[ Dir_Leaf_Node_Half_Degree ];
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : finish to move parent's fdes & mobj_id" << std::endl;
#endif

    // move parent->cobj_id [ index + 1, fcount ] one step back
    for(int i = dobj_parent->fcount; i > index; i--){
        dobj_parent->cobj_id[ i + 1 ] = dobj_parent->cobj_id[ i ];
    }
    dobj_parent->cobj_id[ index + 1 ] = n_dobj_child_id;
#ifdef DEBUG
    std::cout << "[DEBUG] in split_child : finish to move parent's cobj_id" << std::endl;
#endif
    dobj_parent->fcount++;
}

File_Nat_Entry_ID_Type DirBTree::get_file_meta_obj_id(
    struct file_descriptor fdes)
{
    struct dir_meta_obj* dobj = dir_meta_read_by_obj_id( root_dir_node_id );
    if( dobj == nullptr ){
        //throw
    }

DirBTree_get_restart:
    for( int i = 0; i < dobj->fcount; ++i){
        if( dobj->fdes[i].fhash == fdes.fhash ){
            return dobj->mobj_id[i];
        }
        if( dobj->fdes[i].fhash > fdes.fhash ){
            if( dobj->is_leaf ){
                // not found
            }
            else{
                Dir_Nat_Entry_ID_Type dobj_id = dobj->cobj_id[i];
                dobj = dir_meta_read_by_obj_id( dobj_id );
                goto DirBTree_get_restart;
            }
        }
    }
    Dir_Nat_Entry_ID_Type dobj_id = dobj->cobj_id[ dobj->fcount + 1 ];
    dobj = dir_meta_read_by_obj_id( dobj_id );
    goto DirBTree_get_restart;

    //not found
}

void print_dir_node( struct dir_meta_obj* dobj, Dir_Nat_Entry_ID_Type dobj_id)
{
  std::cout << "dobj{ " << dobj_id << " } = ";

  if( dobj->is_leaf ){
    std::cout << "leaf_node [ " << dobj->fcount << " ] : ";
  }
  else{
    std::cout << "iner_node [ " << dobj->fcount << " ] : "; 
  }
  for(int i = 0; i < dobj->fcount; ++i){
    std::cout << " ( " << dobj->fdes[i].fhash << " , " << dobj->mobj_id[i] << " )";
  }
  if( !dobj->is_leaf ){
    std::cout << std::endl << "       child: ";
    for(int i = 0; i <= dobj->fcount; ++i ){
      std::cout << "[" << dobj->cobj_id[i] << "]";
    }
  }
  std::cout << std::endl;
  if( !dobj->is_leaf ){
    for(int i = 0; i <= dobj->fcount; ++i){
      Dir_Nat_Entry_ID_Type id = dobj->cobj_id[i];
      struct dir_meta_obj* obj = dir_meta_read_by_obj_id( id );
      print_dir_node( obj, id );
    }
  }
}

void DirBTree::display()
{
  struct dir_meta_obj* dobj = dir_meta_read_by_obj_id( root_dir_node_id );
  if( dobj == nullptr ){
    //thorw
  }
  
  Dir_Nat_Entry_ID_Type dobj_id = root_dir_node_id;
  print_dir_node( dobj, dobj_id );
}



#define uint16_size 65536

dir_meta_number_t dir_obj_id_table[ uint16_size ];
char dir_obj_table_use[ uint16_size ];

struct dir_meta_obj* dir_obj_table[ uint16_size ];

void dir_meta_init()
{
  for(dir_meta_number_t i = 0; i < uint16_size; ++i){
    dir_obj_id_table[i] = i;
    dir_obj_table_use[i] = '0';
    dir_obj_table[i] = nullptr;
  }
}

dir_meta_number_t dir_meta_alloc_obj_id()
{
  for(dir_meta_number_t i = 0; i < uint16_size; ++i){
    if( dir_obj_table_use[i] == '0' ){
      dir_obj_table_use[i] = '1';
      dir_obj_table[i] = new struct dir_meta_obj;
      return dir_obj_id_table[i];
    }
  }
  return 0;
}

void dir_meta_write_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id, struct dir_meta_obj* obj)
{
  dir_obj_table[ dobj_id ] = obj;
}

struct dir_meta_obj* dir_meta_read_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id)
{
  return dir_obj_table[ dobj_id ];
}
