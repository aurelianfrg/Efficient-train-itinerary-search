# -*- coding: utf-8 -*-

import tkinter as tk
import tkinter.ttk as ttk
import tkintermapview
from tkinter import scrolledtext
from tkcalendar import Calendar
from datetime import datetime, timedelta, date
from threading import Thread
from donnees import formater_itineraire


# Création de l'interface graphique
class Afficheur(tk.Tk):
  __slots__=['liste_gares','controleur','Heures','Minutes', 'depart','arrivee','liste_arrivee','date_choisie','thread_recherche']

  
  def __init__(self, controleur, liste_gares) :
    super().__init__()
    self.controleur = controleur
    self.title("SNCF")
    self.resizable(False, False)

    self.date_choisie = None
    self.thread_recherche = Thread(target = lambda x: None) #initialisation du thread de recherche pour pouvoir détecter s'il est actif
    #L'utilisation du thread permet d'empêcher la fenêtre de freeze lorsque l'algorithme est en cours

    self.liste_gares = liste_gares
    self.liste_gares.sort() #tri alphabétique des gares pour pouvoir les trouver plus facilement
    self.Heures = ["00","01","02","03","04","05","06","07","08","09",10,11,12,13,14,15,16,17,18,19,20,21,22,23]
    self.Minutes = ["00","01","02","03","04","05","06","07","08","09",10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59]

    self.creer_widgets()
    self.configure(bg="#B7EAFF")

    self.carte = True #True pour afficher la carte de France avec les trajets indiqués dessus (consomme plus de ressources)
    if self.carte:
      self.afficher_carte()


  def creer_widgets(self):
    '''
    Crée les différents widgets de l'interface graphique
    entree : self
    sortie : none
    '''
    #Etiquette gare de depart
    self.depart = tk.Label(self, text = "Gare de départ :",font = 'Arial 10 ', background= '#B7EAFF')
    self.depart.grid(row = 0, column = 0, padx=10, pady=10)

    #Bouton liste déroulante pour gare de départ
    self.liste_depart = ttk.Combobox(self, values = self.liste_gares, font="Arial 12")
    self.liste_depart.grid(row = 0, column = 1, padx=10, pady=10, sticky="w")  

    #Etiquette gare d'arrivée
    self.arrivee = tk.Label(self, text = "Gare d'arrivée :",font = 'Arial 10 ', background= '#B7EAFF')
    self.arrivee.grid(row = 1, column =0, padx=10, pady=10)

    #Bouton liste déroulante pour gare d'arrivée
    self.liste_arrive = ttk.Combobox(self, values = self.liste_gares, font="Arial 12")
    self.liste_arrive.grid(row = 1, column = 1, padx=10, pady=10, sticky="w")  

    #Etiquette pour date de départ
    self.date_depart = tk.Label(self, text = "Date de départ :",font = 'Arial 10 ', background= '#B7EAFF')
    self.date_depart.grid(row = 2, column = 0, pady=5, padx=10)

    #Bouton pour entrer date de depart
    self.bouton_date = tk.Button(self, text="Choisir Date",font = 'Arial 11 bold', activebackground='pink', activeforeground='white')
    self.bouton_date.bind('<Button-1>', self.calendrier)
    self.bouton_date.grid(row=2, column=1, columnspan=2, padx=10, pady=10)

    #Etiquettes pour heure
    self.heure = tk.Label(self, text= "Heure :",font = 'Arial 10 ', background= '#B7EAFF')
    self.heure.grid(row = 0, column = 2, sticky='w', padx=10, pady=10)
    self.h = tk.Label(self, text = "h",font = 'Arial 10 ', background= '#B7EAFF')
    self.h.grid(row = 0, column = 4, sticky="w", padx=10, pady=10)
    self.min = tk.Label(self, text = "min",font = 'Arial 10 ', background= '#B7EAFF')
    self.min.grid(row = 1, column = 4, sticky="w", padx=10, pady=10)

    #Combobox pour entrer heure
    self.liste_heure_depart = ttk.Combobox(self, values = self.Heures, width=4, font="Arial 12")
    self.liste_heure_depart.set("08")
    self.liste_heure_depart.grid(row = 0, column = 3, padx=10, pady=10)
    self.liste_min_depart = ttk.Combobox(self, values = self.Minutes, width=4, font="Arial 12")
    self.liste_min_depart.set("00")
    self.liste_min_depart.grid(row = 1, column = 3, padx=10, pady=10)

    #Zone de texte
    self.text_area = scrolledtext.ScrolledText(self, wrap=tk.WORD, width=60, height=20, font=("Arial 14"), padx=10, pady=10)
    self.text_area.grid(row=5, column=0, columnspan =5, padx=10, pady=10)
    self.text_area.insert(tk.INSERT,"Bienvenue ! Nous sommes là pour vous aider à choisir un train. \nSaisissez la gare de départ, la gare d'arrivée, la date de départ et l'heure de départ." +'\n')
    self.text_area.insert(tk.INSERT,"-------------------------------------------" +'\n')
    self.text_area.configure(state='disabled')

    #Bouton pour lancer la recherche du train
    self.bouton_1 = tk.Button(self, text="Rechercher", font = 'Arial 11 bold', activebackground='pink', activeforeground='white', padx=10, pady=10)
    self.bouton_1.bind('<Button-1>',self.rechercher)
    self.bouton_1.grid(row=4, column=1, columnspan=2 ,pady=10,padx=5)

    #Bouton pour quitter   
    self.bouton_3 = tk.Button(self, text="Quitter", font = 'Arial 12 bold', foreground='blue', activeforeground = 'white', activebackground = 'black')
    self.bouton_3.bind('<Button-1>',self.quitter)
    self.bouton_3.grid(row=9, column=1, columnspan =2, pady=15)

    #Radioboutton
    self.choix = tk.StringVar() 
    self.choix.set("Oui")

    #Bouton pour confirmer ou non le train 
    self.bouton_confirmation = tk.Button(self, text="Voulez-vous confirmer ce train ?",font = 'Arial 11 bold', activebackground='pink', activeforeground='white')
    self.bouton_confirmation.bind('<Button-1>', self.confirmer)
    self.bouton_confirmation.grid(row=8, column=1,pady=1)

    self.radio1 = tk.Radiobutton(self, text="Oui", variable = self.choix, value="Oui",  background= '#B7EAFF')
    self.radio1.grid(row=8, column=2)

    self.radio2 = tk.Radiobutton(self, text="Non", variable=self.choix, value="Non", background= '#B7EAFF')
    self.radio2.grid(row=8, column=3)

  
  #Recherche du train
  def rechercher(self, event):
    """
    Callback du bouton de recherche d'itinéraire.
    Récupère les informations entrées par l'utilisateur dans la fenetre principale et applique l'algorithme avant d'afficher les résultats.
    entree : self, event
    sortie : none
    """
    if not self.thread_recherche.is_alive():
      #récupération des informations entrées par l'utilisateur
      self.gare_depart = self.liste_depart.get() 
      self.gare_arrivee = self.liste_arrive.get()
      self.h_depart = self.liste_heure_depart.get()
      self.min_depart = self.liste_min_depart.get()
  
      if self.gare_depart == "" or self.gare_arrivee == "" or self.h_depart == "" or self.min_depart == "" or self.date_choisie == None or self.gare_depart == self.gare_arrivee or self.gare_depart not in self.liste_gares or self.gare_arrivee not in self.liste_gares:
        self.ecrire_text_area("Veuillez remplir tous les champs." + '\n') 
      
      else:
        timedelta_heure_choisie = timedelta(hours = int(self.h_depart), minutes=int(self.min_depart))
        datetime_depart = self.date_choisie + timedelta_heure_choisie      
        stoppoint_depart = self.controleur.donnees.dico_gare_stop_id[self.gare_depart]
        stoppoint_arrivee = self.controleur.donnees.dico_gare_stop_id[self.gare_arrivee]
  
        self.ecrire_text_area("", clear=True) #nettoyage du scrolledtext pour préparer l'affichage du résultat
  
        #on applique l'algorithme de recherche de trajet optimal selon les informations entrées par l'utilisateur
        #on l'execute dans un thread pour liberer le mainloop et pouvoir afficher une barre de chargement sans que la fenetre se bloque  
        self.thread_recherche = Thread(target=self.controleur.trouver_trajet_optimal, args=[stoppoint_depart, stoppoint_arrivee, datetime_depart])
        self.thread_recherche.start()

    
  #Afficher les resultats de recherche du train trouvé
  def afficher_resultats_recherche(self, itineraire):
    
    dico_stops = self.controleur.donnees.dico_stops
    
    self.ecrire_text_area("Itinéraire trouvé avec toutes les correspondances:" + "\n\n", clear=True)

    if self.carte:
      self.map_widget.delete_all_path()
      self.map_widget.delete_all_marker()

    # if itineraire is not None: #un itineraire a été trouvé
    #   itineraire_formate = formater_itineraire(itineraire)
    #   for segment in itineraire_formate:
    #     nom_gare_depart = dico_stops[segment[0]][0]
    #     nom_gare_arrivee = dico_stops[segment[1]][0]
    #     datetime_depart = segment[2]
    #     datetime_arrivee = segment[3]
    #     id_train = segment[4][0]
    #     heure_depart = str(datetime_depart.hour)
    #     heure_arrivee = str(datetime_arrivee.hour)
    #     min_depart = str(datetime_depart.minute)
    #     min_arrivee = str(datetime_arrivee.minute)
    #     if len(min_depart) < 2: min_depart = "0"+min_depart
    #     if len(min_arrivee) < 2: min_arrivee = "0"+min_arrivee
        
    #     if "Correspondance" not in id_train:
    #       num_tgv = id_train.split("N")[1].split("F")[0]
    #       ligne1 = f"TGV n°{num_tgv} \n-->Départ de {nom_gare_depart} à {heure_depart}h{min_depart}"
    #       ligne2 = f"-->Arrivée à {nom_gare_arrivee} à {heure_arrivee}h{min_arrivee}"
    #       self.ecrire_text_area(ligne1 + "\n" + ligne2 + "\n\n")   
    #     else:
    #       self.ecrire_text_area(id_train[:-10]+"\n\n")

    #     #affichage éventuel sur la carte
    #   if self.carte:
    #     for trajet in itineraire:
    #       self.map_widget.set_path( [(float(dico_stops[trajet.gare_depart][1]),(float(dico_stops[trajet.gare_depart][2]))), (float(dico_stops[trajet.gare_arrivee][1]), (float(dico_stops[trajet.gare_arrivee][2])))] , width=5)
          
      self.ecrire_text_area(itineraire)
    
    else:  #pas d'itineraire trouvé
      self.ecrire_text_area(f"Aucun itinéraire n'a pu être trouvé en moins de {self.controleur.duree_heures_recherche_max}h en TGV...", clear = True)

    
  #Création du calendrier pour choisir la date de départ
  def calendrier(self, event):
    '''
    cree la fentêtre de choix de date, avec le calendrier
    entree : self, event
    sortie : none
    '''
    #Créer une autre fenetre pour le calendrier
    self.fenetre_calendrier = tk.Tk()
    self.fenetre_calendrier.geometry("400x400")
    self.fenetre_calendrier.configure(bg="#B7EAFF")

    date_actuelle = date.today()
    # Add Calendar
    self.cal = Calendar(self.fenetre_calendrier, selectmode = 'day', year = date_actuelle.year, month = date_actuelle.month, day = date_actuelle.day) 
    self.cal.pack(pady = 20)
    bouton = ttk.Button(self.fenetre_calendrier, text = "Confirmer la date", command=self.recuperer_date_calendrier, width=20)
    bouton.pack(pady = 20)
    self.fenetre_calendrier.mainloop()

  
  #Recuperation de la date choisie par l'utilisateur
  def recuperer_date_calendrier(self):
    '''
    recupere la date choisie par l'utilisateur et retourne à la fenetre principale
    entree : self
    sortie : none
    '''
    #on recupere la date entree par l'utilisateur dans le calendrier
    date_utilisateur = self.cal.get_date()
    mois_str, jour_str, annee_str = date_utilisateur.split("/")
    mois = int(mois_str)
    jour = int(jour_str)
    annee = int("20"+annee_str)
    self.date_choisie = datetime(annee, mois, jour)
    
    #on ferme le calendrier pour retourner sur la fenetre principale
    self.fenetre_calendrier.destroy()

  
  #Confirmer ou non le train choisi
  def confirmer(self, event):
    '''
    Callback des radiobutton de confirmation de train
    entree : self, event
    sortie : none
    '''
    if self.choix.get() == "Oui":       
        # Create Object
        self.train_confirmer = tk.Tk()
        # Set geometry
        self.train_confirmer.geometry("800x300")
        self.train_confirmer.title("Confirmation du train")
        #texte
        self.text = scrolledtext.ScrolledText(self.train_confirmer, wrap=tk.WORD, width=60, height=20, font=("Arial 15 italic"))
        self.text.grid(row=5, column=1, columnspan =2)
        self.text.insert(tk.INSERT,"Train confirmé ! Nous vous souhaitons un bon voyage !" +'\n')
    if self.choix.get() == "Non":       
        # Create Object
        self.train_nonconfirmer = tk.Tk()
        # Set geometry
        self.train_nonconfirmer.geometry("800x300")
        self.train_nonconfirmer.title("Non confirmation du train")
        #texte
        self.text = scrolledtext.ScrolledText(self.train_nonconfirmer, wrap=tk.WORD, width=60, height=20, font=("Arial 15 italic"))
        self.text.grid(row=5, column=1, columnspan =2)
        self.text.insert(tk.INSERT,"Nous sommes désolés que ce trajet ne corresponde pas à votre demande. Veuillez réeffectuer une recherche. " +'\n')

  
  def ecrire_text_area(self, texte, clear = False):
    '''
    Ecrit le texte dans la zone de texte
    entree : self, texte : chaine de caractères, clear : booléen
    sortie : none
    '''
    self.text_area.configure(state='normal')
    if clear:
      self.text_area.delete('1.0', tk.END)
    self.text_area.insert(tk.INSERT, texte)
    self.text_area.configure(state='disabled')


  def afficher_carte(self):
    '''
    Afficher la carte de France quand l'interface graphique est démarée
    entree : self
    sortie : none
    '''
    self.frame_carte = tk.Frame(master=self)
    self.frame_carte.grid(column=5, row=0, rowspan=10, padx=10, pady=10)

    top_left_position = (52.0, -3.5)
    bottom_right_position = (43.0, 8.5)

    # create map widget
    self.map_widget = tkintermapview.TkinterMapView(master=self.frame_carte, width=700, height=700)
    self.map_widget.grid(row=0, column=0, padx=10, pady=10)

    # set current widget position and zoom
    self.map_widget.fit_bounding_box(top_left_position, bottom_right_position)

  #Bouton quitter
  def quitter(self, event=None):
    self.destroy()


