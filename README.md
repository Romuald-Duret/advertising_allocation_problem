/*!
 * @mainpage README.md

# Méthodes exactes pour un problème d’ordonnancement de spots publicitaires télévisuels

## Sujet
Nous sommes ici dans le domaine de la recherche opérationnelle et le sujet s’intéresse à l’optimisation de l’allocation de spots publicitaires télévisuels pour différentes marques potentiellement compétitives. Plus précisément, des spots publicitaires sont organisés par des propriétaires télévisuels. Ces spots ayant un certain prix ainsi qu’une certaine durée, ils sont mis à la vente pour des marques souhaitant se promouvoir à travers ces spots pour un certains budget qu’ils possèdent et sous certaines conditions. Les spots sont regroupés en écrans publicitaires correspondant chacun à une pause publicitaire de quelques minutes.
Il existe un type d’écrans publicitaires spécial : les écrans de type PRIME. Ces écrans correspondent à un écran publicitaire où l’audience sera plus grande. Cela peut s’expliquer simplement par le fait que ces écrans sont situés entre ou pendant la diffusion de programmes impliquant une hausse importante de l’audience (Exemples : match de foot, prime d’une émission) ou simplement par le fait qu’ils sont situés à des horaires où l’audience est plus grande (Exemples : le matin vers 7h, le soir vers 21h). Il existe également au sein de chaque écran publicitaire des spots spéciaux : les spots de type PREMIUM. Ces spots représentent les moments où l’audience au sein de l’écran est la plus grande et correspondent aux 2 premiers et 2 derniers spots d’un écran publicitaire. En effet, les téléspectateurs sont plus souvent présents sur les 2 premiers spots publicitaires car ils suivent la fin de leur programme télévisuel et sur les 2 derniers spots, se préparant pour suivre le programme télévisuel suivant.

L’optimisation de ce problème revient donc à satisfaire au mieux les 2 partis que l’on peut distinguer soit : les propriétaires télévisuels cherchant à maximiser leurs revenus grâce à la vente de leurs spots mis à disposition, et les marques cherchant globalement à maximiser leur quota d’audience touchée par l’achat de séquences publicitaires. L’optimisation est de plus soumise à de nombreuses contraintes complexifiant l’allocation de spots.

## Objectifs et contraintes considérées

Nous ne considérons que 2 objectifs :

 - Maximiser l'audience touchée pour les marques.
 - Maximiser les revenus des propriétaires télévisuels.
 
Les contraintes sont les suivantes :

- Une marque ne peut apparaître qu’une seule fois sur un écran publicitaire.
- On ne peut avoir qu’une allocation d’une marque sur un écran publicitaire.
- Ne pas dépasser le budget que chaque marque possède pour l’allocation.
- La durée totale d’allocation des espaces de diffusion des spots alloués ne doit pas dépasser la limite de temps de l’écran.
- Ne pas allouer de spots à des marques compétitives au sein d’un même écran.
- Atteindre un niveau de GRP minimum pour chaque marque.
- Le pourcentage de bugdet investi dans des spots premium ne doit pas dépasser la valeur définie par la marque.
- Le pourcentage de bugdet investi dans des écran prime ne doit pas dépasser la valeur définie par la marque.

## Fonctionnalités

Les fonctionnalités sont les suivantes : 

- Résolution du problème avec l'approche epsilon-contrainte avec enregistrement des solutions obtenues sur un fichier.
- Résolution du problème avec l'approche de la somme pondérée avec enregistrement des solutions obtenues sur un fichier.
- Méthode permettant de tester la faisabilité des solution à partir d'un fichier.
- Comparaison des solutions obtenues entre 2 exécutions à partir de 2 fichiers.

## Installation

### Sur macOS

Pour installer le projet, suivez les étapes suivantes :

1. Cloner le dépôt Git sur votre machine locale avec l'IDE XCode sur un nouveau projet :
```bash
https://github.com/Romuald-Duret/advertising_allocation_problem.git
```

2. Installer CPLEX :

Lien : https://www.ibm.com/docs/fr/icos/12.8.0.0?topic=v1280-installing-cplex-optimization-studio

3. Récupérer et intégrer les librairies CPLEX nécessaires sur XCode :

Il y a 3 fichiers à intégrer sur le projet XCode :

- `libconcert.a`
- `libilocplex.a`
- `libcplex.a`

Ces fichiers sont présents sur votre machine à partir du moment où vous avez installé CPLEX, ils sont à ajouter directement dans les paramètres du projet dans "Build Phases" -> "Link Binary With Libraries".

Il faut également ajouter dans les paramètres du projet le chemin vers les headers des librairies "Build Settings" -> "Header Search Paths" et ajouter :

```bash
/Application/CPLEX_Studio221/concert/include
/Application/CPLEX_Studio221/cplex/include
```

Enfin il faut également ajouter dans les paramètres du projet le chemin vers les librairies "Build Settings" -> "Librairy Search Paths" et ajouter :

```bash
/Application/CPLEX_Studio221/concert/lib/x86-64_osx/static_pic
/Application/CPLEX_Studio221/cplex/lib/x86-64_osx/static_pic
```

(Il est possible que ces chemins soient à adapter selon la structure de votre installation personnelle)

### Sur Windows

1. Cloner le dépôt Git sur votre machine locale :
```bash
https://github.com/Romuald-Duret/advertising_allocation_problem.git
```

2. Suivre le tutoriel du fichier :

Lien : https://adam-rumpf.github.io/documents/cplex_in_cpp.pdf

## Exécution 

Ajoutez le code necéssaire dans le fichier `main_resolve.cpp`.
