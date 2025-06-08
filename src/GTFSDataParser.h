/*************************************************************************
                           GTFSDataParser  -  description
                             -------------------
    début                : $DATE$
    copyright            : (C) $YEAR$ par $AUTHOR$
    e-mail               : $EMAIL$
*************************************************************************/

//---------- Interface de la classe <GTFSDataParser> (fichier GTFSDataParser.h) ----------------
#if ! defined ( GTFSDataParser_H )
#define GTFSDataParser_H

//--------------------------------------------------- Interfaces utilisées
//containers
#include <algorithm>
#include <chrono>
#include <ctime>
#include <list>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstring>
#include <set>

#include "Trajet.h"

typedef class Trajet Trajet;

using namespace std;
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
// struct StopData
// {
//     string stopID;
//     string stopName;
//     float latitude;
//     float longitude;
// };
// typedef struct StopData StopData;

// A single leg between two StopAreas, on one specific date:
// struct Trajet {
//     string tripId;
//     string departureStopArea;  // StopArea:OCExxxxx
//     string arrivalStopArea;    // StopArea:OCEyyyyy
//     string departureDate;      // "YYYYMMDD"
//     int departureTimeSec;      // seconds since midnight (0–86399)
//     string arrivalDate;        // possibly same as departureDate or +1 day
//     int arrivalTimeSec;        // seconds since midnight
// };



//------------------------------------------------------------------------
// Rôle de la classe <GTFSDataParser>
//
//
//------------------------------------------------------------------------

class GTFSDataParser
{
//----------------------------------------------------------------- PUBLIC

public:
//----------------------------------------------------- Méthodes publiques
    // Naïve CSV splitter (no embedded commas)
    static vector<string> splitCSV(const string& line) {
        vector<string> out;
        string cell;
        istringstream ss(line);
        while (getline(ss, cell, ',')) out.push_back(cell);
        return out;
    }

    // Parse "HH:MM:SS" (where HH may exceed 23) → pair<dayOffset, secondsOfDay>
    // e.g. "25:03:00" → {1, 3600*1 + 3*60}
    static pair<int,int> parseHMS(const string& s) {
        int h,m,t;
        char c1,c2;
        istringstream ss(s);
        ss >> h >> c1 >> m >> c2 >> t;
        int dayOff = h / 24;
        int sec = (h % 24)*3600 + m*60 + t;
        return {dayOff, sec};
    }

    void parse(const string dataFolder);

    void printNbTrajetsParGare ();

    void printTrajetsParGare ();

    void printStopAreaInfos ();

    list<Trajet> formaterItineraire (const list<Trajet> & trajetsSuccessifs);

    const unordered_map<string,tuple<string,double,double>> & getStopsAreaInfo();

    const unordered_map<string,set<Trajet>> & getTrajetsParGare();

    string affichageTrajet(const Trajet & trajet);

    void ecrireResultat(ostream & os, const list<Trajet> & itineraire);

    void ecrireGares(ostream & os);


//------------------------------------------------- Surcharge d'opérateurs
    GTFSDataParser & operator = ( const GTFSDataParser & unGTFSDataParser );
    // Mode d'emploi :
    //
    // Contrat :
    //


//-------------------------------------------- Constructeurs - destructeur
    GTFSDataParser ( const GTFSDataParser & unGTFSDataParser );
    // Mode d'emploi (constructeur de copie) :
    //
    // Contrat :
    //

    GTFSDataParser ( );
    // Mode d'emploi :
    //
    // Contrat :
    //

    virtual ~GTFSDataParser ( );
    // Mode d'emploi :
    //
    // Contrat :
    //

//------------------------------------------------------------------ PRIVE

protected:
//----------------------------------------------------- Méthodes protégées

//----------------------------------------------------- Attributs protégés

    unordered_map<string,tuple<string,double,double>> stopsAreaInfo;
    unordered_map<string,string> stopPointToArea;
    unordered_map<string,vector<string>> datesByService;
    unordered_map<string,string> serviceByTrip;
    struct ST { int seq; int dayOffArr,secArr; int dayOffDep,secDep; string stopArea; };
    unordered_map<string,vector<ST>> timesByTrip;
    unordered_map<string,set<Trajet>> trajetsParGare; //structure de données principale pour l'algorithme 

};
typedef class GTFSDataParser GTFSDataParser;

//-------------------------------- Autres définitions dépendantes de <GTFSDataParser>

#endif // GTFSDataParser_H






