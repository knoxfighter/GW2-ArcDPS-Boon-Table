<p align="center">
 <h2 align="center">ArcDPS Boon Table</h2>
</p>

<p align="center">
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/releases"><img alt="Versions" src="https://img.shields.io/github/v/release/knoxfighter/GW2-ArcDPS-Boon-Table?label=Version" /></a>
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/graphs/contributors"><img alt="Contributeurs" src="https://img.shields.io/github/contributors/knoxfighter/GW2-ArcDPS-Boon-Table?label=Contributeurs" /></a>
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/issues"><img alt="Tickets" src="https://img.shields.io/github/issues/knoxfighter/GW2-ArcDPS-Boon-Table?color=0088ff&label=Tickets" /></a>
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/pulls"><img alt="Demandes d'ajout" src="https://img.shields.io/github/issues-pr/knoxfighter/GW2-ArcDPS-Boon-Table?color=0088ff&label=Demandes%20d%27ajout" /></a>
</p>

<p align="center">
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/releases">Téléchargement</a>
    ·
    <a href="https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/issues/new?template=bug_report.yml">Signaler un bug</a>
</p>

<p align="center">
    <a href="../README.MD">English</a>
    ·
    Français
</p>

<p align="center">
    <img src="https://raw.githubusercontent.com/knoxfighter/GW2-ArcDPS-Boon-Table/master/Screenshot.png" />
</p>

## Utilisation

Alt+Maj+B (remplace le raccourci clavier par défaut du tableau des avantages dans Arc) ou case à cocher dans les options d'Arc

Clic droit pour accéder aux options permettant de modifier les colonnes affichées et d'autres paramètres.
Vous pouvez réorganiser les colonnes du tableau en faisant glisser leur en-tête.
Pour trier les données d'une colonne, cliquez sur son en-tête. Par défaut, le tri s'effectue par nom de personnage.

### Options
- Joueurs : afficher/masquer tous les joueurs du tableau (par défaut : Activé)
- Sous-groupes : afficher/masquer la moyenne des sous-groupes (par défaut : Activé)
- Total : afficher/masquer la moyenne du groupe/de l'escouade (par défaut : Activé)
- PNJ : afficher/masquer les avantages des PNJs (par défaut : Activé)
    - disponible uniquement pour 3 rencontres (Deimos, Escorte et Rivière des âmes)
- Configuration des colonnes : afficher/masquer les colonnes du tableau.
- Style : option de mise en forme pour cette fenêtre
    - Afficher la valeur sous forme de barre de progression : affiche les valeurs sous forme de barre de progression en plus du nombre réel. (par défaut : Activé)
    - Couleur lignes en alternance : affiche/masque la couleur de l'arrière-plan pour mieux distinguer les lignes. (par défaut : Activé)
    - Afficher l'en-tête avec du texte au lieu d'icônes : remplace les icônes de l'en-tête par du texte (par défaut : Désactivé)
    - Masquer l'en-tête de la fenêtre : Masque l'en-tête de la fenêtre imgui. L'en-tête du tableau restera visible (par défaut : Désactivé)
    - Afficher uniquement votre sous-groupe : Dans la section des joueurs, n'affiche que les joueurs qui font partie de votre sous-groupe. (Par défaut : Désactivé)
    - Afficher uniquement vous-même : Dans la section des joueurs, n'affiche que vous-même (Par défaut : Désactivé)
    - Mode de couleur : mode de coloration du texte et des barres de progression. Peut-être réglé sur « Défaut » (gris), « Par pourcentage » (du rouge au vert) ou « Par profession ». (Par défaut : Défaut)
    - Alignement : aligne le texte sur la colonne. Peut-être réglé sur Gauche, Droite, Centre et Non aligné. « Non aligné » signifie qu’il est placé le plus à gauche possible, sans chevaucher la barre de progression. (Par défaut : Droite)
    - Règlage de la taille : modifie la manière dont la fenêtre est redimensionnée.
        - Automatique : cette option redimensionne automatiquement la fenêtre à chaque fois. Les colonnes « Nom » et « Groupe » peuvent être redimensionnées en glissant les bordures.
        - Adapter le contenu : pour modifier la taille de la fenêtre en faisant glisser le coin inférieur gauche ou droit. Les colonnes s'ajusteront automatiquement pour s'adapter à la fenêtre.
        - Manuelle : pour modifier la taille de la fenêtre en faisant glisser le coin inférieur gauche ou droit. Lorsque le tableau dépasse la taille de la fenêtre, une barre de défilement s'affiche.
    - Largeur des colonnes : modifie la largeur des colonnes « Avantages », toutes les colonnes « Avantages » ont la même largeur. Cette option n'est pas disponible lorsque l'option « Ajuster le contenu » est activée. (Par défaut : 80)
    - Couleur personnelle : modifier la couleur de votre nom dans la liste. (Par défaut : arcdps CCOL_YELLOW)

## Traduction
Le plugin est traduit en anglais, français, allemand, espagnol et chinois simplifié. La traduction en espagnol n'est qu'une traduction approximative ; merci de signaler tout problème que vous pourriez rencontrer.

Pour apporter une modification ou une correction sur l'une des traductions disponibles au sein de Boon table les fichiers contenant les textes sont les suivants :
* [Lang.cpp](https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/blob/master/ArcDPS%20Boon%20Table/Lang.cpp)
* [ExtensionTranslations.h](https://github.com/Zinn-o-Matics/arcdps-extension/blob/main/ExtensionTranslations.h)

Il est également possible d'ajouter des traductions à l'aide de fichiers. Si vous souhaitez ajouter une traduction, créez un fichier au format `arcdps_table_lang_<langCode>.ini` et placez-le dans le répertoire `addons/arcdps/`.
Le code « langCode » sert d'identifiant et est également utilisé dans le fichier de sauvegarde pour conserver la langue sélectionnée. Dans les paramètres, la clé de traduction « LangageName » est utilisée pour afficher le nom. La traduction en anglais est disponible sous le modèle suivant « arcdps_table_lang_en.ini ». Les commentaires commencent par « ; » et en l'absence de traduction le texte s'affichera par défaut en anglais.

## LICENCE
Ce projet est placé sous la [licence MIT](https://fr.wikipedia.org/wiki/Licence_MIT).

### Dear ImGui
[Dear ImGui](https://github.com/ocornut/imgui) est sous licence MIT.

### Magic-Enum
[Magic-Enum](https://github.com/Neargye/magic_enum) est sous licence MIT.

### arcdps-extension
[arcdps-extension](https://github.com/knoxfighter/arcdps-extension) est sous licence MIT.

### arcdps-unofficial-extras
[arcdps-unofficial-extras](https://github.com/Krappa322/arcdps_unofficial_extras_releases) est sous licence MIT.

### modernIni
[modernIni](https://github.com/knoxfighter/modernIni) est sous licence MIT.

### Game Graphics
© 2026 ArenaNet LLC. Tous droits réservés. NCSOFT, ArenaNet, Guild Wars, Guild Wars 2, GW2, Heart of Thorns, Path of Fire, End of Dragons, Secrets of the Obscure, Janthir Wilds, Visions of Eternity, et tous les logos, dessins et marques composites associés sont des marques commerciales ou déposées de NCSOFT Corporation. Toutes les autres marques appartiennent à leurs propriétaires respectifs.
