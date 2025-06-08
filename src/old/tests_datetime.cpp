#include <ctime>
#include <string>
#include <iostream>

using namespace std;


int main(int argc, char** argv) { //args must be: stoppointDepart stoppointArrivee datetimeDepart
	
//---------traitement de l'input---------- // TODO

	string depStopArea = "StopArea:OCE87192039";

	string arrStopArea = "StopArea:OCE87481705";
	float objLatitude = 0.0; //à fixer selon la gare objectif, utilisés par les trajets pour le calcul de l'heuristique
	float objLongitude = 0.0; 

	struct tm depDatetime;
	depDatetime.tm_year = 2025 - 1900;
	depDatetime.tm_mon = 1 - 1; //janvier
	depDatetime.tm_mday = 22; // 22 janvier 2025
	depDatetime.tm_hour = 12;
	depDatetime.tm_min = 0;
	depDatetime.tm_sec = 0;
	depDatetime.tm_isdst = -1; //computer timezone settings
	time_t depTimestamp = mktime(&depDatetime);

	cout << depTimestamp << endl;

	time_t timestamptest = 1737551520;
	struct tm * datetimetest = localtime(&timestamptest);
	cout << datetimetest->tm_mday << "/" << datetimetest->tm_mon + 1 << "/" << datetimetest->tm_year +1900 << " " << datetimetest->tm_hour << "h" << datetimetest->tm_min << endl;

}