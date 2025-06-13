# -*- coding: utf-8 -*-
from donnees import telechargement_donnees
from controleur import Controleur
if __name__ == "__main__":
  telechargement_donnees()
  app = Controleur()
