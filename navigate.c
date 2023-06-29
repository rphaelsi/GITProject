#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zip.h>
#include <libgen.h>
#include <stdbool.h>
#include "navigate.h"

#define PATH_MAX 4096
#define COMMAND_MAX 256
#define MAX_DEPTH 100

char current_dir[PATH_MAX] = "";
char command[COMMAND_MAX];

zip_t *zip = NULL;
zip_int64_t num_entries = 0;

// Fonction pour ouvrir le fichier ZIP
int open_zip(const char *zipname, const char *password) {
    int err = 0;
    zip_error_t ziperror;

    if (!zipname) {
        printf("Erreur: Pas de nom de fichier fourni.\n");
        return -1;
    }

    zip = zip_open(zipname, 0, &err);

    if (!zip) {
        zip_error_init_with_code(&ziperror, err);
        printf("Erreur: Impossible d'ouvrir le fichier zip: %s\n", zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return -1;
    }

    if (password) {
        // Check password by trying to extract first file
        zip_file_t *file = zip_fopen_encrypted(zip, zip_get_name(zip, 0, 0), 0, password);
        if (file == NULL) {
            printf("Erreur: Impossible d'ouvrir le fichier dans l'archive avec le mot de passe fourni\n");
            zip_close(zip);
            zip = NULL;
            return -1;
        }

        // Buffer for reading file
        char buffer[4096];
        if (zip_fread(file, buffer, sizeof(buffer)) < 0) {
            printf("Erreur: Mot de passe incorrect.\n");
            zip_fclose(file);
            zip_close(zip);
            zip = NULL;
            return -1;
        }

        zip_fclose(file);
    }

    printf("Fichier ZIP ouvert: %s\n", zipname);
    return 0;
}

// Changer le dossier actuel
int change_dir(const char *dir) {
    // Si la commande est 'cd ..', remonter au dossier parent
    if(strcmp(dir, "..") == 0) {
        // Si current_dir est déjà une chaîne vide (nous sommes déjà à la racine)
        if(strcmp(current_dir, "") == 0) {
            // Ne fait rien, nous ne pouvons pas remonter plus haut que la racine
        } else {
            // Trouve la dernière occurrence de '/' dans current_dir
            char *last_slash = strrchr(current_dir, '/');

            // S'il y a un slash (il devrait toujours y en avoir un), le coupe
            if(last_slash != NULL) {
                *last_slash = '\0'; // Cela termine la chaîne à l'emplacement du dernier '/'
                
                // Maintenant, trouvez l'avant-dernier '/'
                last_slash = strrchr(current_dir, '/');
                
                // Si nous trouvons un avant-dernier '/', coupe la chaîne là aussi
                if(last_slash != NULL) {
                    *(last_slash + 1) = '\0'; // Cela laisse le slash dans la chaîne, mais coupe tout ce qui vient après
                } else {
                    // Si nous ne trouvons pas d'avant-dernier '/', cela signifie que nous retournons à la racine
                    strcpy(current_dir, "");
                }
            }
        }
    } else{

    // Création d'un nouveau chemin à partir du chemin actuel et du nom du dossier
    char new_path[PATH_MAX] = {0};
    char new_path_with_slash[PATH_MAX] = {0};

    // Verifie si le chemin actuel est vide
    if (strcmp(current_dir, "") == 0) {
    snprintf(new_path, PATH_MAX, "%s", dir);
    snprintf(new_path_with_slash, PATH_MAX, "%s/", dir);
    } else {
        snprintf(new_path, PATH_MAX, "%s/%s", current_dir, dir);
        snprintf(new_path_with_slash, PATH_MAX, "%s/%s/", current_dir, dir);
    }

    // Vérifie si le nouveau chemin existe
    printf("Checking path: '%s'\n", new_path);
    
    zip_int64_t index = zip_name_locate(zip, new_path, ZIP_FL_ENC_GUESS);
    if(index < 0) {
        // Si le chemin sans slash ne peut pas être trouvé, essayez avec le slash
        index = zip_name_locate(zip, new_path_with_slash, ZIP_FL_ENC_GUESS);
        if (index < 0) {
            printf("Erreur: Le chemin '%s' n'existe pas dans l'archive.\n", new_path);
            return -1;
        } else {
            // Si le chemin avec slash a été trouvé, mettez à jour new_path
            strncpy(new_path, new_path_with_slash, PATH_MAX);
        }
    }

    if (new_path[strlen(new_path) - 1] != '/') {
        strcat(new_path, "/");
    }
    strcpy(current_dir, new_path);

    }

    return 0;
}


void list_files() {
    // Le nombre total d'entrées dans l'archive ZIP
    zip_int64_t num_entries = zip_get_num_entries(zip, 0);

    // Longueur du répertoire courant
    int directory_length = strlen(current_dir);

    // Tableau pour stocker les noms de dossiers déjà imprimés
    char **printed_directories = NULL;
    int printed_directories_count = 0;

    // Parcourir toutes les entrées
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(zip, i, ZIP_FL_UNCHANGED);

        // Vérifiez si le nom commence par le répertoire courant
        if (strncmp(name, current_dir, directory_length) == 0) {
            // Récupérer le chemin restant après le répertoire courant
            const char *remaining_path = name + directory_length;

            // Vérifiez si le chemin restant est un fichier/dossier direct (c'est-à-dire, il ne contient pas '/')
            char *next_slash = strchr(remaining_path, '/');

            // S'il n'y a pas de slash suivant, imprimez le nom du fichier
            if (next_slash == NULL) {
                printf("%s\n", remaining_path);
            }
            else {
                // S'il y a un slash suivant, récupérez le nom du dossier
                char *dir_name = strndup(remaining_path, next_slash - remaining_path);

                // Vérifiez si le dossier a déjà été imprimé
                bool already_printed = false;
                for (int j = 0; j < printed_directories_count; j++) {
                    if (strcmp(dir_name, printed_directories[j]) == 0) {
                        already_printed = true;
                        break;
                    }
                }

                // Si le dossier n'a pas été imprimé, imprimez-le et ajoutez-le à la liste
                if (!already_printed) {
                    printf("%s/\n", dir_name);
                    printed_directories = realloc(printed_directories, sizeof(char*) * (printed_directories_count + 1));
                    printed_directories[printed_directories_count] = strdup(dir_name);
                    printed_directories_count++;
                }

                free(dir_name);
            }
        }
    }

    // Libérez la mémoire
    for (int i = 0; i < printed_directories_count; i++) {
        free(printed_directories[i]);
    }
    free(printed_directories);
}

