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


// insert pair : fdes -> mobj_id
// add_new_file()
// insert_not_full()
// split_child()

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





// search by fdes
// return mobj_id

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





// delete pair : fdes -> mobj_id
// according to fdes
// delete_not_half()
// union_child()

bool DirBTree::del_file(struct file_descriptor fdes)
{
    struct dir_meta_obj* dobj = dir_meta_read_by_obj_id( root_dir_node_id );
    if( dobj == nullptr ){
        //return;
    }
    Dir_Nat_Entry_ID_Type dobj_id = root_dir_node_id;

    delete_not_half( dobj, dobj_id, fdes);
}

void delete_not_half(strucy dir_meta_obj* dobj,
        Dir_Nat_Entry_ID_Type dobj_id,
        struct file_descriptor fdes)
{
    int index = 0;
    while(index < dobj->fcount && fdes.fhash > dobj->fdes[index].fhash){
        index++;
    }//locate index : fdes.fhash <= dobj->fdes[index].fhash

    // leaf node
    // delete it directly
    if( dobj->is_leaf ){
        if( index < dobj->fcount && fdes.fhash == dobj->fdes[index].fhash ){
            for(int i = index; i < dobj->fcount - 1; ++i){
                dobj->fdes[i] = dobj->fdes[i+1];
                dobj->mobj_id[i] = dobj->mobj_id[i+1];
            }
            dobj->fcount--;
            dir_meta_write_by_obj_id( dobj_id, dobj );
            return;//directly delete
        }
        else{
            // not found this fdes
        }
    }
    // dobj has child
    // fdes.fhash == dobj->fdes[index].fhash
    // maybe : union_child
    else if( index < dobj->fcount && fdes.fhash == dobj->fdes[index].fhash ){
        // move left_child[max] to parent[index]
        // delete left_child[max]
        Dir_Nat_Entry_ID_Type left_child_obj_id = dobj->cobj_id[ index ];
        struct dir_meta_obj* left_child_obj = dir_meta_read_by_obj_id( left_child_obj_id );
        if( left_child_obj->fcount > Dir_Leaf_Node_Half_Degree ){
            dobj->fdes[index] = left_child_obj->fdes[ left_child_obj->fcount - 1 ];
            dobj->mobj_id[index] = left_child_obj->mobj_id[ left_child_obj->fcount - 1];
            dir_meta_write_by_obj_id( dobj_id, dobj );
            delete_not_half( left_child_obj, left_child_obj_id, 
                left_child_obj->fdes[ left_child_obj->fcount - 1 ] );
            return;
        }
        // move right_child[min] to parent[index]
        // delete right_child[min]
        Dir_Nat_Entry_ID_Type right_child_obj_id = dobj->cobj_id[ index + 1 ];
        struct dir_meta_obj* right_child_obj = dir_meta_read_by_obj_id( right_child_obj_id );
        if( right_child_obj->fcount > Dir_Leaf_Node_Half_Degree ){
            dobj->fdes[index] = right_child_obj->fdes[ 0 ];
            dobj->mobj_id[index] = right_child_obj->mobj_id[ 0 ];
            dir_meta_write_by_obj_id( dobj_id, dobj );
            delete_not_half( right_child_obj, right_child_obj_id, 
                right_child_obj->fdes[ 0 ] );
            return;
        }
        //
        // both child->fcount == Dir_Leaf_Node_Half_Degree
        // need to union child & delete parent[index]
        //                                ┌---------------------------┐
        // { dobj }                       ↑                           ↓
        //   fdes =  index-1   index   index+1          index-1    'index'
        //                \    /   \   /    \      ==>        \   /       \
        //   cobj =       index  index+1  index+2            index        'index+1'
        //              { left } { right }   ↓          { left + right }      ↑
        //                                   |        { left } += { right }   |
        //                                   |                                |
        //                                   └--------------------------------┘
        union_child(dobj, dobj_id,
            left_child_obj, left_child_obj_id,   // left id  == dobj->cobj_id[ index ]
            right_child_obj, right_child_obj_id, // right id == dobj->cobj_id[ index + 1 ]
            index);
        // after union_child()
        // the dobj->fdes[index] will be added to dobj->cobj_id[ index ]
        // so we need to delete from cobj[ index ]
        // dir_meta_write_by_obj_id( dobj_id, dobj );      // write parent
        // dir_meta_dealloc_obj_id( right_child_obj_id );  // dealloc right child
        delete_not_half( left_child_obj, left_child_obj_id, fdes );
    }
    // situation 1. index == dobj->fcount
    //              which means fdes > all fdes in dobj
    // situation 2. dobj->fdes[ index - 1 ] < fdes < dobj->fdes[ index ]
    // in both situation, we need to delete it from dobj->cobj_id[ index ]
    else{
        Dir_Nat_Entry_ID_Type cobj_id = dobj->cobj_id[ index ];
        struct dir_meta_obj * cobj = dir_meta_read_by_obj_id( cobj_id );

        if( cobj->fcount > Dir_Leaf_Node_Half_Degree ){// delete from cobj 
            delete_not_half( cobj, cobj_id, fdes );
        }
        // cobj->fcount == Dir_Leaf_Node_Half_Degree
        // index ∈ [ 0, dobj->fcount ]
        else{
            Dir_Nat_Entry_ID_Type left_brother_cobj_id = 0;
            struct dir_meta_obj* left_brother_cobj = nullptr;
            Dir_Nat_Entry_ID_Type right_brother_cobj_id = 0;
            struct dir_meta_obj* right_brother_cobj = nullptr;
            // borrow one from its left brother
            if( index > 0 ){
                left_brother_cobj_id = dobj->cobj_id[ index - 1 ];
                left_brother_cobj = dir_meta_read_by_obj_id( left_brother_cobj_id );
                if( left_brother_cobj->fcount > Dir_Leaf_Node_Half_Degree ){// OK to brorrow
                    // brorrow from left brother and delete
                    return;
                }
            }
            // borrow one from its right brother
            if( index < dobj->fcount ){
                right_brother_cobj_id = dobj->cobj_id[ index + 1 ];
                right_brother_cobj = dir_meta_read_by_obj_id( right_brother_cobj_id );
                if( right_brother_cobj->fcount > Dir_Leaf_Node_Half_Degree ){// OK to brorrow
                    // brorrow from left brother and delete
                    return;
                }
            }
            // both left and right brother cannot brorrow
            // then union with one brother and delete
            // limited by union_child()
            //     1. if index ∈ [ 0 , dobj->fcount - 1 )
            //        then 1) union cobj with right
            //             2) write parent
            //             3) dealloc right 
            //             4) delete fdes from cobj
            //     2. if index == dobj->fcount
            //        then 1) union left with cobj 
            //             2) write parent
            //             3) dealloc cobj
            //             4) delete from left
            if( index == dobj->fcount ){
                // assert left_brother_cobj != nullptr
                union_child(dobj, dobj_id,
                    left_brother_cobj, left_brother_cobj_id,
                    cobj, cobj_id,
                    index - 1);
                //dir_meta_write_by_obj_id( dobj_id, dobj );  // write parent
                //dir_meta_dealloc_obj_id( cobj_id );
                delete_not_half( left_brother_cobj, left_brother_cobj_id, fdes );
            }
            else{ // index ∈ [ 0 , dobj->fcount - 1 )
                // assert right_brother_cobj != nullptr
                union_child(dobj, dobj_id,
                    cobj, cobj_id,
                    right_brother_cobj, right_brother_cobj_id,
                    index);
                //dir_meta_write_by_obj_id( dobj_id, dobj );  // write parent
                //dir_meta_dealloc_obj_id( right_brother_cobj_id );
                delete_not_half( cobj, cobj_id, fdes );
            }
        }
    }
}

