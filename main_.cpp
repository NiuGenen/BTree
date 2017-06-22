#include "BTree_.h"

int main()
{
  dir_meta_init();
  DirBTree b;
  b.init();
  
  char asd;

  std::cout << "=========== insert test begin ===============" << std::endl;
  struct file_descriptor fdes[32];
  File_Nat_Entry_ID_Type mobj_id[32];
  for(int i=0;i<32;++i){
    std::cout << "---------" << i << "---------" << std::endl;

    fdes[i].fhash = i;
    mobj_id[i] = i+1000;
    b.add_new_file(fdes[i], mobj_id[i]);
    b.display();

    std::cin >> asd;
    if( asd == 'q' ){
      std::cout << "=========== insert test finish ==============" << std::endl;
      break;
    }
  }

  

}
