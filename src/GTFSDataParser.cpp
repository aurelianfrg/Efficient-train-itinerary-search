/*************************************************************************
                           GTFSDataParser  -  description
                             -------------------
    début                : $DATE$
    copyright            : (C) $YEAR$ par $AUTHOR$
    e-mail               : $EMAIL$
*************************************************************************/

//---------- Réalisation de la classe <GTFSDataParser> (fichier GTFSDataParser.cpp) ------------

//---------------------------------------------------------------- INCLUDE

//-------------------------------------------------------- Include système
using namespace std;
#include <iostream>
#include <assert.h>

//------------------------------------------------------ Include personnel
#include "GTFSDataParser.h"

//------------------------------------------------------------- Constantes

//----------------------------------------------------------------- PUBLIC

//----------------------------------------------------- Méthodes publiques
void GTFSDataParser::parse(const string dataFolder) {
    // 1) Read stops.txt → StopArea info + StopPoint→StopArea map
    // unordered_map<string,tuple<string,double,double>> stopsAreaInfo;
    // unordered_map<string,string> stopPointToArea;

    clog << "reading " << dataFolder + "/stops.txt" << endl;
    ifstream f(dataFolder + "/stops.txt");
    assert(f.is_open());
    string line;
    getline(f,line);  // header
    while(getline(f,line)){
        auto F = splitCSV(line);
        if (F.size() < 8) {
            cout << "Skipping line, only " << F.size() << " fields: " << line << endl; //debug
            continue;
        }
        const auto& stop_id = F[0];
        if (stop_id.rfind("StopArea:",0)==0){
            // StopArea:OCExxxx
            string name = F[1];
            double lat = stod(F[3]);
            double lon = stod(F[4]);
            stopsAreaInfo[stop_id] = {name,lat,lon};
            //cout << stop_id << " " << name << " " << lat << " " << lon << endl;
        } else if (stop_id.rfind("StopPoint:",0)==0){
            // StopPoint:OCETGV... → parent_station in F[8]
            stopPointToArea[ stop_id ] = F[8];
        }
    }

// 2) Read calendar_dates.txt → service_id → set of YYYYMMDD
// unordered_map<string,vector<string>> datesByService;
    clog << "reading " << dataFolder+"/calendar_dates.txt" << endl;
    f = ifstream(dataFolder+"/calendar_dates.txt");
    assert(f.is_open());
    getline(f,line);
    while(getline(f,line)){
        auto F = splitCSV(line);
        if (F.size()<3) continue;
        const auto& svc = F[0];
        const auto& date= F[1];   // already "YYYYMMDD"
        int ex = stoi(F[2]); // 1=added,2=removed
        if (ex==1) datesByService[svc].push_back(date);
        else {
            auto& v = datesByService[svc];
            v.erase(remove(v.begin(),v.end(),date), v.end());
        }
    }

// 3) Read trips.txt → trip_id→service_id
// unordered_map<string,string> serviceByTrip;

    clog << "reading " << dataFolder+"/trips.txt" << endl;
    f = ifstream(dataFolder+"/trips.txt");
    assert(f.is_open());
    getline(f,line);
    while(getline(f,line)){
        auto F = splitCSV(line);
        if (F.size()<3) continue;
        serviceByTrip[ F[2] ] = F[1];
    }

// 4) Read stop_times.txt → group StopTime entries by trip_id
// struct ST { int seq; int dayOffArr,secArr; int dayOffDep,secDep; string stopArea; };
// unordered_map<string,vector<ST>> timesByTrip;
    clog << "reading " << dataFolder +"/stop_times.txt"<< endl;
    f = ifstream(dataFolder+"/stop_times.txt");
    assert(f.is_open());
    getline(f,line);
    while(getline(f,line)){
        auto F = splitCSV(line);
        if (F.size()<5) continue;
        auto tripId = F[0];
        auto[arrOff, arrSec] = parseHMS(F[1]);
        auto[depOff, depSec] = parseHMS(F[2]);
        auto stopPoint     = F[3];
        int seq            = stoi(F[4]);
        // map StopPoint → StopArea
        auto itA = stopPointToArea.find(stopPoint);
        if (itA==stopPointToArea.end()) continue;
        string stopArea = itA->second;

        timesByTrip[tripId].push_back({seq, arrOff,arrSec, depOff,depSec, stopArea});
    }

    // 5) Build trajetsParGare, duplicating per service‐date
    //unordered_map<string, list<Trajet>> trajetsParGare;
    for (auto& [tripId, stops] : timesByTrip){
        // find service
        auto itSvc = serviceByTrip.find(tripId);
        if (itSvc==serviceByTrip.end()) continue;
        const auto& svc = itSvc->second;

        // get all operating dates
        auto itDates = datesByService.find(svc);
        if (itDates==datesByService.end() || itDates->second.empty()) continue;
        auto& dates = itDates->second;

        // sort by stop_sequence
        sort(stops.begin(), stops.end(),
                  [&](auto &a, auto &b){ return a.seq < b.seq; });

        // for each leg in this trip
        for (size_t i = 1; i < stops.size(); ++i){
            const auto &prev = stops[i-1];
            const auto &cur  = stops[i];
            // for each date the service runs
            for (auto &date : dates){
                //cout << "date : " << date;
                // arrival = maybe +1 day
                // compute arrivalDate as YYYYMMDD + cur.dayOffArr
                string arrivalDate;
                time_t timestampArr, timestampDep;
                {
                    tm tmDep = {};
                    istringstream ssDep(date);
                    ssDep >> get_time(&tmDep, "%Y%m%d");
                    if (ssDep.fail()) {
                        cerr << "  ** date parsing failed for “" << date << "”\n";
                    }
                    time_t baseMidnight = mktime(&tmDep);
                    // cout 
                    // << "  date=" << date 
                    // << "  → baseMidnight=" << baseMidnight 
                    // << "  prev.secDep=" << prev.secDep 
                    // << "  sum=" << (baseMidnight + prev.secDep) 
                    // << "\n";
                    timestampDep = baseMidnight + prev.secDep;

                    tm tmArr = tmDep; // copy
                    tmArr.tm_mday += cur.dayOffArr;
                    timestampArr = mktime(&tmArr) + cur.secArr;

                    ostringstream os;
                    os << put_time(&tmArr, "%Y%m%d");
                    arrivalDate = os.str();
                }           
                //cout << tripId << endl;

                Trajet trajet(tripId, prev.stopArea, cur.stopArea, date, prev.secDep, arrivalDate, cur.secArr,timestampDep, timestampArr);
                // store
                trajetsParGare[trajet.departureStopArea].insert(trajet);
                
            }
        }
    }
}

