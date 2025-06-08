/*************************************************************************
                           Trajet  -  description
                             -------------------
    début                : $DATE$
    copyright            : (C) $YEAR$ par $AUTHOR$
    e-mail               : $EMAIL$
*************************************************************************/

//---------- Réalisation de la classe <Trajet> (fichier Trajet.cpp) ------------

//---------------------------------------------------------------- INCLUDE

//-------------------------------------------------------- Include système
using namespace std;
#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

//------------------------------------------------------ Include personnel
#include "Trajet.h"

//------------------------------------------------------------- Constantes
constexpr double EARTH_RADIUS_M = 6371000.0; // Earth's radius in meters

//----------------------------------------------------------------- PUBLIC

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);

    lat1 = toRadians(lat1);
    lat2 = toRadians(lat2);

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS_M * c;
}

//----------------------------------------------------- Méthodes publiques

void Trajet::setTimestampArr( time_t timestamp ) {
    this->timestampArr = timestamp;
}

void Trajet::setScoreAvecHeuristique( const double & latitudeObjectif, const double & longitudeObjectif, const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo ) {
    time_t scoreHeuristique;

// heuristique basée sur la distance à l'arrivée, sachant qu'un TGV va à max 300 km/h
    double distanceToArrival = haversineDistance(latitudeObjectif, longitudeObjectif, get<1>(stopsAreaInfo.at(this->arrivalStopArea)), get<2>(stopsAreaInfo.at(this->arrivalStopArea)));
    //estimation optimiste du temps de parcours restant en secondes
    scoreHeuristique = (time_t) (distanceToArrival/(300.0*3.6));

    this->score = timestampArr + scoreHeuristique;
}


void Trajet::setScoreSansHeuristique( ) {
    this->score = this->timestampArr;
}

const string & Trajet::getArrStopArea ( ) const {
    return this->arrivalStopArea;
}

const string & Trajet::getDepStopArea ( ) const {
    return this->departureStopArea;
}

const time_t & Trajet::getTimestampArr( ) const {
    return this->timestampArr;
}

const time_t & Trajet::getTimestampDep( ) const {
    return this->timestampDep;
}

//------------------------------------------------- Surcharge d'opérateurs

// Trajet & Trajet::operator = (const Trajet & unTrajet) {
// #ifdef MAP
//     cout << "Appel à l'opérateur = de <Trajet>" << endl;
// #endif
//     this->tripId = unTrajet.tripId;
//     this->departureStopArea = unTrajet.departureStopArea;  // StopArea:OCExxxxx
//     this->arrivalStopArea = unTrajet.arrivalStopArea;    // StopArea:OCEyyyyy
//     this->departureDate = unTrajet.departureDate;      // "YYYYMMDD"
//     this->departureTimeSec = unTrajet.departureTimeSec;      // seconds since midnight (0–86399)
//     this->arrivalDate = unTrajet.arrivalDate;        // possibly same as departureDate or +1 day
//     this->arrivalTimeSec = unTrajet.arrivalTimeSec;        // seconds since midnight
//     this->timestampArr = unTrajet.timestampArr;
//     this->timestampDep = unTrajet.timestampDep;
//     this->score = unTrajet.score;
//     // this->arrLatitude = unTrajet.arrLatitude;
//     // this->arrLongitude = unTrajet.arrLongitude;
//     // this->depLatitude = unTrajet.depLatitude;
//     // this->depLongitude = unTrajet.depLongitude;
//     return *this;
// }

bool operator < (const Trajet & trajet1, const Trajet & trajet2) {
#ifdef MAP
    cout << "Appel à l'opérateur < de <Trajet>" << endl;
#endif
    return (trajet1.timestampArr < trajet2.timestampArr);
}

bool operator == (const Trajet & trajet1, const Trajet & trajet2) {
#ifdef MAP
    cout << "Appel à l'opérateur == de <Trajet>" << endl;
#endif
    return ( (trajet1.timestampArr == trajet2.timestampArr) && (trajet1.tripId == trajet2.tripId) );
}

