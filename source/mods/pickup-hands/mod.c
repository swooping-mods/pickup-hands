#include "../../runtime/native/gameplay.h"
#include "runtime.h"
#include "../../runtime/native/services.h"
#include "../sound-framework/sdk/sound.h"
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(PTR,char *,DWORD);
#include "selection.h"
#include "sdk/hands.h"
int _fltused=0;
static PTR hand_log;
static void hand_inventory(PTR game){
    const char *types[]={"MoonToolbelt","MoonCoffee","MoonBrush","MoonDustCanisterInteraction"};
    for(unsigned i=0;i<sizeof(types)/sizeof(*types);i++){
        PTR k=find_class("MoonCleaners",types[i]),it=0,m;
        while(k&&(m=da.methods(k,&it))){
            char b[1600],*p=cat(b,"{\"class\":\"");p=cat(p,types[i]);p=cat(p,"\",\"method\":\"");p=cat(p,da.name(m));
            p=cat(p,"\",\"token\":");p=num(p,method_token(m));p=cat(p,",\"rva\":");p=num(p,(U64)*(PTR *)m-(U64)game);p=cat(p,",\"parameters\":\"");
            for(DWORD j=0;j<da.arity(m);j++){if(j)*p++=';';char *t=da.type_name(da.param(m,j));p=cat(p,t);da.free(t);}p=cat(p,"\"}\n");DWORD n;WriteFile(hand_log,b,(DWORD)(p-b),&n,0);
        }
    }FlushFileBuffers(hand_log);
}
static PTR h_component,h_alive,h_refresh,h_coffee_update,h_dust_update,h_coffee_held,h_dust_held;
static PTR h_index_field,h_prototype_field,h_selected,h_brush_equipped,h_gameobject,h_active;
static int h_test,h_verified;
static void (*h_set)(PTR,PTR,void *),(*h_barrier)(PTR,PTR *,PTR);static int (*h_offset)(PTR);
static DRoot h_world,h_walker,h_belt,h_coffee,h_dust,h_prototype;
static HSelection h_selection;static DWORD h_thread;static int h_started,h_invoking,h_mutating,h_native;
static const char *h_owner;static int h_releasing;
static void h_event(const char *name){char b[180],*p=cat(b,"{\"event\":\"");p=cat(p,name);p=cat(p,"\"}\n");DWORD n;WriteFile(hand_log,b,(DWORD)(p-b),&n,0);FlushFileBuffers(hand_log);}
static PTR h_field(PTR obj,const char *name){PTR out=0,f=obj?get_field(object_class(obj),name):0;if(f)field_value(obj,f,&out);return out;}
static int h_live(PTR obj){int ok=1;void *a[]={obj};PTR v=obj?call(h_alive,0,a,&ok):0;return ok&&v&&*(unsigned char *)unbox(v);}
static PTR h_get_component(PTR walker,const char *name){int ok=1;void *a[]={dtype("MoonCleaners",name)};PTR v=call(h_component,walker,a,&ok);return ok?v:0;}
static int h_read(void *context,int *index,void **prototype){
    (void)context;PTR belt=dget(h_belt);if(!h_live(belt))return 0;
    field_value(belt,h_index_field,index);field_value(belt,h_prototype_field,prototype);
    ddrop(&h_prototype);h_prototype=dhold(*prototype);return !*prototype||h_prototype!=0;
}
static int h_write(void *context,int index,void *prototype){
    (void)context;PTR belt=dget(h_belt);if(!h_live(belt))return 0;
    h_set(belt,h_index_field,&index);h_barrier(belt,(PTR *)((unsigned char *)belt+h_offset(h_prototype_field)),prototype);return 1;
}
static int h_redraw(void *context){
    (void)context;int ok=1;h_mutating++;call(h_refresh,dget(h_belt),0,&ok);h_mutating--;return ok;
}
static const HSelectionIO h_io={0,h_read,h_write,h_redraw};
static void h_coffee_test_drop(void);
static void h_reset(void){
    h_coffee_test_drop();
    if(h_selection.active&&h_live(dget(h_belt)))hs_leave(&h_selection,&h_io);
    DRoot *roots[]={&h_world,&h_walker,&h_belt,&h_coffee,&h_dust,&h_prototype};
    for(unsigned i=0;i<sizeof(roots)/sizeof(*roots);i++)ddrop(roots[i]);
    dzero(&h_selection,sizeof(h_selection));h_owner=0;h_native=0;h_releasing=0;h_event("hands-reset");
}
static int h_context(PTR walker){
    if(!h_thread)h_thread=GetCurrentThreadId();if(GetCurrentThreadId()!=h_thread)return 0;
    PTR world=h_field(walker,"world");
    if(h_world&&(world!=dget(h_world)||walker!=dget(h_walker)||!h_live(dget(h_world))))h_reset();
    if(!h_live(world)||!h_live(walker))return 0;
    if(!h_world){
        h_world=dhold(world);h_walker=dhold(walker);h_belt=dhold(h_get_component(walker,"MoonToolbelt"));
        h_coffee=dhold(h_get_component(walker,"MoonCoffee"));h_dust=dhold(h_get_component(walker,"MoonDustCanisterInteraction"));
        if(!h_world||!h_walker||!h_belt){h_reset();return 0;}
    }return 1;
}
static int h_native_held(void){
    int ok=1,coffee=dget(h_coffee)?boolean(h_coffee_held,dget(h_coffee),&ok):0;
    int dust=dget(h_dust)?boolean(h_dust_held,dget(h_dust),&ok):0;
    return ok?(coffee||dust):1;
}
#include "claims.h"
#include "test_coffee.h"
static void h_update(PTR walker){
    if(!h_context(walker)){if(h_world)h_reset();return;}h_sync();if(h_test)h_coffee_test();
    if(!h_selection.active){h_verified=0;return;}
    if(h_test&&!h_verified){
        int ok=1,index=0;field_value(dget(h_belt),h_index_field,&index);
        PTR selected=call(h_selected,dget(h_belt),0,&ok),brush=h_get_component(walker,"MoonBrush");
        int equipped=boolean(h_brush_equipped,dget(h_belt),&ok);
        PTR model=h_field(brush,"model"),go=h_live(model)?call(h_gameobject,model,0,&ok):0;
        int visible=go?boolean(h_active,go,&ok):0;
        if(ok&&index==-1&&!selected&&!equipped&&!visible){h_verified=1;h_event("hands-held-verified");}
    }
}
static void h_frame_update(void *walker,void *user){(void)user;h_update(walker);}
/* An already-running standalone audio dispatcher may own Update. Borrow its
 * compatibility callback without loading audio or making it a dependency. */
