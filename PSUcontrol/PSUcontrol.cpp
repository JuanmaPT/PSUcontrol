/**
*******************************************************************
* @file PSUcontrol.cpp
* @brief Basic SET/RESET/READ functionalities PSU Keysight 66311B
* @author DEKRA - Juan Manuel Peña juanmaptcg@gmail.com
* @version 1.0
* @date July 2019
******************************************************************/
/* IMPORTANT NOTES :
	-Requires GPIB-USB adapter
	-The power supply GPIB addres must agree the direction assigned
	 in the Keysight Connection Expert software (more details in PSU_config_guide.txt)
	-Tested using Visual Studio 2017 (compiler set-up guide included in InstruccionesdeCompilación.txt)
	*/

#include "stdafx.h"
#include "visa.h"
#include <stdio.h>
#include <windows.h>
#include <typeinfo>
#include <iostream>
#include <string>
#include <conio.h>

#define _WIN32_WINNT 0x0500

using namespace std;

/* Global Variables*/
char buf[256] = { 0 }; //Buffer for receivng instrument information.

/**
* Sets the power supply DC Voltage in Volts[V].
*
* @inst_channel Intrument communication channel
* @newVoltage voltage to which the PUS is adjusted [Volts]
* @return the readed voltage after changing it [Volts]
*/
char* SetVoltage(ViSession inst_channel,string newVoltage)
{
	string sum = ":SOURce:VOLTage:LEVel:IMMediate:AMPLitude " + newVoltage + "\n";
	const char *instrunction = sum.c_str();
	viPrintf(inst_channel, instrunction);
	viPrintf(inst_channel, "MEASure:SCALar:VOLTage:DC?\n");
	viScanf(inst_channel, "%t", buf);
	return buf;
}

/**
* Sets the power supply DC Currents in Amperes[A].
*
* @inst_channel Intrument communication channel
* @newCurrent current to which the PUS is adjusted [A]
* @return the readed current after changing it [A]
*/
char* SetCurrent(ViSession inst_channel, string newVoltage)
{
	string sum = "SOURce:CURRent:LEVel:IMMediate:AMPLitude " + newVoltage + "\n";
	const char *instrunction = sum.c_str();
	viPrintf(inst_channel, instrunction);
	viPrintf(inst_channel, "MEASure:SCALar:CURRent:DC?\n");
	viScanf(inst_channel, "%t", buf);
	return buf;
}

/**
* Reads the power supply DC Voltage in Volts[V].
*
* @inst_channel Intrument communication channel
* @return the voltage[Volts]
*/
char* ObtainVoltage(ViSession inst_channel)
{
	viPrintf(inst_channel, "MEASure:SCALar:VOLTage:DC?\n");
	viScanf(inst_channel, "%t", buf);
	buf[strlen(buf) - 1] = '\0';
	return buf;
}

/**
* Reads the power supply DC current in Amperes[A].
*
* @inst_channel Intrument communication channel
* @return the current meassure[A]
*/
char* ObtainCurrent(ViSession inst_channel)
{
	viPrintf(inst_channel, "MEASure:SCALar:CURRent:DC?\n");
	viScanf(inst_channel, "%t", buf);
	buf[strlen(buf) - 1] = '\0';
	return buf;
}


int main() {

	/*-----------Console size adjustment-------*/
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	MoveWindow(console, r.left, r.top, 500, 300, TRUE); // 500 width, 300 height

	printf("Welcome to PowerSu Controller v1.0\n");

	ViStatus status;				 //For checking errors
	ViSession defaultRM;			 //Communication channel
	ViSession instr;				 //Communication channel
	ViChar rsrcName[VI_FIND_BUFLEN]; //Serial resource name
	ViChar intfDesc[VI_FIND_BUFLEN]; //Port binding description
	ViUInt32 retCount;				 //To hold number of resources
	ViFindList flist;				 //To hold list of resources
	char identifier[256] = { 0 };    //Stores instrument identifier
	int SelectedDevice;
	bool choosen = false;
	string input;
	bool outout;

	/* Begin by initializing the system */
	status = viOpenDefaultRM(&defaultRM);
	if (status < VI_SUCCESS) {
		/* Error Initializing VISA...exiting */
		printf("[Error] when trying to initialize VISA...exiting\n");
		return -1;
	}
	//==============================================================================================
	//======TO DO:  CHANGE THE ADDRESS IN THE SECOND ARGUMENT OF VIOPEN FOR OPENING THE CORRECT PORT
	//==============================================================================================
	status = viOpen(defaultRM, "GPIB0::5::INSTR", VI_NULL, VI_NULL, &instr);
	if (status < VI_SUCCESS)
	{
		printf("Could not open %s, status = 0x%08lX\n", rsrcName, status);
	}
	
	/* Initialize device 
	/* Send an *IDN? string to the device */
	viPrintf(instr, "*IDN?\n");
	/* Read results and store identifier string */
	viScanf(instr, "%t", identifier);

	/* ========  MENU INTERFACE  ============*/
	bool exit = false;
	string NewVoltage, NewCurrent;
	int output_state = 2;
	char option;

	while (!exit)
	{
		system("cls");
		printf("Connected to %s \n", identifier);
		SetConsoleTextAttribute(hConsole, 12); //Change color to red
		printf("========================================================\n");
		printf("||  Voltage : %s V ||  ", ObtainVoltage(instr));
		printf("Current : %s A||\n", ObtainCurrent(instr));
		printf("========================================================\n");
		SetConsoleTextAttribute(hConsole, 15);
		printf("\n");
		if (output_state == 2) {
			printf("	Output in ");
			SetConsoleTextAttribute(hConsole, 14);
			printf("UNKNOW");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" state\n");
		}
		else if (output_state == 1) {
			printf("	Output in ");
			SetConsoleTextAttribute(hConsole, 10);
			printf("ON");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" state\n");
		}
		else if (output_state == 0) {
			printf("	Output in ");
			SetConsoleTextAttribute(hConsole, 12);
			printf("OFF");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" state\n");
		}
		printf("\n");
		printf("  r   -  Refresh values\n");
		printf("  v   -  Change output Voltage\n");
		printf("  c   -  Change output Current\n");
		printf("  0   -  Output OFF\n");
		printf("  1   -  Output ON\n");
		printf("  esc -  Exit the program\n");
		option =_getch();

		switch (option)
		{
		case 'r':
			cout << "REFRESHING\n";
			break;

		case 'v':
			cout << "Introduce voltage: ";
			cin >> NewVoltage;
			system("cls");
			printf("Voltage set to : %s\n", SetVoltage(instr, NewVoltage));
			Sleep(2000);
			break;

		case 'c':
			cout << "Introduce current: ";
			cin >> NewCurrent;
			system("cls");
			printf("Current set to : %s\n", SetCurrent(instr, NewCurrent));
			Sleep(2000);
			break;

		case '0':
			viPrintf(instr, "OUTP OFF\n");
			output_state = 0;
			break;

		case '1':
			viPrintf(instr, "OUTP ON\n");
			output_state = 1;
			break;

		case (char)27: //ESC key
			exit = true;
			break;

		default:
			cout << "Bad key! Please try again.\n";
			Sleep(2000);
		}

	}
	/* Close session */
	viClose(instr);
	viClose(defaultRM);
	return 0;
}


