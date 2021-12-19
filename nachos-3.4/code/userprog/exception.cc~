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
		int exitcode = ptable->ExitUpdate(exitStatus);// gan exitcode = ket qua khi thoat thread hien voi exitStatus truyen vao
		machine->WriteRegister(2,exitcode);// ghi ket qua exitcode vao thanh ghi r2
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

            if (numBytes == 0) // Ký tự rỗng => Không hợp lệ
            {
                machine->WriteRegister(2, 0);
                IncreasePC();
            }
            else
            {
                // Ghi ký tự đầu tiên của buffer vào biến c và ghi vào cho hệ thống
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
            các trường hợp hợp lệ: số âm với ký tự '-' ở đầu, số nguyên  
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
                    continue; //nếu là số âm thì bỏ qua ký tự đầu là '-'
                if (buffer[i] == '\0')
                    break;                              //nếu gặp kí tự null, thoát vòng lặp
                if (buffer[i] < '0' || buffer[i] > '9') //nếu gặp bất kỳ ký tự nào không là số nguyên thì ngừng
                {
                    machine->WriteRegister(2, 0);
                    IncreasePC();
                    return; //chuỗi nhập vào không là số nguyên, trả về 0, return ngắt hàm
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
            int number = machine->ReadRegister(4); //đọc số từ thanh ghi r4
            
            //nếu là số 0 thì in ra chuỗi "0"
            if (number == 0)
            {
                gSynchConsole->Write("0", 1);
                IncreasePC();
                return;
            }
           
            bool isNegative = false; //nếu là số âm isNegative = false, ngược lại, bằng true
            int numberOfNum = 0; //biến lưu số chữ số của number
            int firstNumIndex = 0; //vị trí bắt đầu lưu chữ số của number trong buffer
            
            if (number < 0)
            {
                isNegative = true;
                number *= -1; //đổi number sang số dương để tính số chữ số
                firstNumIndex = 1;
            }

            int t_number = number; //tạo biến tạm của number để tính số chữ số
            //Tính số chữ số của number
            while (t_number)
            {
                numberOfNum++;
                t_number /= 10;
            }

            char *buffer = new char[MaxBufferLength + 1]; //tạo buffer chuỗi để in number ra màn hình
        
            for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
            {
                buffer[i] = (char)((number % 10) + 48);
                number /= 10;
            }
            //Trường hợp number là số âm
            if (isNegative)
            {
                buffer[0] = '-'; //vị trí đầu tiên của buffer lưu ký tự '-'
                gSynchConsole->Write(buffer, numberOfNum + 1); //in buffer ra màn hình
                delete buffer;
                IncreasePC();
                return;
            }
            //Trường hợp number là số dương
            gSynchConsole->Write(buffer, numberOfNum); //in buffer ra màn hình
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

            int virtAdd = machine->ReadRegister(4);   //đọc lên tham số đầu được truyền vào là địa chỉ buffer từ người dùng
            int len = machine->ReadRegister(5);       //  doc len input là độ dài lớn nhất của chuỗi
            char *buffer = User2System(virtAdd, len); //copy tu vung nho userspace sang kernelspace
            gSynchConsole->Read(buffer, len);         //Doc chuoi bang ham Read cua SynchConsole
            System2User(virtAdd, len, buffer);        //copy chuoi tu vung nho kernelspace sang userspace
            delete buffer;
            IncreasePC();
            return;
        }
        case SC_PrintString:
        {
            int virtAdd = machine->ReadRegister(4); //đọc lên tham số đầu được truyền vào là địa chỉ buffer từ người dùng
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