// Fonction Extracte pour récuperer des documents dans l'archive.
int extract(const char *filename) {
    // Concaténer le chemin actuel avec le nom du fichier
    char fullpath[PATH_MAX] = {0};
    if (strcmp(current_dir, "") == 0) {
        snprintf(fullpath, PATH_MAX, "%s", filename);
    } else {
        snprintf(fullpath, PATH_MAX, "%s%s", current_dir, filename);
    }
    // Vérifier si le chemin complet existe dans l'archive ZIP
    zip_int64_t index = zip_name_locate(zip, fullpath, ZIP_FL_ENC_GUESS);
    if (index < 0) {
        printf("Erreur: Le fichier '%s' n'existe pas dans l'archive.\n", fullpath);
        return -1;
    }
    // Vérifier si le chemin complet est un répertoire
    zip_stat_t sb;
    if (zip_stat_index(zip, index, 0, &sb) == 0) {
        if (sb.name[strlen(sb.name) - 1] == '/') {
            printf("Erreur: '%s' est un répertoire, non un fichier.\n", fullpath);
            return -1;
        }
    }
    // Ouvrir le fichier dans l'archive ZIP
    zip_file_t *file = zip_fopen_index(zip, index, 0);
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier '%s' dans l'archive.\n", fullpath);
        return -1;
    }
    // Lire le fichier et l'écrire dans un nouveau fichier sur le système de fichiers
    char buffer[4096];
    zip_int64_t bytes_read;
    FILE *out = fopen(filename, "wb");
    if (!out) {
        printf("Erreur: Impossible de créer le fichier '%s'.\n", filename);
        zip_fclose(file);
        return -1;
    }
    while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, out);
    }
    // Fermer les fichiers et confirmer que l'extraction a été réussie
    fclose(out);
    zip_fclose(file);
    printf("Fichier '%s' extrait avec succès.\n", filename);

    return 0;
}

