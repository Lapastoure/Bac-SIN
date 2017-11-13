int E1 = 4 ;  			// Pin sens rotation du moteur : HIGH (fermeture) ou LOW (ouverture)
int M1 = 5 ; 			// Pin controle PWM (0 à 255) (On prendra 255, soit la vitesse maximale)
int mode = 1 ; 			// Mode : jour = 1 ou nuit = 0
int TC ; 				// Temperature de consigne [°C] (depend de mode)
int TEMP_m = 5 ; 		// Température mesurée [°C] (Peut être fixée pour les tests si absence de capteur)
int t1 ; 				// Début du chronomètre (début rotation moteur) [ms]
int t2 ; 				// Fin du chronomètre (fin rotation moteur) [ms]
int Dt ; 				// Intervalle de temps (temps de rotation moteur) [ms]
int OuvertureMax ; 		// Temps pour passer d'une position extreme de la vanne à l'autre [ms]
int OuvertureVanne ; 	// Etat d'ouverture de la vanne [ms] (Plus c'est grand, plus c'est ouvert) (Plus c'est petit, plus c'est fermé) (entre 0 et OuvertureMax)
float Relation ; 		// Relation entre temps de rotation moteur et température [ms/°C]
char surtension ; 		// Variable qui détecte la surtension du moteur (ici envoie de la lettre "g")

//////////////////////////////////////////
// INITIALISATION OUVERTURE DE LA VANNE //
//////////////////////////////////////////

// 1. Initialisation des pins et communication série
// 2. Initialisation ouverture de la vanne: On part de d'un état de fermeture maximale jusqu'à atteindre l'ouverture maximale (butée => surtension)
// 3. Réglage de l'ouverture de la vanne en fonction du mode de fonctionnement détecté

void setup() {	// Début de la fonction principale pour l'initialisation

// INITIALISATION PIN ET COMMUNICATION

	pinMode(E1,OUTPUT) ; 	// Réglage du pin en mode sortie (sens rotation moteur)
	pinMode(M1,OUTPUT) ; 	// Réglage du pin en mode sortie (vitesse rotation moteur)
	Serial.begin(9600) ; 	// Initialisation de la communication série
	
// INITIALISATION OUVERTURE DE LA VANNE

    Serial.println("Initialisation de la vanne") ;	// Affichage
    digitalWrite(E1,LOW) ; 							// On part d'une position fermée et on veut atteindre la position ouverte (SENS NEGATIF = OUVERTURE)
    analogWrite(M1,255) ;	 						// Debut rotation du moteur (vitesse maximale)
	
	t1 = millis() ;		// Début du chronomètre (début rotation moteur)
	
	while ( true ) {																			// Boucle jusqu'à sortie
		if ( Serial.available() > 0 ) { 														// Si données disponibles
			surtension = Serial.read(); 														// Alors on les récupére
			if ( surtension == 'g' ) { 															// Si on détecte une surtension (lettre "g" envoyée)
				analogWrite(M1,0) ; 															// Arret rotation du moteur
				t2 = millis() ; 																// Fin du chronomètre (arret rotation moteur)
				Serial.println("Le moteur s'est arrété, car une surtension a été détectée.") ; 	// Affichage
				break ;																			// Sortie de la boucle		
			}
		}
	}
	
	Dt = t2 - t1 ; 													// Intervalle de temps [ms] pendant lequel le moteur a tourné avant d'atteindre la butée
	OuvertureMax = Dt ;												// On définit alors l'ouverture maximale
	Relation = OuvertureMax/(30-7) ; 								// On en déduit la relation entre temps de rotation et température
	OuvertureVanne = Dt ; 											// La vanne est ouverture au maximum
	
	Serial.println("Vanne Ouverte, ouverture maximale : [ms]"); 	// Affichage 
	Serial.println(OuvertureMax) ;									// Affichage
	
// REGLAGE DE L'OUVERTURE DE LA VANNE EN FONCTION DU MODE CHOISI

    if ( mode = 1 ) {													// Si on est en mode jour (1)
		TC = 20 ; 														// Temperature de consigne de 20°C
		Serial.println("Mode jour détecté, fermeture de la vanne.") ;	// Affichage
	}
	
    if ( mode = 0 ) {													// Si on est en mode nuit (0)
		TC = 17 ; 														// Temperature de consigne de 17°C
		Serial.println("Mode nuit détecté, fermeture de la vanne.") ;	// Affichage
	}
	
	digitalWrite(E1,HIGH) ;									// On passe de la position ouverte au maximum vers une position intermédiaire (SENS POSITIF = FERMETURE)
	analogWrite(M1,255) ;									// Début rotation du moteur (vitesse maximale)
	delay(OuvertureVanne - (TC-7)*Relation) ;				// Temps de rotation du moteur pour atteindre la position intermédiaire
	analogWrite(M1,0) ; 									// Arret rotation du moteur (vitesse nulle) 
	
	OuvertureVanne = (TC-7)*Relation ; 						// Valeur de l'ouverture de la vanne = (température consigne - température mini) x relation entre temps et température
	
	Serial.println("Etat d'ouverture de la vanne : [%] ") ;	// Affichage
	Serial.println(OuvertureVanne/OuvertureMax*100) ;		// Affichage
	
}