// only used when deleting by fdes
// only used when left & right 's fcount == Dir_Leaf_Node_Half_Degree
//
//                                  ┌---------------------------┐
// { parent }                       ↑                           ↓
//   fdes =  index-1   index   index+1            index-1    'index'
//                \    /   \   /    \      ==>          \   /       \
//   cobj =       index  index+1  index+2              index        'index+1'
//              { left } { right }   ↓            { left + right }      ↑
//                                   |  1.{ left } += { parent }[index] |
//                                   |  2.{ left } += { right }         |
//                                   └----------------------------------┘
//
// step 1. move parent->fdes[ index ] into left_child & consist parent
// step 2. move right->fdes[ all ]    into left_child
// after 1 & 2 : assert left_child->fcount == Dir_Leaf_Node_Degree + 1
// step 3. write parent
// step 4. dealloc right
// step 5. if parent->fcount == 0 : change root
// step 6. return left_child ===> for delete by fdes
// NO need setp 6 : useless without its id
void union_child(
    struct dir_meta_obj* dobj_parent, Dir_Nat_Entry_ID_Type dobj_parent_id,
    struct dir_meta_obj* dobj_left,  Dir_Nat_Entry_ID_Type dobj_left_id,    // left_id  == parent->cobj_id[ index ]
    struct dir_meta_obj* dobj_right, Dir_Nat_Entry_ID_Type dobj_right_id,   // right_id == parent-<cobj_id[ index+1 ]
    int index)
{
    // assert dobj_left_id  == dobj_parent->cobj_id[ index ]
    // assert dobj_right_id == dobj_parent->cobj_id[ index + 1 ]
    // assert dobj_left->fcount  == Dir_Leaf_Node_Half_Degree
    // assert dobj_right->fcount == Dir_Leaf_Node_Half_Degree

    // step 1. move parent->fdes[ index ] into left_child & consist parent
    // { parent }
    //   fdes =    index-1 index index+1
    //                      ↓
    //                      └------------------------------┐
    // { left }                                            ↓
    //   fdes =     fcount-1                 fcount-1   'fcount'
    //              /      \        ==>      /      \    /    \
    //   cobj = fcount-1  fcount         fcount-1  fcount    nullptr 
    //
    dobj_left->fdes[ obj_left->fcount ]    = dobj_parent->fdes[ index ];
    dobj_left->mobj_id[ obj_left->fcount ] = dobj_parent->mobj_id[ index ];
    for(int i = index; i < dobj_parent->fcount - 1; ++i){
        dobj_parent->fdes[ i ]    = dobj_parent->fdes[ i + 1];
        dobj_parent->mobj_id[ i ] = dobj_parent->mobj_id[ i + 1];
    }
    for(int i = index + 1; i < dobj_parent->fcount; ++i){
        dobj_parent->cobj_id[ i ] = dobj_parent->cobj_id[ i + 1];
    }
    dobj_parent->fcount -= 1;

    // step 2. move right->fdes[ all ]    into left_child
    //                                    ┌---------------------------┐
    //  { left }                          ↓             { right }     ↑
    //   fdes =   fcount-1   fcount   'fcount+1'          fdes =     [0]  ... fcount-1
    //            /      \   /    \    /     \                      /   \         \
    //   cobj = fcount-1 fcount 'fcount+1'  'fcount+2'    cobj =  [0]    [1] ...  fcount
    //                              ↑           ↑                  ↓      ↓
    //                              └-----------┼------------------┘      |
    //                                          └-------------------------┘
    for(int i = 0; i < dobj_right->fcount; ++i){
        dobj_left->fdes[ dobj_left->fcount + 1 + i ]    = dobj_right->fdes[ i ];
        dobj_left->mobj_id[ dobj_left->fcount + 1 + i ] = dobj_right->mobj_id[ i ];
    }
    for(int i = 0; i <= dobj_right->fcount; ++i){
        dobj_left->cobj_id[ dobj_left->fcount + 1 + i ] = dobj_right->cobj_id[ i ]; 
    }

    // after 1 & 2 : assert left_child->fcount == Dir_Leaf_Node_Degree + 1
    dobj_left->fcount += dobj_right->fcount + 1;

    // step 3. write parent
    dir_meta_write_by_obj_id( dobj_parent_id, dobj_parent_id );

    // step 4. dealloc right
    dir_meta_dealloc_obj_id( dobj_right_id );

    // step 5. if parent->fcount == 0 : change root
    // could conbine with step 3. to avoid write if parent is empty
    if( dobj_parent->fcount == 0){
        dir_meta_dealloc_obj_id( dobj_parent_id );
        root_dir_node_id = dobj_left_id;
    }
}





// display the whole dir_meta_tree
// display()
// print_dir_node()

void DirBTree::print_dir_node( struct dir_meta_obj* dobj, Dir_Nat_Entry_ID_Type dobj_id, int print_child)
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
  if( print_child ){    // print all child node
    if( !dobj->is_leaf ){
        for(int i = 0; i <= dobj->fcount; ++i){
        Dir_Nat_Entry_ID_Type id = dobj->cobj_id[i];
        struct dir_meta_obj* obj = dir_meta_read_by_obj_id( id );
        print_dir_node( obj, id );
        }
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
  print_dir_node( dobj, dobj_id, 1 );
}





// stub for test

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

void dir_meta_dealloc_obj_id(dir_meta_number_t dobj_id)
{
    dir_obj_table_use[ dobj_id ] = '0';
}

void dir_meta_write_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id, struct dir_meta_obj* obj)
{
    // assert dir_obj_table_use[ dobj_id ] = '1'
    dir_obj_table[ dobj_id ] = obj;
}

struct dir_meta_obj* dir_meta_read_by_obj_id(Dir_Nat_Entry_ID_Type dobj_id)
{
    // assert dir_obj_table_use[ dobj_id ] = '1'
    return dir_obj_table[ dobj_id ];
}
