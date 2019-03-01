#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <io.h>
#include <conio.h>
#include <windows.h>
#include <fstream>;
using namespace std;
#pragma warning(disable:4996)


enum
{
	ESC = 27,
	ENTER = 13,
	SPACE = 32,
	CURSOR1 = 0,
	CURSOR2 = 224,
	UP = 72,
	DOWN = 80,
	LEFT = 75,
	RIGTH = 77,
};

void Set_Color(WORD wAttributes)
{
	HANDLE hOUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOUTPUT, wAttributes);
}

void SetColor(int b, int w) {
	Set_Color((b << 4) | w);
}

void SetPos(int x, int y) {
	COORD cd;
	cd.X = x;
	cd.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),
		cd);
}

struct MenuColor {
	int bkr;
	int word;
};

struct MainMenu {
	int X, Y;
	int select;
	int Msize;
	int WinHeigth;
	MenuColor defColor;
	MenuColor selColor;
	char **MMenu = nullptr;
	void SetMMenu(char **mas) {
		MMenu = new char*[Msize];
		for (int i = 0; i < Msize; i++) {
			int len = strlen(mas[i]);
			MMenu[i] = new char[len + 1];
			strcpy(MMenu[i], mas[i]);
		}
	}
};

char **EnumFiles(char *FileMask, MainMenu &m);
char *SelectFile(int X, int Y);
void PrintFile(char *FilePath);
void OpenFile(char *FilePath);

void ShowMenu(MainMenu menu, int Clearsize, int start) {

	menu.MMenu += start;
	char *Strclear = new char[Clearsize + 1];
	memset(Strclear, ' ', Clearsize * sizeof(Strclear[0]));
	Strclear[Clearsize] = '\0';

	for (int i = 0; i < menu.WinHeigth; i++) {

		SetPos(menu.X, menu.Y);
		(menu.select == i) ? SetColor(menu.selColor.bkr, menu.selColor.word)
			: SetColor(menu.defColor.bkr, menu.defColor.word);
		cout << Strclear;
		SetPos(menu.X, menu.Y++);
		cout << menu.MMenu[i];

	}
}

int KeyHandler(MainMenu &menu) {

	int key = 0;
	int MaxWidth = 0;
	int start = 0;

	for (int i = 0; i < menu.Msize; i++) {
		int len = strlen(menu.MMenu[i]);
		if (len > MaxWidth) {
			MaxWidth = len;
		}
	}
	menu.WinHeigth = (menu.WinHeigth > menu.Msize) ? menu.Msize : menu.WinHeigth;
	do {

		ShowMenu(menu, MaxWidth, start);
		key = getch();
		if (key == CURSOR1 || key == CURSOR2) {
			key = getch();
			switch (key) {
			case UP:
				if (menu.select > 0) menu.select--;
				else
				{
					if (start>0)
						start--;
				}
				break;
			case DOWN:
				if (menu.select < menu.WinHeigth - 1) menu.select++;
				else
				{
					if (start + menu.WinHeigth < menu.Msize - 1)
						start++;
				}
				break;
			}
		}
		else if (key == ESC) {
			return -1;
		}
		else if (key == ENTER) {
			return menu.select + start;
		}
	} while (true);
}

int main() {
	system("chcp 1251>nul");
	MainMenu Menu;
	Menu.X = 3;
	Menu.Y = 4;
	Menu.select = 0;
	Menu.defColor = { 0,7 };
	Menu.selColor = { 15,0 };
	Menu.WinHeigth = 10;
	char *MMenu[] = {
		"Выбрать файл",
		"Распечатать файл",
		"Открыть файл или папку",
		"Выход(ESC)"
	};
	Menu.Msize = sizeof(MMenu) / sizeof(MMenu[0]);
	Menu.SetMMenu(MMenu);
	int res = 0;
	char *File = nullptr;

	while (res != -1) {
		res = KeyHandler(Menu);
		switch (res) {
		case -1: return 0;
		case 0: File = SelectFile(Menu.X, Menu.Y);
			if (!File) {
				SetPos(3, 9);
				cout << "Файл не выбран";
			}
			else {
				SetPos(3, 15);
				SetColor(Menu.defColor.bkr, Menu.defColor.word);
				cout << "Файл выбран(Выберите действие)";
				SetPos(3, 16);
				cout << File;
			}
			break;
		case 1:
			if (File) {
				system("cls");
				PrintFile(File);
				cin.get();
				system("cls");
			}
			else {
				SetPos(3, 9);
				cout << "Файл не выбран";
			}
			break;
		case 2: if (File) {
			OpenFile(File);
			system("cls");
		}
				else {
					SetPos(3, 9);
					cout << "Файл не выбран";
				};
				break;
		case 3: return 0;
		}
	}

	cin.get(); cin.get();
	return 0;
}

