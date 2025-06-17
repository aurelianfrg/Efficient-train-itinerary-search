# -*- coding: utf-8 -*-

from afficheur import Afficheur
from liste_trajets_ordonnee import ListeTrajetsOrdonnee
from donnees import Donnees, telechargement_donnees
import subprocess


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
    
    command = f"./bin/main {stoppoint_depart} {stoppoint_arrivee} {datetime_depart}"
    result = subprocess.check_output(command, shell=True, text=True)

    self.afficheur.afficher_resultats_recherche(result)


if __name__ == "__main__":
  telechargement_donnees()
  controleur = Controleur()
