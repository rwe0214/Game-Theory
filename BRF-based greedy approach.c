#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITEMFILE "item.txt"
#define PLAYERFILE "player.txt"
#define RED_BOLD "\x1b[;31;1m"
#define BLU_BOLD "\x1b[;34;1m"
#define CYAN_BOLD "\x1b[;36;1m"
#define GRN_BOLD "\x1b[;32;1m"

#define RESET "\x1b[0;m"

typedef int Instance;
typedef struct player {
    int id;
    char *name;
    float bid;
    int *bundle;
    float instance_num;
    float weight;
    int win;
    int key_suc;
    int key_pre;
    float payment;
    float v_c;
} Bidder;

int item_num;
int player_num;

void BRF(Bidder*, Instance*);
Bidder* rank_player(Bidder*);
int greedy_pick(Bidder*, Bidder*, Instance*);
void determine_payment(Bidder*, Bidder*, Instance*);
void print_result(Bidder*, int);

void SWAP(Bidder*, Bidder*);
int partition(Bidder*, int, int);
void qsort_bidders(Bidder*, int, int);

Instance* read_instances();
Bidder* read_players();

float find_payment(int, Bidder*, Bidder*, Instance*);
float find_critical_val(int, Bidder*, Bidder*, Instance*);

int main()
{
    Instance *q = read_instances();
    Bidder *p = read_players();
    BRF(p, q);
    return 0;
}

void BRF(Bidder *p, Instance *q)
{
    Bidder *ranked_p = rank_player(p);
    int winner_num = greedy_pick(p, ranked_p, q);
    determine_payment(p, ranked_p, q);
    print_result(p, winner_num);
    free(p);
    free(q);
}

Bidder* rank_player(Bidder *p)
{
    Bidder *tmp_p = malloc(player_num * sizeof(Bidder));
    memcpy(tmp_p, p, player_num * sizeof(Bidder));

    qsort_bidders(tmp_p, 0, player_num-1);

    //output rank
    printf("\nStep 1: Ranking\n\t");
    for(int i=0; i<player_num-1; i++)
        printf("%s (%.2f)  >>\t", tmp_p[i].name, tmp_p[i].weight);
    printf("%s (%.2f)\n", tmp_p[player_num-1].name, tmp_p[player_num-1].weight);

    return tmp_p;
}

int greedy_pick(Bidder *p,Bidder *rank_p, Instance *q)
{
    int *tmp_q = malloc(item_num * sizeof(int));
    memcpy(tmp_q, q, item_num * sizeof(int));
    int lack, winner_num = 0;
    printf("\nStep 2: Greedy-Picking\n");
    printf("\tItems\t\t| Picked-Player\n");
    printf("\t----------------|--------------------------\n");
    for(int i=0; i<player_num; i++) {
        lack = 0;

        //print numbers of instance of each item
        printf("\t(");
        for(int k=0; k<item_num-1; k++) {
            printf("%d, ", tmp_q[k]);
        }
        printf("%d)\t| ", tmp_q[item_num-1]);

        printf(GRN_BOLD"%s", rank_p[i].name);
        printf(RESET" (");
        for(int j=0; j<item_num; j++) {
            if(rank_p[i].bundle[j] > tmp_q[j]) {
                lack = 1;
                printf(RED_BOLD"%d", rank_p[i].bundle[j]);
                break;
            } else {
                if(j < item_num-1)
                    printf("%d, ", rank_p[i].bundle[j]);
                else
                    printf("%d", rank_p[i].bundle[j]);
            }
        }
        if(!lack) {
            winner_num++;
            rank_p[i].win = 1;
            p[rank_p[i].id].win = 1;
            for(int j=0; j<item_num; j++)
                tmp_q[j] -= rank_p[i].bundle[j];
        }
        printf(RESET")");
        if(lack)
            printf(RED_BOLD" LOSE!");
        else
            printf(CYAN_BOLD" WIN!");
        printf(RESET"\n");
        printf("\t----------------|--------------------------\n");
    }
    printf("\t(");
    for(int k=0; k<item_num-1; k++) {
        printf("%d, ", tmp_q[k]);
    }
    printf("%d)\t|\n\n", tmp_q[item_num-1]);
    free(tmp_q);
    return winner_num;
}

void determine_payment(Bidder *p, Bidder *rank_p, Instance *q)
{
    printf("\nStep 3: Determine Payment / Critical Value\n");
    for(int i=0; i<player_num; i++)
        if(rank_p[i].win)
            p[rank_p[i].id].payment = find_payment(i, rank_p, p, q);
        else
            p[rank_p[i].id].v_c = find_critical_val(i, rank_p, p, q);
    free(rank_p);
}