// Fonction pour créer un nouveau fichier dans l'archive.
int touch(const char *filename) {
    // Concaténer le chemin actuel avec le nom du fichier
    char fullpath[PATH_MAX] = {0};
    if (strcmp(current_dir, "") == 0) {
        snprintf(fullpath, PATH_MAX, "%s", filename);
    } else {
        snprintf(fullpath, PATH_MAX, "%s%s", current_dir, filename);
    }
    // Vérifier si le chemin complet existe déjà dans l'archive ZIP
    zip_int64_t index = zip_name_locate(zip, fullpath, ZIP_FL_ENC_GUESS);
    if (index >= 0) {
        printf("Erreur: Le fichier '%s' existe déjà dans l'archive.\n", fullpath);
        return -1;
    }
    // Créer un nouveau fichier avec un contenu vide
    zip_source_t *src = zip_source_buffer(zip, NULL, 0, 0);
    if (src == NULL) {
        printf("Erreur: Impossible de créer une nouvelle source pour le fichier '%s'.\n", fullpath);
        return -1;
    }
    index = zip_file_add(zip, fullpath, src, ZIP_FL_OVERWRITE);
    if (index < 0) {
        zip_source_free(src);
        printf("Erreur: Impossible d'ajouter le fichier '%s' à l'archive.\n", fullpath);
        return -1;
    }
    printf("Fichier '%s' créé avec succès.\n", filename);
    return 0;
}

// Fonction pour ajouter un fichier à l'archive ZIP
int import(const char *filename) {
    // Préparer le chemin complet du nouveau fichier dans l'archive
    char fullpath[PATH_MAX] = {0};
    if (strcmp(current_dir, "") == 0) {
        snprintf(fullpath, PATH_MAX, "%s", filename);
    } else {
        snprintf(fullpath, PATH_MAX, "%s%s", current_dir, filename);
    }
    // Vérifiez si le fichier existe déjà dans l'archive
    zip_int64_t index = zip_name_locate(zip, fullpath, ZIP_FL_ENC_GUESS);
    if (index >= 0) {
        printf("Erreur: Le fichier '%s' existe déjà dans l'archive.\n", fullpath);
        return -1;
    }
    // Créer une source à partir du fichier local
    zip_source_t *src = zip_source_file_create(filename, 0, 0, NULL);
    if (src == NULL) {
        printf("Erreur: Impossible de créer une source à partir du fichier '%s'.\n", filename);
        return -1;
    }
    // Ajouter le fichier à l'archive
    index = zip_file_add(zip, fullpath, src, ZIP_FL_OVERWRITE);
    if (index < 0) {
        zip_source_free(src);
        printf("Erreur: Impossible d'ajouter le fichier '%s' à l'archive.\n", fullpath);
        return -1;
    }
    printf("Fichier '%s' importé avec succès dans l'archive.\n", filename);
    return 0;
}

// Fonction pour supprimer un fichier de l'archive ZIP
int remove_file(const char *filename) {
    // Préparer le chemin complet du fichier à supprimer dans l'archive
    char fullpath[PATH_MAX] = {0};
    if (strcmp(current_dir, "") == 0) {
        snprintf(fullpath, PATH_MAX, "%s", filename);
    } else {
        snprintf(fullpath, PATH_MAX, "%s%s", current_dir, filename);
    }

    // Vérifiez si le fichier existe dans l'archive
    zip_int64_t index = zip_name_locate(zip, fullpath, ZIP_FL_ENC_GUESS);
    if (index < 0) {
        printf("Erreur: Le fichier '%s' n'existe pas dans l'archive.\n", fullpath);
        return -1;
    }

    // Supprimez le fichier de l'archive
    if (zip_delete(zip, index) < 0) {
        printf("Erreur: Impossible de supprimer le fichier '%s' de l'archive.\n", fullpath);
        return -1;
    }

    printf("Fichier '%s' supprimé avec succès de l'archive.\n", filename);
    return 0;
}