///////////////////////////////////////////
// BOUCLE POUR REGULATION DE TEMPERATURE //
///////////////////////////////////////////

// 1. Mesure de la température
// 2. Fermeture si température trop élevée
// 3. Ouverture si température trop basse
// 4. Temporisation
	
void loop() {																							// Début de la boucle principale pour la régulation
	
// MESURE DE LA TEMPERATURE

		Serial.print("Température mesurée = "  ) ;														// Affichage
		Serial.println(TEMP_m ) ;																		// Affichage

// TEMPERATURE TROP ELEVEE

	if ( TEMP_m > (TC + 1) ) { 																			// Si température mesurée trop elevée

        Serial.println("Température mesurée trop élevée par rapport à la température de consigne.") ;	// Affichage
        Serial.println("Fermeture de la vanne pour regulation de temperature de 3°C.") ;				// On veut fermer mais pas totalement => Baisser de "3°C"
		
        if ( OuvertureVanne > (0 + Relation*3) ) {														// Si la vanne est assez ouverte pour permettre de la fermer de "3°C"
			digitalWrite(E1,HIGH) ;																		// Sens positif pour fermeture
			analogWrite(M1,255) ;																		// Début rotation moteur (vitesse maximale)
			delay(Relation*3) ;																			// Temps de rotation du moteur assez long pour "3°C"
			analogWrite(M1,0) ;																			// Arret rotation moteur (vitesse nulle)
			OuvertureVanne = OuvertureMax - Relation*3 ;												// La vanne est moins ouverte de l'équivalent de "3°C" 
			Serial.println("Etat d'ouverture de la vanne : [%] ") ;										// Affichage
			Serial.println(OuvertureVanne/OuvertureMax*100) ;											// Affichage
		}
		
		else if ( OuvertureVanne <= (0 + Relation*3) ) {												// Si la vanne n'est pas assez ouverte pour mettre de la ferme de "3°C"	
			digitalWrite(E1,HIGH) ;																		// Sens positif pour fermeture
			analogWrite(M1,255) ;																		// Début rotation moteur (vitesse maximale)
			delay(0 + OuvertureVanne) ;																	// Temps de rotation moteur assez long pour atteindre la fermeture maximale
			analogWrite(M1,0) ;																			// Arret rotation moteur (vitesse nulle)
			OuvertureVanne = 0 ;																		// La vanne est donc totalement fermée
			Serial.println("Vanne fermée au maximum.") ;												// Affichage
		}
		
		else if ( OuvertureVanne = 0 ) {																// Si la vanne est totalement fermée
			Serial.println("Vanne déjà fermée au maximum.") ;											// Affichage
		}
		
    }