void GTFSDataParser::printNbTrajetsParGare () {
    // --- Example output: count legs per StopArea ---
    for (auto& [area, fl] : trajetsParGare){
        cout << area
                  << "  →  " << distance(fl.begin(), fl.end())
                  << " legs\n";
    }
}

void GTFSDataParser::printTrajetsParGare () {
    for (auto & [stopArea, trajets] : trajetsParGare) {
        cout << stopArea << " " << endl;
        for (const Trajet & trajet : trajets) {
            cout << "  " << trajet << endl;
        }
    }
}

void GTFSDataParser::printStopAreaInfos () {
    cout << "size of stopsAreaInfo : " << stopsAreaInfo.size() << endl;
    
    for (auto& [stopArea, info] : stopsAreaInfo) { // unordered_map<string,tuple<string,double,double>> stopsAreaInfo;
        cout 
        << stopArea
        << " nom : "
        << get<0>(info)
        << " latitude : "
        << get<1>(info)
        << " longitude : "
        << get<2>(info)
        << endl;
    }
}


list<Trajet> GTFSDataParser::formaterItineraire (const list<Trajet> & trajetsSuccessifs) {

    auto it = trajetsSuccessifs.begin();
    if (trajetsSuccessifs.size() <= 1) return trajetsSuccessifs; //s'il y a un seul trajet il y a un seul train donc le formattage est déjà fait

    list<Trajet> resultat;
    Trajet trajetDepartActuel = *it;
    Trajet trajetEquivalent;
    ++it;
    while (it != trajetsSuccessifs.end())
    {
        if (it->tripId != trajetDepartActuel.tripId) //changement de train détecté
        {
            --it; //le dernier segment parcouru par le train d'avant

            string tripId = trajetDepartActuel.tripId;
            string departureStopArea = trajetDepartActuel.departureStopArea;  
            string arrivalStopArea = it->arrivalStopArea;    
            string departureDate = trajetDepartActuel.departureDate;      
            int departureTimeSec = trajetDepartActuel.departureTimeSec;     
            string arrivalDate = it->arrivalDate;      
            int arrivalTimeSec = it->arrivalTimeSec;      
            time_t timestampDep = trajetDepartActuel.timestampDep;
            time_t timestampArr = it->timestampArr;      

            trajetEquivalent = Trajet(tripId,departureStopArea,arrivalStopArea,departureDate,departureTimeSec,arrivalDate,arrivalTimeSec,timestampDep,timestampArr);
            resultat.push_back(trajetEquivalent);

            ++it; //on retourne au premier segment du train suivant
            trajetDepartActuel = *it;
        }
        ++it;
    }
    //en sortie de boucle il manque à ajouter le dernier trajet qui était en cours
    --it; //le dernier segment parcouru par le train d'avant
    string tripId = trajetDepartActuel.tripId;
    string departureStopArea = trajetDepartActuel.departureStopArea;  
    string arrivalStopArea = it->arrivalStopArea;    
    string departureDate = trajetDepartActuel.departureDate;      
    int departureTimeSec = trajetDepartActuel.departureTimeSec;     
    string arrivalDate = it->arrivalDate;      
    int arrivalTimeSec = it->arrivalTimeSec;      
    time_t timestampDep = trajetDepartActuel.timestampDep;
    time_t timestampArr = it->timestampArr;      

    trajetEquivalent = Trajet(tripId,departureStopArea,arrivalStopArea,departureDate,departureTimeSec,arrivalDate,arrivalTimeSec,timestampDep,timestampArr);
    resultat.push_back(trajetEquivalent);

    return resultat;
}


