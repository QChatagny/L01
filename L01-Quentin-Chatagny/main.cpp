// librairies maisons
#include "C:\Users\quent\CVM\PROG\lab1\cvm 23.h"
// librairies standard
#include <iostream>
#include <algorithm>
// librairies autres 
#include <conio.h>


// PRAGMA
#pragma warning (disable:6031)                                           // valeur de retour ignor�e '_getch'
#pragma warning (disable:26812)                                          // type enum 'type-name' est non d�limit�. Pr�f�rez 'enum class' � 'enum' (enum.3)

using namespace std;

// DIMENSION DU DAMIER

const size_t LIG = 8, COL = 12;                                          // lignes et colonnes du damier

// POSITION DU DAMIER � LA CONSOLE

const size_t START_X = 10;                                               // x du coin sup�rieur gauche du damier dans la console
const size_t START_Y = 5;                                                // y du coin sup�rieur gauche du damier dans la console

// CARACT�RISTIQUES DES CASES � LA CONSOLE

const size_t CASE_X = 6;                                                 // largeur d'une case � l'affichage, 2 minimum
const size_t CASE_Y = 3;                                                 // hauteur d'une case � l'affichage, 2 minimum

const size_t SPACE_X = 2;                                                // nombres de colonnes vides entre les cases, 1 minimum
const size_t SPACE_Y = 1;                                                // nombres de lignes vides entre les cases, 1 minimum

const size_t DELTA_X = CASE_X + SPACE_X;                                 // saut d'une case � l'autre, sur l'axe des X
const size_t DELTA_Y = CASE_Y + SPACE_Y;                                 // saut d'une case � l'autre, sur l'axe des Y

// DIMENSION DE LA FEN�TRE

const size_t WIN_X = 2 * START_X + (COL - 1) * DELTA_X + CASE_X;         // nombres de colonnes pour l'affichage d'une case
const size_t WIN_Y = 2 * START_Y + (LIG - 1) * DELTA_Y + CASE_Y;         // nombres de lignes   pour l'affichage d'une case

// CONFIGURATION DU JEU

const uint8_t DOLLARS_TOTAUX = 15;

// COMMANDES

enum class Arrowkeys                                                     // Code ascii d�cimal des touches fl�ch�es du clavier
{
    up_left = 71,
    up = 72,
    up_right = 73,
    left = 75,
    right = 77,
    down_left = 79,
    down = 80,
    down_right = 81,
};

using Ak = Arrowkeys;                                                    // un alias plus concis

enum class FlowControlInput                                              // Autres input acceptes
{
    null_char = 0,
    esc_char  = 224,
    exit     = 27,
	d        = 100
};

using KbIn = FlowControlInput;

// CASES, DAMIER ET TRANSFORMATIONS

enum Case { CO, CS, CD, CF, CV };                                        // les diff�rentes cases possibles: ordinaire, surprise, dollars, fragile, vide

Case damier[LIG][COL] =                                                  // le damier et ses cases initiales
{
        { CO, CO, CO, CO, CO, CO, CO, CO, CV, CO, CO, CS },
        { CO, CO, CV, CV, CO, CO, CO, CO, CO, CV, CO, CV },
        { CO, CO, CV, CS, CV, CO, CO, CO, CO, CO, CV, CS },
        { CO, CO, CV, CS, CV, CO, CO, CV, CV, CO, CV, CS },
        { CS, CO, CV, CV, CV, CS, CV, CO, CV, CO, CV, CO },
        { CS, CO, CS, CS, CO, CS, CV, CS, CV, CO, CV, CO },
        { CS, CO, CS, CO, CO, CO, CV, CV, CV, CO, CV, CO },
        { CS, CS, CO, CO, CO, CO, CO, CO, CO, CO, CO, CO }
};

Case futur[5] = { CF, CD, CF, CV, CV };                                  // la liste des transformations possibles  --  ex: futur[CO] ==> CF

// COOODONN�E LOGIQUE D'UN D�PLACEMENT

