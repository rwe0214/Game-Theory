#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITEMFILE "item.txt"
#define PLAYERFILE "player.txt"

typedef int Instance;
typedef struct player {
    int id;
    char *name;
    float bid;
    int *bundle;
    float instance_num;
    float weight;
    int win;
    float payment;
} Bidder;

int item_num;
int player_num;

void SWAP(Bidder*, Bidder*);
int partition(Bidder*, int, int);
void qsort_bidders(Bidder*, int, int);
Instance* read_instances();
Bidder* read_players();
Bidder* rank_player(Bidder*);
float find_payment(int, Bidder*, Bidder*, Instance*);
void BRF_greedy(Bidder*, Instance*);

int main()
{
    Instance *q = read_instances();
    Bidder *p = read_players();
    BRF_greedy(p, q);
    return 0;
}

void SWAP(Bidder *x, Bidder *y)
{
    Bidder t;
    t = *x;
    *x = *y;
    *y = t;
}

int partition(Bidder *a, int l, int r)
{
    int i = l-1;
    int j;
    for(j=l; j<r; j++) {
        if((a+j)->weight >= (a+r)->weight) {
            i++;
            SWAP((a+i), (a+j));
        }
    }
    SWAP((a+(i+1)), (a+r));
    return i+1;
}

void qsort_bidders(Bidder *a, int l, int r)
{
    if(l < r) {
        int q = partition(a, l, r);
        qsort_bidders(a, l, q-1);
        qsort_bidders(a, q+1, r);
    }
}

Instance* read_instances()
{
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    char *token;

    FILE *fp;
    int *q;

    if( (fp = fopen(ITEMFILE, "r")) == NULL ) {
        fprintf(stderr, "Error opening file '%s'\n", ITEMFILE);
        exit(0);
    }

    getline(&line_buf, &line_buf_size, fp);
    item_num = atoi(line_buf);
    q = malloc(item_num * sizeof(Bidder));

    getline(&line_buf, &line_buf_size, fp);
    token = strtok(line_buf, "\t");
    for(int i=0; token || i<item_num; i++) {
        *(q+i) = atoi(token);
        token = strtok(NULL, "\t");
    }
    return q;
}

Bidder* read_players()
{
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    char *token;
    Bidder *p;
    FILE *fp;

    if( (fp = fopen(PLAYERFILE, "r")) == NULL ) {
        fprintf(stderr, "Error opening file '%s'\n", PLAYERFILE);
        exit(0);
    }

    getline(&line_buf, &line_buf_size, fp);
    player_num = atoi(line_buf);
    p = malloc(player_num * sizeof(Bidder));

    getline(&line_buf, &line_buf_size, fp);
    token = strtok(line_buf, "\t");


    for(int i=0; token && i<player_num; i++) {
        (p+i)->name = malloc(sizeof(token));
        if(token[strlen(token)-1] == '\n')
            strncpy((p+i)->name, token, strlen(token)-1);
        else
            strncpy((p+i)->name, token, strlen(token));
        token = strtok(NULL, "\t");
    }

    getline(&line_buf, &line_buf_size, fp);
    token = strtok(line_buf, "\t");
    for(int i=0; token && i<player_num; i++) {
        (p+i)->bid = atoi(token);
        token = strtok(NULL, "\t");
    }

    int *tmp;
    for(int i=0; i<player_num; i++)
        (p+i)->bundle = malloc(item_num * sizeof(int));
    for(int i=0; i<item_num; i++) {
        getline(&line_buf, &line_buf_size, fp);
        token = strtok(line_buf, "\t");
        for(int j=0; token && j<player_num; j++) {
            *(((p+j)->bundle)+i) = atoi(token);
            ((p+j)->instance_num) += atoi(token);
            token = strtok(NULL, "\t");
        }
    }
    for(int i=0; i<player_num; i++) {
        (p+i)->id = i;
        (p+i)->weight = (p+i)->bid / (p+i)->instance_num;
    }

    fclose(fp);
    return p;
}

Bidder* rank_player(Bidder *p)
{
    Bidder *tmp_p = malloc(player_num * sizeof(Bidder));
    memcpy(tmp_p, p, player_num * sizeof(Bidder));

    qsort_bidders(tmp_p, 0, player_num-1);

    //output rank
    printf("Player Ranking:\n");
    for(int i=0; i<player_num-1; i++)
        printf("%s  <\t", tmp_p[i].name);
    printf("%s\n", tmp_p[player_num-1].name);

    return tmp_p;
}

float find_payment(int i, Bidder *rank_p, Bidder *p, Instance *q)
{
    int *total_unit = malloc(item_num * sizeof(int));
    memset(total_unit, 0, item_num * sizeof(int));
    //printf("asd");
    int flag = 0;
    for(int j=0; j<i; j++)
        if(rank_p[j].win)
            for(int k=0; k<item_num; k++)
                total_unit[k] += rank_p[j].bundle[k];
    for(int j=i+1; j<player_num; j++) {
        for(int k=0; k<item_num; k++) {
            if(rank_p[j].win) {
                total_unit[k] += rank_p[j].bundle[k];
                flag = 1;
            } else {
                if(total_unit[k] + rank_p[j].bundle[k] > q[k])
                    flag = 1;
            }
        }
        if(!flag)
            return rank_p[j].weight * rank_p[i].instance_num;
    }
    return 0;
}

void BRF_greedy(Bidder *p, Instance *q)
{
    int winner_num = 0;
    int *tmp_q = malloc(item_num * sizeof(int));
    memcpy(tmp_q, q, item_num * sizeof(int));

    //Ranking players
    Bidder *ranked_p = rank_player(p);

    //greedy-pick
    int lack;
    for(int i=0; i<player_num; i++) {
        lack = 0;
        for(int j=0; j<item_num; j++) {
            if(ranked_p[i].bundle[j] > tmp_q[j]) {
                lack = 1;
                break;
            }
        }
        if(!lack) {
            winner_num++;
            ranked_p[i].win = 1;
            p[ranked_p[i].id].win = 1;
            for(int j=0; j<item_num; j++)
                tmp_q[j] -= ranked_p[i].bundle[j];
        }
    }
    free(tmp_q);

    //determine payment
    for(int i=0; i<player_num; i++)
        if(ranked_p[i].win)
            p[ranked_p[i].id].payment = find_payment(i, ranked_p, p, q);
    free(ranked_p);

    //output result
    printf("\nWinner Set: \n{");
    int last;
    for(int i=0; winner_num > 1 && i<player_num; i++) {
        if(p[i].win) {
            winner_num--;
            printf("%s, ", p[i].name);
        }
        last = i+1;
    }
    for(int i=last; i<player_num; i++) {
        if(p[i].win) {
            winner_num--;
            printf("%s}\n", p[i].name);
        }
    }

    printf("\n\nAllocated Result: \n\n\t");
    for(int i=0; i<player_num; i++)
        printf("| %s\t", p[i].name);
    printf("\n--------|-------|-------|-------|-------|-------");
    printf("\nresult\t");

    for(int i=0; i<player_num; i++) {
        if(p[i].win)
            printf("| win\t");
        else
            printf("| \t");
    }
    printf("\n--------|-------|-------|-------|-------|-------");
    printf("\npayment\t");
    for(int i=0; i<player_num; i++) {
        if(p[i].win)
            printf("| %.2f\t", p[i].payment);
        else
            printf("| \t");
    }
    printf("\n\n");
    free(p);
    free(q);
}