char *SelectFile(int X, int Y) {
	MainMenu FileM;
	FileM.X = X + 30;
	FileM.Y = Y;
	FileM.select = 0;
	FileM.defColor = { 0,7 };
	FileM.selColor = { 15,0 };
	FileM.WinHeigth = 10;
	FileM.Msize = 0;
	char buf[1000];
	SetPos(4, 1);
	cout << "Введите имя файла или шаблон выбора: ";
	cin.getline(buf, sizeof(buf) / sizeof(buf[0]));
	char **List = EnumFiles(buf, FileM);
	if (!List) {
		return nullptr;
	}
	FileM.SetMMenu(List);
	for (int i = 0; i < FileM.Msize; i++) {
		delete[]List[i];
	}
	delete[]List;

	int result = KeyHandler(FileM);
	if (result == -1) {
		return nullptr;
	}
	int len1 = strlen(buf);
	int len2 = strlen(FileM.MMenu[result]);
	int len3 = 0;
	for (int i = 0; i < len1; i++, len3++) {

		if (buf[i] == '*' || buf[i] == '?')break;
	}
	if (len3 == len1) {
		char *SelFile = new char[len1 + 1];
		for (int i = 0; i < len1; i++) {
			SelFile[i] = buf[i];
		}
		SelFile[len1] = '\0';
		return SelFile;
	}
	else {
		char *SelFile = new char[len3 + len2 + 1];
		for (int i = 0; i < len3; i++) {
			SelFile[i] = buf[i];
		}
		SelFile[len3] = '\0';
		strcat(SelFile, FileM.MMenu[result]);
		return SelFile;
	}
}

char **EnumFiles(char *FileMask, MainMenu &m) {

	_finddata_t s = { 0 };

	intptr_t hFind = _findfirst(FileMask, &s);
	if (hFind > 0) {
		do {
			if (!((s.name[0] == '.') && (s.name[1] == '\0'))) {
				m.Msize++;
			}
		} while (_findnext(hFind, &s) >= 0);
		_findclose(hFind);
	}
	if (!m.Msize) return nullptr;
	char **FileList = new char*[m.Msize];

	hFind = _findfirst(FileMask, &s);

	int i = 0;
	do {
		if (!((s.name[0] == '.') && (s.name[1] == '\0'))) {

			int len = strlen(s.name);
			FileList[i] = new char[len + 1];
			strcpy(FileList[i++], s.name);
		}
	} while (_findnext(hFind, &s) >= 0);
	_findclose(hFind);
	return FileList;
}

void PrintFile(char *FilePath) {

	ifstream file;
	char buf[500];

	file.open(FilePath);

	if (file) {
		while (!file.eof()) {

			file.getline(buf, 500);
			cout << buf << endl;
		}
		file.close();
	}
	else {
		SetPos(3, 10);
		cout << "Ошибка не удалось вывести файл на консоль!";
	}
}

void OpenFile(char *FilePath) {

	int len = strlen(FilePath);
	char *NewFile = new char[len + 13];
	char buf1[] = "start \" \" \"";
	strcpy(NewFile, buf1);
	strcat(NewFile, FilePath);
	NewFile[len + 11] = '"';
	NewFile[len + 12] = '\0';
	system(NewFile);
	delete[]NewFile;
}