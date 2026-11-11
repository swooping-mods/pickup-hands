/* Explicit private-replay protocol. No key binding or normal-game action.
 * Both MOONLAB_TEST_TOOLS and a request in this run's evidence folder are needed. */
static struct {
    int phase;char request[1200],drop[1200];DRoot kitchen;
    PTR make,take,release;DWORD (__stdcall *attributes)(const char *);
} hc;
static void h_coffee_test_drop(void){ddrop(&hc.kitchen);hc.phase=0;}
static void h_coffee_test(void){
    if(!hc.attributes){
        hc.attributes=(DWORD(__stdcall *)(const char *))GetProcAddress(GetModuleHandleA("kernel32.dll"),"GetFileAttributesA");
        char folder[1000];DWORD n=GetEnvironmentVariableA("MOONLAB_EVIDENCE",folder,sizeof(folder));if(!hc.attributes||!n||n>=sizeof(folder))return;
        cat(cat(hc.request,folder),"\\pickup-coffee.request");cat(cat(hc.drop,folder),"\\pickup-coffee.drop");
    }
    if(!hc.phase){
        if(hc.attributes(hc.request)==0xffffffffu||h_owner||h_native_held())return;
        hc.phase=-1;
        PTR find=dmethod("UnityEngine","Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode");
        hc.make=dmethod("MoonCleaners","MoonCoffee","MakeCoffee","MoonCleaners.MoonKitchen");
        hc.take=dmethod("MoonCleaners","MoonCoffee","TakeCoffee","MoonCleaners.MoonKitchen");hc.release=dmethod("MoonCleaners","MoonCoffee","Drop","");
        if(!find||!hc.make||!hc.take||!hc.release){h_event("test-coffee-bind-failed");return;}
        int ok=1,inactive=0,sort=0;void *a[]={dtype("MoonCleaners","MoonKitchen"),&inactive,&sort};DRoot list=dhold(call(find,0,a,&ok));
        if(ok&&list&&da.array_length(dget(list))>0)hc.kitchen=dhold(*(PTR *)darray(dget(list),0));ddrop(&list);
        if(!h_live(dget(hc.kitchen))){h_event("test-coffee-kitchen-failed");return;}
        void *args[]={dget(hc.kitchen)};call(hc.make,dget(h_coffee),args,&ok);
        if(!ok){h_event("test-coffee-brew-failed");return;}hc.phase=1;h_event("test-coffee-brewing");
    }else if(hc.phase==1){
        int state=0;PTR f=get_field(object_class(dget(hc.kitchen)),"<State>k__BackingField");if(!f)return;
        field_value(dget(hc.kitchen),f,&state);if(state!=2)return;
        int ok=1;void *a[]={dget(hc.kitchen)};call(hc.take,dget(h_coffee),a,&ok);
        if(!ok||!boolean(h_coffee_held,dget(h_coffee),&ok)){hc.phase=-1;h_event("test-coffee-pickup-failed");return;}
        hc.phase=2;h_event("test-coffee-picked-up");
    }else if(hc.phase==2&&h_verified){hc.phase=3;h_event("test-coffee-held");}
    else if(hc.phase==3&&hc.attributes(hc.drop)!=0xffffffffu){
        int ok=1;call(hc.release,dget(h_coffee),0,&ok);hc.phase=4;
        h_event(ok&&!boolean(h_coffee_held,dget(h_coffee),&ok)?"test-coffee-dropped":"test-coffee-drop-failed");
    }
}
