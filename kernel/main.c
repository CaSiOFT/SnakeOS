/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
						针对源码进行了改进
						通过原有的TestABC
						写上可以进行操作的shell
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Xuejinwei, 2018
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define  H  8//地图的高
#define  L 16//地图的长

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
	disp_pos = 0;
	for (j = 0; j < 960; j++) { disp_str(" "); }
	disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n", YELLOW);
	disp_color_str("                                       jCOQQNNDMMRVx      \n", GREEN);
	disp_color_str("                                     NMMNRSZLksMMDkNMMj  ", GREEN);	disp_color_str("j\n", RED);
	disp_color_str("                                    MKOAhfRNQQNSwQNNRj", GREEN);	disp_color_str("cish\n", RED);
	disp_color_str("                                   QHwJCJRi jiccj       ", GREEN);disp_color_str("            jh         \n", GREEN);
	disp_color_str("                                   jMLkPOQ              ", GREEN);disp_color_str("     jMMMMMMMc         \n", GREEN);
	disp_color_str(" MB     LNMNNMR      jpBNNNNDr      aMKhkQj             ", GREEN);disp_color_str("    MMMMMMr            \n", YELLOW);
	disp_color_str(" jMMJ rMMQtQDhRM   cMMNQPQNkhNMQj     NQVlNRj           ", GREEN);disp_color_str("   sM j jjcpKPc        \n", YELLOW);
	disp_color_str("   MMMMNj  kAppNj  MNlLDI jMhhhQMNj    xQZDQMMN         ", GREEN);disp_color_str(" NNBipClpSlSkORMi     \n", YELLOW);
	disp_color_str("           QOCpQ    NOCwDD  MNhlwSNH    jhClLlhM        ", GREEN);disp_color_str("NMCCSVSLSLfLVppDM  \n", YELLOW);
	disp_color_str("           NQfVhR    DOLpDM  cRMMNBNNQhDOOfLlONM        ", GREEN);disp_color_str(" KMMNKhhOOOHRNMM     \n", YELLOW);
	disp_color_str("            NMMNNMACfBNMMMMi    rhNNNNNMMMMMNRa         ", GREEN);disp_color_str("   cDNMMMMMNQC    \n", YELLOW);
	disp_color_str("               cLZRMMNOprj                              ", GREEN);disp_color_str("      NNNND    \n", YELLOW);
	disp_color_str("                  QhDN\n", GREEN);
	disp_color_str("oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n", YELLOW);
	milli_delay(80000);
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
	printf("                                 Kernel on Orange's \n\n");
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
		printl("[root@localhost /] ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (!strcmp(rdbuf, "cal"))
		{
			int year;
			char temp[70];
			printf("input the year:");
			int r = read(fd_stdin, temp, 70);
			temp[r] = 0;
			atoi(temp, &year);
			Calendar(year);
			printf("\n");
			continue;
		}
        else if (!strcmp(rdbuf, "proc"))
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

			Game3(fd_stdin, fd_stdout);
		}
               else if (!strcmp(rdbuf, "game4"))
		{

			//Game4(fd_stdin, fd_stdout);
		}
		else if (!strcmp(rdbuf, "calcu")) {
			calculator(fd_stdin, fd_stdout);
		}
                else if (!strcmp(rdbuf, "timer") )
		{
			printf("haha!");
                        timer(fd_stdin, fd_stdout);

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
	printf("2. flm        : Run the file manager\n");
	printf("3. cl         : Clear the screen\n");
	printf("4. help       : Show operation guide\n");
	printf("5. cal        : Show a calendar\n");
	printf("6. calcu      : open a calculator\n");
	printf("7. game1      : Run a small game1(guess number) on this OS\n");
	printf("8. game2      : Run a small game2(greedy snake) on this OS\n");
	printf("9. timer      : Set Your clock\n");
	printf("10. game3      :Run a small game1(who is bigger) on this OS\n");
        printf("11. game4      :Run a small game1(2048) on this OS\n");

	printf("==============================================================================\n");
}
/*======================================================================*
calculator
计算器生成相关函数
*======================================================================*/

//计算乘方的函数
int Power(int A, int B) {
	int result = 1;
	int i;
	for (i = 0; i < B; i++) {
		result = A * result;
	}
	return result;
}

