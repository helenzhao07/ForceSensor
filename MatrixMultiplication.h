#pragma once
#include <stdio.h>
#include <iostream>
using namespace std;

double CalibrationMatrix[6][6] = {
	{0.04941, - 0.09207, - 0.02743, - 3.43541,   0.07532,   3.40960}, //Fx
	{-0.01404,   3.99394, - 0.01794, - 2.06329, - 0.05223, - 1.88546}, //Fy
	{3.77291,   0.11830,   3.89295, - 0.04394,   3.71715,   0.00160}, //Fz
	{-0.01371,  46.66380,  20.90754, - 24.34604, - 21.82519, - 22.04622}, //Tx
	{-24.99932,   0.20581,  12.83938,  39.98184,  10.73087, - 39.75729}, //Ty
	{-0.07259,  15.34146,   0.04813,  15.30542,   0.48561,  14.99242} }; //Tz


//0.008604. Value2: -0.047586. Value3: 0.059207. Value4: 0.008604. Value5: -0.047258. Value6 0.058550
//just need to bias it once

//NEED TO BIAS THEN READ, FOR READING, IF RUNNING AT X HZ (X SAMPLES PER SECOND), be able to interact with the bias, be able
//to control when to bias

//bool PHY_BUTTON_STATE; //if butt state high -->bias
//bool BUTTON_STORE; //start storing

//rate of 30Hz , 30 samples --> 333 samples
/*


*/

double bias[6];


void GetRawData(int count, double R1, double R2, double R3, double R4, double R5, double R6) {
	

}

void GetBias(bool ButtonState, double R1, double R2, double R3, double R4, double R5, double R6) {
	if (ButtonState == true) {
		bias[0] = R1;
		bias[1] = R2;
		bias[2] = R3;
		bias[3] = R4;
		bias[4] = R5;
		bias[5] = R6;
	}
}


void MatrixMultiplication(double G0, double G1, double G2, double G3, double G4, double G5) {
	double ResultMatrix[1][6];
	double InitialMatrix[6][1] = { {G0- bias[0]}, {G1 - bias[1]}, {G2 - bias[2]}, {G3 - bias[3]}, {G4 - bias[4]}, {G5 - bias[5]} };

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 1; j++) {
			ResultMatrix[i][j] = 0;

			for (int k = 0; k < 6; k++) {
				ResultMatrix[i][j] += CalibrationMatrix[i][k] * InitialMatrix[k][j];
			}

			//cout << ResultMatrix[i][j] << "\t";
		}

		//cout << endl;
	}
	 
	 //return ForceData;
 }