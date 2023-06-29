#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Permet d'utiliser getopt et d'utiliser la méthode access.
#include "manip.h"
#include "bruteforce.h"
#include "navigate.h"

int main(int argc, char *argv[])
{
    zip_t *zip = NULL;
    int num_entries = 0;
    int opt;
    char *archive_file_PATH = NULL;
    char *archive_PATH = NULL;
    char *system_TARGET = NULL;
    char *password = NULL;
    char *wordlist_PATH = NULL;
    char *navigate_PATH = NULL;

    // Parcourir les options de ligne de commande
    while ((opt = getopt(argc, argv, "hb:d:e:i:o:p:v:n:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            // Option -h pour afficher l'aide
            printf("Page d'aide \n");
            printf("\n");
            printf("-h, Permet d'afficher la liste des différentes options qui vont suivrent.\n");
            printf("-b, Permet de bruteforce une archive ZIP, il faut simplement fournir le nom/chemin de l'archive.\n");
            printf("-d, Permet de bruteforce mais avec une Wordlist, il faut fournir le nom/chemin de l'archive ainsi que celui de la Wordlist.");
            printf("-e, Permet d'exporter un fichier de l'archive, il faut fournir le nom/chemin de l'archive, le nom/chemin du fichier à extraire de l'archive et le chemin cible sur le système cible.\n");
            printf("-i, Permet d'importer un fichier dans l'archive, il faut fournir le nom/chemin du fichier à ajouter et le nom/chemin de l'archive. \n");
            printf("-n, Permet de naviger dans l'arborescence d'une archive en précisant le nom/chemin de l'archive. Vous pouvez également ouvrir une archive avec mot de passe, option -p suivi du mot de passe.\n");
            printf("\n");
            printf("Pour les fonctions qui peuvent accueillir plusieurs arguments, vous devez les séparer par une simple virgule. \n");
            exit(0);
        case 'b':
            // Option -b sans argument
            archive_PATH = optarg;
            printf("Option -b spécifiée avec l'argument : %s\n", archive_PATH);
            printf("%d\n", opt);
            // Code pour traiter l'option -b
            bruteforce(archive_PATH);
            break;
        case 'd':
            printf("Option -d spécifiée avec l'argument : %s\n", optarg);
            // Code pour traiter l'option -d avec l'argument optarg
            char *token = strtok(optarg, ",");
            if (token)
            {
                // La première partie est l'archive_PATH
                archive_PATH = strdup(token);

                // Utiliser strtok à nouveau pour obtenir la deuxième partie
                token = strtok(NULL, ",");
                if (token)
                {
                    wordlist_PATH = strdup(token);

                    printf("archive_PATH : %s\n", archive_PATH);
                    printf("wordlist_PATH : %s\n", wordlist_PATH);
                    bruteforce_dico(archive_PATH, wordlist_PATH);
                }
                else
                {
                    printf("Erreur : Deuxième partie manquante.\n");
                }
            }
            else
            {
                printf("Erreur : La chaîne ne contient pas de virgule.\n");
            }

            // Libérer la mémoire allouée avec strdup
            free(archive_PATH);
            free(wordlist_PATH);
            break;
        case 'e':

        {
            char *token = strtok(optarg, ",");
            int argCount = 0;
            while (token != NULL && argCount < 3)
            {
                if (argCount == 0)
                {
                    archive_PATH = token;
                }
                else if (argCount == 1)
                {
                    archive_file_PATH = token;
                }
                else if (argCount == 2)
                {
                    system_TARGET = token;
                }
                argCount++;
                token = strtok(NULL, ",");
            }
        }

            printf("\n");

            // Vérification de l'entrée utilisateur.
            int lenght = strlen(archive_PATH);

            // La fonction "strcmp()" permet de comparer deux chaînes de caractères.
            if (lenght >= 4 && strcmp(archive_PATH + lenght - 4, ".zip") == 0)
            {
                printf("L'entrée est un nom d'archive ZIP valide.\n");
            }
            else
            {
                // Vérification si l'entrée est un chemin valide
                // On peut ajouter ici d'autres conditions spécifiques pour le chemin
                printf("L'entrée n'est pas un nom d'archive ZIP ni un chemin valide.\n");
                return 0;
            }

            // On vérifie l'entrée utilisateur
            size_t len = strlen(system_TARGET);
            if (len > 0 && system_TARGET[len - 1] == '/')
            {
                printf("L'entrée utilisateur est un chemin valide se terminant par \"/\".\n");
                // On vérifie si le chemin donnée par l'utilisateur se termine par "/"
            }
            else
            {
                printf("L'entrée utilisateur n'est pas un chemin valide se terminant par \"/\".\n");
                printf("Veuillez saisir un chemin se terminant par \"/\". \n");
                return 0;
            }

            // Vérification de l'entrée utilisateur.
            // La fonction "access" permet de vérifier l'accès à un fichier, dans cette situation.
            // On donne le fichier en argument mais aussi la méthode "F_OK" qui vérifie simplement l'existence du fichier.
            if (access(archive_file_PATH, F_OK) == 0)
            {
                printf("Le fichier spécifié est valide.\n");
            }
            else
            {
                printf("Erreur dans le fichier spécifié, vérifier si il existe ou si il n'y a pas d'erreurs dans son nom \n");
                return 0;
            }

            // Supprimer le saut de ligne à la fin de l'entrée utilisateur.
            system_TARGET[strcspn(system_TARGET, "\n")] = '\0';

            char *nom_fichier = strrchr(archive_file_PATH, '/');

            // La fonction "strrchr()" permet de recherche un élément dans une chaîne de caractères.
            if (nom_fichier != NULL)
            {
                // Si '/' est trouvé, avancer d'un caractère pour obtenir le nom du fichier
                nom_fichier++;
            }
            else
            {
                // Si '/' n'est pas trouvé, le nom du fichier est déjà dans Fichier
                nom_fichier = archive_file_PATH;
            }

            // Concaténation du nom de fichier à extraire avec le chemin cible. Ansi on va retrouver le fichier à l'endroit indiqué.
            strcat(system_TARGET, nom_fichier);

            printf("\n");
            extracte(archive_PATH, archive_file_PATH, system_TARGET);

            printf("\n");

            // On propose à l'utilisateur de supprimer le fichier résiduel dans l'archive
            char answer;
            printf("Nous avons extrait le fichier de l'archive mais il est toujours présent dans l'archive.\n");
            printf("Voulez-vous supprimer le fichier residuel dans l'archive ? \n");
            printf("1. Oui\n");
            printf("2. Non\n");
            printf(">");
            scanf("%s", &answer);
            getchar();

            if (answer == '1')
            {
                delete_file_archive(archive_PATH, archive_file_PATH);
            }
            else
            {
                return 0;
            }

            break;

        case 'i':

        {
            char *token = strtok(optarg, ",");
            int argCount = 0;
            while (token != NULL && argCount < 2)
            {
                if (argCount == 0)
                {
                    archive_PATH = token;
                }
                else if (argCount == 1)
                {
                    archive_file_PATH = token;
                }
                argCount++;
                token = strtok(NULL, ",");
            }
        }

            printf("\n");

            // Supprimer le saut de ligne à la fin de la chaine.
            archive_PATH[strcspn(archive_PATH, "\n")] = '\0';

            // Vérification de l'entrée utilisateur.
            int leng = strlen(archive_PATH);

            // La fonction "strcmp()" permet de comparer deux chaînes de caractères.
            if (leng >= 4 && strcmp(archive_PATH + leng - 4, ".zip") == 0)
            {
                printf("L'entrée est un nom d'archive ZIP valide.\n");
            }
            else
            {
                // Vérification si l'entrée est un chemin valide
                // On peut ajouter ici d'autres conditions spécifiques pour le chemin
                printf("L'entrée n'est pas un nom d'archive ZIP ni un chemin valide.\n");
                return 0;
            }

            // Vérification de l'entrée utilisateur.
            // La fonction "access" permet de vérifier l'accès à un fichier, dans cette situation.
            // On donne le fichier en argument mais aussi la méthode "F_OK" qui vérifie simplement l'existence du fichier.
            if (access(archive_file_PATH, F_OK) == 0)
            {
                printf("Le fichier spécifié est valide.\n");
            }
            else
            {
                printf("Erreur dans le fichier spécifié, vérifier si il existe ou si il n'y a pas d'erreurs dans son nom \n");
                return 0;
            }

            printf("\n");
            importe(archive_file_PATH, archive_PATH);

            break;
        case 'n':
            // Option -n pour le chemin du fichier à naviguer
            navigate_PATH = optarg;
            printf("Option -n spécifiée avec l'argument : %s\n", navigate_PATH);
            if(navigate_PATH != NULL)
            {
                if (open_zip(navigate_PATH, password) != 0) {
                    return -1;
                }

                num_entries = zip_get_num_entries(zip, 0);
                interactive_session();

                // Fermer le fichier ZIP
                zip_close(zip);
            }
            else
            {
                printf("Aucun chemin de fichier fourni pour la navigation.\n");
            }
            break;
        case '?':
            // Option non reconnue ou argument manquant
            printf("Option non reconnue ou argument manquant.\n");
            exit(1);
        }
    }

    return 0;
}
