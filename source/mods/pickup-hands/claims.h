#ifndef MOON_HAND_CLAIMS_H
#define MOON_HAND_CLAIMS_H
/* Restoration is an occupied transaction until refresh succeeds. Keeping the
 * releasing owner prevents a new pickup over a partially restored toolbelt. */
static int h_sync(void){
    h_native=h_native_held();int prior=h_selection.active;
    int ok=(h_native||(h_owner&&!h_releasing))?hs_enter(&h_selection,&h_io):hs_leave(&h_selection,&h_io);
    if(ok&&h_releasing&&!h_native){h_owner=0;h_releasing=0;}
    if(prior!=h_selection.active)h_event(h_selection.active?"hands-entered":"hands-restored");
    if(!h_selection.active)ddrop(&h_prototype);return ok;
}
static int h_key(const char *owner){if(!owner||!*owner)return 0;unsigned n=0;while(owner[n]&&n<96)n++;return n<96;}
static int h_claim(const char *owner,void *walker){
    if(!h_key(owner)||!h_context(walker))return 0;
    if(h_releasing){h_sync();if(h_releasing)return 0;}
    if(h_owner)return deq(h_owner,owner)&&h_sync();
    if(h_native_held())return 0;
    h_owner=owner;if(h_sync())return 1;h_releasing=1;h_sync();return 0;
}
static int h_release(const char *owner,void *walker){
    if(!h_key(owner)||!h_context(walker)||(h_owner&&!deq(h_owner,owner)))return 0;
    h_releasing=1;return h_sync();
}
static int h_occupied(void *walker){return h_context(walker)&&(h_owner||h_selection.active||h_native_held());}
#endif