void print_result(Bidder *p, int w)
{
    printf("\n\nAllocated Result: \n");
    printf("\tWinner Set: \n\t{");
    int last;
    for(int i=0; w > 1 && i<player_num; i++) {
        if(p[i].win) {
            w--;
            printf("%s, ", p[i].name);
        }
        last = i+1;
    }
    for(int i=last; i<player_num; i++) {
        if(p[i].win) {
            w--;
            printf("%s}\n\n\t\t\t", p[i].name);
        }
    }
    for(int i=0; i<player_num; i++)
        printf("| %s\t\t", p[i].name);
    printf(GRN_BOLD"\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tbundles\t\t");
    for(int i=0; i<player_num; i++){
        printf("|(");
        for(int j=0; j<p[i].instance_num; j++)
            printf("%d, ", p[i].bundle[j]);
        if(p[i].instance_num == 4)
            printf("%d)", p[i].bundle[(int)p[i].instance_num]);
        else
            printf("%d)\t", p[i].bundle[(int)p[i].instance_num]);
    }
    printf(GRN_BOLD"\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tbid\t\t");
    for(int i=0; i<player_num; i++)
        printf("| %.2f\t\t", p[i].bid);
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tnumbers of items");
    for(int i=0; i<player_num; i++)
        printf("| %0.f\t\t", p[i].instance_num);
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tweight\t\t");
    for(int i=0; i<player_num; i++)
        printf("| %.2f\t\t", p[i].weight);
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tkey successor\t");
    for(int i=0; i<player_num; i++) {
        if(p[i].win) {
            if(p[i].key_suc > 0)
                printf("| %s\t\t", p[p[i].key_suc].name);
            else
                printf("| X\t\t");
        } else
            printf("| \t\t");
    }
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tkey predecessor\t");
    for(int i=0; i<player_num; i++) {
        if(!p[i].win) {
            if(p[i].key_pre > 0)
                printf("| %s\t\t", p[p[i].key_pre].name);
            else
                printf("| X\t\t");
        } else
            printf("| \t\t");
    }
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf(BLU_BOLD"\n\tresult\t\t");
    for(int i=0; i<player_num; i++) {
        if(p[i].win)
            printf("| win\t\t");
        else
            printf("| \t\t");
    }
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tpayment\t\t");
    for(int i=0; i<player_num; i++) {
        if(p[i].win)
            if(p[i].payment >= 100)
                printf("| %.2f\t", p[i].payment);
            else
                printf("| %.2f\t\t", p[i].payment);
        else if(p[i].payment >= 100)
            printf("| %.2f\t", p[i].payment);
        else
            printf("| %.2f\t\t", p[i].payment);
    }
    printf("\n\t----------------|---------------|---------------|---------------|---------------|---------------");

    printf("\n\tcritical value\t");
    for(int i=0; i<player_num; i++) {
        if(!p[i].win)
            if(p[i].v_c >= 100)
                printf("| %.2f\t", p[i].v_c);
            else
                printf("| %.2f\t\t", p[i].v_c);
        else if(p[i].v_c >= 100)
            printf("| %.2f\t", p[i].v_c);
        else
            printf("| %.2f\t\t", p[i].v_c);
    }
    printf("\n\n");
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

float find_payment(int i, Bidder *rank_p, Bidder *p, Instance *q)
{
    int *total_unit = malloc(item_num * sizeof(int));
    memset(total_unit, 0, item_num * sizeof(int));

    printf("\tFor winner %s: \n", rank_p[i].name);

    for(int j=0; j<i; j++)
        if(rank_p[j].win)
            for(int k=0; k<item_num; k++)
                total_unit[k] += rank_p[j].bundle[k];
    printf("\t\t(");
    for(int i=0; i<item_num-1; i++) {
        printf("%d, ", total_unit[i]);
    }
    printf("%d) -> ", total_unit[item_num-1]);
    printf("the instance which is used by the player whose rank is higher\n");

    for(int j=i+1; j<player_num; j++) {
        int flag = 0;
        if(rank_p[j].win) {
            printf("\t\t  ***Add the instances which is used by the winner ");
            printf(CYAN_BOLD"%s ", rank_p[j].name);
            printf(RESET"whose rank is lower***\n");
        } else {
            printf("\t\t  ***check the loser ");
            printf(RED_BOLD"%s ", rank_p[j].name);
            printf(RESET"whether become a winner without ");
            printf(CYAN_BOLD"%s", rank_p[i].name);
            printf(RESET"***\n");
        }
        for(int k=0; k<item_num; k++) {
            if(rank_p[j].win) {
                total_unit[k] += rank_p[j].bundle[k];
                flag = 1;
            } else {
                if(total_unit[k] + rank_p[j].bundle[k] > q[k])
                    flag = 1;
            }
        }
        if(!flag) {
            printf(CYAN_BOLD"\t\tFind ");
            printf(GRN_BOLD"%s", rank_p[j].name);
            printf(RESET" !\n");
            p[rank_p[i].id].key_suc = rank_p[j].id;
            return rank_p[j].weight * rank_p[i].instance_num;
        }
        printf(GRN_BOLD"\t\t\tNO!");
        printf(RESET"\n");
    }
    printf(RED_BOLD"\t\tThere is no key-successor");
    printf(RESET"\n");
    p[rank_p[i].id].key_suc = -1;
    return 0;
}

float find_critical_val(int i, Bidder *rank_p, Bidder *p, Instance *q)
{
    int *total_unit = malloc(item_num * sizeof(int));
    memset(total_unit, 0, item_num * sizeof(int));

    printf("\tFor loser %s: \n", rank_p[i].name);

    for(int j=0; j<i; j++) {
        int flag = 0;
        printf("\t\tCheck whether become winner without winner ");
        printf(CYAN_BOLD"%s", rank_p[j].name);
        printf(RESET"\n");
        for(int k=0; k<item_num; k++) {
            total_unit[k] += rank_p[j].bundle[k];
            if(total_unit[k] + rank_p[i].bundle[k] > q[k]) {
                printf(CYAN_BOLD"\t\tFind ");
                printf(GRN_BOLD"%s", rank_p[j].name);
                printf(RESET" !\n");
                p[rank_p[i].id].key_pre = rank_p[j].id;
                return rank_p[j].weight * rank_p[i].instance_num;
            }
        }
        printf(GRN_BOLD"\t\t\tNO!");
        printf(RESET"\n");
        printf("\t\t  ***Add the instances which is used by the winner ");
        printf(CYAN_BOLD"%s", rank_p[j].name);
        printf(RESET"\n");
    }
    printf(RED_BOLD"\t\tThere is no key-predecessor");
    printf(RESET"\n");
    return -1;
}