// Invite de confirmation pour la suppression
int confirm_remove(const char *filename) {
    char response[3];
    printf("Êtes-vous sûr de vouloir supprimer le fichier '%s' de l'archive ? (y/n) ", filename);
    fgets(response, sizeof(response), stdin);
    if (response[0] == 'y' || response[0] == 'Y') {
        return remove_file(filename);
    } else {
        printf("Suppression annulée.\n");
        return 0;
    }
}

// Fonction pour gérer une session interactive
void interactive_session() {
    char current_directory[4096] = "/";  // Start at root

    while(1) {
        printf("%s/> ", current_dir);
        fgets(command, COMMAND_MAX, stdin);
        command[strcspn(command, "\n")] = '\0';  // Remove trailing newline

        if(strcmp(command, "exit") == 0) {
            break;
        } else if(strncmp(command, "cd ", 3) == 0) {
            char *dirname = command + 3;
            if(dirname == NULL || *dirname == '\0') {
                printf("Erreur: Nom de dossier manquant après 'cd '\n");
            } else {
                change_dir(dirname);
            }
        } else if(strcmp(command, "ls") == 0) {
            list_files();
        } else if(strncmp(command, "extract ", 8)==0) {
            char *filename = command + 8;
            if (filename == NULL || *filename =='\0'){
                printf("Erreur: Fichier ou dossier manquant \n");
            } else {
                extract(filename);
            }
        }
        else if(strncmp(command, "touch ", 6) == 0) {
            char *filename = command + 6;
            if (filename == NULL || *filename =='\0'){
                printf("Erreur: Nom de fichier manquant après 'touch '\n");
            } else {
                touch(filename);
                printf("Vous devez relancer le programme pour voir votre nouveau fichier.");
                break;
            }
        }
        else if(strncmp(command, "import ", 7) == 0) {
            char *filename = command + 7;
            if (filename == NULL || *filename =='\0'){
                printf("Erreur: Nom de fichier manquant après 'import '\n");
            } else {
                import(filename);
                printf("Vous devez relancer le programme pour voir votre nouveau fichier.");
                break;
            }
        }
        else if(strncmp(command, "rm ", 3) == 0) {
            char *filename = command + 3;
            if (filename == NULL || *filename =='\0'){
                printf("Erreur: Nom de fichier manquant après 'rm '\n");
            } else {
                confirm_remove(filename);
            }
        } else {
            printf("Commande non reconnue : '%s'\n", command);
        }
    }
}

// int main(int argc, char *argv[]) {
//     char *password = NULL;
//     char *zipname = NULL;

//     // Vérifiez les paramètres
//     for (int i = 1; i < argc; i++) {
//         if (strcmp(argv[i], "-o") == 0) {
//             if(i+1 < argc) {
//                 zipname = argv[++i];
//             } else {
//                 printf("Erreur: Nom de fichier manquant après -o\n");
//                 return -1;
//             }
//         } else if (strncmp(argv[i], "-p=", 3) == 0) {
//             password = argv[i] + 3;
//             if(password == NULL || *password == '\0') {
//                 printf("Erreur: Mot de passe manquant après -p=\n");
//                 return -1;
//             }
//         }
//     }

//     if (open_zip(zipname, password) != 0) {
//         return -1;
//     }

//     num_entries = zip_get_num_entries(zip, 0);
//     interactive_session();


//     // Fermer le fichier ZIP
//     zip_close(zip);


//     return 0;
// }
//Bugfix 0.1
