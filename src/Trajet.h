/*************************************************************************
                           Trajet  -  description
                             -------------------
    début                : $DATE$
    copyright            : (C) $YEAR$ par $AUTHOR$
    e-mail               : $EMAIL$
*************************************************************************/

//---------- Interface de la classe <Trajet> (fichier Trajet.h) ----------------
#if ! defined ( Trajet_H )
#define Trajet_H

//--------------------------------------------------- Interfaces utilisées
#include <ctime>
#include <string>

#include "GTFSDataParser.h"

using namespace std;
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//------------------------------------------------------------------------
// Rôle de la classe <Trajet>
//
//
//------------------------------------------------------------------------

class Trajet
{
//----------------------------------------------------------------- PUBLIC
    
public:

    friend class GTFSDataParser;
    friend struct ComparaisonHeuristiqueTrajet;
//----------------------------------------------------- Méthodes publiques

    void setTimestampArr( time_t timestamp );

    void setScoreAvecHeuristique( const double & latitudeObjectif, const double & longitudeObjectif, const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo  );

    void setScoreSansHeuristique( );

    const string & getArrStopArea ( ) const;

    const string & getDepStopArea ( ) const;

    const time_t & getTimestampArr( ) const;

    const time_t & getTimestampDep( ) const;

//------------------------------------------------- Surcharge d'opérateurs

    friend ostream & operator << (ostream & os, const Trajet & unTrajet);

    //Trajet & operator = (const Trajet & unTrajet); 

    friend bool operator < (const Trajet & trajet1, const Trajet & trajet2); 

    friend bool operator == (const Trajet & trajet1, const Trajet & trajet2); 

//-------------------------------------------- Constructeurs - destructeur
    Trajet ( const Trajet & unTrajet );
    // Mode d'emploi (constructeur de copie) :
    //
    // Contrat :
    //

    Trajet();

    Trajet ( string untripId,string undepartureStopArea,string unarrivalStopArea,string undepartureDate,int undepartureTimeSec,string unarrivalDate,int unarrivalTimeSec, time_t unTimestampDep, time_t unTimestampArr);
    // Mode d'emploi :
    //
    // Contrat :
    //

    virtual ~Trajet ( );
    // Mode d'emploi :
    //
    // Contrat :
    //


protected:

    string tripId;
    string departureStopArea;  // StopArea:OCExxxxx
    string arrivalStopArea;    // StopArea:OCEyyyyy
    string departureDate;      // "YYYYMMDD"
    int departureTimeSec;      // seconds since midnight (0–86399)
    string arrivalDate;        // possibly same as departureDate or +1 day
    int arrivalTimeSec;        // seconds since midnight
    time_t timestampDep;
    time_t timestampArr;       // temps en format entier pour le calcul des priorités
    time_t score;              // score pour l'ordre de la file de priorité prenant en compte l'heuristique
    // double depLatitude;
    // double depLongitude;
    // double arrLatitude;
    // double arrLongitude;

};
typedef class Trajet Trajet;

//-------------------------------- Autres définitions dépendantes de <Trajet>

//struct pour la fonction de comparaison de la priority_queue basée sur le score croissant
struct ComparaisonHeuristiqueTrajet {
    bool operator () (const Trajet & trajet1, const Trajet & trajet2);
};

#endif // Trajet_H