void calculator(int fd_stdin, int fd_stdout)
{
	int A = 0;
	int B = 0;
	int result = 0;    //计算结果
	int select = 0;  //选择的选项
	do         
	{
		//打印出操作界面
		printf("--------------------\n");
		printf("   input your choice \n");
		printf("        1.+       \n");
		printf("        2.-       \n");
		printf("        3.*       \n");
		printf("        4./       \n");
		printf("        5.^       \n");
		printf("        6.exit       \n");
		printf("--------------------\n");

		//输入选择
		printf("Please input your choosed number:");
		char temp[70];
		int r = read(fd_stdin, temp, 70);
		temp[r] = 0;
		atoi(temp, &select);
		if (select == 6) break;
		if (select > 6) {
			printf("Input error，please input again!\n");
			continue;
		}
		
		printf("Input your operand A and B:");
		
		//读取两个操作数
		char at[70];
		char bt[70];
		
		int r1 = read(fd_stdin, at, 70);
		at[r1] = 0;
		atoi(at, &A);

		int r2 = read(fd_stdin, bt, 70);
		bt[r2] = 0;
		atoi(bt, &B);

		if(select==1)
			printf("%d+%d=%d\n", A, B, result = A + B);   //实现加法功能
		else if(select==2)
			printf("%d-%d=%d\n", A, B, result = A - B);     //实现减法功能	
		else if(select==3)
			printf("%d*%d=%d\n", A, B, result = A * B);      // 实现乘法功能
		else if(select==4)
			printf("%d/%d=%d\n", A, B, result = A / B);      //实现除法功能
		else if(select==5)
			printf("%d^%d=%d\n", A, B, result = Power(A,B));      //实现乘方功能
		else
			printf("Input error, please input again!\n");
	} while (select);
}




/*======================================================================*
Calendar
日历生成相关函数
*======================================================================*/

/*思路（利用儒略日进行日历计算）:
（1）首先需要打印年月和月历的周一到周日
（2）判断每个月的1号是周几，这样利用固定的算法就可以依次求出2、3、4、、、等是周几
（3）判断什么时候进行换行；判断是否是闰年。
*/

int f(int year, int month)  
{
	//如果月份<3，则f(年，月)＝年－1；否则，f(年，月)＝年
	if (month < 3) return year - 1;
	else return year;
}

int g(int month)  
{
	//如果月份<3，g(月)＝月＋13；否则，g(月)＝月＋1
	if (month < 3) return month + 13;
	else return month + 1;
}


//计算日期的N值
int n(int year, int month, int day)  
{	
	//N=1461*f(年、月)/4+153*g(月)/5+日
	return 1461L * f(year, month) / 4 + 153L * g(month) / 5 + day;
}

//利用N值算出某年某月某日对应的星期几
int w(int year, int month, int day)
{
	//w=(N-621049)%7(0<=w<7)
	return(int)((n(year, month, day) % 7 - 621049L % 7 + 7) % 7);
}

int date[12][6][7];

//该数组对应了非闰月和闰月的每个月份的天数情况
int day_month[][12] = { { 31,28,31,30,31,30,31,31,30,31,30,31 },
{ 31,29,31,30,31,30,31,31,30,31,30,31 } };

void Calendar(int year)
{
	int sw, leap, i, j, k, wd, day;  //leap用来判断闰年

	char title[] = "SUN MON TUE WED THU FRI SAT";

	sw = w(year, 1, 1);
	leap = year % 4 == 0 && year % 100 || year % 400 == 0;  //判断闰年
	for (i = 0; i<12; i++)
		for (j = 0; j<6; j++)
			for (k = 0; k<7; k++)
				date[i][j][k] = 0;  //日期表置0

	for (i = 0; i<12; i++)  //一年十二个月
	{
		for (wd = 0, day = 1; day <= day_month[leap][i]; day++)
		{	//将第i＋1月的日期填入日期表
			date[i][wd][sw] = day;
			sw = ++sw % 7;        //每星期七天，以0至6计数
			if (sw == 0) wd++;    //日期表每七天一行，星期天开始新的一行
		}
	}

	printf("\n|==================The Calendar of Year %d =====================|\n|", year);

	for (i = 0; i<6; i++)
	{	//先测算第i+1月和第i+7月的最大星期数
		for (wd = 0, k = 0; k<7; k++)//日期表的第六行有日期，则wd!=0
			wd += date[i][5][k] + date[i + 6][5][k];
		wd = wd ? 6 : 5;
		printf("%2d  %s  %2d  %s |\n|", i + 1, title, i + 7, title);
		for (j = 0; j<wd; j++)
		{
				printf("   ");//输出四个空白符
				
				//左栏为第i+1月
				for (k = 0; k<7; k++)
					if (date[i][j][k]) printf("%4d", date[i][j][k]);
					else printf("    ");

				printf("     ");//输出十个空白符

				//右栏为第i+7月
				for (k = 0; k<7; k++)
					if (date[i + 6][j][k]) printf("%4d", date[i + 6][j][k]);
					else printf("    ");
					printf(" |\n|");
		}
	}
	printf("=================================================================|\n");

}

