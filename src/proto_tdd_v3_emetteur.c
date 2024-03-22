/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* C. Vaillant - Univ. de Toulouse III - Paul Sabatier        *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define TEMPO 100
#define NUMEROTATION_FEN 16

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquets[NUMEROTATION_FEN]; /* paquets utilisés par le protocole */
    paquet_t ack; /*paquet de gestion d'erreur*/


    int taille_fen;
    sscanf(argv[1],"%d",&taille_fen); /*lecture de la taille fenètre donné par l'utilisateur*/

    int deb_fen = 0; /*entier correspondant au pointeur du ddébut de la fenètre*/
    int prochain_pack = 0; /*entier corresspondant au pointeur du prochain paquet à envoyer*/

    // int timeout = 0; /*entier permettant de gérer le temporisateur*/ // toremove

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);



    int compteur_fen = 0; // toremove

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {
        if (prochain_pack < deb_fen + taille_fen){
            depart_temporisateur(TEMPO);
            printf("depart temporisateur\n"); // toremove
            //envoie et construction paquets[prochain_pack]
            printf("création du paquet n°%d\n",prochain_pack); // toremove
            for (int i=0; i<taille_msg; i++) {
                paquets[prochain_pack].info[i] = message[i];
            }
            paquets[prochain_pack].num_seq = prochain_pack;
        //tat
            paquets[prochain_pack].lg_info = taille_msg;
            paquets[prochain_pack].type = DATA;
            paquets[prochain_pack].somme_ctrl = creer_somme_ctrl(paquets[prochain_pack]);

            vers_reseau(&paquets[prochain_pack]);//envoie
            printf("⬅️ envoie du paquet n°%d\n",prochain_pack); // toremove
            de_application(message, &taille_msg);

            //incrementation de prochain_pack
            prochain_pack +=1;
            prochain_pack %= NUMEROTATION_FEN;
            
            printf("prochaint_pack incrémenter :%d\n",prochain_pack); // toremove
        }

        
        if (attendre() == -1){
            de_reseau(&ack);
            printf("⮩ reception du ack n°%d\n",ack.num_seq); // toremove
            deb_fen = ack.num_seq; 
            printf("deb_fenetre = %d\n", deb_fen); // toremove

            
            
            if(deb_fen == prochain_pack){
                arret_temporisateur();
                printf("arret temporisateur\n"); // toremove
            }
            else{
                arret_temporisateur();
                depart_temporisateur(TEMPO);
                printf("reset tempo (deb_fen != prochain_pack)\n"); // toremove

            }
        }
        else{
            printf("TIMEOUT\n"); // toremove
            depart_temporisateur(TEMPO);
            printf("depart temporisateur (TIMEOUT)\n"); // toremove
            for(int i = deb_fen ; i<prochain_pack ; i++){
                vers_reseau(&paquets[i]);
                printf("envoie du paquet %d\n",i);
            }
        }

        printf("-%d\n",compteur_fen); // toremove
        compteur_fen ++; // toremove
        
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