//-------------------------------------------- Constructeurs - destructeur
Trajet::Trajet ( const Trajet & unTrajet )
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au constructeur de copie de <Trajet>" << endl;
#endif
    this->tripId = unTrajet.tripId;
    this->departureStopArea = unTrajet.departureStopArea;  // StopArea:OCExxxxx
    this->arrivalStopArea = unTrajet.arrivalStopArea;    // StopArea:OCEyyyyy
    this->departureDate = unTrajet.departureDate;      // "YYYYMMDD"
    this->departureTimeSec = unTrajet.departureTimeSec;      // seconds since midnight (0–86399)
    this->arrivalDate = unTrajet.arrivalDate;        // possibly same as departureDate or +1 day
    this->arrivalTimeSec = unTrajet.arrivalTimeSec;        // seconds since midnight
    this->timestampDep = unTrajet.timestampDep;
    this->timestampArr = unTrajet.timestampArr;
    this->score = unTrajet.score;
    // this->arrLatitude = unTrajet.arrLatitude;
    // this->arrLongitude = unTrajet.arrLongitude;
    // this->depLatitude = unTrajet.depLatitude;
    // this->depLongitude = unTrajet.depLongitude;

} //----- Fin de Trajet (constructeur de copie)


Trajet::Trajet ( string untripId, string undepartureStopArea, string unarrivalStopArea, string undepartureDate, int undepartureTimeSec, string unarrivalDate, int unarrivalTimeSec, time_t unTimestampDep, time_t unTimestampArr)
{
#ifdef MAP
    cout << "Appel au constructeur de <Trajet>" << tripId << endl;
#endif
    this->tripId = untripId;
    this->departureStopArea = undepartureStopArea;  // StopArea:OCExxxxx
    this->arrivalStopArea = unarrivalStopArea;    // StopArea:OCEyyyyy
    this->departureDate = undepartureDate;      // "YYYYMMDD"
    this->departureTimeSec = undepartureTimeSec;      // seconds since midnight (0–86399)
    this->arrivalDate = unarrivalDate;        // possibly same as departureDate or +1 day
    this->arrivalTimeSec = unarrivalTimeSec;        // seconds since midnight
    this->timestampArr = unTimestampArr;
    this->timestampDep = unTimestampDep;
    // this->depLatitude = unDepLatitude;
    // this->depLongitude = unDepLongitude;
    // this->arrLatitude = unArrLatitude;
    // this->arrLongitude = unArrLongitude;
} //----- Fin de Trajet

Trajet::Trajet () {
#ifdef MAP
    cout << "Appel au constructeur de <Trajet>" << endl;
#endif
};

Trajet::~Trajet ( )
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au destructeur de <Trajet>" << endl;
#endif
} //----- Fin de ~Trajet


ostream & operator << (ostream & os, const Trajet & unTrajet) {
    os << "Trajet " 
    << unTrajet.tripId 
    << " : " 
    << unTrajet.departureStopArea 
    << " --> " 
    << unTrajet.arrivalStopArea 
    << " " 
    // << unTrajet.departureDate 
    // << " (" 
    // << unTrajet.departureTimeSec/3600 
    // << "h";
    // if ((unTrajet.departureTimeSec%3600)/60 < 10) os << "0";
    // os << (unTrajet.departureTimeSec%3600)/60 
    // << " --> " 
    // << unTrajet.arrivalTimeSec/3600 
    // << "h";
    // if ((unTrajet.arrivalTimeSec%3600)/60 < 10) os << "0";
    // os << (unTrajet.arrivalTimeSec%3600)/60 
    // << "), timestampDep : " 
    // << unTrajet.timestampDep
    // << ", timestampArr : "
    // << unTrajet.timestampArr
    << put_time(localtime(&(unTrajet.timestampDep)), "%Y-%m-%d %H:%M:%S")
    << " --> "
    << put_time(localtime(&(unTrajet.timestampArr)), "%Y-%m-%d %H:%M:%S")
    << ", score : "
    << unTrajet.score;
    return os;
}


bool ComparaisonHeuristiqueTrajet::operator () (const Trajet & trajet1, const Trajet & trajet2) {
    // Une priority_queue fonctionne par priorité maximale : pour avoir en 1e le Trajet ayant le score le plus faible, l'opérateur de comparaison doit être greater au lieu de less
    if (trajet1.score == 0) {
        cerr << "erreur sur le trajet : " << trajet1 << endl;
        throw domain_error("Le score du trajet précédent n'a pas été calculé préalablement à son utilisation dans la file de priorité.");
    } 
    if (trajet2.score == 0) {
        cerr << "erreur sur le trajet : " << trajet2 << endl;
        throw domain_error("Le score du trajet précédent n'a pas été calculé préalablement à son utilisation dans la file de priorité.");
    } 
    return ( trajet1.score > trajet2.score );
}