/*======================================================================*
小游戏1 猜数字
*======================================================================*/
void Game1(int fd_stdin, int fd_stdout) {
	int result = get_ticks() % 100;   //获取系统启动到当前的tick数，除以100取余，猜数字范围为0-99
	int finish = 0;
	int guess;
	printf("Now the guess number game begin!\n");
	while (!finish) {
		printf("Please input your guess number:");
		
		//读取用户输入的数字
		char temp[70];
		int r = read(fd_stdin, temp, 70);
		temp[r] = 0;
		atoi(temp, &guess);
		
		if (guess < result) {
			printf("Your number is small!\n");
		}
		else if (guess > result) {
			printf("Your number is big!\n");
		}
		else {
			printf("Congratulations! You're right!\n");
			finish = 1;
		}

	}
}


/*======================================================================*
小游戏2 
贪吃蛇
*======================================================================*/

/*需要用到的一些初始值
	坐标体系
	↑x
	→y
*/

	char GameMap[H][L];//游戏地图
	int sum = 1;//蛇的长度
	int over = 0;//判断游戏是否结束

	int  dx[4] = { 0, 0, -1, 1 };  //左、右、上、下的方向
	int  dy[4] = { -1, 1, 0, 0 };
	struct Snake
	{
		int x, y;
		int now;  //当前节点方向，0 左,1右，2上，3下
	}Snake[H*L];
	/*依次为蛇头，蛇身，食物，地图上的普通点*/
	char Shead = '@';
	char Sbody = '#';
	char Sfood = '*';
	char Snode = '.';

	/*生成食物函数*/
	void CreateFood() {
		int x, y;
		while (1) {
			x = get_ticks() % H;
			y = get_ticks() % L;

			if (GameMap[x][y] == '.') {//只能出现在地图上的普通点位置
				GameMap[x][y] = Sfood;
				break;
			}
		}
	}


	/*变长以及判死条件*/
	void HeadTail(int x, int y) {
		if (GameMap[Snake[0].x][Snake[0].y] == '.')  //为空
			GameMap[Snake[0].x][Snake[0].y] = '@';
		else
			if (GameMap[Snake[0].x][Snake[0].y] == '*')  //为食物
			{
				GameMap[Snake[0].x][Snake[0].y] = '@';
				//增加蛇身体长度
				Snake[sum].x = x;
				Snake[sum].y = y;
				Snake[sum].now = Snake[0].now;
				GameMap[Snake[sum].x][Snake[sum].y] = '#';
				sum++;
				CreateFood();  //产生食物
			}
			else
				over = 1;
	}

	void Border() {
		if (Snake[0].x < 0 || Snake[0].x >= H
			|| Snake[0].y < 0 || Snake[0].y >= L)
			over = 1;
	}

	/*蛇移动*/
	void Move() {
		int temp = sum;
		int i, x, y;
		x = Snake[0].x;
		y = Snake[0].y;
		GameMap[x][y] = '.';
		/*开始移动*/
		Snake[0].x = Snake[0].x + dx[Snake[0].now];
		Snake[0].y = Snake[0].y + dy[Snake[0].now];
		Border();//判越界
		HeadTail(x, y);
		/*printf("sum %d \n", sum);*/
		if (sum == temp) {
			//未吃到食物,蛇整体移动
			for (i = 1; i < sum; i++) {
				if (i == 1)//尾节点设置为空
					GameMap[Snake[i].x][Snake[i].y] = '.';
				if (i == sum - 1)//离蛇头最近的节点
				{
					Snake[i].x = x;
					Snake[i].y = y;
					Snake[i].now = Snake[0].now;
				}
				else {
					//其他普通蛇身借点只需往前走一步即可
					Snake[i].x = Snake[i + 1].x;
					Snake[i].y = Snake[i + 1].y;
					Snake[i].now = Snake[i + 1].now;
				}
				GameMap[Snake[i].x][Snake[i].y] = '#'; //移动后赋值为蛇身 
			}

		}
		/*printf("snake[0].x %d  snake[0].y %d\n", Snake[0].x, Snake[0].y);*/
	}


	/*按钮函数*/
	void Button(int fd_stdin, int fd_stdout) {
		char ch[2];
		/*printf("amount %d      kbmount", amount, kb_in.count);*/
		//if (amount != kb_in.count) {
			/*while (1) {*/
		read(fd_stdin, ch, 1);
		/*printf("button= %s \n", *ch);*/
		if (!strcmp(ch, "a")) {
			Snake[0].now = 0;
			return;
		}
		else if (!strcmp(ch, "d")) {
			Snake[0].now = 1;
			return;
		}
		else if (!strcmp(ch, "w")) {
			Snake[0].now = 2;
			return;
		}
		else if (!strcmp(ch, "s")) {
			Snake[0].now = 3;
			return;
		}
		else
			return;
		//}
	}
		//}




	/*地图初始化函数*/
	void Initial(int fd_stdin, int fd_stdout) {
		int hx, hy;
		int i, j;
		memset(GameMap, '.', sizeof(GameMap));//使地图上的所有点为普通点
		clear();
		hx = get_ticks() % H;
		hy = get_ticks() % L;
		/*初始化蛇*/
		GameMap[hx][hy] = Shead;
		Snake[0].x = hx;
		Snake[0].y = hy;
		Snake[0].now = -1;
		CreateFood();


		printf("==================================\n");
		printf("Greedy Snake           \n");
		printf("Welcome !\n");
		printf("==================================\n");

		printf("Press any key to start\n");

		for (i = 0; i < H; i++) {
			for (j = 0; j < L; j++) {
				printf("%c", GameMap[i][j]);
			}
			printf("\n");
		}

		char ch;
		read(fd_stdin, ch, 1);
		/*printf("initial %s\n", ch);*/
		Button(fd_stdin, fd_stdout);
		/*printf("snake[0].now %d", Snake[0].now);*/
	}



	void Game2(int fd_stdin, int fd_stdout) {
		Initial(fd_stdin, fd_stdout);
		int i, j;
		while (1) {
			/*printf("i am delaying");*/
			delay(10);
			/*printf("i am delayed");*/
			Button(fd_stdin, fd_stdout);
			/*printf("i am buttoned");
			printf("snake[0].now %d", Snake[0].now);*/
			Move();
			/*printf("move\n");*/

			if (over) {
				printf("\n Game over!\n");
				return;
			}

			clear();

			printf("==================================\n");
			printf("Greedy Snake           \n");
			printf("Welcome !\n");
			printf("==================================\n");

			printf("Press any key to start\n");


			for (i = 0; i < H; i++)
			{
				for (j = 0; j < L; j++)
					printf("%c", GameMap[i][j]);
				printf("\n");
			}

		}
	}
