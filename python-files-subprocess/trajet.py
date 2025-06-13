# -*- coding: utf-8 -*-
from datetime import datetime


class Trajet:
  """
  Définit un objet Trajet identifiable par son trip_id.\n
  Permet d'accéder facilement à ses caractéristiques:\n
  -heure_depart \n
  -heure_arrivee \n
  -gare_depart \n
  -gare_arrivee \n
  Il se comporte aussi comme un objet vectoriel, dont on peut calculer le produit vectoriel avec un autre trajet selon l'opérateur *.
  Cela permet de quantifier la capacité du trajet à emmener dans une direction souhaitée, ce qui est utile pour le calcul de l'heuristique.
  """
  def __init__(self, identifiant=None, heure_depart=None, heure_arrivee=None, gare_depart=None, gare_arrivee=None, lat_depart=0, long_depart=0, lat_arrivee=0, long_arrivee=0):
    __slots__= ['id', 'heure_depart', 'heure_arrivee', 'gare_depart', 'gare_arrivee', 'lat_depart', 'long_depart', 'lat_arrivee', 'long_arrivee']
    self.id = identifiant #sous forme (trip_id, n)
    self.heure_depart = heure_depart
    self.heure_arrivee = heure_arrivee
    self.gare_depart = gare_depart
    self.gare_arrivee = gare_arrivee
    self.lat_depart = lat_depart
    self.long_depart = long_depart
    self.lat_arrivee = lat_arrivee
    self.long_arrivee = long_arrivee

    #on enregistre le poids heuristique du trajet dans le parcours
    self.poids = datetime(1970,1,1) #valeur par défaut 

    #création d'un vecteur trajet qui sert à créer une sorte d'heuristique pour le programme à l'aide d'un produit scalaire avec le vecteur destination
    self.vecteur = (float(lat_arrivee) - float(lat_depart), float(long_arrivee) - float(long_depart))


  def __str__(self):
    res = f"Trajet: {self.id} entre {self.gare_depart} et {self.gare_arrivee} :  {str(self.heure_depart)} --> {str(self.heure_arrivee)}\n"
    return res
  

  def __repr__(self):
    return str(self)
  

  def __eq__(self, autre_trajet):
    res = False
    if type(autre_trajet) == Trajet and self.id == autre_trajet.id:
       res= True
    return res
  

  def __mul__(self, autre_trajet): #definit la multiplication de 2 trajets comme le produit scalaire de leur vecteur
    if type(autre_trajet) != Trajet:
      raise TypeError("Produit scalaire non supporté entre Trajet et objet de type " + type(autre_trajet))
    else:
      return self.vecteur[0] * autre_trajet.vecteur[0] + self.vecteur[1] * autre_trajet.vecteur[1]