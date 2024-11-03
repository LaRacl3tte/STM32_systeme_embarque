# Généralités
Ce dépôt permet de développer de petits projets utilisant la biliothèque
libopencm3. L'organisation est dérivée du projet libopencm3-template
https://github.com/libopencm3/libopencm3-template.git

# Instructions
 1. cloner ce dépôt Git
 2. aller dans le sous-répertoire contenant votre projet
 3. utiliser une des commandes suivantes :
    - make # contruit le projet (et la bibliothèque libopencm3 initialement)
    - make load # ou make flash : envoie le projet sur la carte STM32
    - make gdb # démarre le projet sous gdb (en remote sur la carte)

# Directories
* libopencm3 contient la bibliothèque permettant de programmer la carte STM32
* common/ contient des fichiers pouvant être utilisés dans plusieurs projets
* blink/ est un petit projet montrant comment faire clignoter une LED
* TP1/ est là où vous trouverez un point de départ pour le premier TP
