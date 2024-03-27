/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
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
#define DEFAULT_WIN_SIZE 7

#define DEBUG 0

#if DEBUG
#define IF_DEBUG(FORM) FORM
#else
#define IF_DEBUG(FORM)
#endif





/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquets[SEQ_NUM_SIZE]; /* paquets utilisés par le protocole */
    paquet_t ack; /*paquet de gestion d'erreur*/


    unsigned int taille_fen;
    if(argc  == 1){
        taille_fen = DEFAULT_WIN_SIZE;
    }else
        sscanf(argv[1],"%u",&taille_fen); /*lecture de la taille fenètre donné par l'utilisateur*/

    int deb_fen = 0; /*entier correspondant au pointeur du début de la fenètre*/
    int prochain_pack = 0; /*entier corresspondant au pointeur du prochain paquet à envoyer*/


    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);


    IF_DEBUG( int compteur_boucle = 0); //seulement en mode debug

    
    
    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || deb_fen != prochain_pack) {
        
        if (dans_fenetre(deb_fen,prochain_pack,taille_fen) && taille_msg != 0){
            /*envoie et construction paquets[prochain_pack]*/
            IF_DEBUG(printf("création du paquet n°%d\n",prochain_pack)); //seulement en mode debug
            for (int i=0; i<taille_msg; i++) {
                paquets[prochain_pack].info[i] = message[i];
                IF_DEBUG(printf("📦")); //seulement en mode debug
            }
            IF_DEBUG(printf("(%doct)\n",taille_msg)); //seulement en mode debug
            paquets[prochain_pack].num_seq = prochain_pack;

            paquets[prochain_pack].lg_info = taille_msg;
            paquets[prochain_pack].type = DATA;
            paquets[prochain_pack].somme_ctrl = creer_somme_ctrl(paquets[prochain_pack]);

    
            if(taille_msg != 0){
                de_application(message, &taille_msg);
            }
            //envoie du paquet
            vers_reseau(&paquets[prochain_pack]);//envoie
            IF_DEBUG(printf("⬅️ envoie du paquet n°%d\n",prochain_pack)); //seulement en mode debug


            if(deb_fen == prochain_pack){
                depart_temporisateur(TEMPO);
                IF_DEBUG(printf("depart temporisateur (deb == prochain pack)\n")); //seulement en mode debug
            }
            //incrementation de prochain_pack
            prochain_pack +=1;
            prochain_pack %= SEQ_NUM_SIZE;
            
            IF_DEBUG(printf("prochaint_pack incrémenter :%d\n",prochain_pack)); //seulement en mode debug

        }
        else {
            IF_DEBUG(printf("no new packages\n")); //seulement en mode debug
            if (attendre() == -1 ){
                de_reseau(&ack);
                IF_DEBUG(printf("⮩ reception du ack n°%d\n",ack.num_seq)); //seulement en mode debug
                if (test_somme_ctrl(ack) && dans_fenetre(deb_fen,ack.num_seq,taille_fen) ){
                    deb_fen = ack.num_seq ; 

                    IF_DEBUG(printf("deb_fenetre = %d\n", deb_fen)); //seulement en mode debug

                    if(deb_fen == prochain_pack){
                        arret_temporisateur();
                        IF_DEBUG(printf("arret temporisateur (ack reçu && deb_fen == prochain_pack)\n")); //seulement en mode debug
                    }
                }
                IF_DEBUG(else printf("❌ ack hors séquence ou somme de ctrl 👎, ignorée (n°%d (%d pas dans fenetre))\n",ack.num_seq,(ack.num_seq)%SEQ_NUM_SIZE)); //seulement en mode debug
            

            }
            else{
                depart_temporisateur(TEMPO);
                IF_DEBUG(printf("⏲️ TIMEOUT\n")); //seulement en mode debug
                IF_DEBUG(printf("depart tempo(TIMEOUT)\n")); //seulement en mode debug


                int i = deb_fen;
                while (i != prochain_pack){
                    vers_reseau(&paquets[i]);
                    i += 1;
                    i %= SEQ_NUM_SIZE;
                }
                
            }

        }

        IF_DEBUG(printf("-%d (deb_fen : %d ; prochain_pack : %d)\n",compteur_boucle,deb_fen,prochain_pack)); //seulement en mode debug
        IF_DEBUG(compteur_boucle ++); //seulement en mode debug
    }

    

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");

    IF_DEBUG(printf("EXIT DEBUG MODE\n");)



    return 0;
}
