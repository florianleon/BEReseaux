#include "../include/mictcp.h"
#include "../include/api/mictcp_core.h"
#define FENETRE 10 // a choisir le plus petit possible
#define NBRPERTES 2 //soit 20%
/**
 * comme la perte admissible est grande donc on peut se permettre de prendre une fenetre plus petite, 
 * parce que si on avait pris une fenetre de 100 et un taux de perte de 20 la qualité serait beaucoup impactée je pense
*/

mic_tcp_sock_addr sock_return;
static int PE = 0;
static int PA = 0;
static int loss = 0; //taux effectif de pertes
static int package = 0; //0 si le paquet est pas perdu, 1 sinon
//(1024 + rand() ) % (65536-1024)
/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(10);

   return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   return socket;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */

/*
L’appel à la primitive d'acceptation d’une requête de connexion (accept()) est bloquant 
jusqu’à ce qu’une requête arrive : la main n’est rendue à l’application en sortie de l’accept() 
que lorsque les échanges de PDU MIC TCP SYN-ACK et ACK ont été finalisés ou sur 
expiration du timer associé à l’envoi du PDU SYN-ACK après N tentatives
*/
//pour l'instant j'ai décidé de ne pas prendre en compte le nombre de tentatives
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    //corps de syn
    mic_tcp_pdu syn_recv;
    syn_recv.payload.data = NULL;
    syn_recv.payload.size = 0;
    //SYN_ACK pour l'établissement de la connecxion
    mic_tcp_pdu syn_ack;
    syn_ack.payload.data = NULL;
    syn_ack.payload.size = 0;
    syn_ack.header.syn = 1;
    syn_ack.header.ack = 1;

    mic_tcp_sock_addr addr_recv;
    unsigned long timer = 5; //10 * RTT
    //la fonction doit se bloquer en attendant la reception du SYN
    while (1) {
        //si on recoit une demande de connexion on envoie un acquitement 
        if (IP_recv(&syn_recv, &addr_recv, timer) != -1) {
            if(syn_recv.header.syn == 1) {
                //on a reçu le bon acquittement
                //IP_send(syn_ack, addr_recv); //je pense que addr_recv == *addr
                break;
            } else {
                continue;
            }
        }
    }
    IP_send(syn_ack, addr_recv);
    mic_tcp_pdu syn_ack_ack;
    syn_ack_ack.payload.data = NULL;
    syn_ack_ack.payload.size = 0;
    syn_ack_ack.header.ack = 0;
    while ((IP_recv(&syn_ack_ack, &addr_recv, timer) == -1) || (syn_ack_ack.header.ack == 0)) {
        IP_send(syn_ack, addr_recv);
    }
    
    return socket;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    sock_return = addr;
    mic_tcp_pdu syn;
    syn.payload.data = NULL;
    syn.payload.size = 0;
    syn.header.syn = 1;
    syn.header.seq_num = PE;
    IP_send(syn, addr);
    //mise en attente d'un syn_ack
    mic_tcp_pdu syn_ack_recv;
    syn_ack_recv.payload.data = NULL;
    syn_ack_recv.payload.size = 0;
    mic_tcp_sock_addr addr_recv;
    unsigned long timer = 5; //10 * RTT
    //pour renvoyer le syn si le premier se perd
    int nb = 0;
    while (1) {
        //Si on recoit le bon acquitement on quitte 
        //Sinon on rerentre dans la boucle 
        //la fonction est donc bloquante
        if (IP_recv(&syn_ack_recv, &addr_recv, timer) != -1) {
            if(syn_ack_recv.header.syn == 1 && syn_ack_recv.header.ack == 1) {
                //on a reçu le bon acquittement
                break;
            } //else {
                //IP_send(syn, addr);
                //continue;
            //}
        }
        nb++;
        if (nb > 3) { //nombre purement arbitraire
            sleep(1); //laisser le temps à la synchro de se faire
            IP_send(syn, addr);
            nb = 0;
        }
    }

    mic_tcp_pdu syn_ack;
    syn_ack.payload.data = NULL;
    syn_ack.payload.size = 0;
    syn_ack.header.ack = 1;
    IP_send(syn_ack, addr);

    return socket;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu pdu;
    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;
    pdu.header.source_port = 0;
    pdu.header.dest_port = 0;
    pdu.header.seq_num = PE;
    pdu.header.ack_num = 0;
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;
    //app_buffer_put(pdu.payload);
    
    mic_tcp_sock_addr a  = sock_return;
    int send_size = IP_send(pdu, a);

    mic_tcp_pdu ack_recv;
    ack_recv.payload.data = NULL;
    ack_recv.payload.size = 0;
    mic_tcp_sock_addr addr_recv;
    unsigned long timer = 5; //10 * RTT
    while ((IP_recv(&ack_recv, &addr_recv, timer)) == -1 || (ack_recv.header.ack_num == PE)) {
        if (loss < NBRPERTES) {
    		loss++;
    		break;
    	} else {
    		send_size = IP_send(pdu, a);
    	} 
    }
    package = (package + 1) % FENETRE;

    if (package == 0){
    	loss = 0;
    }
    PE = (PE + 1) % 2;
    return send_size;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_payload p;
    p.data = mesg;
    p.size = max_mesg_size;
    int get_size = app_buffer_get(p);
    return get_size;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return socket;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    //ACK pour la récption d'un message
    mic_tcp_pdu ack;
    ack.payload.data = NULL;
    ack.payload.size = 0;
    ack.header.ack = 1;
    
    
    if (pdu.header.seq_num == PA) {
        if (pdu.header.syn == 1) {
            printf("Syn reçu\n");
        } else {
            app_buffer_put(pdu.payload);
            PA = (PA + 1) % 2;
            ack.header.ack_num = PA;
            IP_send(ack, addr);
        }
        
    } else {
        ack.header.ack_num = PA;
        IP_send(ack, addr);
    }
}
 