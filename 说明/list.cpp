#include <iostream>
#include <windows.h>
#include <process.h>

using namespace std;

const int NODENUM=500000;

struct node{
	int id;
	char name[20];
	struct node* pNext;
};

struct node *pHead=NULL, *pTail=NULL;

void AddTail(node* pNode){
	if(pHead == NULL){
		//当前链表为空链表
		pHead=pNode;
		pTail=pNode;
		pNode->pNext=NULL;
	}
	else{
		//当前链表不空
		pTail->pNext=pNode;
		pTail=pNode;
		pTail->pNext=NULL;
	}
}

void print()
{
	struct node *pCur;
	pCur=pHead;
	while(pCur != NULL) {
		printf("%d\n", pCur->id);
		pCur=pCur->pNext;
	}
	printf("=========================\n");
}

void stat()
{
	int count=0;
	struct node *pCur;

	printf("Begin stating....\n");
	pCur=pHead;
	while(pCur != NULL) {
		count++;
		pCur=pCur->pNext;
	}
	printf("total number of nodes : %d\n",count);
}

unsigned __stdcall threadTest1(void *pArg)
{
	struct node* p;
	int i;
	for(i=0; i<NODENUM; i++){
		p=new node;
		p->id=i;
		AddTail(p);
	}
	return 0;
}

unsigned __stdcall threadTest2(void *pArg)
{
	struct node* p;
	int i;
	for(i=0; i<NODENUM; i++){
		p=new node;
		p->id=i + NODENUM;
		AddTail(p);
	}
	return 0;
}

int main()
{
	HANDLE hThread[2];

	hThread[0]=(HANDLE)_beginthreadex(NULL,0,threadTest1,NULL,0,NULL);
	hThread[1]=(HANDLE)_beginthreadex(NULL,0,threadTest2,NULL,0,NULL);

	WaitForSingleObject(hThread[0],-1);
	WaitForSingleObject(hThread[1],-1);

	stat();

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	return 0;
}
