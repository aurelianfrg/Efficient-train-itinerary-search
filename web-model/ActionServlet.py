from fastapi import FastAPI
from typing import Union
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles

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