static int h_standalone_audio_frames(void){
    char path[1200];DWORD n=GetModuleFileNameA(0,path,1000);if(!n||n>=1000)return 0;
    while(n&&path[n-1]!='\\'&&path[n-1]!='/')n--;if(!n)return 0;
    cat(path+n,"MoonLab\\mods\\sound-framework\\mod.dll");PTR module=GetModuleHandleA(path);if(!module)return 0;
    MoonSoundsGet get=(MoonSoundsGet)GetProcAddress(module,"MoonLab_Sounds_v1");
    const MoonSounds *sounds=get?get(MA_ABI,sizeof(MoonSounds)):0;
    return sounds&&sounds->on_update&&sounds->on_update(h_update);
}
static int h_before(int id,Context *c){
    if(h_thread&&GetCurrentThreadId()!=h_thread)return 0;
    PTR self=(PTR)c->rcx;
    if(id==0){h_update(self);return 0;}
    if(id==1||id==2){
        if(h_invoking)return 0;
        PTR walker=h_field(self,"walker");if(!h_context(walker))return 0;
        if(h_owner){c->rax=0;return 1;}
        /* Native Update inlines pickup code. Observe its actual result rather
         * than guessing E bindings, ray hits or success at method entry. */
        h_sync();int ok=1;h_invoking++;call(id==1?h_coffee_update:h_dust_update,self,0,&ok);h_invoking--;
        h_sync();if(!ok)h_event("native-carry-update-failed");c->rax=0;return 1;
    }
    if(self!=dget(h_belt))return 0;
    if(id==3||id==4){if(h_selection.active){if(h_test)h_event("hands-switch-blocked");c->rax=0;return 1;}return 0;}
    if(h_selection.active&&!h_mutating)h_write(0,-1,0);
    return 0;
}
static const MoonHands h_api={MH_ABI,sizeof(MoonHands),h_claim,h_release,h_occupied};
__declspec(dllexport) const MoonHands *MoonLab_Hands_v1(unsigned abi,unsigned size){return h_started&&abi==MH_ABI&&size==sizeof(MoonHands)?&h_api:0;}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
    if(h_started)return 1;
    char folder[1000],path[1200];DWORD n=GetEnvironmentVariableA("MOONLAB_EVIDENCE",folder,1000);if(!n||n>=1000)return 0;
    cat(cat(path,folder),"\\pickup-hands.jsonl");hand_log=CreateFileA(path,0x40000000,1,0,1,0x80,0);if(hand_log==(PTR)(U64)-1)return 0;
    if(!bind_runtime(domain,game)||!dapi(game))return 0;
    hand_inventory(game);
    h_set=(void(*)(PTR,PTR,void *))GetProcAddress(game,"il2cpp_field_set_value");
    h_barrier=(void(*)(PTR,PTR *,PTR))GetProcAddress(game,"il2cpp_gc_wbarrier_set_field");
    h_offset=(int(*)(PTR))GetProcAddress(game,"il2cpp_field_get_offset");if(!h_set||!h_barrier||!h_offset)return 0;
    h_component=dmethod("UnityEngine","Component","GetComponent","System.Type");h_alive=dmethod("UnityEngine","Object","op_Implicit","UnityEngine.Object");
    h_refresh=dmethod("MoonCleaners","MoonToolbelt","RefreshSelection","");
    h_coffee_update=dmethod("MoonCleaners","MoonCoffee","Update","");h_dust_update=dmethod("MoonCleaners","MoonDustCanisterInteraction","Update","");
    h_coffee_held=dmethod("MoonCleaners","MoonCoffee","get_Holding","");h_dust_held=dmethod("MoonCleaners","MoonDustCanisterInteraction","get_IsCarrying","");
    PTR belt=find_class("MoonCleaners","MoonToolbelt");h_index_field=get_field(belt,"selectedIndex");h_prototype_field=get_field(belt,"prototypeTool");
    if(!h_component||!h_alive||!h_refresh||!h_coffee_update||!h_dust_update||!h_coffee_held||!h_dust_held||!h_index_field||!h_prototype_field||h_offset(h_prototype_field)<16)return 0;
    h_selected=dmethod("MoonCleaners","MoonToolbelt","get_SelectedTool","");h_brush_equipped=dmethod("MoonCleaners","MoonToolbelt","get_BrushEquipped","");
    h_gameobject=dmethod("UnityEngine","Component","get_gameObject","");h_active=dmethod("UnityEngine","GameObject","get_activeInHierarchy","");
    char test[8];h_test=GetEnvironmentVariableA("MOONLAB_TEST_TOOLS",test,sizeof(test))==1&&test[0]=='1';
    if(!h_selected||!h_brush_equipped||!h_gameobject||!h_active)return 0;
    if(moon_runtime_loader_present()){
        if(!moon_runtime_subscribe_frame(h_frame_update,0))return 0;
    }else if(!h_standalone_audio_frames()&&!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game))return 0;
    if(!prepare_hook(h_coffee_update,100664257,1,game)||
       !prepare_hook(h_dust_update,100666583,2,game)||
       !prepare_hook(dmethod("MoonCleaners","MoonToolbelt","SelectSlot","System.Int32"),100671255,3,game)||
       !prepare_hook(dmethod("MoonCleaners","MoonToolbelt","EquipPrototype","MoonCleaners.MoonToolDefinition"),100671246,4,game)||
       !prepare_hook(dmethod("MoonCleaners","MoonToolbelt","LateUpdate",""),100671257,5,game)||
       !prepare_hook(h_refresh,100671256,6,game)){h_event("hands-hook-shape-failed");return 0;}
    before_hook=h_before;if(!install_hooks())return 0;
    (void)num;(void)integer;(void)dcopy;(void)dnumber;(void)dvector;(void)darray;
    if(moon_runtime_loader_present()){
        const MoonRuntimeApiV1 *runtime=moon_runtime_connect();
        const MoonServiceLimitV1 limits[]={{"max_claims",1}};
        MoonServicePublicationLimitsV1 publication={.base={.size=sizeof(publication),.service_id="hands",.abi=MH_ABI,.api_size=sizeof(h_api),.api=&h_api},.limit_count=sizeof(limits)/sizeof(limits[0]),.reserved=0,.limits=limits};
        if(!runtime||!runtime->publish(&publication.base))return 0;
    }
    h_started=1;h_event("ready");return 1;
}
