#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include <vfs/vfs.h>
#include <vfs/ece391_vfs.h>
#include <boot/x86_desc.h>
#include <errno.h>  

#define COMMAND_LEN             128
#define TASK_COUNT              2               /* for cp3 we only need to support two tasks */

/* kernel, physical addr */
#define KERNEL_STAK_BEGIN       0x800000        /* the pyphisical addr begin at the end of 8MB; it should go up wards*/
#define KERNEL_STAK_SZ          0x400000        /* Per kernel stack size is 4MB */

/* user-level, physical addr */
#define  FIRST_USR_BEGIN        0x800000        /* the first user addr begin at 8MB (to 12 MB) */
#define  SECOND_USR_BEGIN       0xC00000        /* the first user addr begin at 12MB (to 16 MB) */

/* user-level, virtual addr */
#define PROGRAM_IMG_BEGIN       0x08000000      /*  */


typedef uint32_t gid_t;

typedef struct process {
    uint32_t        pid;                    /* process id number */
    uint8_t         *arg;                   /* store the arguments */
    struct process* parent_addr;                /* parent process addr */

    uint32_t        kernel_ESP;
    uint16_t        kernel_SS;

    uint32_t*        PCB_Pyh_addr;              /* pyhisical address of pcb */

    vfs_t           pro_vfs;
    files           pro_files;                  

} process_t;


extern process_t     task_map[TASK_COUNT];
extern process_t*    curr_process;

extern int32_t parse_arg_to_process(uint8_t* command, uint8_t* stored_pro, uint8_t* stored_file);


extern void process_context_switch(process_t* curr_pro);

#endif /* _PROCESS_H */

