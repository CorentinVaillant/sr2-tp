/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define TEMPO 100

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t ack; /*paquet de gestion d'erreur*/

    int timeout; /*gestion temporisateur*/

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.somme_ctrl = creer_somme_ctrl(paquet);
        do{
            /* remise à la couche reseau */
            vers_reseau(&paquet);

            /*gestion timeout */
            depart_temporisateur(TEMPO);
            if(attendre() != -1){
                printf("error temp\n");
                timeout = 1;
            }
            else{
                arret_temporisateur();
                timeout = 0;

                /*reception du ack*/
                de_reseau(&ack);
                /*gestion d'erreur*/
                if(ack.type == NACK){
                    printf("error ctrl sum : %d\n",paquet.somme_ctrl);
                }
            }

        }while(ack.type == NACK || (timeout==1));

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
