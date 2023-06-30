[![Slack](https://img.shields.io/badge/slack-join-blue.svg)](https://example.com/community/join-us-on-slack/)
[![Email](https://img.shields.io/badge/email-join-blue.svg)](https://groups.google.com/forum/#!forum/example)
[![Documentation](https://img.shields.io/badge/docs-view-green.svg)](https://documentation.example.com)
[![Documentation](https://img.shields.io/badge/web-view-green.svg)](https://doc.test.com)
[![Coverity](https://scan.coverity.com/projects/10992/badge.svg)](https://scan.coverity.com/projects/)
[![Twitter](https://img.shields.io/twitter/follow/sgt?style=social)](https://twitter.com/)
[![YouTube](https://img.shields.io/youtube/views/peTSzcAueEc?style=social)](https://www.youtube.com/watch?v=jIQ6UV2onyI&ab_channel=TuNeoTec)

# Introduction : Compre-C

Le projet voit le jour dans un contexte scolaire dans le but de mettre en pratique les différentes notions du langage C dans un outil de manipulation des fichiers ZIP. L'objectif étant d'obtenir un outil performant, une structrure de fichier et un code bien organisé ainsi que la manipulation de différentes autres notions.


# Les différentes fonctionnalités de l'outil

- Possibilité de bruteforce des fichiers ZIP protégés par mot de passe avec une bibliothèque fournie par l'utilsiteur.
- Déchiffrer une archive vérouillée par un mot de passe en récupérant le mot de passe qui a été utilisé pour la chiffrer.
- Naviguer dans une Archive, pouvoir en récupérer le contenu sur le poste de l'hôte ou ajouter du contenu présent sur le poste de l'hôte.
- Nous retrouverons une rubrique "help" permettant à l'utilisateur de retrouver les différentes options à sa disposition ainsi qu'une explication brève sur leur fonctionnement.

## Architecture du projet

Le projet sera organisé en plusieurs fichiers source et en-têtes pour une meilleure modularité et maintenabilité du code. Voici la structure du projet :
- navigate.c/navigate.h  => Fonction mettant en place la fonctionnalité de naviger dans l'arboresence d'un zip, comme dans un prompt classique.
- manip.c/manip.h => Fonction permettant de mettre en place la fonction d'import et d'export de fichier dans l'archive.
- bruteforce.c/bruteforce.h => Fonction permettant de trouver le mot de passe d'une archive, de manière agressive, en essayant de le bruteforce.
- main.c => Fichier accueillant l'ensemble de nos fonctions et permet de lier l'ensemble du code qui est fais séparemment. 


## Dépendance devant être installées pour utiliser le projet

Pour utiliser le projet, vous devez installer uniquement une seule chose. La librairie LibZip.

Pour l'installer, il vous suffit d'utiliser la commande suivante dans votre terminal :
```shell
sudo apt install libzip-dev
```

Une fois la librairie installée, vous serez en mesure d'utiliser le code.

Vous êtes prêt à utiliser notre programme !

## Comment compiler le projet ?

Une fois les fichiers récupérés, placer l'ensemble dans un même dossier. Pour compiler, vous pouvez utiliser la commande suivante :

```shell
gcc main.c navigate.c bruteforce.c manip.c -o [nom_programme] -lzip
```
Il est important de préciser que sans l'option "-lzip", il sera impossible d'utiliser le programme.
(le compilateur est gcc, il est normalement installé par défaut sur les différents systèmes. Si ce n'est pas le cas, vous pouvez trouver des sources d'installation partout sur Internet.)

## Utilisation du programme, une fois compilé

Avant de voir les différentes options, comment utiliser le programme :
```shell
./[nom_programme] -options
```

Les différentes options sont les suivantes, nous expliquerons les différents arguments attendus et l'utilisation de ces dernières : 
```shell
-h, aide pour connaître les différentes options et comment les utiliser.
-b, permet de bruteforce un zip protégé par un mot de passe en donnant le nom/chemin de l'archive cible.
-d, permet de brutefroce un zip protégé par un mot de passe avec une WordList en donnant le nom/chemin de l'archive ainsi que le nom/chemin de la Wordlist.
-e, permet d'extraire un fichier d'une archive en spécifiant le nom/chemin de l'archive cible, le nom/chemin du fichier à extraire de l'archive, le nom/chemin où le fichier doit être extrait
-i, permet d'importer un fichier dans l'archive en spécifiant le nom/chemin du fichier à ajouter et le nom/chemin de l'archive dans laquelle on veut importer le fichier
-n, permet de naviguer dans une archive donnée en argument, on peut également combiner cela avec l'option -p pour spécifier un mot de passe d'archive.
```

Maintenant que vous connaissez, les différentes options, nous allons vous donner plusieurs exemples d'utilisation pour être vraiment claire :
```shell
./[nom_programme] -h
```

```shell
./[nom_programme] -b archive_test.zip
```

```shell
./[nom_programme] -d archive_test.zip,wordlist.txt
```

Il est important de séparer les différents arguments par une virgule (si il y a plusieurs arguments dans l'option utilisée).
Si les deux arguments ne sont pas séparés par une virgule, ils ne seront pas bien prit en compte par le programme.

!!! Il est également important de donner les arguments dans le bonne ordre !!!! sinon le programme risque de rencontrer des erreurs.

Des vérifications des entrées utilisateurs sont présentes, pas d'inquiétudes :)))))

## Conclusion

Vous pouvez maintenant utiliser notre outil sans problèmes.

En cas de demande particulières, vous pouvez rédiger une Issue directement sur le Github de ce projet.

Vous pouvez retrouver des templates pour les différents bugs que vous pouvez rencontrer.

Si vous avez une idée d'amélioration, vous pouvez retrouver une template pour proposer cette fontionnalité.

Have Fun :))))
