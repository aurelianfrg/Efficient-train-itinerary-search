# -*- coding: utf-8 -*-

from afficheur import Afficheur
from liste_trajets_ordonnee import ListeTrajetsOrdonnee
from donnees import Donnees, telechargement_donnees
from trajet import Trajet
import socket
from datetime import datetime

PORT = 5001 #port de connection au serveur c++


class Controleur:
  """Controleur du programme principal de l'application.
  Gère une instance de Donnees, d'Afficheur et l'algorithme de parcours de graphe."""
  __slots__= ['donnees', 'afficheur', 'duree_heures_recherche_max', 'heure_depart_initiale', 'liste_trajets_ordonnee']

  def __init__(self):
    self.donnees = Donnees()
    liste_gares = self.donnees.liste_gares
    self.afficheur = Afficheur(controleur=self, liste_gares=liste_gares)
    
    self.afficheur.mainloop()


  def trouver_trajet_optimal(self, stoppoint_depart, stoppoint_arrivee, datetime_depart):
    """
    Algorithme fondamental de recherche d'itineraire optimal entre 2 gares à une date données.    Cet algorithme se base sur l'algorithme A*, en ordonnant la priorité des arrêtes selon l'heure d'arrivée du train en gare suivante, mais en pondérant cette priorité par la prise en compte de la capacité d'un trajet à rapprocher géographiquement du point d'arrivée.
    L'itinéraire obtenu par l'algorithme est donc proche de celui qui permet d'arriver le plus tôt à destination en partant au plus tôt à l'heure de départ donnée. 
    Cependant, les trains ne partant qu'à une heure fixe, le graphe est dynamique dans le temps.
    """

    requete = stoppoint_depart+";"+stoppoint_arrivee+";"+str(datetime_depart.timestamp())

    clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
      clientsocket.connect(('127.0.0.1', PORT))
      clientsocket.send(bytes(requete, "UTF-8"))
      result = clientsocket.recv(4096).decode("utf-8") #attention à la taille du buffer
      print(result)
    except ConnectionRefusedError:
      result = None

    if result is None or len(result) <= 2:
      self.afficheur.ecrire_text_area("La connexion au serveur a été perdue.")
      return
    
    [itineraire_formate_csv, itineraire_detaille_csv] = result.split('$')
    itineraire_formate = list()
    itineraire_detaille = list()
    # (gare_depart, gare_arrivee, datetime_depart, datetime_arrivee, tripId)
    for ligne in itineraire_formate_csv.splitlines():
      attributs = ligne.split(";")
      itineraire_formate.append( ( attributs[0], attributs[1], datetime.fromtimestamp(int(attributs[2])), datetime.fromtimestamp(int(attributs[3])), attributs[4] ) )
    for ligne in itineraire_detaille_csv.splitlines():
      attributs = ligne.split(";")
      itineraire_detaille.append( ( attributs[0], attributs[1], datetime.fromtimestamp(int(attributs[2])), datetime.fromtimestamp(int(attributs[3])), attributs[4] ) )

    self.afficheur.afficher_resultats_recherche(itineraire_formate)
    self.afficheur.afficher_itineraire_carte(itineraire_detaille)


if __name__ == "__main__":
  telechargement_donnees()
  controleur = Controleur()
