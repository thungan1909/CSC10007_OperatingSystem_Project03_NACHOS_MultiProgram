#include "syscall.h"
//test cac syscall
int main()
{

	int n;
	char c;
	char s[255];

	PrintString("Nhap mot so nguyen: ");
	n = ReadInt();
	PrintString("So vua nhap la: ");
	PrintInt(n);
	PrintString("\n");

	PrintString("Nhap mot ky tu: ");
	c = ReadChar();
	PrintString("Ky tu vua nhap la: ");
	PrintChar(c);
	PrintString("\n");

	PrintString("Nhap mot chuoi: ");
    	ReadString(s,255);
	PrintString("Chuoi vua nhap la: ");
	PrintString(s);
	PrintString("\n");

}
