#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>

using namespace std;

const char* dataFolder = "data";
const int pathLentgh = strlen(dataFolder) + 20;
char * filepath = new char [pathLentgh];

// ---- Data structures ----

struct Trajet {
    string tripId;
    string departureStop;
    string arrivalStop;
    int departureTime;  // seconds since midnight
    int arrivalTime;    // seconds since midnight
};

// helper to parse "HH:MM:SS" → seconds since midnight
int parseTime(const string& hhmmss) {
    int h, m, s;
    char c1, c2;
    istringstream ss(hhmmss);
    ss >> h >> c1 >> m >> c2 >> s;
    return h*3600 + m*60 + s;
}

// generic CSV‐line splitter (naïve, assumes no commas inside fields)
vector<string> splitCSV(const string& line) {
    vector<string> out;
    string cell;
    istringstream ss(line);
    while (getline(ss, cell, ',')) {
        out.push_back(cell);
    }
    return out;
}

int main() {
    // 1) Read calendar_dates.txt
    //    service_id,date,exception_type
    unordered_map<string, set<string>> validDatesByService;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/calendar_dates.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
        getline(f, line); // skip header
        while (getline(f, line)) {
            auto fields = splitCSV(line);
            if (fields.size() < 3) continue;
            const string& svc = fields[0];
            const string& date = fields[1];            // YYYYMMDD
            int exceptionType = stoi(fields[2]);       // 1 = added, 2 = removed
            if (exceptionType == 1) {
                validDatesByService[svc].insert(date);
            } else {
                validDatesByService[svc].erase(date);
            }
        }
    }

    // 2) Read trips.txt → map trip_id → service_id
    //    route_id,service_id,trip_id,...
    unordered_map<string,string> serviceByTrip;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/trips.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
        getline(f, line); // skip header
        while (getline(f, line)) {
            auto F = splitCSV(line);
            if (F.size() < 3) continue;
            string tripId    = F[2];
            string serviceId = F[1];
            serviceByTrip[tripId] = serviceId;
        }
    }

    // 3) Read stop_times.txt and group by trip_id
    //    trip_id,arrival_time,departure_time,stop_id,stop_sequence,...
    struct StopTime { int seq, arr, dep; string stopId; };
    unordered_map<string, vector<StopTime>> stopTimesByTrip;
    {
        strcpy(filepath, dataFolder);
        strcat(filepath, "/stop_times.txt");
        clog << "reading " << filepath << endl;
        ifstream f(filepath);
        string line;
        getline(f, line); // skip header
        while (getline(f, line)) {
            auto F = splitCSV(line);
            if (F.size() < 5) continue;
            string tripId = F[0];
            int arr  = parseTime(F[1]);
            int dep  = parseTime(F[2]);
            string stopId = F[3];
            int seq  = stoi(F[4]);
            stopTimesByTrip[tripId].push_back({seq, arr, dep, stopId});
        }
    }

    // 4) Build trajetsParGare
    unordered_map<string, list<Trajet>> trajetsParGare;
    // (choose a date you care about, e.g. "20250125")
    string targetDate = "20250125";
    for (auto& [tripId, stops] : stopTimesByTrip) {
        // check service validity
        auto itSvc = serviceByTrip.find(tripId);
        if (itSvc == serviceByTrip.end()) continue;
        const string& svc = itSvc->second;
        if (!validDatesByService[svc].count(targetDate)) continue;

        // sort by stop_sequence
        sort(stops.begin(), stops.end(),
                  [](auto &a, auto &b){ return a.seq < b.seq; });

        // create a Trajet for each leg
        for (size_t i = 1; i < stops.size(); ++i) {
            const auto &prev = stops[i-1];
            const auto &cur  = stops[i];
            Trajet t;
            t.tripId         = tripId;
            t.departureStop  = prev.stopId;
            t.arrivalStop    = cur.stopId;
            t.departureTime  = prev.dep;
            t.arrivalTime    = cur.arr;
            trajetsParGare[t.departureStop].push_front(t);
        }
    }

    // Example: print how many outgoing legs each station has:
    for (auto& [stopId, flist] : trajetsParGare) {
        cout << stopId
                  << " → " 
                  << distance(flist.begin(), flist.end())
                  << " outgoing legs\n";
    }

    return 0;
}
