#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>
#include  <conio.h>
#include  <stdlib.h>
#include  <time.h>
#include  "app_cfg.h"

#define         OS_TIME_DELAY
#define         LOOP_CNT         100

#define         TASK_STK_SIZE    256      /* Size of each task's stacks (# of OS_STK)      */

#define     TITLE_DELAY()   OSTimeDly(OS_TICKS_PER_SEC)
#define     LOOP_DELAY()    OSTimeDly(1)

/********************************************************************************************/
/*                                     GLOBAL VARIABLES                                     */
/********************************************************************************************/
OS_STK          TaskStartStk[TASK_STK_SIZE];   
OS_STK          Task1Stk[TASK_STK_SIZE];       
OS_STK          Task2Stk[TASK_STK_SIZE];      
OS_STK          Task3Stk[TASK_STK_SIZE];                         

void TaskStart(void* pdat);
void Task1(void* pdat);
void Task2(void* pdat);
void Task3(void* pdat);

/********************************************************************************************/
/*                                        외부 참조                                         */
/********************************************************************************************/
OS_EVENT* mB, * mQ, * sem;

void* Queue[10];

typedef struct {
	int id;
	int val;
}Message;

int main(int argc, char* argv[]) {
#if OS_TASK_NAME_EN > 0u
	CPU_INT08U  os_err;
#endif

	CPU_IntInit();
	Mem_Init();                                                 /* Initialize Memory Managment Module                   */
	CPU_IntDis();                                               /* Disable all Interrupts                               */
	CPU_Init();
	OSInit();													/* uC/OS-II를 초기화한다    */

	/* 최초 태스크 생성 */
	OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE - 1], 16);

#if OS_TASK_NAME_EN > 0u
	OSTaskNameSet(APP_CFG_STARTUP_TASK_PRIO,
		(INT8U*)"Startup Task",
		&os_err);
#endif

	OSStart();                                                  /* uC/OS-II 기동            */

	return 0;
}

void TaskStart(void* pdat) {
	INT8U err;
	int num;
	Message * mb;
	int loopCnt = 0; 
	srand((unsigned int)time(NULL));

	mB = OSMboxCreate((void*)0);
	mQ = OSQCreate(Queue, 10);
	sem = OSSemCreate(1);

	err = 0;

	err += OSTaskCreate(Task1, (void*)0, &Task1Stk[TASK_STK_SIZE - 1], 11); 
	err += OSTaskCreate(Task2, (void*)0, &Task2Stk[TASK_STK_SIZE - 1], 12); 
	err += OSTaskCreate(Task3, (void*)0, &Task3Stk[TASK_STK_SIZE - 1], 13); 

	while (1) {
		loopCnt++;
		num = rand() % 10; 
		OSQPost(mQ, (void*)num);
		OSTimeDlyHMSM(0, 0, 0, 100);
		mb=OSMboxAccept(mB);
		if (mb == (Message*)0) {
			printf("<no mbox>");
			continue;
		}
		printf("%d==>%d\n",num, mb->val);

		if (mb->val > 100 || loopCnt >= LOOP_CNT) {
			OSTaskDel(11);
			OSTaskDel(12);
			OSTaskDel(13);
			sem = OSSemDel(sem, OS_DEL_ALWAYS, &err);
			if (sem == (OS_EVENT*)0) {
				printf("Deleted Semaphore sem\n");
			}
			mQ = OSQDel(mQ, OS_DEL_ALWAYS, &err); 
			if (mQ == (OS_EVENT*)0) {
				printf("Deleted Queue MQueue\n");
			}
			mB = OSMboxDel(mB, OS_DEL_ALWAYS, &err); 
			if (mB == (OS_EVENT*)0) {
				printf("Deleted Mailbox Mbox\n");
			}
			OSTaskDel(OS_PRIO_SELF);
		}
	};
}

void Task1(void* pdat) {
	INT8U err, val;
	static Message msg;
	static int tot = 0;
	int dly;
	srand((unsigned int)time(NULL));

	while (1) {
		dly = rand() % 875;
		OSTimeDlyHMSM(0, 0, 0, dly);
		OSSemPend(sem, 0, &err);
		val = (int)OSQPend(mQ, 0, &err);
		OSSemPost(sem);

		tot += val; 
		msg.id = 1;
		msg.val = tot;
		printf("\t\t%d[Task1] %d\n",dly, tot);

		OSMboxPost(mB, (void*) &msg);
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}

void Task2(void* pdat) {
	INT8U err, val;
	static Message msg;
	static int tot = 0;
	int dly;
	srand((unsigned int)time(NULL));

	while(1) {
		dly = rand() % 937;
		OSTimeDlyHMSM(0, 0, 0, dly);
		OSSemPend(sem, 0, &err);
		val = (int)OSQPend(mQ, 0, &err);
		OSSemPost(sem);

		tot += val; 
		msg.id = 2;
		msg.val = tot;
		printf("\t\t\t\t%d[Task2] %d\n",dly, tot);

		OSMboxPost(mB, (void*) &msg);
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}

void Task3(void* pdat) {
	INT8U err, val;
	static Message msg;
	static int tot = 0;
	int dly;
	srand((unsigned int)time(NULL));

	while (1) {
		dly = rand() % 777;
		OSTimeDlyHMSM(0, 0, 0, dly);
		OSSemPend(sem, 0, &err);
		val = (int)OSQPend(mQ, 0, &err);
		OSSemPost(sem);

		tot += val; 
		msg.id = 3;
		msg.val = tot;
		printf("\t\t\t\t\t\t%d[Task3] %d\n", dly,tot);

		OSMboxPost(mB, (void*) &msg);
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}