/*======================================================================*
                            Timer
 *======================================================================*/
void timer(int fd_stdin,int fd_stdout)
{
clear();
printf("\n \n ");     
printf("          Please set your timer !(NOTICE: the unit is second!^.^)\n");
		char rdbuf[128];
		int tmp = 0;
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		atoi(rdbuf,&tmp);

		int i = tmp;
		printf("the time you set is %d\nNow begin....\n",tmp);
		for(;i >=0;i--){
			printf("%d left\n",i);
			milli_delay(10000);
		}
		printf("Time up!\n");
printf(" ...  ..............************.......... \n");             
printf("        ...  .........]/@@@@@@@@@@@@@@@Oo\`*........\n");           
printf("            ......,/@@@@@[`........,[@@@@@@Oo\**... .\n");          
printf("       .... ..../@@@@`.......=@@@^......,\@@@@Oo]*.... .   \n");    
printf("       ......./@@@`..........=@@@O.........,@@@@OO^*..... \n");     
printf("       .....,@@@`./@^........=@@@O.......,@@.,@@@@Oo`.... \n");     
printf("       ...=@@/......,@@^.....=@@@O....=@@/......\@@@@O\*.. \n");    
printf("       ..*@@@........  ......=@@@O...............@@@@@O^*... \n");  
printf("      ...=@@`....       .....=@@@O.......    ....,@@@@OO`..\n");    
printf("    . ..*@@@........   ......=@@@O................@@@@@O\*..\n");   
printf("     ...*@@/.@@@@@@^   .....=@@@@@@@@@@@\`=@@@@@@.=@@@@Oo*..  \n"); 
printf("     ...*@@@.,[[[[[.    ....=@@@@@@@@@@@@O.[[[[[`./@@@@Oo*.. \n");  
printf("     ...*@@@........   ......,OOO/[*****....   ...@@@@@Oo*..   \n");
printf("      ..*O@@^....   ........................   ..=@@@@@Oo*..   \n");
printf("      ..*=@@@`...   .,]................]`...   .,@@@@@@O^...   \n");
printf("    . ...*\@@@`....,@@@`.......... ...,@@@`....,@@@@@@Oo*...   \n");
printf("       ...,o@@@^..@@@^...   ..]]........,@@@..=@@@@@@Oo*....   \n");
printf("       ....,o@@@@`.......   .=@@^.     .....,@@@@@@@Oo`... .   \n");
printf("       .....*\O@@@@]....    .=@@^.     ...]@@@@@@@@Oo*....     \n");
printf("       ......*=OO@@@@@\`......@@`.....,/@@@@@@@@@OO`*...       \n");
printf("           ....*[OO@@@@@@@@@@\\]/@@@@@@@@@@@@@@OO^*.....       \n");
printf("               .....*,\oOOO@@@@@@@@@@@@OOOo/`*......           \n");
printf("               .... ....**,[[oooooooo/[`**......               \n");

}

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
void Game3(int fd_stdin, int fd_stdout) {
	
	int finish = 0;
        int a,b,max;
        int score_A,score_B;
        score_A=0;
        score_B=0;
	while (finish!=3) {

              printf("Referee shows the max input number please.");

 //读取caipan输入的数字
		char RAND_MAX[70];
		int r = read(fd_stdin, RAND_MAX, 70);
		RAND_MAX[r] = 0;
		atoi(RAND_MAX, &max);

              printf("Now please input your number respectively\n");
              clear();
              printf("The max input number is %d \n",max);
              printf("Please input your number(A):");
//读取用户A输入的数字
		char A[70];
		int rA = read(fd_stdin, A, 70);
		A[rA] = 0;
		atoi(A, &a);
                while(a>max)
                {
                 printf("Input Error.\n Please input your number(A) again:");
		char A[70];
		int rA = read(fd_stdin, A, 70);
		A[rA] = 0;
		atoi(A, &a);
                }
                clear();
              printf("The max input number is %d \n",max);
                printf("Please input your number(B):");
//读取用户B输入的数字
		char B[70];
		int rB = read(fd_stdin, B, 70);
		B[rB] = 0;
		atoi(B, &b);
                
                 while(b>max)
                {
                 printf("Input Error.\n Please input your number(B) again:");
		char B[70];
		int rB = read(fd_stdin, B, 70);
		B[rB] = 0;
		atoi(B, &b);
                }
               
		if (a < b&& a<b-5) {
			printf("This round A wins!\n");
                        score_A+=100;
                        
		}
		else if (a < b&& a>b-5) {
			printf("This round B wins!\n");
                        score_B+=100;
		}
		else if (a > b&& a>b-5) {
			printf("This round B wins!\n");
                        score_B+=100;
		}
                else if (a > b&& a<b-5) {
			printf("This round A wins!\n");
                        score_A+=100;
		}
                finish++;
             
	}
        if(score_A>score_B)
        printf("A is the winner!\n");
        else
        printf("B is the winner!\n");
}


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
文件操作相关函数
*======================================================================*/

