/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* C. Vaillant - Univ. de Toulouse III - Paul Sabatier        *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define NUMEROTATION_FEN 16

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

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {


        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);

        
        if(test_somme_ctrl(paquet)){
            if(paquet.num_seq == paquet_a_recevoir){
                printf("somme ctrl 👍\n paquet n°%d reçu\n",paquet.num_seq);
                
                /* extraction des donnees du paquet recu */
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);


                ack.num_seq = paquet_a_recevoir;
                ack.somme_ctrl = creer_somme_ctrl(ack);
                
                paquet_a_recevoir ++; paquet_a_recevoir %= SEQ_NUM_SIZE;


            }
            else{
                printf("mauvais paquet ❌ %d reçu\n \t-demande %d\n",paquet.num_seq,paquet_a_recevoir);
            }
        }
        else{
            printf("somme ctrl 👎\n \t-ctrl sum : %d -> %d\n \t-demande du paquet %d\n",paquet.somme_ctrl,paquet.somme_ctrl ^ creer_somme_ctrl(paquet),paquet_a_recevoir);
        }
        vers_reseau(&ack);
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
