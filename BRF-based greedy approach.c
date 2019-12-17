#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSTANCEFILE "instance.txt"
#define PLAYERFILE "player.txt"

typedef int Instance;
typedef struct player{
	int id;
	char *name;
	float bid;
	int *bundle;
	float item_num;
	float weight;
	int win;
}Bidder;

int instance_num;
int player_num;

void SWAP(Bidder* , Bidder*);
int partition(Bidder* , int , int);
void qsort_bidders(Bidder* , int , int);
Instance* read_instances();
Bidder* read_players();
void BRF_greedy(Bidder* , Instance*);

int main(){
	Instance *q = read_instances();
	Bidder *p = read_players();
	BRF_greedy(p, q);
	return 0;
}

void SWAP(Bidder *x, Bidder *y){
	Bidder t;
	t = *x;
	*x = *y;
	*y = t;
}

int partition(Bidder *a, int l, int r){
	int i = l-1;
	int j;
	for(j=l; j<r; j++){
		if((a+j)->weight >= (a+r)->weight){
			i++;
			SWAP((a+i), (a+j));
		}
	}
	SWAP((a+(i+1)), (a+r));
	return i+1;
}

void qsort_bidders(Bidder *a, int l, int r){
	if(l < r){
		int q = partition(a, l, r);
		qsort_bidders(a, l, q-1);
		qsort_bidders(a, q+1, r);
	}
}

Instance* read_instances(){
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	char *token;
	
	FILE *fp;
	int *q;

	if( (fp = fopen(INSTANCEFILE, "r")) == NULL ){
		fprintf(stderr, "Error opening file '%s'\n", INSTANCEFILE);
		exit(0);
	}

	getline(&line_buf, &line_buf_size, fp);
	instance_num = atoi(line_buf);
	q = malloc(instance_num * sizeof(Bidder));

	getline(&line_buf, &line_buf_size, fp);
	token = strtok(line_buf, "\t");
	for(int i=0; token || i<instance_num; i++){
		*(q+i) = atoi(token);
		token = strtok(NULL, "\t");
	}
	return q;
}

Bidder* read_players(){
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	char *token;
	Bidder *p;
	FILE *fp;
	
	if( (fp = fopen(PLAYERFILE, "r")) == NULL ){
		fprintf(stderr, "Error opening file '%s'\n", PLAYERFILE);
		exit(0);
	}

	getline(&line_buf, &line_buf_size, fp);
	player_num = atoi(line_buf);
	p = malloc(player_num * sizeof(Bidder));

	getline(&line_buf, &line_buf_size, fp);
	token = strtok(line_buf, "\t");
	

	for(int i=0; token && i<player_num; i++){
		(p+i)->name = malloc(sizeof(token));
		if(token[strlen(token)-1] == '\n')
			strncpy((p+i)->name, token, strlen(token)-1);
		else
			strncpy((p+i)->name, token, strlen(token));
		token = strtok(NULL, "\t");
	}

	getline(&line_buf, &line_buf_size, fp);
	token = strtok(line_buf, "\t");
	for(int i=0; token && i<player_num; i++){
		(p+i)->bid = atoi(token);
		token = strtok(NULL, "\t");
	}

	int *tmp;
	for(int i=0; i<player_num; i++)
		(p+i)->bundle = malloc(instance_num * sizeof(int));
	for(int i=0; i<instance_num; i++){
		getline(&line_buf, &line_buf_size, fp);
		token = strtok(line_buf, "\t");
		for(int j=0; token && j<player_num; j++){
			*(((p+j)->bundle)+i) = atoi(token);
			((p+j)->item_num) += atoi(token);
			token = strtok(NULL, "\t");
		}
	}
	for(int i=0; i<player_num; i++){
		(p+i)->id = i;
		(p+i)->weight = (p+i)->bid / (p+i)->item_num;
	}

	fclose(fp);
	return p;
}

void BRF_greedy(Bidder *p, Instance *q){
	int winner_num = 0;
	
	//Ranking players

	Bidder *tmp = malloc(player_num * sizeof(Bidder));
	memcpy(tmp, p, player_num * sizeof(Bidder)); 
	qsort_bidders(tmp, 0, player_num-1);

	printf("Player Ranking:\n");
	for(int i=0; i<player_num-1; i++)
		printf("%s  <\t", tmp[i].name);
	printf("%s\n", tmp[player_num-1].name);

	//greedy-pick

	int lack;
	for(int i=0; i<player_num; i++){
		lack = 0;
		for(int j=0; j<instance_num; j++){
			if(tmp[i].bundle[j] > q[j]){
				lack = 1;
				break;
			}
		}
		if(lack == 0){
			winner_num++;
			p[tmp[i].id].win = 1;
			for(int j=0; j<instance_num; j++)
				q[j] -= tmp[i].bundle[j];
		}
	}
	free(tmp);

	//output

	printf("\nWinner Set: \n{");
	int cut;
	for(int i=0; winner_num > 1 && i<player_num; i++){
		if(p[i].win){
			winner_num--;
			printf("%s, ", p[i].name);
		}
		cut = i+1;
	}
	for(int i=cut; i<player_num; i++){
		if(p[i].win){
			winner_num--;
			printf("%s}\n", p[i].name);
		}
	}

	printf("\nAllocated Result: \n");
	for(int i=0; i<player_num; i++)
		printf("%s\t", p[i].name);
	printf("\n");
	
	for(int i=0; i<player_num; i++){
		if(p[i].win)
			printf("win\t");
		else
			printf(" \t");
	}
	printf("\n");
	free(p);
	free(q);
}