struct LC                                                                // ligne et colonne (l,c) d'une case du damier[l][c]
{
    size_t l, c;                                                         // ligne: [0..LIG-1] -- colonne: [0..COL-1]
};

struct Move                                                              // coordonn�es des 2 cases impliqu�es dans un d�placement
{
    LC from, to;
};

// AFFICHAGE DES CASES

struct XY                                                                // coordonn�e graphique (x,y) d'une case dans la console
{
    size_t x, y;
};

struct Style                                                            // le style pour l'affichage d'une case
{
    Color color; char c;                                                // le membre 'c' est le caract�re � utiliser pour l'affichage
};

Style map[5] =                                                          // les style pour toutes les cases
{
        { Color::blu, '\xB2' },                                         // ex: map[CO].c == '\xB2'                      ==> le caract�re � utiliser
        { Color::blu, '\xB2' },
        { Color::grn, '\x24' },                                         // ex: map[CD].color == Color::grn              ==> la couleur � utiliser
        { Color::pur, '\xB0' },
        { Color::blk, '\x00' }
};

// AFFICHAGE DU CURSEUR

char cursor[3][3] =                                                     // informations pour l'affichage du curseur
{
        { '\xC9', '\xCB', '\xBB' },
        { '\xCC', '\xCE', '\xB9' },
        { '\xC8', '\xCA', '\xBC' }
};

int toursJoues = 0;

    uint8_t dollars = 0;
void printDamier() {                                                     // pour débug
	for (int i = 0; i < LIG + 1; i++) {
		for (int j = 0; j <= COL; j++) {
			if (i == 0) {
				if (j == 0) { std::cout << "   "; }
				else {
					if (j < 10) { std::cout << " " << j << " "; }
					else { std::cout << j << " "; }
				}
			}
			else {
				if (j == 0) {
					if (i == 0) { std::cout << "   "; }
					else { std::cout << " " << i << " "; }
				}
				else {
					switch (damier[i - 1][ j - 1 ]) {
					case CO: {
						std::cout << "CO ";
					}break;
					case CS: {
						std::cout << "CS ";
					}break;
					case CV: {
						std::cout << ".. ";
					}break;
					case CF: {
						std::cout << "CF ";
					}break;
					case CD: {
						std::cout << "CD ";
					}break;
					}
				}
			}
		}
		std::cout << '\n';
	}
	std::cout << '\n';
	std::cout << "dollars: " << (int)dollars << '\n';
}

void debugMode(Move move) {
	size_t inputX = 0, inputY = 18;
	gotoxy(0, 12);
	char ce;
	do {
		std::cout << "Debug mode:\n 1) auto win (dol == 15)\n 2) change player position\n 3) select predefined position\n\n ";
		ce = _getch();
		switch (ce) {
		case '1': dollars = 15;
			break;
		case '2': {
			gotoxy(inputX, inputY);
			int ligne, colonne, bakLigne, bakColonne;

			std::cin >> ligne >> colonne;
			bakLigne = move.from.l; bakColonne = move.from.c;
			move.from.l = ligne; move.from.c = colonne;
		} break;
		}
	} while (1);
}


