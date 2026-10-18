#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <iomanip>
#include <fstream>
#include <random>

using namespace std;
using namespace chrono;

struct Resultat {
    string sousChaine;
    long long comparaisons = 0;
};

// 1. Generer un fichier contenant des mots aleatoires
bool genererFichierMots(const string& nomFichier,
                       size_t nombreMots) {
    ofstream fichier(nomFichier);

    if (!fichier.is_open()) {
        cerr << "Erreur : impossible de creer "
             << nomFichier << endl;
        return false;
    }

    random_device rd;
    mt19937 gen(rd());

    // Longueur aleatoire entre 5 et 15 caracteres
    uniform_int_distribution<int> longueurDist(5, 15);

    // Lettres minuscules de a a z
    uniform_int_distribution<int> lettreDist(0, 25);

    for (size_t i = 0; i < nombreMots; ++i) {
        int longueur = longueurDist(gen);
        string mot;

        for (int j = 0; j < longueur; ++j) {
            char lettre = char('a' + lettreDist(gen));
            mot += lettre;
        }

        fichier << mot << '\n';
    }

    fichier.close();

    cout << "Fichier genere : " << nomFichier << endl;
    cout << "Nombre de mots generes : "
         << nombreMots << endl;

    return true;
}

// 2. Construction du tableau LPS utilise par KMP
vector<size_t> construireLPS(const string& motif,
                             long long& comparaisons) {
    vector<size_t> lps(motif.size(), 0);

    size_t longueur = 0;
    size_t i = 1;

    while (i < motif.size()) {
        ++comparaisons;

        if (motif[i] == motif[longueur]) {
            lps[i] = longueur + 1;
            ++longueur;
            ++i;
        }
        else if (longueur != 0) {
            longueur = lps[longueur - 1];
        }
        else {
            lps[i] = 0;
            ++i;
        }
    }

    return lps;
}

// 3. Recherche d'un motif dans un texte avec KMP
bool rechercherKMP(const string& texte,
                   const string& motif,
                   long long& comparaisons) {
    if (motif.empty())
        return true;

    if (texte.size() < motif.size())
        return false;

    vector<size_t> lps =
        construireLPS(motif, comparaisons);

    size_t i = 0;
    size_t j = 0;

    while (i < texte.size()) {
        ++comparaisons;

        if (texte[i] == motif[j]) {
            ++i;
            ++j;

            if (j == motif.size())
                return true;
        }
        else if (j != 0) {
            j = lps[j - 1];
        }
        else {
            ++i;
        }
    }

    return false;
}

// 4. Recherche de la plus longue sous-chaine
// commune a au moins deux mots differents
Resultat rechercher(const vector<string>& mots) {
    Resultat resultat;

    if (mots.size() < 2)
        return resultat;

    size_t longueurMax = 0;

    for (const string& mot : mots) {
        if (mot.size() > longueurMax)
            longueurMax = mot.size();
    }

    // Tester les longueurs de la plus grande a la plus petite
    for (size_t longueur = longueurMax;
         longueur > 0;
         --longueur) {

        // Memoriser le premier mot contenant chaque candidate
        unordered_map<string, size_t> premierMot;

        for (size_t i = 0; i < mots.size(); ++i) {
            const string& mot = mots[i];

            if (mot.size() < longueur)
                continue;

            for (size_t debut = 0;
                 debut + longueur <= mot.size();
                 ++debut) {

                string candidate =
                    mot.substr(debut, longueur);

                auto it = premierMot.find(candidate);

                if (it == premierMot.end()) {
                    premierMot.emplace(candidate, i);
                }
                else if (it->second != i) {
                    if (rechercherKMP(
                            mots[it->second],
                            candidate,
                            resultat.comparaisons)) {

                        resultat.sousChaine = candidate;
                        return resultat;
                    }
                }
            }
        }
    }

    return resultat;
}

// 5. Lire les mots depuis le fichier
vector<string> lireFichierMots(const string& nomFichier) {
    vector<string> mots;
    ifstream fichier(nomFichier);
    string mot;

    if (!fichier.is_open()) {
        cerr << "Erreur : impossible de lire "
             << nomFichier << endl;
        return mots;
    }

    while (fichier >> mot) {
        mots.push_back(mot);
    }

    fichier.close();
    return mots;
}

// 6. Programme principal
int main() {
    const string nomFichier = "mots.txt";
    const size_t nombreMots = 100000;

    // Etape 1 : generer le fichier aleatoire
    if (!genererFichierMots(nomFichier, nombreMots)) {
        return 1;
    }

    // Etape 2 : lire le fichier genere
    vector<string> mots = lireFichierMots(nomFichier);

    if (mots.size() != nombreMots) {
        cerr << "Erreur : nombre de mots incorrect." << endl;
        cerr << "Nombre lu : " << mots.size() << endl;
        return 1;
    }

    cout << "Nombre de mots lus : "
         << mots.size() << endl;

    // Etape 3 : lancer la recherche et mesurer son temps
    cout << "\nRecherche avec KMP en cours..." << endl;
    cout << "Veuillez patienter." << endl;

    auto debut = high_resolution_clock::now();

    Resultat r = rechercher(mots);

    auto fin = high_resolution_clock::now();

    double temps = duration<double, milli>(
        fin - debut
    ).count();

    // Etape 4 : afficher les resultats
    cout << "\n===== RESULTATS =====" << endl;

    cout << "Plus longue sous-chaine commune : "
         << (r.sousChaine.empty()
             ? "(aucune)"
             : r.sousChaine)
         << endl;

    cout << "Longueur : "
         << r.sousChaine.size()
         << endl;

    cout << "Comparaisons de caracteres dans KMP : "
         << r.comparaisons
         << endl;

    cout << fixed << setprecision(3);

    cout << "Temps de recherche : "
         << temps << " ms" << endl;

    cout << "\nFichier utilise : " << nomFichier << endl;

    return 0;
}
