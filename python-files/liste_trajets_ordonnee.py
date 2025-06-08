# -*- coding: utf-8 -*-
from datetime import timedelta
from trajet import Trajet
from math import sqrt

class ListeTrajetsOrdonnee:
  

  def __init__(self, trajets_gare_depart, heure_depart_souhaitee, lat_depart, long_depart, lat_arrivee, long_arrivee, duree_heures_recherche_max):
    """
    Définit une liste contenant les trajets non testés en mémoire dans le parcours de graphe.\n
    Elle est ordonnée par ordre croissant d'horaire d'arrivée du trajet et se réordonne à chaque ajout d'un trajet.\n
    Ainsi, la première itération de la gare objectif dans la recherche de chemin est forcément celle permettant d'y arriver le plus tôt.
    La liste des trajets depuis la gare doit être ordonnée par ordre croissant d'heure d'arrivée.
    """
    __slots__= ['liste_trajets', 'duree_heures_recherche_max', 'liste_initiale', 'dico_trajet_priorite', 'heure_depart_initiale', 'lat_gare_objectif', 'long_gare_objectif', 'vecteur_objectif_initial']

    self.liste_trajets = list()  #liste principale de l'objet, initialisée vide pour pouvoir être triée dans un premier temps
    self.liste_initiale = list()
    self.dico_trajet_priorite = dict()
    self.heure_depart_initiale = heure_depart_souhaitee
    self.duree_heures_recherche_max = duree_heures_recherche_max
    self.lat_gare_objectif = lat_arrivee
    self.long_gare_objectif = long_arrivee 
    self.vecteur_objectif_initial = Trajet(lat_depart=lat_depart, long_depart=long_depart, lat_arrivee=lat_arrivee, long_arrivee=long_arrivee)
    #la valeur du poids heuristique est déterminée expérimentalement pour obtenir un trajet proche de l'idéalité tout en réduisant drastiquement le temps de calcul

    ### POIDS DE L'HEURISTIQUE
    self.poids_heuristique = 900
    #mettre à 0 cette valeur revient à utiliser l'algorithme de Djikstra. 
    #L'augmenter peut considérablement diminuer le temps de calcul si un itinéraire existe, mais, en l'augmentant trop, l'itinéraire trouvé peut être faussé. Dans le meilleur des cas il peut être sous-optimal voir complètement aberrant (ex: de Valence pour aller à Lyon en passant par Paris car c'est ce qui emmenait le plus au Nord depuis Valence)


    #on ajoute dans la liste_initiale les trains qui partent de la gare de départ après l'horaire de départ souhaité, et arrivent au plus tard 24h apres le depart (24*3600 secondes)
    for trajet in trajets_gare_depart:
      if trajet.heure_depart > heure_depart_souhaitee and trajet.heure_arrivee < heure_depart_souhaitee + timedelta(seconds=self.duree_heures_recherche_max*3600): 
        priorite = self.calcul_priorite(trajet, vecteur_objectif=self.vecteur_objectif_initial)
        self.liste_initiale.append(trajet)
        self.dico_trajet_priorite[trajet.id] = priorite

    self.tri_initial()
    self.liste_trajets_deja_essayes = trajets_gare_depart.copy()


  def calcul_priorite(self, trajet, vecteur_objectif):
    """Calcule la priorité d'un trajet en utilisant l'heuristique créée à partir des coordonnées
    entrée : trajet
    sortie : priorité (datetime)
    """
    try:
      priorite = trajet.heure_arrivee - timedelta(seconds = (trajet * vecteur_objectif * self.poids_heuristique)/(sqrt(vecteur_objectif.vecteur[0]**2+vecteur_objectif.vecteur[1]**2)*sqrt(trajet.vecteur[0]**2+trajet.vecteur[1]**2)))
    except ZeroDivisionError: #on est trop proche de l'arrivée
      priorite = self.heure_depart_initiale 
    return priorite


  def tri_initial(self):
    """Tri optimisé des trajets de la gare de départ selon leur priorité"""
    self.liste_trajets = self.liste_initiale.copy()
    self.liste_trajets.sort(key=lambda trajet: self.dico_trajet_priorite[trajet.id])


  def ajouter_trajet(self, trajet):
    """Ajoute un nouveau trajet à la liste de sorte qu'elle reste triée par ordre croissant d'heure d'arrivée. L'ajoute seulement si ce trajet n'a pas deja été essayé dans le parcours et s'il est après l'heure de départ initiale.
    entrée : trajet
    """

    if trajet not in self.liste_trajets_deja_essayes and trajet.heure_depart > self.heure_depart_initiale and trajet.heure_arrivee < self.heure_depart_initiale + timedelta(seconds=self.duree_heures_recherche_max*3600):
      element_ajoute = False
      vecteur_objectif_trajet = Trajet(lat_depart=trajet.lat_depart, long_depart=trajet.long_depart, lat_arrivee=self.lat_gare_objectif, long_arrivee=self.long_gare_objectif)
      priorite = trajet.heure_arrivee - timedelta(seconds = (trajet * vecteur_objectif_trajet) * self.poids_heuristique)
      i=0
      while not element_ajoute and i < len(self.liste_trajets):
        trajet_act = self.liste_trajets[i]
        priorite_act = self.dico_trajet_priorite[trajet_act.id]
        if priorite < priorite_act:
          self.liste_trajets.insert(i, trajet)
          element_ajoute = True #on a ajouté le trajet, sortie de boucle
        i += 1

      if not element_ajoute:
        #l'element à ajouter était plus grand que tous les autres de la liste, on l'insère à la fin
        self.liste_trajets.append(trajet)

      #ce trajet a ete ajouté à la liste des trajets à tester, on ne veut plus l'y ajouter
      self.liste_trajets_deja_essayes.append(trajet)
      self.dico_trajet_priorite[trajet.id] = priorite


  def ajouter_tous_les_trajets(self, trajets_depuis_gare, heure_minimale):
    """Permet d'ajouter simultanément tous les trajets au départ d'une gare à la liste ordonnée des trajets, a condition qu'ils partent après l'heure d'arrivee à cette gare
    entrée : trajets_depuis_gare, heure_minimale
    """
    for trajet in trajets_depuis_gare:
      heure_depart_trajet = trajet.heure_depart
      if heure_depart_trajet >= heure_minimale: #on part du principe qu'une correspondance de 0min est gérable pour faciliter les tests
        self.ajouter_trajet(trajet)

  def next(self):
    """Renvoie le premier trajet en liste d'attente et le retire de la liste"""
    if len(self.liste_trajets) > 0:
      res = self.liste_trajets.pop(0)
    else:
      res = None #signal qu'il n'y a plus de trajet à tester
    return res


  def duree_attente_heure(self):
    """Renvoie le temps entre l'heure de départ théorique et l'heure d'arrivée dans le meilleur cas à un avancement donné de l'algorithme.\n
    Permet de vérifier dans l'algorithme principal que le temps nécessaire pour effectuer un trajet ne devient pas trop grand (signe que l'itinéraire pourrait ne pas exister).
    entrée : self
    sortie : duree_heure"""
    if len(self.liste_trajets) > 0:
      premier_trajet = self.liste_trajets[0]
      heure_arrivee = premier_trajet.heure_arrivee
      delta_attente = heure_arrivee - self.heure_depart_initiale
      duree_heure = 24 * delta_attente.days + delta_attente.seconds // 3600      
    else:
      duree_heure = 0
    return duree_heure


  def __str__(self):
    res = ""
    for trajet in self.liste_trajets:
      res += f"Trajet pour {trajet.gare_arrivee}:  {str(trajet.heure_depart)} --> {str(trajet.heure_arrivee)}\n"
    return res

  def __repr__(self) -> str:
    pass

  def __len__(self):
    return len(self.liste_trajets)