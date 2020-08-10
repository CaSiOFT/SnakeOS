/*************************************************************************//**
 *****************************************************************************
 * @file   keyboard.c
 * @brief  
 * @author OS开发小组
 * @date   2020.7~2020.8
 * 
 * 
 * 
 *****************************************************************************
 *****************************************************************************/
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

struct File {
		char name[128];  //文件名
		struct File* next;  //下一文件
};

struct Directory {
		char name[128];  //目录名
		struct Directory* next;  //下一目录
		struct Directory* child;  //子目录
		struct Directory* parent;  //父目录
		struct File* link;  //该目录下首文件
};

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");
	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

        

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}

/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

/*****************************************************************************
*                                clear
*****************************************************************************/
void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}

void colorful()
{
	clear();
	int j;
	int delay_time = 2000;
	int start_loc = 1520;
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
                            
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("         \n",GREEN);                                                                
disp_color_str(" ",GREEN);disp_color_str("\n",RED);                                                                
disp_color_str("",GREEN);disp_color_str("\n",RED);                                                                
disp_color_str("                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("         \n",GREEN);                                                                
disp_color_str(" ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("",GREEN);disp_color_str("sh\n",RED);                                                                
disp_color_str("                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("         \n",GREEN);                                                                
disp_color_str("j  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("         \n",GREEN);                                                                
disp_color_str("MMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("Rj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("Vx         \n",GREEN);                                                                
disp_color_str("kNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("NNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("MRVx         \n",GREEN);                                                                
disp_color_str("MDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("wQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("j                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("                                 ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("DMMRVx         \n",GREEN);                                                                
disp_color_str("sMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("NSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("ccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                   ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                   ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                   ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("N                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("hM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("NM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("a                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("NNDMMRVx         \n",GREEN);                                                                
disp_color_str("LksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("QQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                     ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                     ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("j                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("MMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("LlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("lONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("NRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("QQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("SZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("RNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("i jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("j                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("DQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("ClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("fLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("MMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("COQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("NRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("hfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("JRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("OQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("kQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("VlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("QZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("OOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("MMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str(" jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("MMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("OAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("JCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("kPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("KhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str(" xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("  jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("hDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("NMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("   jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str(" NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("HwJCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("MLkPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("aMKhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("  NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("   xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("    jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("NQhDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("NNNMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("     jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("   NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("  MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str(" QHwJCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str(" jMLkPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("  aMKhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("    NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("j    xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("NH    jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("BNNQhDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("NNNNNMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("       jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("     NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("    MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("   QHwJCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("   jMLkPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("    aMKhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("j     NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("MNj    xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("wSNH    jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("MNBNNQhDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("rhNNNNNMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("         jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("       NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("      MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("     QHwJCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("     jMLkPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("      aMKhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("MQj     NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("hQMNj    xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("hlwSNH    jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("RMMNBNNQhDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("  rhNNNNNMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("           jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("         NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("        MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("       QHwJCJRi jiccj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("       jMLkPOQ                                       ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("Dr      aMKhkQj                                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("hNMQj     NQVlNRj                                    ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("hhhQMNj    xQZDQMMN                                  ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("MNhlwSNH    jhClLlhM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str(" cRMMNBNNQhDOOfLlONM                                 ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("    rhNNNNNMMMMMNRa                                  ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("             jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("           NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("          MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("         QHwJCJRi jiccj                              ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("         jMLkPOQ                                     ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NNDr      aMKhkQj                                    ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NkhNMQj     NQVlNRj                                  ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("jMhhhQMNj    xQZDQMMN                                ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("  MNhlwSNH    jhClLlhM                               ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("M  cRMMNBNNQhDOOfLlONM                               ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("Mi    rhNNNNNMMMMMNRa                                ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("               jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("             NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("            MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("           QHwJCJRi jiccj                            ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("           jMLkPOQ                                   ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NNNNDr      aMKhkQj                                  ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("PQNkhNMQj     NQVlNRj                                ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("I jMhhhQMNj    xQZDQMMN                              ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("DD  MNhlwSNH    jhClLlhM                             ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("pDM  cRMMNBNNQhDOOfLlONM                             ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("MMMi    rhNNNNNMMMMMNRa                              ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("rj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                 jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("               NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("              MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("             QHwJCJRi jiccj                          ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("             jMLkPOQ                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("pBNNNNDr      aMKhkQj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("NQPQNkhNMQj     NQVlNRj                              ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("LDI jMhhhQMNj    xQZDQMMN                            ",GREEN);disp_color_str("                      \n",YELLOW);                          
disp_color_str("CwDD  MNhlwSNH    jhClLlhM                           ",GREEN);disp_color_str("                      \n",YELLOW);                             
disp_color_str("OLpDM  cRMMNBNNQhDOOfLlONM                           ",GREEN);disp_color_str("                      \n",YELLOW);                           
disp_color_str("NMMMMi    rhNNNNNMMMMMNRa                            ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("Oprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                   jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                 NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("               QHwJCJRi jiccj                          ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("               jMLkPOQ                                 ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str(" jpBNNNNDr      aMKhkQj                                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("MMNQPQNkhNMQj     NQVlNRj                              ",GREEN);disp_color_str("                      \n",YELLOW);                         
disp_color_str("NlLDI jMhhhQMNj    xQZDQMMN                            ",GREEN);disp_color_str("                     N\n",YELLOW);                          
disp_color_str("NOCwDD  MNhlwSNH    jhClLlhM                           ",GREEN);disp_color_str("                    NM\n",YELLOW);                             
disp_color_str(" DOLpDM  cRMMNBNNQhDOOfLlONM                           ",GREEN);disp_color_str("                     K\n",YELLOW);                           
disp_color_str("fBNMMMMi    rhNNNNNMMMMMNRa                            ",GREEN);disp_color_str("                      \n",YELLOW);                              
disp_color_str("MNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                     jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                   NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                  MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                 QHwJCJRi jiccj                        ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                 jMLkPOQ                               ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("   jpBNNNNDr      aMKhkQj                              ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str(" cMMNQPQNkhNMQj     NQVlNRj                            ",GREEN);disp_color_str("                     s\n",YELLOW);                         
disp_color_str(" MNlLDI jMhhhQMNj    xQZDQMMN                          ",GREEN);disp_color_str("                   NNB\n",YELLOW);                          
disp_color_str("  NOCwDD  MNhlwSNH    jhClLlhM                         ",GREEN);disp_color_str("                  NMCC\n",YELLOW);                             
disp_color_str("   DOLpDM  cRMMNBNNQhDOOfLlONM                         ",GREEN);disp_color_str("                   KMM\n",YELLOW);                           
disp_color_str("ACfBNMMMMi    rhNNNNNMMMMMNRa                          ",GREEN);disp_color_str("                     c\n",YELLOW);                              
disp_color_str("RMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                       jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                     NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                    MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                   QHwJCJRi jiccj                      ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                   jMLkPOQ                             ",GREEN);disp_color_str("                     j\n",GREEN);                         
disp_color_str("     jpBNNNNDr      aMKhkQj                            ",GREEN);disp_color_str("                    MM\n",GREEN);                         
disp_color_str("   cMMNQPQNkhNMQj     NQVlNRj                          ",GREEN);disp_color_str("                   sM \n",YELLOW);                         
disp_color_str("j  MNlLDI jMhhhQMNj    xQZDQMMN                        ",GREEN);disp_color_str("                 NNBip\n",YELLOW);                          
disp_color_str("    NOCwDD  MNhlwSNH    jhClLlhM                       ",GREEN);disp_color_str("                NMCCSV\n",YELLOW);                             
disp_color_str("R    DOLpDM  cRMMNBNNQhDOOfLlONM                       ",GREEN);disp_color_str("                 KMMNK\n",YELLOW);                           
disp_color_str("NMACfBNMMMMi    rhNNNNNMMMMMNRa                        ",GREEN);disp_color_str("                   cDN\n",YELLOW);                              
disp_color_str("LZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                         jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                       NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                      MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                     QHwJCJRi jiccj                    ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                     jMLkPOQ                           ",GREEN);disp_color_str("                   jMM\n",GREEN);                         
disp_color_str("R      jpBNNNNDr      aMKhkQj                          ",GREEN);disp_color_str("                  MMMM\n",GREEN);                         
disp_color_str("RM   cMMNQPQNkhNMQj     NQVlNRj                        ",GREEN);disp_color_str("                 sM j \n",YELLOW);                         
disp_color_str("pNj  MNlLDI jMhhhQMNj    xQZDQMMN                      ",GREEN);disp_color_str("               NNBipCl\n",YELLOW);                          
disp_color_str("pQ    NOCwDD  MNhlwSNH    jhClLlhM                     ",GREEN);disp_color_str("              NMCCSVSL\n",YELLOW);                             
disp_color_str("VhR    DOLpDM  cRMMNBNNQhDOOfLlONM                     ",GREEN);disp_color_str("               KMMNKhh\n",YELLOW);                           
disp_color_str("MNNMACfBNMMMMi    rhNNNNNMMMMMNRa                      ",GREEN);disp_color_str("                 cDNMM\n",YELLOW);                              
disp_color_str(" cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);			
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                           jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                         NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                        MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                       QHwJCJRi jiccj                  ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                       jMLkPOQ                         ",GREEN);disp_color_str("                 jMMMM\n",GREEN);                         
disp_color_str("NMR      jpBNNNNDr      aMKhkQj                        ",GREEN);disp_color_str("                MMMMMM\n",GREEN);                         
disp_color_str("DhRM   cMMNQPQNkhNMQj     NQVlNRj                      ",GREEN);disp_color_str("               sM j jj\n",YELLOW);                         
disp_color_str("AppNj  MNlLDI jMhhhQMNj    xQZDQMMN                    ",GREEN);disp_color_str("             NNBipClpS\n",YELLOW);                          
disp_color_str("OCpQ    NOCwDD  MNhlwSNH    jhClLlhM                   ",GREEN);disp_color_str("            NMCCSVSLSL\n",YELLOW);                             
disp_color_str("QfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM                   ",GREEN);disp_color_str("             KMMNKhhOO\n",YELLOW);                           
disp_color_str("NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa                    ",GREEN);disp_color_str("               cDNMMMM\n",YELLOW);                              
disp_color_str("   cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
 	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                             jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                           NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                          MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                         QHwJCJRi jiccj                ",GREEN);disp_color_str("                      \n",GREEN);                         
disp_color_str("                         jMLkPOQ                       ",GREEN);disp_color_str("               jMMMMMM\n",GREEN);                         
disp_color_str("MNNMR      jpBNNNNDr      aMKhkQj                      ",GREEN);disp_color_str("              MMMMMMr \n",GREEN);                         
disp_color_str("tQDhRM   cMMNQPQNkhNMQj     NQVlNRj                    ",GREEN);disp_color_str("             sM j jjcp\n",YELLOW);                         
disp_color_str(" kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN                  ",GREEN);disp_color_str("           NNBipClpSlS\n",YELLOW);                          
disp_color_str(" QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM                 ",GREEN);disp_color_str("          NMCCSVSLSLfL\n",YELLOW);                             
disp_color_str(" NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM                 ",GREEN);disp_color_str("           KMMNKhhOOOH\n",YELLOW);                           
disp_color_str("  NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa                  ",GREEN);disp_color_str("             cDNMMMMMN\n",YELLOW);                              
disp_color_str("     cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
 	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                               jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                             NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                            MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                           QHwJCJRi jiccj              ",GREEN);disp_color_str("                    jh\n",GREEN);                         
disp_color_str("                           jMLkPOQ                     ",GREEN);disp_color_str("             jMMMMMMMc\n",GREEN);                         
disp_color_str("LNMNNMR      jpBNNNNDr      aMKhkQj                    ",GREEN);disp_color_str("            MMMMMMr   \n",GREEN);                         
disp_color_str("MQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj                  ",GREEN);disp_color_str("           sM j jjcpKP\n",YELLOW);                         
disp_color_str("j  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN                ",GREEN);disp_color_str("         NNBipClpSlSkO\n",YELLOW);                          
disp_color_str("   QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM               ",GREEN);disp_color_str("        NMCCSVSLSLfLVp\n",YELLOW);                             
disp_color_str("   NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM               ",GREEN);disp_color_str("         KMMNKhhOOOHRN\n",YELLOW);                           
disp_color_str("    NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa                ",GREEN);disp_color_str("           cDNMMMMMNQC\n",YELLOW);                              
disp_color_str("       cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
 	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                                 jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                               NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                              MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                             QHwJCJRi jiccj            ",GREEN);disp_color_str("                  jh  \n",GREEN);                         
disp_color_str("                             jMLkPOQ                   ",GREEN);disp_color_str("           jMMMMMMMc  \n",GREEN);                         
disp_color_str("  LNMNNMR      jpBNNNNDr      aMKhkQj                  ",GREEN);disp_color_str("          MMMMMMr     \n",GREEN);                         
disp_color_str("rMMQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj                ",GREEN);disp_color_str("         sM j jjcpKPc \n",YELLOW);                         
disp_color_str("MNj  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN              ",GREEN);disp_color_str("       NNBipClpSlSkORM\n",YELLOW);                          
disp_color_str("     QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM             ",GREEN);disp_color_str("      NMCCSVSLSLfLVppD\n",YELLOW);                             
disp_color_str("     NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM             ",GREEN);disp_color_str("       KMMNKhhOOOHRNMM\n",YELLOW);                           
disp_color_str("      NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa              ",GREEN);disp_color_str("         cDNMMMMMNQC  \n",YELLOW);                              
disp_color_str("         cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
 	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                                   jCOQQNNDMMRVx         \n",GREEN);                                                                
disp_color_str("                                 NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                                MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                               QHwJCJRi jiccj          ",GREEN);disp_color_str("                jh    \n",GREEN);                         
disp_color_str("                               jMLkPOQ                 ",GREEN);disp_color_str("         jMMMMMMMc    \n",GREEN);                         
disp_color_str("    LNMNNMR      jpBNNNNDr      aMKhkQj                ",GREEN);disp_color_str("        MMMMMMr       \n",GREEN);                         
disp_color_str("J rMMQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj              ",GREEN);disp_color_str("       sM j jjcpKPc   \n",YELLOW);                         
disp_color_str("MMMNj  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN            ",GREEN);disp_color_str("     NNBipClpSlSkORMi \n",YELLOW);                          
disp_color_str("       QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM           ",GREEN);disp_color_str("    NMCCSVSLSLfLVppDM \n",YELLOW);                             
disp_color_str("       NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM           ",GREEN);disp_color_str("     KMMNKhhOOOHRNMM  \n",YELLOW);                           
disp_color_str("        NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa            ",GREEN);disp_color_str("       cDNMMMMMNQC    \n",YELLOW);                              
disp_color_str("           cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
 	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                                     jCOQQNNDMMRVx       \n",GREEN);                                                                
disp_color_str("                                   NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                                  MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                                 QHwJCJRi jiccj        ",GREEN);disp_color_str("              jh      \n",GREEN);                         
disp_color_str("                                 jMLkPOQ               ",GREEN);disp_color_str("       jMMMMMMMc      \n",GREEN);                         
disp_color_str("B     LNMNNMR      jpBNNNNDr      aMKhkQj              ",GREEN);disp_color_str("      MMMMMMr         \n",GREEN);                         
disp_color_str("MMJ rMMQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj            ",GREEN);disp_color_str("     sM j jjcpKPc     \n",YELLOW);                         
disp_color_str(" MMMMNj  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN          ",GREEN);disp_color_str("   NNBipClpSlSkORMi   \n",YELLOW);                          
disp_color_str("         QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM         ",GREEN);disp_color_str("  NMCCSVSLSLfLVppDM   \n",YELLOW);                             
disp_color_str("         NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM         ",GREEN);disp_color_str("   KMMNKhhOOOHRNMM    \n",YELLOW);                           
disp_color_str("          NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa          ",GREEN);disp_color_str("     cDNMMMMMNQC      \n",YELLOW);                              
disp_color_str("             cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);				
	disp_pos = 0;
	for (j = 0; j < start_loc; j++) { disp_str(" "); }
	milli_delay(delay_time);                                                                                                                                                                                                                                                                          
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                      
disp_color_str("                                       jCOQQNNDMMRVx      \n",GREEN);                                                                
disp_color_str("                                     NMMNRSZLksMMDkNMMj  ",GREEN);disp_color_str("j\n",RED);                                                                
disp_color_str("                                    MKOAhfRNQQNSwQNNRj",GREEN);disp_color_str("cish\n",RED);                                                                
disp_color_str("                                   QHwJCJRi jiccj       ",GREEN);disp_color_str("            jh         \n",GREEN);                         
disp_color_str("                                   jMLkPOQ              ",GREEN);disp_color_str("     jMMMMMMMc         \n",GREEN);                         
disp_color_str(" MB     LNMNNMR      jpBNNNNDr      aMKhkQj             ",GREEN);disp_color_str("    MMMMMMr            \n",GREEN);                         
disp_color_str(" jMMJ rMMQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj           ",GREEN);disp_color_str("   sM j jjcpKPc        \n",YELLOW);                         
disp_color_str("   MMMMNj  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN         ",GREEN);disp_color_str(" NNBipClpSlSkORMi     \n",YELLOW);                          
disp_color_str("           QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM        ",GREEN);disp_color_str("NMCCSVSLSLfLVppDM  \n",YELLOW);                             
disp_color_str("           NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM        ",GREEN);disp_color_str(" KMMNKhhOOOHRNMM     \n",YELLOW);                           
disp_color_str("            NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa         ",GREEN);disp_color_str("   cDNMMMMMNQC    \n",YELLOW);                              
disp_color_str("               cLZRMMNOprj\n",GREEN);                                                                                                 
disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n",YELLOW);                                                                                                                                                                              
	milli_delay(10000);
	disp_pos = 0;
	for (j = 0; j < start_loc + 1600; j++) { disp_str(" "); }
	disp_pos = 0;
	for (j = 0; j < start_loc + 80; j++) { disp_str(" "); }
disp_color_str("                                 9BBB                 \n",GREEN);                                         
disp_color_str("iBBBBBBBBBB                      hBBB                 ",GREEN); disp_color_str(",i:i 2222ss,  BBBBBBBBBB\n",YELLOW);                
disp_color_str(":BBB:    BB                      2BBB                 ",GREEN); disp_color_str("GBBBiBBBBBBr  BBBs    MB\n",YELLOW);                
disp_color_str(",BBB        sBBB, ,i: ,2ss5hXhh  2BBB  rBi  ,2ss2hhX2:",GREEN); disp_color_str("2BBB      Bi  BBB,      \n",YELLOW);                
disp_color_str(":BBB2:,     sBBBBBBBB 2BBBBBBBB  2BBB  BB   sBBBBBBBBS",GREEN); disp_color_str("sBBB      Bi  BBB9::,   \n",YELLOW);                
disp_color_str(":BBBBBBBBBB iBBB,  BB rBBB   9B  2BBB BB    :BBB    Bs",GREEN); disp_color_str("sBBB      Bi  BBBBBBBBBB\n",YELLOW);                
disp_color_str("       BBBB iBBB   BB rBBB   hB  2BBB BBS   ,BBB MBBB2",GREEN); disp_color_str("sBBB      Bi        9BBB\n",YELLOW);                
disp_color_str("       GBBM iBBB   BB rBBB   GB  2BBB  sBB2  BBB  irs ",GREEN); disp_color_str("sBBB      Bi        sBBB\n",YELLOW);                
disp_color_str("BBirri:BBBB rBBB   BB sBBBBBBBB: hBBB    9B: BBBi     ",GREEN); disp_color_str("2BBB,    ,B: BBrrrr:BBBB\n",YELLOW);                
disp_color_str("GBBBBBBBBBB iBBB   BB rBBBBGrMBB sBBB        BBBBBBBBh",GREEN); disp_color_str("GBBBBBBBBBBr sBBBBBBBBBB\n",YELLOW);                
	milli_delay(30000);
	disp_pos = 0;   
	clear();
}

void clearview()
{
	int i = 0;
	for (i = 0; i < 25; i++)
		printf("\n");
}

void welcome()
{
	clear();
	printf("                        ==================================\n");
	printf("                                   	  SnakeOS            \n");
	printf("                                 Based on Orange's \n\n");
	printf("                                      1851008\n");
	printf("                                      1852657\n");
	printf("                                      1853144\n");
	printf("                                      1854204\n");
	printf("                                      1854304\n");
	printf("                                Enjoy using our OS:)\n");
	printf("                        ==================================\n");
	printf("\n\n\n\n\n\n\n\n\n\n");
}

/*======================================================================*
                               TestA
							   主界面
 *======================================================================*/
void TestA()
{
	int fd;
	int i, n;

	char tty_name[] = "/dev_tty0";

	char rdbuf[128];


	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);


	const char bufw[80] = {0};

	colorful();
	clearview();
	welcome();

	while (1) {
		printl("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (!strcmp(rdbuf, "proc"))
        {
			ProcessManager(fd_stdin, fd_stdout);
			continue;
        }

		else if (!strcmp(rdbuf, "flm"))
		{
			printf("File Manager is already running on TTY1 ! \n");
			TestB();
			continue;

		}
		else if (!strcmp(rdbuf, "help"))
		{
			help();
		}
		else if (!strcmp(rdbuf, "game1"))
		{

			Game1(fd_stdin, fd_stdout);
		}
		else if (!strcmp(rdbuf, "game2"))
		{

			Game2(fd_stdin, fd_stdout);
		}
               else if (!strcmp(rdbuf, "game3"))
		{

			//Game3(fd_stdin, fd_stdout);
		}
               else if (!strcmp(rdbuf, "game4"))
		{

			//Game4(fd_stdin, fd_stdout);
		}
		else if (!strcmp(rdbuf, "calcu")) {
			//calculator(fd_stdin, fd_stdout);
		}
                else if (!strcmp(rdbuf, "timer") )
		{
			printf("haha!");
                        //timer(fd_stdin, fd_stdout);

		}
		else if (strcmp(rdbuf, "cl") == 0)
		{
			clearview();
			welcome();
		}
		

		else
			printf("Command not found, please input help to get help!\n");
	}

}


void help()
{
	printf("=============================================================================\n");
	printf("Command List     :\n");
	printf("1. proc       : A process manage,show you all process-info here\n");
	printf("2. cl         : Clear the screen\n");
	printf("3. help       : Show operation guide\n");
	printf("5. cal        : Show a calendar\n");
	printf("6. calcu      : open a calculator\n");
	printf("7. game1      : Run a small game1(guess number) on this OS\n");
	printf("8. game2      : Run a small game2(greedy snake) on this OS\n");
	printf("9. timer      : Set Your clock\n");
	printf("10. game3      :Run a small game1(who is bigger) on this OS\n");
    printf("11. game4      :Run a small game1(2048) on this OS\n");
	printf("\nUse alt+F2 to Run the file manager.\n");
	printf("==============================================================================\n");
}

/*======================================================================*
小游戏1 文字冒险
*======================================================================*/
int GameProgress=0;
void Game1(int fd_stdin, int fd_stdout);
void Scene2(int fd_stdin, int fd_stdout);
void Scene3(int fd_stdin, int fd_stdout);
void Scene4(int fd_stdin, int fd_stdout);
void Scene5(int fd_stdin, int fd_stdout);
void Scene6(int fd_stdin, int fd_stdout);
void Scene7(int fd_stdin, int fd_stdout);
void Scene8(int fd_stdin, int fd_stdout);
void Scene9(int fd_stdin, int fd_stdout);

void Scene1(int fd_stdin, int fd_stdout){//场景1
	char choice[2];//输入的选项
	GameProgress=1;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("When you wake up, you find that you yourself in a broken wooden bed.\n");
	printf("With brids singing, The sunlight came in through the window.\n");
	milli_delay(10000);
	printf("Yes, you are in a small broken wooden house and it's morning right now. \n");
	printf("But you remember nothing about this place. You decide to ___\n");
	milli_delay(10000);
	printf("A.get up and check outside.\n");
	printf("B.continue sleeping.\n");
	printf("C.check around inside this house .\n");
	
	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "a")){
		Scene2(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "b")){
		Scene3(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "c")){
		Scene4(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Game1(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene1(fd_stdin, fd_stdout);
	}
}

void Scene2(int fd_stdin, int fd_stdout){//场景2
	char choice[2];
	GameProgress=2;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(1000);	
	printf("When you go out, you find yourself in the forest. When you walk around, \n");
	printf("you see a tiger. At this time, the tiger also finds you. He pours on you.\n");
	printf("You can't escape and are eaten directly.\n");

	read(fd_stdin, choice, 1);		
	if(!strcmp(choice, "e")){
		Scene1(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene2(fd_stdin, fd_stdout);
	}
}

void Scene3(int fd_stdin, int fd_stdout){//场景3
	char choice[2];//输入的选项
	GameProgress=3;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("Since then, you never wake up.\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "e")){
		Scene1(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene3(fd_stdin, fd_stdout);
	}
}

void Scene4(int fd_stdin, int fd_stdout){//场景4
	char choice[2];//输入的选项
	GameProgress=4;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You look around and find that there is only one shotgun in the room.\n"); 
	printf("You pick up the shotgun, and you decide——\n");
	milli_delay(10000);
	printf("A.check outside.\n");
	printf("B.continue sleeping.\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "a")){
		Scene5(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "b")){
		Scene3(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene1(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene4(fd_stdin, fd_stdout);
	}
}

void Scene5(int fd_stdin, int fd_stdout){//场景5
	char choice[2];//输入的选项
	GameProgress=5;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("When you go out, you find yourself in the forest. When you walk around, \n");
	printf("you see a tiger. At this time, the tiger also finds you. He pours on you.\n");
	printf("Fortunately, you took out your gun in time and shot and killed the tiger.\n");
	milli_delay(10000);
	printf("But the gunfire is so loud that the creatures around you seem to be agitated\n");
	printf(". You decide__\n");
	milli_delay(10000);
	printf("A.ingnore it and continue going.\n");
	printf("B.Cover yourself with mud and grass and hide in the bushes.\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "a")){
		Scene6(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "b")){
		Scene7(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene4(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene5(fd_stdin, fd_stdout);
	}
}

void Scene6(int fd_stdin, int fd_stdout){//场景6
	char choice[2];//输入的选项
	GameProgress=6;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You feel the earth shaking regularly, and the creatures around you are \n");
	printf("running wildly. Then a huge monster appears in your sight. It walks to the \n");
	printf("dead tiger and roars up to the sky. \n");
	printf("The whole forest trembles with it!\n");
	milli_delay(10000);
	printf("It sees you, and then it raises its huge claws and slaps you.\n");
	printf("You react quickly and want to shoot back,\n");
	printf("but the bullet doesn't seem to do anything to it, \n");
	printf("and then you're torn by the sharp claws.\n");

	read(fd_stdin, choice, 1);
	
	if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene5(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene6(fd_stdin, fd_stdout);
	}
}

void Scene7(int fd_stdin, int fd_stdout){//场景7
	char choice[2];//输入的选项
	GameProgress=7;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You feel the earth shaking regularly, and the creatures around you are \n");
	printf("running wildly. Then a huge monster appears in your sight. It walks to the \n");
	printf("dead tiger and roars up to the sky. \n");
	printf("The whole forest trembles with it!\n");
	milli_delay(10000);
	printf("The monster looked around and didn't seem to find the murderer who was \n");
	printf("spreading wild on its territory.\n");
	printf("So he swallowed up the tiger's body and was ready to leave.\n");
	printf("At this time, you find that there seems to be an obvious weakness behind it.\n");
	printf("You decide to__\n");
	milli_delay(10000);
	printf("A.Shoot that weakness with a shotgun.\n");
	printf("B.escape from it and go back to that wooden house.\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "a")){
		Scene8(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "b")){
		Scene9(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene6(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene7(fd_stdin, fd_stdout);
	}
}

void Scene8(int fd_stdin, int fd_stdout){//场景8
	char choice[2];//输入的选项
	GameProgress=8;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You are like a well-trained sniper. You hit the monster's heart with one shot,\n");
	printf("and the Forest Lord fell down before he knew what was going on.\n");
	printf("This makes you very excited, you feel the joy of victory, \n");
	printf("seems to untie the knot for a long time. \n");
	milli_delay(10000);
	printf("And then fainted.\n");
	milli_delay(10000);
	printf("In the dark, you seem to see a glimmer of light far away, you decide to__\n");
	milli_delay(10000);
	printf("A.Towards the light.\n");
	printf("B.do nothing.\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "a")){
		Scene10(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "b")){
		Scene11(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene7(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene8(fd_stdin, fd_stdout);
	}
}

void Scene9(int fd_stdin, int fd_stdout){//场景9
	char choice[2];//输入的选项
	GameProgress=9;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You go back to the cabin and think about what happened just now.\n");
	milli_delay(10000);
	printf("You fall asleep and never wake up again\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene7(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene9(fd_stdin, fd_stdout);
	}
}

void Scene10(int fd_stdin, int fd_stdout){//场景10
	char choice[2];//输入的选项
	GameProgress=10;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You rush to the light at all costs, in your sleep, vaguely hear the familiar voice.\n");
	milli_delay(10000);
	printf("They're your family, you remember, they're surprised, they're crying.\n");
	milli_delay(10000);
	printf("You remember everything.\n");
	printf("You used to be a soldier on the battlefield\n");
	printf("and lost consciousness in an explosion,They say you've become a vegetable, \n");
	printf("but you've finally untied the knot and you've finally come to life.\n");
	milli_delay(20000);
	printf("                     At least, you think you're waking up now...              \n");
	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene8(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene10(fd_stdin, fd_stdout);
	}
}

void Scene11(int fd_stdin, int fd_stdout){//场景11
	char choice[2];//输入的选项
	GameProgress=11;
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game. \n");
	printf("Press a,b... to input your choice!\n");
	printf("Press e to go back\n");
	printf("==============================================================================\n");
	milli_delay(10000);	
	printf("You give up the light and sleep in the dark forever.\n");
	
	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "q")){
		return;
	}
	else if(!strcmp(choice, "e")){
		Scene8(fd_stdin, fd_stdout);
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Scene11(fd_stdin, fd_stdout);
	}
}

void Game1(int fd_stdin, int fd_stdout){
	char choice[2];//输入的选项
    //欢迎与说明
	clear();
	printf("==============================================================================\n");
	printf("Press q to quit the game.\n");
	printf("Press c to continue your last rate of progress.\n");
	printf("Press s and let's start adv!\n");
	printf("===============================================================================\n");

	read(fd_stdin, choice, 1);
	if(!strcmp(choice, "s")){
		Scene1(fd_stdin, fd_stdout);
	}
	else if(!strcmp(choice, "q")){
		printf("Game Over!\n");
		return;
	}
	else if(!strcmp(choice, "c")){//读档
		if(GameProgress==0){
			printf("There is no record of GameProgress! Why not start now?\n");
			Game1(fd_stdin, fd_stdout);
		}
		else if(GameProgress==1){
			Scene1(fd_stdin, fd_stdout);
		}
		else if(GameProgress==2){
			Scene2(fd_stdin, fd_stdout);
		}
		else if(GameProgress==3){
			Scene3(fd_stdin, fd_stdout);
		}
		else if(GameProgress==4){
			Scene4(fd_stdin, fd_stdout);
		}
		else if(GameProgress==5){
			Scene5(fd_stdin, fd_stdout);
		}
		else if(GameProgress==6){
			Scene6(fd_stdin, fd_stdout);
		}
		else if(GameProgress==7){
			Scene7(fd_stdin, fd_stdout);
		}
		else if(GameProgress==8){
			Scene8(fd_stdin, fd_stdout);
		}
		else if(GameProgress==9){
			Scene9(fd_stdin, fd_stdout);
		}
		else if(GameProgress==10){
			Scene10(fd_stdin, fd_stdout);
		}
		else if(GameProgress==11){
			Scene11(fd_stdin, fd_stdout);
		}
	}
	else {
		printf("I don't know what's the meaning of this. How about check the board up there?\n");
		milli_delay(50000);
		Game1(fd_stdin, fd_stdout);
	}
	printf("Game Over!");
}
/*======================================================================*
小游戏2 
五子棋
*======================================================================*/
int px,py;
int ROWS=10,COLS=10;//棋盘尺寸
char board[10][10];
void init_board();
void Display_board(int fd_stdin, int fd_stdout);
int player_move(int fd_stdin, int fd_stdout);
void computer_move();
static int Is_full( int row, int col);
int check_win(int m,int n);
char Is_win();

void init_board()
{
	for(int i=0;i<ROWS;i++){
		for(int j=0;j<COLS;j++){
			board[i][j]=' ';
		}
	}
}
void Display_board(int fd_stdin, int fd_stdout)
{
	clear();
	menu();
	int i = 0;
	for (i = 0; i < ROWS; i++)
	{
		if(i==0) printf("                           1  2  3  4  5  6  7  8  9  10\n");
		if (i != ROWS-1){
			printf("                        %d  %c |%c |%c |%c |%c |%c |%c |%c |%c |%c\n", i+1, board[i][0], board[i][1], board[i][2], board[i][3], board[i][4], board[i][5], board[i][6], board[i][7], board[i][8], board[i][9]);
			printf("                           --|--|--|--|--|--|--|--|--|--\n");          //打印棋盘规格
		}
		if(i==ROWS-1) printf("                        10 %c |%c |%c |%c |%c |%c |%c |%c |%c |%c\n",board[i][0], board[i][1], board[i][2], board[i][3], board[i][4], board[i][5], board[i][6], board[i][7], board[i][8], board[i][9]);
	}
}
int player_move(int fd_stdin, int fd_stdout)
{
	while (1)
	{
		clear();
		Display_board(fd_stdin,fd_stdout);
		printf("Please input your next location:");
		char cx[3];
		char cy[3];
		int r = read(fd_stdin, cx, 2);
		cx[r] = 0;
		if(!strcmp(cx,"q")){
			return -1;
		}
		atoi(cx, &px);
		int t = read(fd_stdin, cy, 2);
		cy[t] = 0;
		atoi(cy, &py);
		px--;
		py--;
		if (((px >= 0) && (px < ROWS)) && ((py >= 0) && (py < COLS)))
		{
			if (board[px][py] == ' ')
			{
				board[px][py] = 'X';      //玩家1落子记为*
				break;
			}
			else{
				printf("WRONG POS! Please input correct location!\n");
				milli_delay(10000);
			}
		}
		else{
			printf("WRONG POS! Please input correct location!\n");
			milli_delay(10000);
		}
	}
}
void computer_move()
{
	while (1)
	{
		int x = get_ticks() % 10;   //获取系统启动到当前的tick数，除以10取余，范围为0-9
		int y = get_ticks() % 10;
		if (board[x][y] == ' ')
		{
			board[x][y] = 'O' ;     //电脑落子记为O
			break;
		}
	}
}
static int Is_full( int row, int col)
{
	int i = 0; int j = 0;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			if (board[i][j] == ' ')
				return 0;
		}
	}
	return 1;
}


int check_win(int m,int n){
	int tot = 1; 
	int i,j;
	for(i = m-1; i >= 0; i--){
		if(board[i][n] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = m+1; i < 10; i++){
		if(board[i][n] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = n-1; i >= 0; i--){
		if(board[m][i] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = n+1; i < 10; i++){
		if(board[m][i] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = m+1,j = n+1; i < 10 && j < 10; i++,j++){
		if(board[i][j] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i =m-1,j = n+1; i >= 0 && j < 10; i--,j++){
		if(board[i][j] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = m-1,j = n-1; i >= 0 && j >= 0; i--,j--){
		if(board[i][j] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	tot = 1;
	for(i = m+1,j=n-1; i < 10 && j >= 0; i++,j--){
		if(board[i][j] == board[m][n]){
			tot++;
			if(tot>=5) return 1;
		}
		else break;
	}
	return 0; //没有胜利。
}


char Is_win()//判断胜利条件
{
	for (int x = 0; x < ROWS; x++)
	{
		for(int y = 0; y < COLS; y++){
			if(board[x][y]=='X'){
				if(check_win(x,y)) return board[x][y];
			}
			if(board[x][y]=='O' && check_win(x,y)){
				if(check_win(x,y)) return board[x][y];
			}
		}
	}
	if (Is_full(ROWS, COLS))
	{
		return 'Q';
	}
	return ' ';
}
void game(int fd_stdin, int fd_stdout)
{
	char ret;
	init_board(board);
	while (1)
	{
		if(player_move(fd_stdin, fd_stdout) == -1) return;           //玩家1落子

		ret = Is_win();
		if (ret!= ' ') break;

		Display_board(fd_stdin, fd_stdout);           //打印棋盘

		computer_move();            

		ret = Is_win();
		if (ret!= ' ') break;

		Display_board(fd_stdin, fd_stdout);
	}
	Display_board(fd_stdin, fd_stdout);
	if (ret == 'X'){
		printf("******************************You Win!****************************\n");
		milli_delay(20000);
	}
	else if (ret == 'O'){
		printf("*****************************You LOSE!****************************\n");
		milli_delay(20000);
	}
	else if (ret == 'Q'){
		printf("*******************************TIED!******************************\n");
		milli_delay(20000);
	}
}

void menu()
{
	printf("############################################################################\n");
	printf("    ####################       p:Play  q:Quit       ###################\n");           //选择游戏还是退出
	printf("############################################################################\n");
}
void Game2(int fd_stdin, int fd_stdout)
{
	clear();
	char input[2];
	menu();

	printf("Please input your choice:>\n");
	read(fd_stdin, input,1);
		
	if(!strcmp(input,"p")){
		game(fd_stdin, fd_stdout);
	}
	else if(!strcmp(input,"q")){
		return;
	}
	else{
		printf("WRONG choice!\n");
		milli_delay(10000);
	}
	return 0;
}

	
/*======================================================================*
                            Timer
 *======================================================================*/

/*======================================================================*
game3                              WHO IS BIGGER
							compare ur number
The refereee give a random number under 100 as the max input number firstly.
2 players input a number less than the above number respectively.
Only if A(B) is more than B(A) but no more than 5, can A(B) win.
Otherwise B(A) wins and get 100points.
TOTAL 3 ROUNDS
THE HIGHER SCORE WINS FINALLY.
 *======================================================================*/

/*======================================================================*
                               TestB
							文件系统管理
 *======================================================================*/

/*
文件系统较为简洁
在调用oranges系统已有的文件操作基础上进行了界面设计(参阅/fs/文件夹)
仅提供文件创建,文件删除，文件打开，文件读写等操作
*/

struct Directory D[100];
struct File F[100];
int Dpos = 0;
int Fpos = 0;

struct Directory* root = 0;
struct Directory* current = 0;

void TestB()
{
	char tty_name[] = "/dev_tty1";

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	/*打印界面*/
	printf("                        ==================================\n");
	printf("                                    File Manager           \n");
	printf("                                 Kernel on Orange's \n\n");
	printf("                        ==================================\n");

	/*定义相关参数*/


	char file_name[128];//操作的文件名
	char cmd[8];//进行的操作
	char rdbuf[128];

	root = &D[Dpos];
	Dpos++;
	root->child = 0;
	root->link = 0;
	root->next = 0;
	root->parent = 0;
	strcpy(root->name, "root");

	current = root;

	while (1) {

		makepath(root, current);
		printf(" $ ");

		/*读取用户输入*/
		int r = read(fd_stdin, rdbuf, 80);
		rdbuf[r] = 0;

		/*进行判断*/
		if (!strcmp(rdbuf, "help")) {
			help_b();
			continue;
		}
		else if (!strcmp(rdbuf, "ls")) {
			displayCurrentDirectory(root);
			continue;
		}
		else {
			/*将用户输入分解成cmd+filename*/
			int i = 0;
			int j = 0;
			while (rdbuf[i] != ' ') {
				cmd[i] = rdbuf[i];
				i++;
				if (i > 128)break;
			}
			if (i > 128) {
				printf("Command not found, please input help to get help!\n");
				continue;
			}

			cmd[i++] = 0;

			while (rdbuf[i] != 0) {
				file_name[j] = rdbuf[i];
				i++;
				j++;
			}
			file_name[j] = 0;

			/*开始执行命令*/
			if (!strcmp(cmd, "crefile")) {
				createFile(file_name);
				continue;
			}
			else if (!strcmp(cmd, "rdfile")) {
				readFile(file_name);
				continue;
			}
			else if (!strcmp(cmd, "wtfile")) {
				writeFile(file_name, fd_stdin);
				continue;
			}
			else if (!strcmp(cmd, "delfile")) {
				deleteFile(file_name);
				continue;
			}
			else if (!strcmp(cmd, "credir")) {
				createDirectory(file_name);
				continue;
			}
			else if (!strcmp(cmd, "deldir")) {
				deleteDirectory(file_name);
				continue;
			}
			else if (!strcmp(cmd, "cd")) {
				if(strcmp(file_name, "..") == 0) BackParentsDirectory();
				else EnterChildDirectory(file_name);
				continue;
			}
			else {
				printf("Command not found, please input help to get help!\n");
				continue;
			}
		}
	}
}


void help_b() {
	printf("=============================================================================\n");
	printf("Command List     :\n");
	printf("1. ls                   : list the all files \n");
	printf("2. crefile [filename]   : Create a new file \n");
	printf("3. credir [dirname]     : Create a new directory \n");
	printf("4. rdfile [filename]    : Read the file\n");
	printf("5. wtfile [filename]    : Write at the end of the file\n");
	printf("6. delfile [filename]   : Delete the file\n");
	printf("7. deldir [dirname]     : Delete the directory \n");
	printf("8. cd [dirname]         : Enter the directory \n");
	printf("9. cd ..                : Return to parents directory \n");
	printf("x. help                 : Show operation guide\n");
	printf("==============================================================================\n");
}

/*======================================================================*
								ProcessManager
								进程任务管理
*======================================================================*/

void Processhomepage()
{
	clear();
	printf("      ====================================================================\n");
	printf("      ||                           SnakeOS For You                      ||\n");
	printf("      ====================================================================\n");
	printf("      ||                              Welcome to                        ||\n");
	printf("      ||                           Process Manager                      ||\n");
	printf("      ||                                                                ||\n");
	printf("      ||                                                                ||\n");
	printf("      ||                              Enter:help                        ||\n");
	printf("      ||                           Get command list                     ||\n");
	printf("      ====================================================================\n");
}
void help_c() {
	printf("====================================================================\n");
	printf("|-------------------------SnakeOS For You--------------------------|\n");
	printf("|--------------------process system command list-------------------|\n");
	printf("|------------------------------------------------------------------|\n");
	printf("|-------1.show-------|-------Show your process information---------|\n");
	printf("|-------2.restart----|-------Restart a process---------------------|\n");
	printf("|-------3.kill-------|-------Kill a process------------------------|\n");
	printf("|-------4.clear------|-------Clear the screen----------------------|\n");
	printf("|-------5.help-------|-------Show process system command list------|\n");
	printf("|-------6.quit-------|-------Quit the process manger---------------|\n");
	printf("====================================================================\n");
}

/*添加进程函数*/
int addProcess()
{
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; ++i)//逐个遍历
	{
		if (proc_table[i].priority == 0)
			break;//挂起的就退出循环
	}
	return i;
}
/*结束进程*/
void kill_p1(int fd_stdin)
{
	int _pid;
	printf("Input the ID(you want to kill): ");
	char cstr[60];
	int par = read(fd_stdin, cstr, 60);
	cstr[par] = 0;
	atoi(cstr, &_pid);
	if (!strcmp(proc_table[_pid].name, "TestA")) {
		printf("Can't killed sysytem process!\n");
		return;
	}
	if (!strcmp(proc_table[_pid].name, "TestB")) {
		printf("Can't killed sysytem process!\n");
		return;
	}
	if (!strcmp(proc_table[_pid].name, "TestC")) {
		proc_table[_pid].priority = 0;
		printf("kill successful!\n");
		return;
	}
	else {
		if (proc_table[_pid].priority == 0)
		{
			printf("kill failed!\n");
			return;
		}
		/*让其优先级为零 挂起 近似于kill*/
		proc_table[_pid].priority = 0;
		//proc_table[_pid].name[0] = 0;
		printf("kill successful!\n");
		return;
	}
}

//结束进程
void kill_p(int fd_stdin)
{
	int p_pid;
	printf("Input the ID(you want to kill): ");
	char cstr[60];
	int par = read(fd_stdin, cstr, 60);
	cstr[par] = 0;
	atoi(cstr, &p_pid);

	//健壮性处理以及结束进程
	if (p_pid >= NR_TASKS + NR_PROCS || p_pid < 0)
	{
		printf("The PID out of the range\n");
		return;
	}
	else {
		if (p_pid == 4 || p_pid == 5)
		{
			printf("Cannot kill this process!please retry\n");
			return;
		}
		else {
			if (p_pid == 6)
			{
				proc_table[p_pid].priority = 0;
				printf("Successful!you killed this process\n");
					//return;
			}
			else {
				if (proc_table[p_pid].priority == 0)
				{
					printf("Kill failed!the process has been killed\n");
					return;
				}
				proc_table[p_pid].priority = 0;
				printf("Successful!you killed this task\n");
			}
		}
	}
	show_p();
}
/*重启进程*/
void restart_p1()
{
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if (proc_table[i].priority == 0)
		{
			break;
		}
	}
	if (i == NR_TASKS + NR_PROCS)
		printf("process list is full! \n");
	else
	{
		proc_table[i].priority = 10;
		/*	memset(proc_table[i].name, "new process", 20);*/
			/*proc_table[i].name[0] = "new process";*/
		printf("a new process is running!\n");
	}
}
void restart_p(int fd_stdin)
{
	int id;
	printf("Input the ID(you want to restart): ");
	char cstr[10];
		int par=read(fd_stdin, cstr,10);
		cstr[par] = 0;
		atoi(cstr, &id);
	if (id >= NR_TASKS + NR_PROCS || id < 0)
	{
		printf("The pid out of the range\n");
		return;
	}else if (proc_table[id].priority == 0)
		{
			proc_table[id].priority = 10;
			printf("Restart successful!\n");
			return;
		}
		else {
			printf("The process is running!No need to restart\n");
		}
	show_p();
}
/*查看系统内运行情况*/
void show_p()
{
	printf("========================================================================\n");
	printf("*     PID      |       Name     |    Priority    |    Running?         *\n");
	//进程号，进程名，优先级，是否是系统进程，是否在运行
	printf("========================================================================\n");
	for (int i = 0; i < NR_TASKS + NR_PROCS; ++i)//逐个遍历
	{
		printf("        %d", proc_table[i].pid);
		printf("              %s", proc_table[i].name);
		if (proc_table[i].priority == 0) {
			if (i == 0 || i == 1) {
				printf("                  %d", proc_table[i].priority);
			}
			else if (i == 2 || i == 3) {
				printf("                   %d", proc_table[i].priority);
			}
			else if (i == 6) {
				printf("                %d", proc_table[i].priority);
			}
		}else if (i == 2 || i==3) {
			printf("                  %d", proc_table[i].priority);
		}
		else if (i == 4 || i == 5 || i == 6) {
			printf("                %d", proc_table[i].priority);
		}
		else printf("                 %d", proc_table[i].priority);
		if (proc_table[i].priority != 0)
		{
			printf("             yes\n");
		}
		else
		{
			printf("             no\n");
		}
		/*if (proc_table[i].priority == 0)
			continue;//系统资源跳过
		printf("        %d           %s              %d            yes\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
*/	}
	printf("========================================================================\n");
}
void wrongCommond()
{
	printf("Wrong command.you can input help to get the command list\n");
	printf("please retry!\n");
	return;
}


void ProcessManager(int fd_stdin,int fd_stdout)
{
	int i;
	char readbuf[128];
	Processhomepage();
	while (1) {
		printf("SnakeOS for you: $ ");
		int r = read(fd_stdin, readbuf, 70);
		readbuf[r] = 0;

		if (strcmp(readbuf, "help") == 0) {
			help_c();
			continue;
		}
		else if (strcmp(readbuf, "clear") == 0) {
			Processhomepage();
			continue;
		}
		else if (strcmp(readbuf, "show") == 0) {
			show_p();
			continue;
		}
		else if (strcmp(readbuf, "restart") == 0) {
			restart_p(fd_stdin);
			continue;
		}
		else if (strcmp(readbuf, "kill") == 0) {
			kill_p(fd_stdin);
			continue;
		}
		else if (strcmp(readbuf, "quit") == 0) {
			clear();
			return;
		}
		else {
			wrongCommond();
			continue;
		}
	}
}





/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

