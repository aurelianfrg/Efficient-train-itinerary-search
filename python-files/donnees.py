# -*- coding: utf-8 -*-

import os
import zipfile
import requests
import csv
from datetime import datetime
from datetime import timedelta
from trajet import Trajet

url_tgv = "https://eu.ftp.opendatasoft.com/sncf/gtfs/export_gtfs_voyages.zip"
url_ter = "https://eu.ftp.opendatasoft.com/sncf/gtfs/export-ter-gtfs-last.zip"
url_intercites = "https://eu.ftp.opendatasoft.com/sncf/plandata/export-intercites-gtfs-last.zip"

url = url_tgv #l'utilisation des données de TER sans modification est possible, mais leur quantité bien plus grande implique des temps de calculs trop longs pour être utilisable


def telechargement_donnees():
  """
  Télécharge les données publiques sur les trains au format GTFS si elles ne sont pas deja d'aujourd'hui
  entree : none
  sortie : none
  """

  try:
    response = requests.get(url, stream=True)
  except requests.exceptions.ConnectionError:
    raise BaseException("Veuillez vérifier votre connexion internet, le téléchargement des données TGV a échoué.")
  
  #Le téléchargement est disponible, on peut verifier l'information sur la date du dernier téléchargement
  telechargement_necessaire = True
  aujourdhui = datetime.today()
  date_du_jour= str(aujourdhui.day)+"_"+str(aujourdhui.month)+"_"+str(aujourdhui.year)
  if "telechargement.tmp"in os.listdir():
    with open("telechargement.tmp", "r", encoding='utf-8') as temoin:
      date_enregistree = temoin.read()
      if date_enregistree == date_du_jour and "data" in os.listdir():
        telechargement_necessaire = False

  if telechargement_necessaire:

    if "data" not in os.listdir():
      os.system("mkdir data")
    else:
      #on vide le dossier
      os.system("del /Q data")

    with open("data/dossier.zip", "wb") as f:
      for chunk in response.iter_content(chunk_size=128):
        f.write(chunk)
    #cette portion vient de creer un dossier zip qui contient les txt qui nous intéressent, plus qu'à extraire le zip
    with zipfile.ZipFile("data/dossier.zip", "r") as dossier:
      dossier.extractall("data")  #on précise dans quel dossier on veut extraire

    #on cree le fichier temoin pour ne pas retelecharger les donnees du jour
    with open("telechargement.tmp", "w", encoding='utf-8') as f:
      f.write(date_du_jour)
  


class Donnees:
  """
  Permet d'extraire les données des fichiers csv de la sncf et de les organiser dans des     dictionnaires exploitables
  """

  def __init__(self):
    self.dico_stops = dict()
    self.dico_stoppoint_stopid = dict()
    self.liste_gares = list()
    self.dico_gare_stop_id = dict()

    self.extraire_csv_stops()
    self.creer_dico_gare_stop_id()
  
    

  def extraire_csv_stops(self):
    """
    Ouvre le fichier stops.text et rempli le dictionnaire dico_stops, de clef l'identifiant de la gare et de valeur une liste contenant le nom de la gare, sa latitude et sa longitude : dico_stops = {stops_id = [stop_name, stop_lat, stop_long]}, ainsi que le dico dico_stoppoint_stopid[stop_point] = stop_area
    entrée : self
    sortie : none
    """
    
    fichier = 'data/stops.txt'  
    with open(fichier, newline="", encoding="utf-8") as csvfile:
      reader = csv.reader(csvfile, delimiter=",")
      reader.__next__() # skip first row

      for ligne in reader :
        stop_id = ligne[0]
        stop_name = ligne[1]
        stop_lat = ligne[3]
        stop_lon = ligne[4]
        if "StopArea" in stop_id : #il y a deux types de stop_id, mais seuls les "StopArea" nous intéressent
          self.dico_stops[stop_id] = [stop_name, stop_lat, stop_lon]
          self.liste_gares.append(stop_name) # compléter la liste des gares pour la création du réseau
        else: # dico pour lier les stop_id qui se trouvent dans la même zone (ex: pour Paris Gare de Lyon il y a plusieurs gares et on veut qu'elles soient liées)
          stop_point = stop_id
          stop_area = ligne[8]
          self.dico_stoppoint_stopid[stop_point] = stop_area
          

  def creer_dico_gare_stop_id(self):
    """ 
    Rempli le dictionnaire dico_gare_stop_id, le dictionnaire inverse de dico_stop_id_gare : dico_gare_stop_id = {'gare': 'stop_id',...}
    entrée : self
    sortie : none
    """ 
    for stop_id, (gare, lat ,long) in self.dico_stops.items():
        self.dico_gare_stop_id[gare] = stop_id



def formater_itineraire(itineraire):
  return itineraire
      



if __name__ == "__main__":
  telechargement_donnees()
  donnees = Donnees()
  print(donnees.dico_stops)
  print(donnees.dico_gare_stop_id)
  print(donnees.liste_gares)
  print(donnees.dico_stoppoint_stopid)





