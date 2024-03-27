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

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquets[SEQ_NUM_SIZE]; /* paquets utilisés par le protocole */
    paquet_t ack; /*paquet de gestion d'erreur*/


    uint8_t taille_fen;
    if(argc  == 1){
        taille_fen = 7;
    }else
        sscanf(argv[1],"%hhu",&taille_fen); /*lecture de la taille fenètre donné par l'utilisateur*/

    uint8_t deb_fen = 0; /*entier correspondant au pointeur du début de la fenètre*/
    uint8_t prochain_pack = 0; /*entier corresspondant au pointeur du prochain paquet à envoyer*/


    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);


    int compteur_boucle = 0; // toremove

    
    
    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {
        
        if (dans_fenetre(deb_fen,prochain_pack,taille_fen)){
            /*envoie et construction paquets[prochain_pack%NUMEROTATION_FEN]*/
            printf("création du paquet n°%d\n",prochain_pack); // toremove
            for (int i=0; i<taille_msg; i++) {
                paquets[prochain_pack].info[i] = message[i];
            }
            paquets[prochain_pack].num_seq = prochain_pack;

            paquets[prochain_pack].lg_info = taille_msg;
            paquets[prochain_pack].type = DATA;
            paquets[prochain_pack].somme_ctrl = creer_somme_ctrl(paquets[prochain_pack]);

            de_application(message, &taille_msg);
            vers_reseau(&paquets[prochain_pack]);//envoie
            printf("⬅️ envoie du paquet n°%d\n",prochain_pack); // toremove

            //incrementation de prochain_pack
            prochain_pack +=1;
            prochain_pack %= SEQ_NUM_SIZE;
            
            printf("prochaint_pack incrémenter :%d\n",prochain_pack); // toremove

            if(deb_fen == prochain_pack){
                depart_temporisateur(TEMPO);
                printf("depart temporisateur (deb == prochain pack)\n"); // toremove
            }

        }
        else {

            if (attendre() == -1){
                de_reseau(&ack);
                printf("⮩ reception du ack n°%d\n",ack.num_seq); // toremove
                if (test_somme_ctrl(ack) && dans_fenetre(deb_fen,ack.num_seq,taille_fen) ){
                    deb_fen = ack.num_seq+1; 
                    deb_fen %= SEQ_NUM_SIZE;
                    printf("deb_fenetre = %d\n", deb_fen); // toremove

                    if(deb_fen == prochain_pack)
                        arret_temporisateur();
                    printf("arret temporisateur (ack reçu && deb_fen == prochain_pack)\n"); // toremove
                }
                else printf("\x1B[31m----\x1B[0mack hors séquence ou somme de ctrl 👎, ignorée (n°%d)\n",ack.num_seq); // toremove
            

            }
            else{
                printf("⏲️ TIMEOUT\n"); // toremove
                depart_temporisateur(TEMPO);
                printf("depart tempo(TIMEOUT)\n"); // toremove

                int i = deb_fen;
                while (i != prochain_pack){
                    vers_reseau(&paquets[i]);
                    i += 1;
                    i %= SEQ_NUM_SIZE;
                }
                
            }

        }

        printf("-%d (deb_fen : %d ; prochain_pack : %d)\n",compteur_boucle,deb_fen,prochain_pack); // toremove
        compteur_boucle ++; // toremove
        
        
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
