#include <types.h>
#include <vfs.h>
#include <copyinout.h>
#include <limits.h>
#include <syscall.h>


int sys___chdir(const_userptr_t pathname){
    int result;
    char path[__PATH_MAX];
    size_t size = 0;
    
    result = copyinstr(pathname,path,__PATH_MAX,&size);
    if(result){
        //kfree(kbuf);
        return result;
    }
    
    result = vfs_chdir(path);
    if(result){
        return result;
    }
    return 0;
}