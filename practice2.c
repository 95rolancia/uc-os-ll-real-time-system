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
OS_STK          ADtaskStk[TASK_STK_SIZE];  
OS_STK          TempTaskStk[TASK_STK_SIZE];
OS_STK          HumidTaskStk[TASK_STK_SIZE];
OS_STK          ErrHandlerStk[TASK_STK_SIZE];


void TaskStart(void* pdat);
void ADtask(void* pdat);
void TempTask(void* pdat);
void HumidTask(void* pdat);
void ErrHandler(void* pdat);

/********************************************************************************************/
/*                                        외부 참조                                         */
/********************************************************************************************/
OS_EVENT *TDataMbox, *HDataMbox, *ErrMsgQ, *sem;
OS_MEM* ErrMsgPart;

OS_MEM ErrMsgPart_Buf[OS_MAX_MEM_PART];	/* Free MCB List 공간			 */
void* Queue[10];
typedef struct {
	int id;
	int value;
}Message;

typedef struct {
	int id[2];
	int value[2];
} CurState;
CurState curState;

typedef struct {
	int code;
	int time;
	char* msg;
}ErrMsg;



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
	srand((unsigned int)time(NULL));
	TDataMbox = OSMboxCreate((void*)0);
	HDataMbox = OSMboxCreate((void*)0);
	ErrMsgQ = OSQCreate(Queue, 10);
	sem = OSSemCreate(1);
	ErrMsgPart = OSMemCreate(&ErrMsgPart_Buf[0], OS_MAX_MEM_PART, sizeof(ErrMsg), &err);

	err = 0;
	err += OSTaskCreate(ADtask, (void*)0, &ADtaskStk[TASK_STK_SIZE - 1], 11);
	err += OSTaskCreate(TempTask, (void*)0, &TempTaskStk[TASK_STK_SIZE - 1], 12);
	err += OSTaskCreate(HumidTask, (void*)0, &HumidTaskStk[TASK_STK_SIZE - 1], 13);
	err += OSTaskCreate(ErrHandler, (void*)0, &ErrHandlerStk[TASK_STK_SIZE - 1], 14);

	while (1) {
		OSTimeDlyHMSM(0, 0, 0, 100);;
	};
}


void ADtask(void* pdat) {
	int dly;
	char ch;
	int a, b;
	char binary[8];
	Message* msg_temp = (Message *)malloc(sizeof(Message));
	Message* msg_humid = (Message*)malloc(sizeof(Message));
	srand((unsigned int)time(NULL));

	while (1) {
		dly = rand() % 700;
		OSTimeDlyHMSM(0, 0, 0, dly);
		printf("\n\n***PUSH THE ANY KEY***\n"); // 키입력
		ch = _getch();
		
		a = 0; b = 0;

		for (int i = 128, j = 0; i > 0; i /= 2, j++) {
			if (i & ch) {
				binary[j] = 1;
			}
			else {
				binary[j] = 0;
			}
			if (j >= 1 && j <= 3) {
				a += binary[j] * (i/16);
			}
			if(j >= 5 && j <= 7) {
				b += binary[j] * i;
			}
		}
		msg_temp->id = 1;
		msg_temp->value = a;

		msg_humid->id = 2;
		msg_humid->value = b;
		
		OSMboxPost(TDataMbox, (void*)msg_temp);
		OSMboxPost(HDataMbox, (void*)msg_humid);
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}


void TempTask(void* pdat) {
	INT8U err;
	int dly;
	Message* mb;
	ErrMsg* errMsg;
	srand((unsigned int)time(NULL));
	while (1) {
		dly = rand() % 800;
		OSTimeDlyHMSM(0, 0, 0, dly);
		mb = OSMboxPend(TDataMbox, 10, &err);
		if (mb == (void *)0) {
			printf("<TDataMBox IS EMPTY>\n");
			continue;
		}
		else if (mb->id != 1) {
			printf("<WRONG ID>\n");
			continue;
		} 

		if (mb->value >= 4 && mb->value <= 7) {
			errMsg = (ErrMsg*)OSMemGet(ErrMsgPart, &err);
			errMsg->code = 1;
			errMsg->time = OSTimeGet();
			errMsg->msg = "Temp is going up";
			OSQPost(ErrMsgQ, (void*)errMsg);
		}
		else if (mb->value >= 7) {
			errMsg = (ErrMsg*)OSMemGet(ErrMsgPart, &err);
			errMsg->code = 2;
			errMsg->time = OSTimeGet();
			errMsg->msg = "Temp is so high";
			OSQPost(ErrMsgQ, (void*)errMsg);
		}
		else {
			OSSemPend(sem, 0, &err);
			curState.id[0] = mb->id;
			curState.value[0] = mb->value;
			OSSemPost(sem);
		}
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}

void HumidTask(void* pdat) {
	INT8U err;
	int dly;
	Message* mb;
	ErrMsg* errMsg;
	srand((unsigned int)time(NULL));
	while (1) {
		dly = rand() % 900;
		OSTimeDlyHMSM(0, 0, 0, dly);
		mb = OSMboxPend(HDataMbox, 10, &err);
		
		if (mb == (void *)0) {
			printf("<HDataMbox IS EMPTY>\n");
			continue;
		} 
		else if (mb->id != 2) {
			printf("<WRONG ID>\n");
			continue;
		}

		if (mb->value >= 4 && mb->value <= 7) {	
			errMsg = (ErrMsg*)OSMemGet(ErrMsgPart, &err);
			errMsg->code = 1;
			errMsg->time = OSTimeGet();
			errMsg->msg = "Humid is going down";
			OSQPost(ErrMsgQ, (void*)errMsg);
		}
		else if (mb->value >= 7) {
			errMsg = (ErrMsg*)OSMemGet(ErrMsgPart, &err);
			errMsg->code = 2;
			errMsg->time = OSTimeGet();
			errMsg->msg = "Humid is so low";
			OSQPost(ErrMsgQ, (void*)errMsg);
		}
		else {
			OSSemPend(sem, 0, &err);
			curState.id[1] = mb->id;
			curState.value[1] = mb->value;
			OSSemPost(sem);
		}
		OSTimeDlyHMSM(0, 0, 0, dly);
	}
}

void ErrHandler(void* pdat) {
	INT8U err;
	int dly;
	ErrMsg *errMsg;
	srand((unsigned int)time(NULL));
	while (1) {
		dly = rand() % 1000;
		OSTimeDlyHMSM(0, 0, 0, dly);
		errMsg = OSQAccept(ErrMsgQ, &err);
		if (errMsg != (void*)0) {	
			printf("CODE: %d\nTIME: %d\nMSG: %s\n", errMsg->code, errMsg->time, errMsg->msg);
			OSMemPut(ErrMsgPart, (void*)errMsg);
		}
		else {
			OSSemPend(sem, 0, &err);
			printf("It is safe now \n");
			printf("Temp : %d  Humid : %d\n", 20 + curState.value[0] * 12,
				90 - curState.value[1] * 10);
			OSSemPost(sem);
		}
		OSTimeDlyHMSM(0, 0, 0, dly);	
	}
}


