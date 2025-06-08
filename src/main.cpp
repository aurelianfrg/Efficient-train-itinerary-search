
#include "GTFSDataParser.h"
#include "Trajet.h"
#include <queue>
#include <unordered_map>
#include <assert.h>

using namespace std;


const float w = 1.0; //poids de l'heuristique 


// fonction pour l'ajout d'un trajet dans la file de priorité en calculant au préalable son score de la façon désirée, avec ou sans heuristique
void insererTrajet(priority_queue<Trajet,vector<Trajet>,ComparaisonHeuristiqueTrajet> & file, Trajet & trajet, const double & latitudeObjectif, const double & longitudeObjectif, const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo ) {
//version sans heuristique, le score du trajet est égal à son timestamp d'arrivée --> Djikstra
	//trajet.setScoreSansHeuristique(); 
//version avec heuristique
	trajet.setScoreAvecHeuristique(latitudeObjectif, longitudeObjectif, stopsAreaInfo);

	file.push(trajet);
}


int main(const int argc, const char** argv) { //args must be: stoppointDepart stoppointArrivee datetimeDepart
	
//---------TRAITEMENT DE L INPUT----------

	clog << argc << endl;

	string depStopArea = "StopArea:OCE87723197";
	string arrStopArea = "StopArea:OCE87481705";

	struct tm depDatetime;
	depDatetime.tm_year = 2025 - 1900;
	depDatetime.tm_mon = 1 - 1; //janvier
	depDatetime.tm_mday = 22; // 22 janvier 2025
	depDatetime.tm_hour = 12;
	depDatetime.tm_min = 0;
	depDatetime.tm_sec = 0;
	depDatetime.tm_isdst = -1; //computer timezone settings
	time_t depTimestamp = mktime(&depDatetime);

	if (argc >= 4) {
		depStopArea = argv[1];
		arrStopArea = argv[2];
		depTimestamp = atoll(argv[3]);
	}

	clog << depStopArea << " " << arrStopArea << " " << depTimestamp << endl;


//---------EXTRACTION DES DONNEES----------

	string tgvDataFolder = "data/tgv";
	string terDataFolder = "data/ter";
    GTFSDataParser dataParser = GTFSDataParser();

    dataParser.parse(tgvDataFolder);
    //dataParser.parse(terDataFolder);

	const unordered_map<string,set<Trajet>> & trajetsParGare = dataParser.getTrajetsParGare();
	const unordered_map<string,tuple<string,double,double>> & stopsAreaInfo = dataParser.getStopsAreaInfo();

	double objLatitude = get<1>(dataParser.getStopsAreaInfo().at(arrStopArea));
	double objLongitude = get<2>(dataParser.getStopsAreaInfo().at(arrStopArea));

	//affichage
    //dataParser.printNbTrajetsParGare();
    //dataParser.printStopAreaInfos();
    //dataParser.printTrajetsParGare();

	//test operator ==
	// auto setTrajets = trajetsParGare.find("StopArea:OCE87473181");
	// assert(setTrajets != trajetsParGare.end());
	// size_t nbTrajets = setTrajets->second.size();
	// cout << "Nombre de trajets : " << nbTrajets << endl;
	// if (nbTrajets < 2) {
	// 	cerr << "Erreur : pas assez de trajets pour tester operator== !" << endl;
	// 	exit(0);
	// }
	// auto it = setTrajets->second.begin();
	// Trajet trajet1 = *it;
	// ++it;
	// Trajet trajet2 = *it;
	// cout << "Résultat de la comparaison : " << (trajet1 == trajet2) << endl;

	clog << "début algo" << endl;
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
	// retourner trajetsEmpruntes


//-------------------AFFICHAGE DES RESULTATS-------------------

	clog << "Itinéraire trouvé en : " << nbIter << " itérations." << endl;
	//dataParser.ecrireResultat(cout, trajetsEmpruntes);
	clog << endl << "formattage de cet itinéraire train par train" << endl << endl;
	dataParser.ecrireResultat(cout, dataParser.formaterItineraire(trajetsEmpruntes));

	


//tests file de priorité
	// auto setTrajets = trajetsParGare.find("StopArea:OCE87473181");
	// assert(setTrajets != trajetsParGare.end());
	// size_t nbTrajets = setTrajets->second.size();
	// cout << "Nombre de trajets : " << nbTrajets << endl;
	// if (nbTrajets < 2) {
	// 	cerr << "Erreur : pas assez de trajets pour tester operator== !" << endl;
	// 	exit(0);
	// }
	// auto it = setTrajets->second.begin();
	// Trajet trajet1 = *it;
	// ++it;
	// Trajet trajet2 = *it;
	// ++it;
	// Trajet trajet3 = *it;
	// trajet1.setScoreSansHeuristique();
	// trajet2.setScoreSansHeuristique();
	// trajet3.setScoreSansHeuristique();
	// file.push(trajet1);
	// file.push(trajet2);
	// file.push(trajet3);
	// cout << "1e trajet en sortie : \n" << file.top();
	// file.pop();
	// cout << "\n2e trajet en sortie : \n" << file.top();
	// file.pop();
	// cout << "\n3e trajet en sortie : \n" << file.top();

	//écriture des gares
	// ofstream output("gare_tgv_ter.txt");
	// dataParser.ecrireGares(output);
	// output.close();
}