int main()
{
	setwsize(WIN_Y, WIN_X);                                              // redimensionner la fen�tre de la console
	clrscr();
    show(false);                                                         // afficher (oui/non) le trait d'affichage de la console

    Move m;
    m.from = { 0,0 };                                                    // coordonn�e logique {l,c} du curseur au d�part du jeu
	uint8_t deplacements = 0;
    bool running = true; 
    bool inBounds = true;
	bool inputValide = false;
    bool enferme = false;
    bool gagne = false;
    uint8_t c;

    while (running) {
		c = _getch();
		clrscr();

		if (c == (uint8_t)KbIn::null_char || c == (uint8_t)KbIn::esc_char) {
			if (_kbhit()) {
				c = _getch();
				switch ((Ak)c) {

					case Ak::up: {
						if (!(m.to.l == 0)) {
							m.to.l = m.from.l - 1;
							m.to.c = m.from.c;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::down: {
						if (!(m.to.l == LIG - 1)) {
							m.to.l = m.from.l + 1;
							m.to.c = m.from.c;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::left: {
						if (!(m.from.c == 0)) {
						m.to.l = m.from.l;
						m.to.c = m.from.c - 1;
						inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::right: {
						if (!(m.from.c == COL - 1)) {
							m.to.l = m.from.l;
							m.to.c = m.from.c + 1;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::up_left: { 
						if (!(m.to.l == 0 && m.to.c == 0)) {
							m.to.l = m.from.l - 1;
							m.to.c = m.from.c - 1;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::up_right: { 
						if (!(m.to.l == 0 && m.to.c == COL - 1)) {
							m.to.l = m.from.l - 1;
							m.to.c = m.from.c + 1;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::down_left: {
						if (!(m.to.l == LIG - 1 && m.to.c == 0)) {
							m.to.l = m.from.l + 1;
							m.to.c = m.from.c - 1;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

					case Ak::down_right: { 
						if (!(m.to.l == LIG - 1 && m.to.c == COL - 1)) {
							m.to.l = m.from.l + 1;
							m.to.c = m.from.c + 1;
							inputValide = true;
						}
						else { inBounds = false; }
					} break;

				}
			}
			if (inputValide && inBounds) { 	                         // ce que l'on fait avec chaque type de case darrivee		
				switch (damier[m.to.l][m.to.c]) {
					case CO: case CS: case CF: {
						damier[m.to.l][m.to.c] = futur[damier[m.to.l][m.to.c]];
						deplacements++;
					}
						   break;

					case CD: {
						damier[m.to.l][m.to.c] = futur[damier[m.to.l][m.to.c]];
						dollars++;
						deplacements++;
					}
						   break;

					//case CV: {                                       // la case est vide, le déplacement est annulé
					//	m.to.l = m.from.l;
					//	m.to.c = m.from.c;
					//}
					//   break;
				}

				for (int8_t deltaL = -1; deltaL <= 1; deltaL++) {                                                                   // est-on enffermé
					for (int8_t deltaC = -1; deltaC <= 1; deltaC++) { // utilise une autre variable temp signée pour eviter l'overflow lorsque l'on accede au move
						int8_t checkL = m.to.l + deltaL;
						int8_t checkC = m.to.c + deltaC;

						if (checkL < 0 || checkC < 0 || checkL >= LIG || checkC >= COL || (checkL == m.to.l && checkC == m.to.c)) { // elimine les valeurs negatives ou superieures 
							// aux limites du damier et la case ou le joueur est situe
						}
						else {
							if (damier[checkL][checkC] == CO || damier[checkL][checkC] == CS || damier[checkL][checkC] == CD || damier[checkL][checkC] == CF ){
								enferme = false;
								break;                                                                                              // des qu'il y a une case active cest assez 
							}
							else {
								enferme = true;
							}
						}
					}
				}
				m.from = m.to;
			}
		}
		else {
			switch ((KbIn)c) {
				case KbIn::exit: {
					running = false;
				} break;
				case KbIn::d :{
				   debugMode(m);
				}
			}
		}

		printDamier();

		if (dollars == DOLLARS_TOTAUX) {
			running = false;
			gagne = true;
			break;
		} else if (enferme == true) {
			running = false;
			break;
		}
		/*
				NOTE 3)
						Utilisez le calcul �nonc� dans la sp�cification au point 10) pour retrouver la coordonn�e graphique (x,y) d'une case � partir de sa coordonn�e logique (l,c)
		*/
	    } 
	if (gagne) {
		std::cout << "Wow felicitation";
		return -2;
	}
	else {
		std::cout << "Perdu! a la position"<< m.from.l << " , " << m.from.c;
		return -1;
	}
	return 0;
}

// -TODO
// temps de jeu
// inclure la verification des limites exterieures dans le switch suivant, evite de multiplier lacces a la memoire
// interface graphique
// clean up variables, ranger tout le code, faire ca propre quoi


// -TESTS 
// out of bounds [x]
// inputs non acceptes [x] 
// gagner [x]
// perdre (enferme) [x]
// quitter le jeu [x]
// 
