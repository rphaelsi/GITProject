#include "manip.h"

int importe(char *file_PATH, char *archive_PATH)
{

    // Créer une structure (objet) archive pour permettre d'ouvrir l'archive avec "zip.h"
    struct zip *archive = zip_open(archive_PATH, ZIP_CHECKCONS, NULL); // On ouvre l'archive en donnant le nom, la méthode "ZIP_CHECKCONS" permet de vérifier l'intégrité de l'archive.
    // Vérification de sa structure pour voir si elle rentre dans les normes ZIP.
    if (archive == NULL)
    {
        printf("Impossible d'ouvrir l'archive '%s'\n", archive_PATH);
        return -1;
        // Si la commande renvoie NULL, alors on renvoie -1 pour indiquer une erreur d'impossibilité d'ouverture de l'archive.
    }

    // On récupère la source du fichier à ajouter dans l'archive.
    struct zip_source *source = zip_source_file(archive, file_PATH, 0, -1);
    // On utilise la méthode "zip_source_file" pour obtenir le fichier en donnant, l'archive dans laquelle on va l'ajouter. Le chemin du fichier.
    // Le 0 est l'offset, le point de départ de lecture du fichier et le -1 indique qu'on lit l'ensemble du fichier.
    if (source == NULL)
    {
        printf("Impossible d'obtenir la source du fichier '%s'\n", file_PATH);
        zip_close(archive);
        return -1;
        // On regarde si le fichier source récupéré est égal à NULL, si oui, on affiche un message d'erreur
        // Et on ferme l'archive retournant -1.
    }

    // A partir du chemin du fichier, on récupère le nom du fichier qui va nous être utile.
    const char *FileName = file_PATH;              // Initialisation d'une variable FileName avec le chemin du fichier file_PATH.
    char *FileNameAlone = strrchr(file_PATH, '/'); // le nomfichierseul
    if (FileNameAlone != NULL)
    {
        FileName = FileNameAlone + 1;
    }

    // Ajouter le fichier à l'archive
    int add = zip_file_add(archive, FileName, source, ZIP_FL_OVERWRITE);
    // On ajoute à l'aide de la méthode "zip_file_add" avec l'archive en argument, le nom du fichier, la source du fichier à ajouter.
    // La commande "ZIP_FL_OVERWRITE" permet d'écraser le fichier si il existe déjà.
    if (add < 0)
    {
        // la méthode renvoie l'index du fichier dans l'archive, si l'index est négatif alors l'ajoue à échouer.
        // On affiche message d'erreur
        printf("Impossible d'ajouter le fichier '%s' à l'archive '%s'\n", file_PATH, archive_PATH);
        zip_source_free(source);
        // On libère la structure de la source.
        zip_close(archive);
        // On ferme la structure vers l'archive.
        return -1;
        // On renvoie -1.
    }

    // Fermer l'archive
    if (zip_close(archive) == -1)
    {
        printf("Erreur lors de la fermeture de l'archive '%s'\n", archive_PATH);
        return -1;
    }

    // Afficher un message de réussite
    printf("Le fichier '%s' a été ajouté avec succès à l'archive '%s'\n", file_PATH, archive_PATH);
    return 0;
}

