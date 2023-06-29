#include "navigate.h"



// Fonction pour ouvrir le fichier ZIP
zip_t* open_zip(int* num_entries, const char* zipname, const char* password) {
    zip_t *zip = NULL;
    zip_error_t ziperror;
    int err = 0;

    if (!zipname) {
        printf("Erreur: Pas de nom de fichier fourni.\n");
        return NULL;
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
    *num_entries = zip_get_num_entries(zip, 0);
    printf("Fichier ZIP ouvert: %s\n", zipname);
    return zip;
}


// Fonction pour afficher la hiérarchie des dossiers et des fichiers pour un répertoire donné
void display_directory(const char *directory) {
    zip_t *zip = NULL;
zip_int64_t num_entries = 0;
    int directory_length = strlen(directory);

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(zip, i, ZIP_FL_UNCHANGED);

        // Check if the name starts with the given directory
        if (strncmp(name, directory, directory_length) == 0) {
            const char *remaining_path = name + directory_length;

            // Check if the remaining path is a direct file/directory (i.e., it does not contain '/')
            char *next_slash = strchr(remaining_path, '/');

            // If there's no next slash, print the file name
            if (next_slash == NULL) {
                printf("%s\n", remaining_path);
            }
            // If there is a next slash and it's the last character, print the directory name
            else if (strcmp(next_slash, "/") == 0) {
                printf("%s\n", remaining_path);
            }
            // If there is a next slash and it's not the last character, print the directory name up to the slash
            else {
                char *dir_name = strndup(remaining_path, next_slash - remaining_path);
                printf("%s/\n", dir_name);
                free(dir_name);
            }
        }
    }
}

//Structure pour représenter un nœud dans l'arborescence
// typedef struct path_node {
//     char *name;
//     struct path_node *next;
// } path_node;

// typedef struct Node {
//     char *name;
//     struct Node **children;
//     int num_children;
// } Node;

path_node *path_head = NULL;
char current_dir[PATH_MAX] = "";
char command[COMMAND_MAX];

// Fonction pour créer un nouveau nœud
Node *new_node(const char *name) {
    Node *node = malloc(sizeof(Node));
    node->name = strdup(name);
    node->children = NULL;
    node->num_children = 0;
    return node;
}

// Fonction pour ajouter un enfant à un nœud
void add_child(Node *node, Node *child) {
    node->children = realloc(node->children, (node->num_children + 1) * sizeof(Node *));
    node->children[node->num_children++] = child;
}

// Fonction pour trouver un enfant avec un nom donné
Node *find_child(Node *node, const char *name) {
    for (int i = 0; i < node->num_children; i++) {
        if (strcmp(node->children[i]->name, name) == 0) {
            return node->children[i];
        }
    }
    return NULL;
}

// Fonction pour construire l'arborescence à partir des noms de fichiers dans l'archive ZIP
Node *build_tree(zip_t *zip) {
    Node *root = new_node("");

    for (zip_int64_t i = 0; i < zip_get_num_entries(zip, 0); i++) {
        const char *name = zip_get_name(zip, i, 0);
        char *name_copy = strdup(name);

        Node *current = root;
        char *segment = strtok(name_copy, "/");
        while (segment) {
            Node *child = new_node(segment);
            add_child(current, child);
            current = child;
            segment = strtok(NULL, "/");
        }

        free(name_copy);
    }

    return root;
}

// Fonction pour afficher l'arborescence
void print_tree(Node *node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("\t");
    }
    printf("%s\n", node->name);
    for (int i = 0; i < node->num_children; i++) {
        print_tree(node->children[i], depth + 1);
    }
}

// Changer le dossier actuel
int change_dir(zip_t* zip, const char *dir) {
    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
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



void list_files(zip_t* zip) {
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


// Fonction pour gérer une session interactive
void interactive_session(zip_t* zip) {
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
        } else {
            printf("Commande non reconnue : '%s'\n", command);
        }
    }
}

// int navigate_zip(zip_t* zip, int num_entries, const char* zipname, const char* password) {
//     if (open_zip(zipname, password) != 0) {
//         return -1;
//     }

//     num_entries = zip_get_num_entries(zip, 0);
//     interactive_session();

//     // Fermer le fichier ZIP
//     zip_close(zip);

//     return 0;
// }


int main(int argc, char *argv[]) {
    char *password = NULL;
    char *zipname = NULL;
    zip_t *zip = NULL;
    int num_entries = 0;

    // Vérifiez les paramètres
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if(i+1 < argc) {
                zipname = argv[++i];
            } else {
                printf("Erreur: Nom de fichier manquant après -o\n");
                return -1;
            }
        } else if (strncmp(argv[i], "-p=", 3) == 0) {
            password = argv[i] + 3;
            if(password == NULL || *password == '\0') {
                printf("Erreur: Mot de passe manquant après -p=\n");
                return -1;
            }
        }
    }

    zip = open_zip(&num_entries, zipname, password);
    if (zim == NULL) {
        return -1;
    }

    // printf("Affichage de la hiérarchie des dossiers et des fichiers :\n");
    // Node *root = build_tree(zip);
    // print_tree(root, 0);

    num_entries = zip_get_num_entries(zip, 0);
    interactive_session();


    // Fermer le fichier ZIP
    zip_close(zip);

    return 0;
}
