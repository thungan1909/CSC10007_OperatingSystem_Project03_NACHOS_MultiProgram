#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

PCB::PCB(int id) 
{
	joinsem = new Semaphore("JoinSem", 0);
	exitsem = new Semaphore("ExitSem", 0);	
	mutex = new Semaphore("Mutex", 1);
	pid = id; 
	exitcode = 0;
	numwait = 0;              
	if(id)
		parentID = -1;  //gan ID cua tien trinh cha =-1
	else
		parentID = currentThread->processID;  //gan ID tien trinh cha = ID currendThread
		thread = NULL;
}

PCB::~PCB() 
{
	if (joinsem != NULL)
		delete joinsem;
	if (exitsem != NULL)
		delete exitsem;
	if (mutex != NULL)
		delete mutex;
}

//------------------------------------------------------------------
int PCB::GetID()
{
	return pid;
}

int PCB::GetNumWait()
{
	return numwait;
}

int PCB::GetExitCode()
{
	return exitcode;	
}

void PCB::SetExitCode(int ec)
{
	exitcode= ec;
}

void PCB::IncNumWait()
{
	numwait++;
}

void PCB::DecNumWait()
{
	if(numwait)
		numwait--;
}

char* PCB::GetNameThread()
{
	return thread->getName();
}

//-------------------------------------------------------------------
void PCB::JoinWait()
{
	JoinStatus = parentID; 
	IncNumWait(); 
	joinsem->P();
}

void PCB::JoinRelease()
{
	DecNumWait(); 
	joinsem->V();
}

void PCB::ExitWait()
{
	exitsem->P();
}

void PCB::ExitRelease()
{
	exitsem->V();
}

//-------------------------------------------------------------------
int PCB::Exec(char *filename, int pID)
{
	mutex->P();  //Goi de giup tranh tinh trang nap 2 tien trinh cung 1 luc
	thread = new Thread(filename);
	if (thread == NULL){  //Kiem tra thread da khoi tao thanh cong chua
		printf("\nLoi: Khong tao duoc tien trinh moi !!!\n");  //neu chu thi bao loi
		mutex->V();
		return -1;
	}
	thread->processID = pID;	//dat processID cua thread nay la id
	parentID = currentThread->processID;  //Dat parrentID cia thread nay nay la processID cua thread goi thuc thi
	thread->Fork(MyStartProcess,pID);  //Goi thuc thi Fork => Ta cast thread thanh kieu int, sau do khi xu li ham StarProcess ta cast thread ve dung kieu cua no
	mutex->V();
	return pID;
} 


//*************************************************************************************
void MyStartProcess(int pID)
{
	char *filename = ptable->GetName(pID);
	AddrSpace *space= new AddrSpace(filename);
	if(space == NULL)
	{
		printf("\nLoi: Khong du bo nho de cap phat cho tien trinh !!!\n");
		return; 
	}
	currentThread->space= space;

	space->InitRegisters();		// set the initial register values
	space->RestoreState();		// load page table register

	machine->Run();			// jump to the user progam
	ASSERT(FALSE);			// machine->Run never returns;
						// the address space exits
						// by doing the syscall "exit"
}
