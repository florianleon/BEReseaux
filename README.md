# Bureau d'étude Réseaux
LEON Florian
3 MIC E2
 ---
Je ferais souvent référence au pdf contenant les machines à états des différentes fonctions décrites en TD

A consulter [ici](./MAE_BE_Reseaux.pdf)

 ### Version 1 :
Cette première version consistait à coder une phase de transfert de données sans aucune garantie de fiabilité.

Il s'agit essentiellement de reprendre les machines à états vu en TD et de coder les fonctions :
```c
int mic_tcp_send(int mic_sock, char* mesg, int mesg_size);
int mic_tcp_recv(int socket, char* mesg, int max_mesg_size);
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr);
```
 Ici comme il n'y a aucune garantie de fiabilité, c'est un peu comme si on avait un codé un **UDP** bis. 
 
 ### Version 2 : 
 Dans cette version, on introduit une fiabilité totale, tous les paquets perdus sont retransmis.
 
 On rajoute alors : 
 ```c
 // PE pour Prochaine trame à émettre
static int PE = 0;
 // PA pour Prochaine trame à recevoir
static int PA = 0;
```
La synchronisation de ces 2 variables qu'on initialise alors dans le *header du pdu* nous permet alors de garantir une fiabilité totale. 

Le plus "gros" du traitement à lieu dans la fonction **process_received_PDU** où on rajoute la gestion de PA.

> ***NB :*** la fonction **mic_tcp_recv** n'est pas modifié d'une version à l'autre

### Version 3
Dans cette version, on rajoute la gestion d'une fiabilité partielle. Ainsi, on ne va plus retransmettre tous les paquets non arrivés à destination mais seulement un petit pourcentage. 

Pour cela, on utilise une fenêtre glissante : 
```c
#define FENETRE 10 
#define NBRPERTES 2 
```
En prenant une fenêtre de taille 10 et en fixant le nombre de paquet qu'on peut se permettre de perdre, on se retrouve avec une perte admissible de **20%**. L'essentiel des modifications se passe dans la fonction **process_received_PDU**. 

La taille de la fenêtre est à chosir la plus petite possible. Comme la perte admissible est grande donc on peut se permettre de prendre une fenetre plus petite parce que si on avait pris une fenetre de 100 et un taux de perte de 20 (et on aurait pu) la qualité serait beaucoup impactée je pense. Par exemple, on pourrait perdre jusqu'à 20 paquets de suite et la qualité de retransmission serait alors fortement impactée. 
 
 
 ### Version 3 bis
 Cette version est la même que la précedente mais j'ai rajouté l'établissement de la phase de connexion. 
 
 J'ai donc modifié les fonctions suivantes en suivant ce qui était décrit sur la page du BE de moodle: 
 ```c 
 int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr);
 int mic_tcp_connect(int socket, mic_tcp_sock_addr addr);
 ```
 J'ai eu le temps de réaliser les tests en texte uniquement, même si je ne vois pas pourquoi elle ne marcherait pas en video.
 Cependant, je ne sais pas si c'est correct car ça reste une simple ébauche. Il y a beaucoup d'amélioration possible mais j'avais voulu tenter le coup et voir ce que je pouvais faire dessus. 
 
 ---
 > ***NB1 :*** Si la video ne se lance pas sur votre ordinateur c'est peut être normal car j'ai travaillé sur mac et j'ai du adapter l'exécutable **tsock_video**. Il suffira normalement de décommenter les 2 premières lignes et de commenter les 2 dernières. 
 
 ```bash
#killall -9 cvlc > /dev/null 2>&1
#killall -9 vlc > /dev/null 2>&1
killall -9 VLC > /dev/null 2>&1
killall -9 gateway > /dev/null 2>&1
```
 
  > ***NB2 :*** Dans mes fichiers pour la V2 et V3, j'ai gardé un **set_loss_rate(10)**, il serait préférable de réaliser les tests avec un **set_loss_rate(5)**. Un taux de pertes de 10 est un peu trop élévé je pense. 
 
 > ***NB3 :*** Dans les fichiers tests, il faudrait mettre en commentaire les *printf()* car ça ralentit énormément la lecture de la video. C'est pratique pour le déboggage mais c'est tout. 