int extracte(char *archive_PATH, char *archive_file_PATH, char *system_PATH)
{
    struct zip *archive = zip_open(archive_PATH, ZIP_CHECKCONS, NULL);
    if (archive == NULL)
    {
        printf("Impossible d'ouvrir l'archive '%s'\n", archive_PATH);
        return -1;
    }

    int out = zip_name_locate(archive, archive_file_PATH, ZIP_FL_NOCASE);
    // La méthode "zip_name_locate" permet de localiser l'index d'un fichier dans l'archive en se basant sur son chemin et son nom
    // On donne le chemin de l'archive, ainsi que le chemin du fichier dans l'archive.
    // La méthode "ZIP_FL_NOCASE" indique que la recherche sera insensible à la casse.
    if (out < 0)
    {
        // Out retourne l'index du fichier dans l'archive, si erreur renvoie un chiffre négatif.
        printf("Le fichier '%s' n'a pas été trouvé dans l'archive '%s'\n", archive_file_PATH, archive_PATH);
        // Message d'erreur et on ferme l'archive.
        zip_close(archive);
        return -1;
    }

    struct zip_file *file = zip_fopen_index(archive, out, 0);
    // méthode "zip_fopen_index" permet de trouver un fichier dans une archive à l'aide de son index.
    // en paramètre on donne l'archive, l'index du fichier et le 0 indique que le fichier doit être ouvert en lecture seule.
    // Renvoie NULL si le fichier n'a pas pu être ouvert.
    if (file == NULL)
    {
        // Si "zip_fopen_index" renvoie NULL alors on renvoie une erreur.
        printf("Impossible d'ouvrir le fichier '%s' dans l'archive '%s'\n", archive_file_PATH, archive_PATH);
        // Affiche message d'erreur et ferme archive.
        zip_close(archive);
        return -1;
    }

    FILE *file_target = fopen(system_PATH, "wb");
    // On créer un fichier que l'on ouvre en écriture binaire.
    if (file_target == NULL)
    {
        // SI le retour de l'ouverture est NULL alors on a rencontrer une erreur.
        printf("Impossible de créer le fichier cible '%s'\n", system_PATH);
        // On affiche un message d'erreur et on ferme le fichier.
        zip_fclose(file);
        zip_close(archive);
        // On ferme également l'archive.
        return -1;
    }

    char buffer[1024];
    // On déclare un tableau de caractères avec une taille de 1024 octets
    // Cette variable sert de tampon pour stocker le contenu du fichier de l'archive.
    zip_int64_t nbBytes;
    // On créer une variable de type "zip_int64_t" qui va accueillir le nombre d'octet du fichier dans l'archive à chaque itération.
    while ((nbBytes = zip_fread(file, buffer, sizeof(buffer))) > 0)
    {
        // La boucle écrit les données dans le fichier cible.
        //  LA fonction "zip_fread" est utilisé pour lire les données du fichier dans l'archive.
        // Les données sont stockées dans le buffer avec une taille de lecture maximum spécifiée par "sizeof"
        // zip_fread renvoie le nomnbre d'octet lu à partir du fichier dans l'archive.
        // Boucle continue tant que "nbBytes" est supérieur à 0.
        fwrite(buffer, nbBytes, 1, file_target);
        //"fwrite" écrit les données dans le fichier cible.
        // L'argument "1" indique le nombre d'éléments à écrire, fixé à 1 car nous écrivons tous les octets d'une seule fois.
    }

    // Fermer l'archive
    if (zip_close(archive) != 0)
    {
        printf("Erreur lors de la fermeture de l'archive.\n");
        return -1;
    }

    printf("Le fichier '%s' a été extrait avec succès de l'archive '%s' vers '%s'\n", archive_file_PATH, archive_PATH, system_PATH);
    printf("\n");
}

int delete_file_archive(char *PATH_archive, char *PATH_archive_file)
{
    // Ouvrir l'archive en lecture/écriture
    struct zip *archive = zip_open(PATH_archive, ZIP_CHECKCONS, NULL);
    if (!archive)
    {
        printf("Impossible d'ouvrir l'archive %s\n", PATH_archive);
        return -1;
    }

    // Rechercher l'index du fichier dans l'archive
    int fileIndex = zip_name_locate(archive, PATH_archive_file, 0);
    if (fileIndex < 0)
    {
        printf("Le fichier %s n'a pas été trouvé dans l'archive.\n", PATH_archive_file);
        zip_close(archive);
        return -1;
    }

    // Supprimer le fichier de l'archive
    int result = zip_delete(archive, fileIndex);
    if (result != 0)
    {
        printf("Erreur lors de la suppression du fichier %s de l'archive.\n", PATH_archive_file);
        zip_close(archive);
        return -1;
    }

    // Fermer l'archive
    if (zip_close(archive) != 0)
    {
        printf("Erreur lors de la fermeture de l'archive.\n");
        return -1;
    }

    printf("Le fichier %s a été supprimé de l'archive.\n", PATH_archive_file);
    return 0;
}
