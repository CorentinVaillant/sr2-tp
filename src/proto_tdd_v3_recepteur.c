/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1   3                               *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* C. Vaillant - Univ. de Toulouse III - Paul Sabatier        *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"


#define DEBUG 0

#if DEBUG
#define IF_DEBUG(FORM) FORM
#else
#define IF_DEBUG(FORM)
#endif


/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int paquet_a_recevoir = 0;
    int fin = 0; /* condition d'arrêt */

    

    paquet_t ack; /*gestion d'erreur*/
    ack.type = ACK;
    ack.lg_info = 0;
    ack.num_seq = paquet_a_recevoir;
    ack.somme_ctrl = creer_somme_ctrl(ack);

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    IF_DEBUG(int compteur_boucle = 0); // si le debug mode active

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {


        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);
        
        if(test_somme_ctrl(paquet)){
            if(paquet.num_seq == paquet_a_recevoir){
                IF_DEBUG (printf("somme ctrl 👍\n paquet n°%d reçu\n",paquet.num_seq));// si le debug mode active
                
                /* extraction des donnees du paquet recu */
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);

                paquet_a_recevoir ++; paquet_a_recevoir %= SEQ_NUM_SIZE;

                ack.num_seq = paquet_a_recevoir;
                ack.somme_ctrl = creer_somme_ctrl(ack);
            
            }
            IF_DEBUG(else
                printf("mauvais paquet ❌ %d reçu\n \t-demande %d\n",paquet.num_seq,ack.num_seq)); //si le debug mode active

        }
        IF_DEBUG(else 
            printf("somme ctrl 👎\n \t-ctrl sum : %d -> %d\n \t-demande du paquet %d\n",paquet.somme_ctrl,paquet.somme_ctrl ^ creer_somme_ctrl(paquet),ack.num_seq));        
        vers_reseau(&ack);


        IF_DEBUG(printf("-%d (paquet_a_recevoir : %d ; fin : %d)\n",compteur_boucle,paquet_a_recevoir,fin)); // si le debug mode active
        IF_DEBUG(compteur_boucle ++); // si le debug mode active
    }


    // cas du dernier ack perdu 
    depart_temporisateur(200);
    while(attendre() == -1){
        de_reseau(&paquet);
        arret_temporisateur();
        vers_reseau(&ack);
        depart_temporisateur(200);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
