#include <windows.h>
#include <stdio.h>
#include "NIDAQmx.h"
#include <iostream>
#include <fstream>
#include <array>
#include <cmath>
using namespace std;

//FT43238 ATI

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);

bool ButtonState;
bool SaveState;
bool GetState;
bool GetVector;

double bias[6];

int	optionInput;

int setNumber;
int SampleNb = 0;

double SaveMatrix[1500][6][1];//every 300 samples is one set assume 5 sets

double CalibrationMatrix[6][6] = {
	{0.04941, -0.09207, -0.02743, -3.43541,   0.07532,   3.40960}, //Fx
	{-0.01404,   3.99394, -0.01794, -2.06329, -0.05223, -1.88546}, //Fy
	{3.77291,   0.11830,   3.89295, -0.04394,   3.71715,   0.00160}, //Fz
	{-0.01371,  46.66380,  20.90754, -24.34604, -21.82519, -22.04622}, //Tx
	{-24.99932,   0.20581,  12.83938,  39.98184,  10.73087, -39.75729}, //Ty
	{-0.07259,  15.34146,   0.04813,  15.30542,   0.48561,  14.99242} }; //Tz

double VectorReading(double x, double y, double z) {
	double resultingVector;
	resultingVector = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	return resultingVector;
}

void GetBias(bool ButtonState, double R1, double R2, double R3, double R4, double R5, double R6) {
	if (ButtonState == true) {
		bias[0] = R1;
		bias[1] = R2;
		bias[2] = R3;
		bias[3] = R4;
		bias[4] = R5;
		bias[5] = R6;

		cout << "Bias done" << endl;
	}
}

void MatrixMultiplication(bool runState, bool SaveState, bool vectorState, int samplenb, double G0, double G1, double G2, double G3, double G4, double G5) {
	double ResultMatrix[1][6];
	double InitialMatrix[6][1] = { {G0 - bias[0]}, {G1 - bias[1]}, {G2 - bias[2]}, {G3 - bias[3]}, {G4 - bias[4]}, {G5 - bias[5]} };
	double VectorCoord[1][3];

	if (runState == true) {
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 1; j++) {
				ResultMatrix[i][j] = 0;
				for (int k = 0; k < 6; k++) {
					ResultMatrix[i][j] += CalibrationMatrix[i][k] * InitialMatrix[k][j];
				}
				if (SaveState == true) {
					SaveMatrix[samplenb][i][j] = ResultMatrix[i][j];
				}
				if (vectorState == true) {
					if (i < 3) {
						VectorCoord[i][j] = ResultMatrix[i][j];
					}
				}
			}
		}
        if (vectorState == true) {
		    cout << VectorReading(VectorCoord[0][0], VectorCoord[0][1], VectorCoord[0][2]);
        }
	}
}

void Menu() {
	cout << "Choose an option:" << endl;
	cout << "1 - Bias" << endl;
	cout << "2 - Test for 300 samples" << endl;
	cout << "3 - Exit" << endl;
	cout << "4 - Print out all sets" << endl;
	cin >> optionInput;
	getchar();
}

int main(void)
{
	int32       error = 0;
	TaskHandle  taskHandle = 0;
	char        errBuff[2048] = { '\0' };

	Menu();

	while (optionInput != 3) {
		// DAQmx Configure Code
		DAQmxErrChk(DAQmxCreateTask("", &taskHandle));

		DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0:5", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));

		if (optionInput == 1) {

			ButtonState = true;
			SaveState = false;
			GetState = false;
			GetVector = false;

			DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 1000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 100)); //DAQmx_Val_ContSamps
			DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandle, "/Dev1/PFI0", DAQmx_Val_Rising));

			DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 100, 0, EveryNCallback, NULL));
			DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

			DAQmxErrChk(DAQmxStartTask(taskHandle));

			getchar();

			//constant reading
			GetVector = true;
			DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1000));

			DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1000, 0, EveryNCallback, NULL));
			DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

			DAQmxErrChk(DAQmxStartTask(taskHandle));

			printf("Acquiring samples continuously. Press Enter to interrupt\n");
			getchar();

			Menu();

		} else if (optionInput == 2) {
			ButtonState = false;
			GetState = true;
			SaveState = true;
			GetVector = false;

			DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 3000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 30000)); //DAQmx_Val_ContSamps
			DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandle, "/Dev1/PFI0", DAQmx_Val_Rising));

		    DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 100, 0, EveryNCallback, NULL));
		    DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

			DAQmxErrChk(DAQmxStartTask(taskHandle));

			setNumber += 1;
			cout << "Please wait for 10 seconds" << endl;
			getchar();

			Menu();

		} else if (optionInput == 4) {
			ButtonState = false;
			int	setNbOut = 1;
			for (int i = 0; i != SampleNb; ++i) {
				cout << "Set number:" << setNbOut << "  " << "Sample number:" << i << "  ";
				for (int x = 0; x != 3; ++x) {		
					for (int y = 0; y != 1; ++y) {
						cout << SaveMatrix[i][x][y] << ", ";
					}
				}
				cout << endl;
				if ((i % 300 == 0) && (i !=0)) {
					setNbOut += 1;
				}
			}
			Menu();
		}
	
	}
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}


int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData)
{
	int32       error = 0;
	char        errBuff[2048] = { '\0' };
	static int  totalRead = 0;
	int32       read = 0;
	float64     data[100000];

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	//int32 DAQmxReadAnalogF64 (TaskHandle taskHandle, int32 numSampsPerChan, float64 timeout, bool32 fillMode, float64 readArray[], uInt32 arraySizeInSamps, int32 *sampsPerChanRead, bool32 *reserved);
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 10, 10.0, DAQmx_Val_GroupByChannel, data, 60, &read, NULL));
	if (read > 0) {
		//cout << endl;
		//printf("Total %d. Value1: %f. Value2: %f. Value3: %f. Value4: %f. Value5: %f. Value6 %f.\r", (int)(totalRead += read), data[5], data[15], data[25], data[35], data[45], data[55]);
		//cout << endl;
		GetBias(ButtonState, data[5], data[15], data[25], data[35], data[45], data[55]);
		MatrixMultiplication(GetState, SaveState, GetVector, SampleNb, data[5], data[15], data[25], data[35], data[45], data[55]);
		SampleNb += 1;
		fflush(stdout);
	}

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}


int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData)
{
	int32   error = 0;
	char    errBuff[2048] = { '\0' };

	// Check to see if an error stopped the task.
	DAQmxErrChk(status);
	DAQmxStopTask(taskHandle);
	DAQmxClearTask(taskHandle);

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}
