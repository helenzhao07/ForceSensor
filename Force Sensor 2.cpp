#include <stdio.h>
#include "NIDAQmx.h"
#include "MatrixMultiplication.h"

//FT43238 ATI

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);

//being able to change rate (look at Hz) --> input
//access data --> output
//

int main(void)
{
	int32       error = 0;
	TaskHandle  taskHandle = 0;
	char        errBuff[2048] = { '\0' };
	char		BiasStatus = false;
	char		BiasINStatus;
	bool		RunningStatus = true;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0:5", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));



	// ai0:5 used to read channels from channel 0 to 5 (6 channels total) needed for reading ATI Nano 17 ;
	DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1000));

	//looping for bias
	while (RunningStatus) {
		cout << "Do you want to bias? (Y/N)";
		cin >> BiasINStatus;
		if (BiasINStatus == "Y") {
			BiasStatus = true;
		}
		DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1000, 0, EveryNCallback, NULL));
		DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

		/*********************************************/
		// DAQmx Start Code
		/*********************************************/
		DAQmxErrChk(DAQmxStartTask(taskHandle));

		printf("Acquiring samples continuously. Press Enter to interrupt\n");
		getchar();
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
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1000, 10.0, DAQmx_Val_GroupByScanNumber, data, 6000, &read, NULL));
	if (read > 0) {
		printf("Total %d. Value1: %f. Value2: %f. Value3: %f. Value4: %f. Value5: %f. Value6 %f.\r", (int)(totalRead += read), data[500], data[1500], data[2500], data[3500], data[4500], data[5500]);
		GetBias(true, data[500], data[1500], data[2500], data[3500], data[4500], data[5500]);
		MatrixMultiplication(data[500], data[1500], data[2500], data[3500], data[4500], data[5500]);
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

Error:
	if (DAQmxFailed(error)) {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}