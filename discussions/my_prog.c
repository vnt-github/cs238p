#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point{
    int x;
    int y;
    struct Point* next;
};

double get_distance(struct Point *p, struct Point *q){
    int dx = q->x - p->x;
    int dy = q->y - p->y;
    return sqrt(dx*dx + dy*dy);
}
    

int main(){
    int n_dots = 16, i;
    double dis = 0;
    //struct Point dots[n_dots];
    struct Point *dots = NULL, *d = NULL;
    d = dots = (struct Point*) calloc(1, sizeof(struct Point));
    
    //fill the dots
    for(i=0; i<n_dots; i++){
        d->x = i;
        d->y = i*i;
        d->next = (struct Point*) calloc(1, sizeof(struct Point));
        d = d->next;
    }

    //print the distance between dots
    d = dots; //reset pointer
    for(i=0; i<n_dots; i++){
        dis = get_distance(d, d->next);
        printf("(%d,%d)--- %2f ---(%d,%d)\n",
               d->x, d->y, dis,
               d->next->x, d->next->y);
        d = d->next;
    }
    return 0;
}
