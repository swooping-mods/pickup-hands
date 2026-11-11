/* Temporary native selection only. No saved loadout or slot capacity writes. */
#ifndef MOON_HAND_SELECTION_H
#define MOON_HAND_SELECTION_H
typedef struct {int active,index;void *prototype;} HSelection;
typedef struct {
    void *context;
    int (*read)(void *,int *,void **);
    int (*write)(void *,int,void *);
    int (*refresh)(void *);
} HSelectionIO;
static int hs_enter(HSelection *s,const HSelectionIO *io){
    if(s->active)return 1;
    int index;void *prototype;
    if(!io->read(io->context,&index,&prototype))return 0;
    s->index=index;s->prototype=prototype;s->active=1;
    if(io->write(io->context,-1,0)&&io->refresh(io->context))return 1;
    /* Keep the snapshot if rollback fails, so the next release can retry. */
    if(io->write(io->context,index,prototype)&&io->refresh(io->context))s->active=0;
    return 0;
}
static int hs_leave(HSelection *s,const HSelectionIO *io){
    if(!s->active)return 1;
    if(!io->write(io->context,s->index,s->prototype)||!io->refresh(io->context))return 0;
    s->active=0;s->prototype=0;return 1;
}
#endif
