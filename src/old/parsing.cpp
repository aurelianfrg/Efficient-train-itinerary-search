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

using namespace std;

const char* dataFolder = "data";
const int pathLentgh = strlen(dataFolder) + 20;
char * filepath = new char [pathLentgh];

// ---- Data structures ----

// A single leg between two StopAreas, on one specific date:
struct Trajet {
    string tripId;
    string departureStopArea;  // StopArea:OCExxxxx
    string arrivalStopArea;    // StopArea:OCEyyyyy
    string departureDate;      // "YYYYMMDD"
    int departureTimeSec;           // seconds since midnight (0–86399)
    string arrivalDate;        // possibly same as departureDate or +1 day
    int arrivalTimeSec;             // seconds since midnight
};

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

int main(){
    // 1) Read stops.txt → StopArea info + StopPoint→StopArea map
    unordered_map<string,tuple<string,double,double>> stopsAreaInfo;
    unordered_map<string,string> stopPointToArea;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/stops.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
        getline(f,line);  // header
        while(getline(f,line)){
            auto F = splitCSV(line);
            if (F.size() < 9) continue;
            const auto& stop_id = F[0];
            if (stop_id.rfind("StopArea:",0)==0){
                // StopArea:OCExxxx
                string name = F[1];
                double lat = stod(F[3]);
                double lon = stod(F[4]);
                stopsAreaInfo[stop_id] = {name,lat,lon};
                cout << stop_id << " " << name << " " << lat << " " << lon << endl; //debugging line
            } else if (stop_id.rfind("StopPoint:",0)==0){
                // StopPoint:OCETGV... → parent_station in F[8]
                stopPointToArea[ stop_id ] = F[8];
            }
        }
    }

    // 2) Read calendar_dates.txt → service_id → set of YYYYMMDD
    unordered_map<string,vector<string>> datesByService;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/calendar_dates.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
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
    }

    // 3) Read trips.txt → trip_id→service_id
    unordered_map<string,string> serviceByTrip;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/trips.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
        getline(f,line);
        while(getline(f,line)){
            auto F = splitCSV(line);
            if (F.size()<3) continue;
            serviceByTrip[ F[2] ] = F[1];
        }
    }

    // 4) Read stop_times.txt → group StopTime entries by trip_id
    struct ST { int seq; int dayOffArr,secArr; int dayOffDep,secDep; string stopArea; };
    unordered_map<string,vector<ST>> timesByTrip;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/stop_times.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
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
    }

    // 5) Build trajetsParGare, duplicating per service‐date
    unordered_map<string, list<Trajet>> trajetsParGare;
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
                Trajet T;
                T.tripId             = tripId;
                T.departureStopArea  = prev.stopArea;
                T.arrivalStopArea    = cur.stopArea;
                // departure
                T.departureDate      = date;
                cout << prev.secDep << endl;
                T.departureTimeSec   = prev.secDep;
                // arrival = maybe +1 day
                // compute arrivalDate as YYYYMMDD + cur.dayOffArr
                {
                    tm tm = {};
                    istringstream ss(date);
                    ss >> get_time(&tm, "%Y%m%d");
                    // add day offset
                    tm.tm_mday += cur.dayOffArr;
                    mktime(&tm);
                    ostringstream os;
                    os << put_time(&tm, "%Y%m%d");
                    T.arrivalDate = os.str();
                }
                T.arrivalTimeSec    = cur.secArr;

                // store
                trajetsParGare[T.departureStopArea].push_front(move(T));
            }
        }
    }

    // // --- Example output: count legs per StopArea ---
    // for (auto& [area, fl] : trajetsParGare){
    //     cout << area
    //               << "  →  " << distance(fl.begin(), fl.end())
    //               << " legs\n";
    // }

    //print trajets
    // for (auto& [area, fl] : trajetsParGare){
    //     cout << area << endl;
    //     for (Trajet & trajet : fl) {
    //         cout << "  " << trajet.tripId << " " << trajet.departureDate << endl;
    //     }
    // }
}