void makepath()
{
	printf("\\root");
	if(current != root) {
		struct Directory* p = root->child;
		while(p != current->child) {
			printf("\\%s", p->name);
			p = p->child;
		}
	}
}

void initDirectory(struct Directory *p)
{
	p->child = 0;
	p->link = 0;
	p->next = 0;
	p->parent = 0;
}

void initFile(struct File *p)
{
	p->next = 0;
}

int createDirectory(char* file_name)
{
	struct Directory* p = current->child;
	if (p == 0) {
		current->child = &D[Dpos];
		Dpos++;
		initDirectory(current->child);
		p = current->child;
	}
	else {
		while (p->next != 0) p = p->next;
		p->next = &D[Dpos];
		Dpos++;
		initDirectory(p->next);
		p = p->next;
	}
	p->parent = current;
	strcpy(p->name, file_name);
	printf("Directory create successful: %s\n", file_name);
}

int createFile(char* file_name) 
{
	int fd;
	fd = open(file_name, O_CREAT | O_RDWR);
	if(fd == -1){
		printf("Create file failed! the file has been existed.\n");
		return -1;
	}

	struct File* p = current->link;
	if (p == 0) {
		current->link = &F[Fpos];
		Fpos++;
		initFile(current->link);
		p = current->link;
	}
	else {
		while (p->next != 0) p = p->next;
		p->next = &F[Fpos];
		Fpos++;
		initFile(p->next);
		p = p->next;
	}
	strcpy(p->name, file_name);
	
	char buf[128];
	buf[0] = 0;
	write(fd, buf, 1);
	printf("File create successful: %s (fd %d)\n", file_name, fd);
	close(fd);
	return 1;
}

