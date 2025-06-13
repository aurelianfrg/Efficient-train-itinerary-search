
#include "GTFSDataParser.h"
#include "Trajet.h"
#include <queue>
#include <unordered_map>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;


const float w = 1.0; //poids de l'heuristique 
const int PORT = 5001; //port du socket
const int MAX_CONNEXION_BUFFER = 5;


// fonction pour l'ajout d'un trajet dans la file de priorité en calculant au préalable son score de la façon désirée, avec ou sans heuristique
void insererTrajet(priority_queue<Trajet,vector<Trajet>,ComparaisonHeuristiqueTrajet> & file, Trajet & trajet, const double & latitudeObjectif, const double & longitudeObjectif, const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo ) {
//version sans heuristique, le score du trajet est égal à son timestamp d'arrivée --> Djikstra
	//trajet.setScoreSansHeuristique(); 
//version avec heuristique
	trajet.setScoreAvecHeuristique(latitudeObjectif, longitudeObjectif, stopsAreaInfo);

	file.push(trajet);
}


int main(const int argc, const char** argv) { //args must be: stoppointDepart stoppointArrivee datetimeDepart



//---------EXTRACTION DES DONNEES----------

	string tgvDataFolder = "data/tgv";
	string terDataFolder = "data/ter";
    GTFSDataParser dataParser = GTFSDataParser();

    dataParser.parse(tgvDataFolder);
    //dataParser.parse(terDataFolder);

	const unordered_map<string,set<Trajet>> & trajetsParGare = dataParser.getTrajetsParGare();
	const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo = dataParser.getStopsAreaInfo();


//---------INITIALISATION DU SOCKET----------

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //0 pour TCP

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if ( bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		cerr << "binding failed" << endl;
		close(serverSocket);
		exit(-1);
	};
	clog << "binding to port " << PORT << " success" << endl;

	while (1) { //rajouter un signal de fermeture

		listen(serverSocket, MAX_CONNEXION_BUFFER);
		clog << "listening on port " << PORT << endl;
		// il faut ajouter du multi-processing ou threading pour prendre plusieurs connection en parallèles		
		int clientSocket = accept(serverSocket, nullptr, nullptr);

		char buffer[1024] = {0};
		recv(clientSocket, buffer, sizeof(buffer), 0);


		stringstream ss(buffer);
		string depStopArea, arrStopArea, depTimestamp_str;
		// le message doit être au format gare_depart;gare_arrivee;depart_timestamp ex: StopArea:OCE87723197;StopArea:OCE87481705;

		getline(ss, depStopArea, ';');
		getline(ss, arrStopArea, ';');
		getline(ss, depTimestamp_str, ';');
		time_t depTimestamp = atoll(depTimestamp_str.c_str());
		
		clog << "connection received with request : " << buffer << endl;


		double objLatitude = get<1>(dataParser.getStopsAreaInfo().at(arrStopArea));
		double objLongitude = get<2>(dataParser.getStopsAreaInfo().at(arrStopArea));
		//---------ALGORITHME PRINCIPAL----------  

		int nbIter = 0;
		// fonction trouverItineraire( stoppointDepart, stoppointArrivee, datetimeDepart )
		
		// //initialisation structures de données	
		// garesEstatteinte <-- {} clé: id gare, valeur: 0 non atteint / 1 atteint (hashmap pour accès en O(1))
		unordered_map<string, int> garesEstatteinte;
		for (auto it = stopsAreaInfo.begin(); it != stopsAreaInfo.end(); ++it) {
			garesEstatteinte[it->first] = false;
		}
		// arbreTrajetsParents <-- {} clé: id gare, valeur: trajet parent dans l'arborescence 
		unordered_map<string, Trajet> arbreTrajetsParents;
		

		// // initalisation variables
		// gareAtteinte <-- stoppointDepart
		string gareAtteinte = depStopArea;
		// fileTrajets une file de priorité de Trajet sur l'heure d'arrivée des trajets
		priority_queue<Trajet,vector<Trajet>,ComparaisonHeuristiqueTrajet> file;
		
		// initialiser la file des trajets avec ceux qui partent du départ
		if  (trajetsParGare.find(depStopArea) == trajetsParGare.end()) throw domain_error("La gare de départ n'est pas connue"); //erreur de clé, la gare de départ est inconnue 
		set<Trajet> trajetsInitiaux = trajetsParGare.at(depStopArea);
		for (Trajet trajet : trajetsInitiaux) {
			if ( trajet.getTimestampDep() >= depTimestamp )
			{
				insererTrajet(file, trajet, objLatitude, objLongitude, dataParser.getStopsAreaInfo());
			}
		}

		Trajet trajetATester;
		// Tant que  gareAtteinte != stoppointArrivee  et taille(file) > 0
		while (gareAtteinte != arrStopArea && file.size() > 0)
		{
			++nbIter;

			// trajetATester <-- premier trajet dans la file 
			trajetATester = file.top();
			file.pop();
			// gareAtteinte <-- gare d'arrivée de trajetATester
			gareAtteinte = trajetATester.getArrStopArea();
			// Si la gare n'a pas encore été atteinte
			if (!garesEstatteinte[gareAtteinte]) {	
				// listeGaresAtteintes[gareAtteinte] <-- 1
				garesEstatteinte[gareAtteinte] = true;
				// arbreTrajetsParents[gareAtteinte] <-- trajetAtester
				arbreTrajetsParents[gareAtteinte] = trajetATester;
				// ajouter à fileTrajets les trajets partants après l'heure d'arrivée de trajetATester à gareAtteinte
				if  (trajetsParGare.find(gareAtteinte) == trajetsParGare.end()) {
					// cerr << gareAtteinte << endl;
					// throw domain_error("La gare atteinte précédente ne possède aucun trajet à son départ."); //erreur de clé, la gare de départ est inconnue 
				}
				else 
				{
					set<Trajet> trajetsARajouter = trajetsParGare.at(gareAtteinte);
					for (Trajet trajet : trajetsARajouter) {
						if ( trajet.getTimestampDep() > trajetATester.getTimestampArr() )
						insererTrajet(file, trajet, objLatitude, objLongitude, dataParser.getStopsAreaInfo());
					}
				}
			}
		}

		// //reconstruction de l'itinéraire avec l'arbre des parents

		// trajetsEmpruntes <-- [] liste des trajets empruntes dans l'itinéraire trouvé 
		list<Trajet> trajetsEmpruntes;
		// Si gareAtteinte == stoppointArrivee
		if (gareAtteinte == arrStopArea)
		{
			//trajetsEmpruntes[0] <-- trajetATester
			trajetsEmpruntes.push_front(trajetATester);
			//Tant que  le dernier trajet ajouté ne part pas de stoppointDepart
			while ( trajetsEmpruntes.front().getDepStopArea() != depStopArea)
			{
				//ajouter à trajetsEmpruntes le trajet parent de la gare de départ du dernier trajet ajouté
				Trajet parent = arbreTrajetsParents[trajetsEmpruntes.front().getDepStopArea()];
				trajetsEmpruntes.push_front(parent);
			}
		}
		// Sinon il n'existe pas d'itinéraire
		else 
		{
			cerr << "Aucun initinéraire n'a été trouvé";
		}

		// reponse au client 
		stringstream response_stream;
		dataParser.ecrireResultat(response_stream, dataParser.formaterItineraire(trajetsEmpruntes)); //faire un formaterItineraireJSON qui soit compatible avec l'usage web
		char response[1024];
		strcpy(response, response_stream.str().c_str());
		clog << response << endl;
		send(clientSocket, response, strlen(response), 0);
		close(clientSocket);
	}
	close(serverSocket);
	return 0;
}


