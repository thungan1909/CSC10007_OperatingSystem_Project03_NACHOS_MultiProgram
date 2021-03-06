// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxBufferLength 255
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

char *User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; //need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    //printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        //printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

void IncreasePC() //goi ham nay cuoi cac syscall
{
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which)
    {
    case NoException:
        return;
    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            printf("\n\nShutdown, initiated by user program. ");
            //bien toan cuc gSynchConsole
            //char c[5];
            //gSynchConsole->Read(c,5);
            //gSynchConsole->Write(c,5);

            interrupt->Halt();
            break;
	case SC_Exit: {
		int exitStatus = machine->ReadRegister(4);// doc dia chi exitStatus tu thanh ghi r4
		int result = ptable->ExitUpdate(exitStatus);// gan bien result = ket qua khi thoat thread hien voi exitStatus truyen vao
		machine->WriteRegister(2,result);// ghi ket qua bien res vao thanh ghi r2
		IncreasePC();
		break;
	}
	case SC_Exec: {
		int addr = machine->ReadRegister(4); //doc dia chi ten chuong trinh "name" tu thanh ghi r4
		char *name = User2System(addr, 255); // chuyen vung nho tu user space sang system space
		if (fileSystem->Open(name) == NULL){ // neu bi loi
			gSynchConsole->Write("\nKhong mo duoc file", 19); // thi bao "khong mo duoc file"
			machine->WriteRegister(2, -1); 	// gan -1 vao thanh ghi 2
			break;
		}//neu khong loi
		int ID = ptable->ExecUpdate(name); 	// goi ptable->ExecUpdate(namefile)
		machine->WriteRegister(2, ID); // luu ket qua thuc thi vao thanh ghi r2
		IncreasePC();
		break;
	}
	case SC_Join: {
		int ID = machine->ReadRegister(4);	//doc dia chi ID tu thanh ghi r4
		int exitcode = ptable->JoinUpdate(ID);//gan vao exitcode ket qua join tu thread hien tai vao thread con
		machine->WriteRegister(2, exitcode);// luu ket qua vao vao thanh ghi r2
		IncreasePC();
		break;
	}
	case SC_Create:
		break;
	case SC_Open:
		break;
	case SC_Read:
		break;
	case SC_Write:
		break;
	case SC_Close:
		break;
	case SC_Fork:
		break;
	case SC_Yield:
		break;	
        case SC_ReadChar:
        {
            char *buffer = new char[MaxBufferLength];
            int numBytes = gSynchConsole->Read(buffer, MaxBufferLength);

            if (numBytes == 0) // K?? t??? r???ng => Kh??ng h???p l???
            {
                machine->WriteRegister(2, 0);
                IncreasePC();
            }
            else
            {
                // Ghi k?? t??? ?????u ti??n c???a buffer v??o bi???n c v?? ghi v??o cho h??? th???ng
                char c = buffer[0];
                machine->WriteRegister(2, c);
                IncreasePC();
            }

            delete buffer;
            break;
        }
        case SC_PrintChar:
        {

            char c = (char)machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
            gSynchConsole->Write(&c, 1);             // In ky tu tu bien c, 1 byte
            IncreasePC();
            break;
        }
        case SC_ReadInt:
        {
            /*
            syscall doc len 1 chuoi 
            c??c tr?????ng h???p h???p l???: s??? ??m v???i k?? t??? '-' ??? ?????u, s??? nguy??n  
            */
            char *buffer;
            buffer = new char[MaxBufferLength + 1]; //chua them ky tu null
            gSynchConsole->Read(buffer, MaxBufferLength);
            bool isNegative = false;
            int result = 0;
            if (buffer[0] == '-')
                isNegative = true;
            for (int i = 0; i <= MaxBufferLength; i++)
            {
                if (isNegative && i == 0)
                    continue; //n???u l?? s??? ??m th?? b??? qua k?? t??? ?????u l?? '-'
                if (buffer[i] == '\0')
                    break;                              //n???u g???p k?? t??? null, tho??t v??ng l???p
                if (buffer[i] < '0' || buffer[i] > '9') //n???u g???p b???t k??? k?? t??? n??o kh??ng l?? s??? nguy??n th?? ng???ng
                {
                    machine->WriteRegister(2, 0);
                    IncreasePC();
                    return; //chu???i nh???p v??o kh??ng l?? s??? nguy??n, tr??? v??? 0, return ng???t h??m
                }
                result = result * 10 + (int)(buffer[i] - 48);
            }
            if (isNegative)
                result *= -1;
            machine->WriteRegister(2, result);
            IncreasePC();
            break;
        }
        case SC_PrintInt:
        {
            int number = machine->ReadRegister(4); //?????c s??? t??? thanh ghi r4
            
            //n???u l?? s??? 0 th?? in ra chu???i "0"
            if (number == 0)
            {
                gSynchConsole->Write("0", 1);
                IncreasePC();
                return;
            }
           
            bool isNegative = false; //n???u l?? s??? ??m isNegative = false, ng?????c l???i, b???ng true
            int numberOfNum = 0; //bi???n l??u s??? ch??? s??? c???a number
            int firstNumIndex = 0; //v??? tr?? b???t ?????u l??u ch??? s??? c???a number trong buffer
            
            if (number < 0)
            {
                isNegative = true;
                number *= -1; //?????i number sang s??? d????ng ????? t??nh s??? ch??? s???
                firstNumIndex = 1;
            }

            int t_number = number; //t???o bi???n t???m c???a number ????? t??nh s??? ch??? s???
            //T??nh s??? ch??? s??? c???a number
            while (t_number)
            {
                numberOfNum++;
                t_number /= 10;
            }

            char *buffer = new char[MaxBufferLength + 1]; //t???o buffer chu???i ????? in number ra m??n h??nh
        
            for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
            {
                buffer[i] = (char)((number % 10) + 48);
                number /= 10;
            }
            //Tr?????ng h???p number l?? s??? ??m
            if (isNegative)
            {
                buffer[0] = '-'; //v??? tr?? ?????u ti??n c???a buffer l??u k?? t??? '-'
                gSynchConsole->Write(buffer, numberOfNum + 1); //in buffer ra m??n h??nh
                delete buffer;
                IncreasePC();
                return;
            }
            //Tr?????ng h???p number l?? s??? d????ng
            gSynchConsole->Write(buffer, numberOfNum); //in buffer ra m??n h??nh
            delete buffer;
            IncreasePC();
            return;
        }
        case SC_ReadString:
        {
            //tham so dau vao (input): char[]buffer, int length
            //Trong do:
            // - buffer la chuoi ky tu duoc doc vao
            //- length la do dai lon nhat cua chuoi

            int virtAdd = machine->ReadRegister(4);   //?????c l??n tham s??? ?????u ???????c truy???n v??o l?? ?????a ch??? buffer t??? ng?????i d??ng
            int len = machine->ReadRegister(5);       //  doc len input l?? ????? d??i l???n nh???t c???a chu???i
            char *buffer = User2System(virtAdd, len); //copy tu vung nho userspace sang kernelspace
            gSynchConsole->Read(buffer, len);         //Doc chuoi bang ham Read cua SynchConsole
            System2User(virtAdd, len, buffer);        //copy chuoi tu vung nho kernelspace sang userspace
            delete buffer;
            IncreasePC();
            return;
        }
        case SC_PrintString:
        {
            int virtAdd = machine->ReadRegister(4); //?????c l??n tham s??? ?????u ???????c truy???n v??o l?? ?????a ch??? buffer t??? ng?????i d??ng
            char *buffer = User2System(virtAdd, MaxBufferLength);
            gSynchConsole->Write(buffer, MaxBufferLength);

            IncreasePC();
            break;
        }
        default:
            printf("\nUnexpected user mode exeption(%d %d)", which, type);
            interrupt->Halt();
        }
        break;
    case PageFaultException:
        DEBUG('a', "\nNo valid translation found.");
        printf("\n\nNo valid translation found.");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        DEBUG('a', "\nWrite attempted to page  marked \"read-only\".");
        printf("\n\nWrite attempted to page  marked \"read-only\".");
        interrupt->Halt();
        break;
    case BusErrorException:
        DEBUG('a', "\nTranslation resulted in an invalid physical address.");
        printf("\n\nTranslation resulted in an invalid physical address.");
        interrupt->Halt();
        break;
    case AddressErrorException:
        DEBUG('a', "\nUnaligned reference or one that was beyond the end of the address space.");
        printf("\n\nUnaligned reference or one that was beyond the end of the address space.");
        interrupt->Halt();
        break;
    case OverflowException:
        DEBUG('a', "\nInteger overflow in add or sum.");
        printf("\n\nInteger overflow in add or sum.");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        DEBUG('a', "\nUnimplemented or reserved instr.");
        printf("\n\nInteger overflow in add or sum.");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        break;
    }
}