int inCurrentF(char* file_name)
{
	struct File* p = current->link;
	while(p != 0){
		if(strcmp(p->name, file_name) == 0){
			return 1;
		}
		p = p->next;
	}
	return 0;
}

int inCurrentD(char* file_name)
{
	struct Directory* p = current->child;
	while(p != 0){
		if(strcmp(p->name, file_name) == 0){
			return 1;
		}
		p = p->next;
	}
	return 0;
}

int readFile(char* file_name) 
{
	int fd;
	fd = open(file_name, O_RDWR);
	if (fd ==-1) {
		printf("Open file failed! please check the filename \n");
		return -1;
	}
	if(!inCurrentF(file_name)) {
		printf("Open file failed! it isn't in current directory \n");
		return -1;
	}

	char buf[1024];
	int n = read(fd, buf, 1024);
	printf("%s\n", buf);
	close(fd);
	return 1;
}

int writeFile(char* file_name,int fd_stdin) 
{
	int fd;
	fd = open(file_name, O_RDWR);
	if (fd == -1)
	{
		printf("Open file failed! please check the filename \n");
		return -1;
	}
	if(!inCurrentF(file_name)) {
		printf("Open file failed! it isn't in current directory \n");
		return -1;
	}

	char buf[128];
	int r = read(fd_stdin, buf, 80);
	buf[r] = 0;

	int m = write(fd, buf, r+ 1);
	close(fd);
	return 1;
}

int deleteFile(char* file_name) 
{
	if(!inCurrentF(file_name)) {
		printf("Delete file failed! it isn't in current directory \n");
		return -1;
	}
	struct File* p = current->link;
	struct File* q = p;
	while(strcmp(p->name, file_name) != 0) p = p->next;
	if(p == current->link) current->link = p->next;
	else {
		while(q->next != p) q = q->next;
		q->next = p->next;
	}

	int r = unlink(file_name);
	if (r == 0)
	{
		printf("File delete successful!\n");
		return 1;
	}
	else
	{
		printf("Delete file failed! Please check the fileaname!\n");
		return -1;
	}
}

void clearFile(struct Directory* d, struct File* f)
{
	if (d->link == f) {
		d->link = f->next;
	}
	else {
		struct File* p = d->link;
		while (p->next != f) p = p->next;
		p->next = f->next;
	}
	int r = unlink(f->name);
}

void Clear(struct Directory* x)
{
	while (x->link != 0) clearFile(x, x->link);
	while (x->child!= 0) {
		struct Directory* p = x->child;
		x->child = p->next;
		Clear(p);
	}
}

int deleteDirectory(char* file_name)
{
	if(!inCurrentD(file_name)) {
		printf("Delete directory failed! it isn't in current directory \n");
		return -1;
	}
	struct Directory* p = current->child;
	while (p != 0) {
		if (strcmp(p->name, file_name) == 0) break;
		p = p->next;
	}
	if (p == current->child) current->child = p->next;
	else {
		struct Directory* q = current->child;
		while (q->next != p) q = q->next;
		q->next = p->next;
	}
	Clear(p);
	printf("Directory delete successful!\n");
	return 1;
}

