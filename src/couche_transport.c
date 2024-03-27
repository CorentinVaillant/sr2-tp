#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...

/*fonction retournant la somme de controle d'un paquet*/
int8_t creer_somme_ctrl(paquet_t paquet){
    int8_t somme = 0;
    somme = paquet.type;
    somme ^= paquet.num_seq;
    somme ^= paquet.lg_info;
    for(int i = 0 ; i<paquet.lg_info ; i++){
        somme ^= paquet.info[i];
    }
    
    return somme;
}

int test_somme_ctrl(paquet_t paquet){
    int8_t ctrl_somme = paquet.somme_ctrl;
    int8_t somme = creer_somme_ctrl(paquet);
    return((ctrl_somme ^ somme) == 0x0000);
    
}



/* ===================== Fenêtre d'anticipation ============================= */

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
