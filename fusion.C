

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
