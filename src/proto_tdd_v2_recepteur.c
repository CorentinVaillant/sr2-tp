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

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */

    paquet_t ack; /*gestion d'erreur*/

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        do{
            // attendre(); /* optionnel ici car de_reseau() fct bloquante */
            de_reseau(&paquet);

            
            if(test_somme_ctrl(paquet)){
                ack.type = ACK;
                printf("somme ctrl 👍\n paquet n°%d reçu\n",paquet.num_seq);
            }
            else{
                ack.type = NACK;
                printf("somme ctrl 👎\n, ctrl sum : %d -> %d\n",paquet.somme_ctrl,paquet.somme_ctrl ^ creer_somme_ctrl(paquet));
            }
            vers_reseau(&ack);
        }while (ack.type == NACK);
        
        /* extraction des donnees du paquet recu */
        for (int i=0; i<paquet.lg_info; i++) {
            message[i] = paquet.info[i];
        }
        /* remise des données à la couche application */
        fin = vers_application(message, paquet.lg_info);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