int EnterChildDirectory(char* file_name)
{
	if(!inCurrentD(file_name)) {
		printf("Enter directory failed! it isn't in current directory \n");
		return -1;
	}
	struct Directory* p = current->child;
	while(strcmp(p->name, file_name) != 0) p = p->next;
	current = p;
	printf("Enter directory successfully!\n");
	return 1;
}

int BackParentsDirectory()
{
	if(current == root){
		printf("root has no parents directory!\n");
		return -1;
	}
	else{
		current = current->parent;
		printf("Back parents directory successfully!\n");
		return 1;
	}
}

void displayCurrentDirectory()
{
	if(current->link == 0 && current->child == 0)
		printf("NOTHING!\n");
	else{
		struct Directory* p = current->child;
		struct File* q = current->link;
		while(p != 0) {
			printf("Floader: %s\n", p->name);
			p = p->next;
		}
		while(q != 0) {
			printf("File: %s\n", q->name);
			q = q->next;
		}
	}
}

void TestC() {
	while (1) {

	}
}
/*======================================================================*
								ProcessManager
								进程任务管理
*======================================================================*/


void help_c() {
	printf("=============================================================================\n");
	printf("Command List    :\n");
	printf("1.info          : show your process information\n");
	printf("2.create        : Create a new process\n");
	printf("1.kill          : kill a process \n");
	printf("4.clear         : clear the screen\n");
	printf("5.help         : Show operation guide\n");
	printf("6.exit         : exit the process manger \n");
	printf("==============================================================================\n");
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
/*查看系统内运行情况*/
void ProcessInfo()
{
	printf("=============================================================================\n");
	printf("      PID      |    name       | priority    | running?\n");
	//进程号，进程名，优先级，是否是系统进程，是否在运行
	printf("-----------------------------------------------------------------------------\n");
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; ++i)//逐个遍历
	{
		if (proc_table[i].priority == 0)
			continue;//系统资源跳过
		printf("        %d           %s              %d            yes\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
	}
	printf("=============================================================================\n");
}



void ProcessManager(int fd_stdin,int fd_stdout)
{
	int i, n;

	char rdbuf[128];

	clear();
	printf("                        ==================================\n");
	printf("                                   Process Manager           \n");
	printf("                                 Kernel on Orange's \n\n");
	printf("                        ==================================\n");


	while (1) {
		printl("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if (!strcmp(rdbuf, "help")) {
			help_c();
			continue;
		}
		else if (!strcmp(rdbuf, "clear")) {
			clear();
			printf("                        ==================================\n");
			printf("                                   Process Manager           \n");
			printf("                                 Kernel on Orange's \n\n");
			printf("                        ==================================\n");
			continue;
		}
		else if (!strcmp(rdbuf, "info")) {
			ProcessInfo();
			continue;
		}
		else if (!strcmp(rdbuf, "create")) {
			int i;
			for (i = 0; i < NR_TASKS + NR_PROCS; ++i)
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
				i = addProcess();
				proc_table[i].priority = 10;
			/*	memset(proc_table[i].name, "new process", 20);*/
				/*proc_table[i].name[0] = "new process";*/
				printf("a new process is running!\n");
			}
			continue;
		}
		else if (!strcmp(rdbuf, "kill")) {
			int _pid;
			printf("Input the pro-ID #");
			char buf[70];
			int m = read(fd_stdin, buf, 70);
			buf[m] = 0;
			atoi(buf, &_pid);
			if (!strcmp(proc_table[_pid].name, "TestA")) {
				printf("Can't killed sysytem process!\n");
				continue;
			}
			if (!strcmp(proc_table[_pid].name, "TestB")) {
				printf("Can't killed sysytem process!\n");
				continue;
			}
			if (!strcmp(proc_table[_pid].name, "TestC")) {
				printf("kill successful!\n");
				continue;
			}
			else {
				if (proc_table[_pid].priority == 0)
				{
					printf("kill failed!\n");
					continue;
				}
				/*让其优先级为零 挂起 近似于kill*/
				proc_table[_pid].priority = 0;
				proc_table[_pid].name[0] = 0;
				printf("kill successful!\n");
				continue;
			}
		}
		else if (!strcmp(rdbuf, "exit")) {
			clear();
			printf("                        ==================================\n");
			printf("                                   Xinux v1.0.0             \n");
			printf("                                 Kernel on Orange's \n");
			printf("                                     Welcome !\n");
			printf("                        ==================================\n");
			return;
		}
		else {
			printf("Command not found, please input help to get help!\n");
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

