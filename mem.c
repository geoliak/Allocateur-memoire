#include "mem.h"
struct fb { /* fb pour free block */
size_t size ;
struct fb *next ;
};

struct fb* tete;
mem_fit_function_t* g_fonction_defaut;
void* memoire;
size_t taille_totale;

void mem_init(char* mem, size_t taille){

	if(memoire != NULL){
		free(memoire);
	}
	struct fb *(*mem_fit_first_ptr)(struct fb*, size_t) = &mem_fit_first; //crétion prointeur de mem_fit_first
	mem_fit(*mem_fit_first_ptr); // on défninie mem_fit_first comme recherche par defaut
	mem = malloc(taille); //on alloue la plage mémoire
	taille_totale = taille;
	memoire = mem;        //On garde l'adresse de la plage memoire pour pouvoir eventuellement la liberer
	tete =(struct fb *) mem; //on place la tête de liste en début de la plage mémoire
	tete->size = taille;
}

//change la fonction de recherche
void mem_fit(mem_fit_function_t* fonction){
	g_fonction_defaut = fonction;
}

//renvoit l'adresse du premier block de taille ou égale à size ou renvoie NULL
struct fb* mem_fit_first(struct fb* list, size_t size){
	struct fb* tete = list;
	while(tete != NULL && tete->size < size + sizeof(struct fb)){
		tete = tete->next;
	}
	return tete;
}

//reçoit taille à allouer, retourne une adresse ou NULL
void* mem_alloc(size_t taille){

	size_t taille_a_ecrire = taille + sizeof(size_t); //taille totale = taille + 8 octets
	struct fb* libre = g_fonction_defaut(tete,taille_a_ecrire);
	if(libre == NULL){
		return NULL;
	}
	else{
		size_t* ecriture = (size_t*)( ((void*)libre) + libre->size - taille_a_ecrire);//adresse écriture taille= adresse zone libre + taille zone libre - taille totale à allouer
		*ecriture = taille_a_ecrire;//écrit la taille
		libre->size = libre->size - taille_a_ecrire;
		return (void*) ecriture++;//retourne l'adresse = adresse écriture taille + 8 octets
	}


}

size_t mem_get_size(void *zone){
	return *(size_t*)zone;
}

void mem_free(void *zone){
	struct fb *cp_tete = tete;
	struct fb *z_avant = NULL;
	struct fb *z_apres_next = NULL;

	while(cp_tete->next!=NULL){
		if(&cp_tete->next == (zone + (size_t)zone)){
			z_apres_next = cp_tete;
		}
		if ((&cp_tete + cp_tete->size)==zone){
			z_avant = cp_tete;
		}
		cp_tete=cp_tete->next;
	}

	if(z_avant==NULL && z_apres_next == NULL){
		size_t temp = (size_t) zone;
		cp_tete = tete;
		tete = (struct fb*)zone;
		tete->next = cp_tete;
		tete->size = temp;
	}
	else if (z_avant!=NULL && z_apres_next==NULL){
		z_avant->size += (size_t) zone;
	}
	else if (z_avant==NULL && z_apres_next!=NULL){
		struct fb* n; // nouvelle structure fb
		n = (struct fb *) zone; //adresse en début de zone
		n->next = z_apres_next->next->next; //copie de l'adresse
		n->size = (size_t) zone + z_apres_next->next->size; //ajout des deux tailles
		z_apres_next->next = n; //pointeur précédent dans la chaîne changé
	}
	else{
		z_avant->size += (size_t) zone + z_apres_next->next->size;
		z_apres_next->next = z_apres_next->next->next;

	}

}

void mem_show(void (*print)(void *, size_t, int free)){

	struct fb* current = memoire;

		//printf("oups! mauvaise alignement current = %p memoire = %p tete = %p", current, memoire, tete);

	while((void *)current <= (void *)(memoire + taille_totale)){
		struct fb* tete_test = tete;
		int found = 0;
		while(tete_test != NULL){
			if(tete_test == current){
				print(current,current->size,1);
				found = 1;
			}
			tete_test = tete_test->next;
		}
		if(found == 0){
			print(current,current->size,0);
			found = 1;
			void * temp = ((void*)current) + (size_t)current;
			current = temp;
		}
		else{
			current = current + current->size;
		}

	}
}



