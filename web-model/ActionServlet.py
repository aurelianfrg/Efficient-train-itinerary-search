from fastapi import FastAPI
from typing import Union
from fastapi.responses import HTMLResponse

app = FastAPI()

@app.get("/", response_class=HTMLResponse)
def root():
    fichier = open("web-model/index.html","r")
    return fichier.read()


@app.get("/search")
def search(dep:str, arr:str, deptime:int):
    return {
        "parameters": {
            "dep": dep,
            "arr": arr,
            "deptime": deptime
        }
    }

