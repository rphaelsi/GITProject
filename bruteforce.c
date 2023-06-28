#include "bruteforce.h"

int bruteforce_dico(char *zipname, char *wordlist)
{
    struct zip *zip = NULL;
    int err = 0;
    zip_error_t ziperror;
    clock_t debut, fin;
    double temps_execution;

    if (!zipname)
    {
        printf("Erreur: Pas de nom de fichier fourni.\n");
        return -1;
    }
    // Initialisation de la wordlist en read only
    FILE *wordlist_file = fopen(wordlist, "r");
    char line[MAX_LINE_SIZE];
    if (!wordlist_file)
    {
        printf("\nErreur: Impossible d'ouvrir la wordlist: %s\n", wordlist);
        return -1;
    }

    debut = clock();
    int i = 1;
    struct zip_file *zip_file = NULL;

    // Boucle qui itère sur l'ensemble des lignes de la wordlist
    while (fgets(line, sizeof(line), wordlist_file))
    {
        // Supprimer le retour chariot
        line[strcspn(line, "\n")] = '\0';
        printf("%s\n", line);
        zip = zip_open(zipname, 0, &err);

        if (!zip)
        {
            zip_error_init_with_code(&ziperror, err);
            printf("\nErreur: Impossible d'ouvrir le fichier zip: %s\n", zip_error_strerror(&ziperror));
            zip_error_fini(&ziperror);
            return -1;
        }
        // Tentative d'ouverture du fichier zip avec le mot de passe courant
        zip_file = zip_fopen_encrypted(zip, zip_get_name(zip, 0, 0), 0, line);
        struct zip_error *wrong_pass_code = zip_get_error(zip);
        // Récupération du code d'erreur pour vérifier si c'est le bon MDP
        if (zip_error_code_zip(wrong_pass_code) != ZIP_ER_WRONGPASSWD)
        {
            fin = clock();
            temps_execution = (double)(fin - debut) / CLOCKS_PER_SEC;
            printf("\nMot de passe trouvé: \033[32m%s\033[0m | Trouvé en : %.6f secondes\n", line, temps_execution);
            zip_close(zip);
            break;
        }
        zip_close(zip);
        i++;
    }

    fclose(wordlist_file);

    return 0;
}

int bruteforce(char *zipname)
{
    zip_t *zip = NULL;
    int err = 0;
    zip_error_t ziperror;
    clock_t debut, fin;
    double temps_execution;
    char *charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    int passwordLength = 6;
    int charsetSize = strlen(charset);
    int *indices = calloc(passwordLength, sizeof(int));
    char *password = malloc((passwordLength + 1) * sizeof(char));
    password[passwordLength] = '\0';

    if (!zipname)
    {
        printf("Erreur: Pas de nom de fichier fourni.\n");
        free(indices);
        free(password);

        return -1;
    }

    debut = clock();
    while (1)
    {
        for (int i = 0; i < passwordLength; i++)
        {
            password[i] = charset[indices[i]];
        }

        printf("%s\n", password);
        password[strcspn(password, "\n")] = '\0';
        // Ouverture du zip
        zip = zip_open(zipname, 0, &err);
        if (!zip)
        {
            zip_error_init_with_code(&ziperror, err);
            printf("\nErreur: Impossible d'ouvrir le fichier zip: %s\n", zip_error_strerror(&ziperror));
            zip_error_fini(&ziperror);
            return -1;
        }
        // Tentative d'ouverture du fichier avec le mot de passe
        zip_file_t *file = zip_fopen_encrypted(zip, zip_get_name(zip, 0, 0), 0, password);

        // Vérification de si l'on a bien reçu un pointeur sur un fichier zip en retour si oui c'est le bon mdp
        if (file)
        {

            fin = clock();
            temps_execution = (double)(fin - debut) / CLOCKS_PER_SEC;
            printf("\nMot de passe trouvé: \033[32m%s\033[0m | Trouvé en : %.6f secondes\n", password, temps_execution);
            zip_close(zip);
            break;
        }
        zip_close(zip);

        int i = passwordLength - 1;
        while (i >= 0 && indices[i] == charsetSize - 1)
        {
            indices[i] = 0;
            i--;
        }

        if (i < 0)
        {
            break;
        }
        indices[i]++;
    }

    free(indices);
    free(password);
    return 0;
}