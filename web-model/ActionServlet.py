from fastapi import FastAPI
from typing import Union
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles
import socket
from datetime import datetime 

PORT = 5001

app = FastAPI()

app.mount("/static", StaticFiles(directory="web-model"), name="static")
#rend tous les fichiers du dossiers web-model accessibles à l'adresse du serveur


@app.get("/", response_class=RedirectResponse)
def root():
    return RedirectResponse("/static/index.html")

@app.get("/page", response_class=HTMLResponse)
def page():
    fichier = open("web-model/index.html","r")
    return fichier.read()

@app.get("/search")
def search(dep:str, arr:str, deptime:int):
    print(
    {
        "parameters": {
            "dep": dep,
            "arr": arr,
            "deptime": deptime
        }
    }
    )

    requete = dep+";"+arr+";"+str(deptime)

    clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
      clientsocket.connect(('127.0.0.1', PORT))
      clientsocket.send(bytes(requete, "UTF-8"))
      result = clientsocket.recv(4096).decode("utf-8") #attention à la taille du buffer
      print(result)
    except ConnectionRefusedError:
      result = None

    if result is None or len(result) <= 2:
      print("La connexion au serveur a été perdue.")
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

    #creer un format pour les itinéraires qui soit un dictionnaire pour que FastAPI puisse faire la conversion en JSON

    return {
       "itineraire_formate" : itineraire_formate,
       "itineraire_detaille" : itineraire_detaille
    }