const unordered_map<string,tuple<string,double,double>> & GTFSDataParser::getStopsAreaInfo() {
    return this->stopsAreaInfo;
}

const unordered_map<string,set<Trajet>> & GTFSDataParser::getTrajetsParGare() {
    return this->trajetsParGare;
}

string GTFSDataParser::affichageTrajet(const Trajet & trajet) {
    stringstream ores;
    ores << "Trajet : "
    << trajet.tripId
    << " de "
    << get<0>(this->stopsAreaInfo.at(trajet.departureStopArea))
    << " à "
    << get<0>(this->stopsAreaInfo.at(trajet.arrivalStopArea))
    << " "
    << put_time(localtime(&(trajet.timestampDep)), "%Y-%m-%d %H:%M:%S")
    << " --> "
    << put_time(localtime(&(trajet.timestampArr)), "%Y-%m-%d %H:%M:%S")
    ;
    return ores.str();
}

void GTFSDataParser::ecrireResultat(ostream & os, const list<Trajet> & itineraire) {
    os << "Itinéraire trouvé : " << endl;
    for (Trajet trajet : itineraire) {
        os << this->affichageTrajet(trajet) << endl;
    }
}

void GTFSDataParser::ecrireGares(ostream & os) {
    //unordered_map<string,tuple<string,double,double>> stopsAreaInfo
    for (auto & [stopArea, infos] : stopsAreaInfo) {
        os << stopArea << " : " << get<0>(infos) << endl;
    }
}


//------------------------------------------------- Surcharge d'opérateurs

//-------------------------------------------- Constructeurs - destructeur
GTFSDataParser::GTFSDataParser ( const GTFSDataParser & unGTFSDataParser )
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au constructeur de copie de <GTFSDataParser>" << endl;
#endif
} //----- Fin de GTFSDataParser (constructeur de copie)


GTFSDataParser::GTFSDataParser ()
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au constructeur de <GTFSDataParser>" << endl;
#endif

} //----- Fin de GTFSDataParser


GTFSDataParser::~GTFSDataParser ( )
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au destructeur de <GTFSDataParser>" << endl;
#endif
} //----- Fin de ~GTFSDataParser


//------------------------------------------------------------------ PRIVE

//----------------------------------------------------- Méthodes protégées

