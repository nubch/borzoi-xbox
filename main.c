#include <SDL.h>
#include <hal/video.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define W 320
#define H 240
#define MAX_TRI 4200
#define PI 3.14159265358979323846f
#define RGB(r,g,b) (((uint32_t)(r)<<16)|((uint32_t)(g)<<8)|(uint32_t)(b))
typedef struct { float x,y,z; } V3;
typedef struct { V3 a,b,c; uint32_t color; } Tri;
typedef struct { float x,y,z; } P3;
static Tri tris[MAX_TRI];
static int ntri=0;
static uint32_t screen[W*H];
static float depthbuf[W*H];
static FILE *music;
static int music_playing=0;
/* Stage log survives a crash and can be collected via FTP from E:\\borzoi_debug.log */
static void trace_stage(const char *msg){
  FILE *f=fopen("E:\\borzoi_debug.log","a");
  if(f){fprintf(f,"%s\n",msg);fclose(f);}
}

static V3 v(float x,float y,float z){ V3 p={x,y,z}; return p; }
static V3 add(V3 a,V3 b){return v(a.x+b.x,a.y+b.y,a.z+b.z);}
static V3 sub(V3 a,V3 b){return v(a.x-b.x,a.y-b.y,a.z-b.z);}
static V3 mul(V3 a,float f){return v(a.x*f,a.y*f,a.z*f);}
static V3 cross(V3 a,V3 b){return v(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
static float dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static V3 normal(V3 a){float l=sqrtf(dot(a,a));return l>0.0001f?mul(a,1.0f/l):v(0,1,0);}
static float clampf(float x,float lo,float hi){return x<lo?lo:(x>hi?hi:x);}
static void tri(V3 a,V3 b,V3 c,uint32_t color){
  if(ntri<MAX_TRI){Tri *t=&tris[ntri++];t->a=a;t->b=b;t->c=c;t->color=color;}
}
static void ball(V3 p,float sx,float sy,float sz,int rings,int segs,uint32_t col){
  int i,j;
  for(i=0;i<rings;i++){
    float t0=PI*(float)i/rings,t1=PI*(float)(i+1)/rings;
    for(j=0;j<segs;j++){
      float a0=2*PI*(float)j/segs,a1=2*PI*(float)(j+1)/segs;
      V3 p0=v(p.x+sx*sinf(t0)*cosf(a0),p.y+sy*cosf(t0),p.z+sz*sinf(t0)*sinf(a0));
      V3 p1=v(p.x+sx*sinf(t1)*cosf(a0),p.y+sy*cosf(t1),p.z+sz*sinf(t1)*sinf(a0));
      V3 p2=v(p.x+sx*sinf(t1)*cosf(a1),p.y+sy*cosf(t1),p.z+sz*sinf(t1)*sinf(a1));
      V3 p3=v(p.x+sx*sinf(t0)*cosf(a1),p.y+sy*cosf(t0),p.z+sz*sinf(t0)*sinf(a1));
      if(i)tri(p0,p1,p3,col);
      if(i+1<rings)tri(p3,p1,p2,col);
    }
  }
}
static void bone(V3 a,V3 b,float r0,float r1,int segs,uint32_t col){
  V3 axis=normal(sub(b,a));
  V3 helper=(fabsf(axis.z)<.85f)?v(0,0,1):v(0,1,0);
  V3 u=normal(cross(axis,helper)), w=normal(cross(axis,u));
  int i;
  for(i=0;i<segs;i++){
    float c0=2*PI*i/segs,c1=2*PI*(i+1)/segs;
    V3 d0=add(mul(u,cosf(c0)),mul(w,sinf(c0)));
    V3 d1=add(mul(u,cosf(c1)),mul(w,sinf(c1)));
    V3 a0=add(a,mul(d0,r0)),a1=add(a,mul(d1,r0));
    V3 b0=add(b,mul(d0,r1)),b1=add(b,mul(d1,r1));
    tri(a0,b0,b1,col);tri(a0,b1,a1,col);
    tri(a,a1,a0,col);tri(b,b0,b1,col);
  }
}
static void create_borzoi(void){
  const uint32_t fur=RGB(220,211,192), ivory=RGB(247,241,224);
  const uint32_t cream=RGB(234,222,199);
  const uint32_t ear=RGB(158,127,118), nose=RGB(43,36,42);
  int s;
  ntri=0;
  ball(v(-.15,.62,0),1.12,.43,.33,7,12,fur);
  ball(v(.65,.61,0),.49,.57,.39,7,12,ivory);
  ball(v(-.96,.68,0),.39,.37,.29,7,12,cream);
  ball(v(-.20,.83,0),.68,.28,.31,5,10,ivory);
  bone(v(.74,.78,0),v(1.18,1.54,0),.30,.19,12,ivory);
  ball(v(1.20,1.45,0),.36,.34,.24,7,12,ivory);
  ball(v(1.49,1.54,0),.38,.27,.22,7,12,fur);
  bone(v(1.69,1.49,0),v(2.36,1.32,0),.175,.10,12,ivory);
  ball(v(2.39,1.315,0),.112,.075,.12,5,10,nose);
  for(s=-1;s<=1;s+=2){
    bone(v(1.28,1.75,s*.145f),v(1.16,2.045,s*.155f),.13,.015,8,ear);
    ball(v(1.68,1.59,s*.205f),.046,.050,.027,5,8,nose);
    ball(v(1.694,1.607,s*.229f),.012,.012,.010,4,6,RGB(254,250,232));
  }
  for(s=-1;s<=1;s+=2){
    float z=s*.26f;
    bone(v(.67,.37,z),v(.60,-.24,z),.15,.090,9,cream);
    bone(v(.60,-.24,z),v(.67,-.88,z),.091,.063,9,ivory);
    ball(v(.77,-.89,z),.18,.077,.112,5,10,fur);
    bone(v(-.94,.50,z*.85f),v(-1.15,-.16,z),.21,.105,9,fur);
    bone(v(-1.15,-.16,z),v(-1.06,-.86,z),.104,.068,9,cream);
    ball(v(-.96,-.88,z),.19,.074,.117,5,10,fur);
  }
  bone(v(-1.18,.76,0),v(-1.58,.46,.05),.12,.09,9,ivory);
  bone(v(-1.58,.46,.05),v(-1.83,.21,.04),.095,.025,9,cream);
  ball(v(-1.76,.24,.04),.21,.11,.1,4,9,ivory);
  bone(v(.82,.45,.0),v(.65,.13,.01),.22,.07,9,ivory);
  for(s=-1;s<=1;s+=2){
    bone(v(.34,.36,s*.19f),v(.14,.17,s*.21f),.13,.015,7,ivory);
    bone(v(-.08,.33,s*.20f),v(-.24,.17,s*.20f),.10,.015,7,cream);
  }
}
static P3 project(V3 p,float cs,float sn){
  float xx=p.x*cs-p.z*sn, zz=p.x*sn+p.z*cs;
  float yy=.955f*p.y-.296f*zz, zview=.296f*p.y+.955f*zz;
  float d=5.8f-zview;
  P3 o={160.0f+285.0f*xx/d,132.0f-285.0f*yy/d,d};
  return o;
}
static float edge(float ax,float ay,float bx,float by,float px,float py){
  return (px-ax)*(by-ay)-(py-ay)*(bx-ax);
}
static void draw_triangle(P3 a,P3 b,P3 c,uint32_t color){
  float area=edge(a.x,a.y,b.x,b.y,c.x,c.y);
  int x0=(int)floorf(fminf(a.x,fminf(b.x,c.x)));
  int y0=(int)floorf(fminf(a.y,fminf(b.y,c.y)));
  int x1=(int)ceilf(fmaxf(a.x,fmaxf(b.x,c.x)));
  int y1=(int)ceilf(fmaxf(a.y,fmaxf(b.y,c.y)));
  int x,y;
  if(fabsf(area)<.001f)return;
  float sign=area<0?-1.0f:1.0f, invarea=1.0f/fabsf(area);
  if(x0<0)x0=0;if(y0<0)y0=0;if(x1>W-1)x1=W-1;if(y1>H-1)y1=H-1;
  for(y=y0;y<=y1;y++)for(x=x0;x<=x1;x++){
    float px=(float)x+.5f,py=(float)y+.5f;
    float w0=sign*edge(b.x,b.y,c.x,c.y,px,py);
    float w1=sign*edge(c.x,c.y,a.x,a.y,px,py);
    float w2=sign*edge(a.x,a.y,b.x,b.y,px,py);
    if(w0>=0&&w1>=0&&w2>=0){
      float d=(w0*a.z+w1*b.z+w2*c.z)*invarea;
      int at=y*W+x;
      if(d<depthbuf[at]){depthbuf[at]=d;screen[at]=color;}
    }
  }
}
static uint32_t shade(uint32_t color,float power){
  int r=(int)(((color>>16)&255)*power);
  int g=(int)(((color>>8)&255)*power);
  int b=(int)((color&255)*power);
  return 0xff000000u|RGB(r,g,b);
}
static void backdrop(uint32_t ticks){
  int x,y;
  for(y=0;y<H;y++){
    float t=(float)y/H;
    int r=(int)(12.0f+17.0f*t),g=(int)(14.0f+11.0f*t),b=(int)(31.0f+14.0f*t);
    for(x=0;x<W;x++){
      float dx=(x-160)/160.0f,dy=(y-125)/150.0f;
      float glow=1.0f-clampf(dx*dx+dy*dy,0,1);
      unsigned int seed=(unsigned int)(x*73856093u ^ y*19349663u ^ 92311u);
      int grain=(int)((seed>>12)&3)-1;
      int nr=r+(int)(glow*13)+grain,ng=g+(int)(glow*9)+grain,nb=b+(int)(glow*14)+grain;
      screen[y*W+x]=0xff000000u|RGB(nr,ng,nb);
      depthbuf[y*W+x]=10000.0f;
    }
  }
  for(y=164;y<213;y++)for(x=30;x<290;x++){
    float dx=(x-160)/126.f,dy=(y-186)/19.f,rr=dx*dx+dy*dy;
    if(rr<1.0f){
      int amt=(int)(25.0f*(1.0f-rr));
      int idx=y*W+x;
      uint32_t c=screen[idx];
      int r=(c>>16)&255,g=(c>>8)&255,b=c&255;
      screen[idx]=0xff000000u|RGB(r+amt/2,g+amt,b+amt+4);
    }
  }
  for(x=0;x<45;x++){
    uint32_t n=(uint32_t)(x*48271u+9137u);
    int sx=(int)((n*1664525u+1013904223u)%W);
    int sy=(int)(((n^0x4f1bbcdcu)*69069u)%H);
    if(sy<160){
      unsigned int intensity=((ticks/700u+x/3u)%8u==0u)?175:85;
      screen[sy*W+sx]=0xff000000u|RGB(intensity,intensity,intensity+15);
    }
  }
}
static void scene(float angle,uint32_t ticks){
  float cs=cosf(angle),sn=sinf(angle);
  int i;
  V3 light=normal(v(-.65f,1.0f,1.1f));
  backdrop(ticks);
  for(i=0;i<ntri;i++){
    const Tri *t=&tris[i];
    V3 a=t->a,b=t->b,c=t->c;
    V3 n=normal(cross(sub(b,a),sub(c,a)));
    V3 nr=v(n.x*cs-n.z*sn,n.y,n.x*sn+n.z*cs);
    float brightness=.40f+.58f*clampf(fabsf(dot(nr,light)),0,1);
    draw_triangle(project(a,cs,sn),project(b,cs,sn),project(c,cs,sn),shade(t->color,brightness));
  }
}
static void audio_callback(void *userdata,Uint8 *stream,int len){
  size_t got=0;
  (void)userdata;
  memset(stream,0,(size_t)len);
  if(!music||!music_playing)return;
  while(got<(size_t)len){
    size_t n=fread(stream+got,1,(size_t)len-got,music);
    got+=n;
    if(n==0){
      clearerr(music);
      if(fseek(music,0,SEEK_SET)!=0)break;
      if(got==0){
        long end;
        if(fseek(music,0,SEEK_END)!=0)break;
        end=ftell(music);
        if(end<=0)break;
        rewind(music);
      }
    }
  }
}
static int start_audio(void){
  SDL_AudioSpec want,have;
  music=fopen("D:\\halo.pcm","rb");
  if(!music)music=fopen("halo.pcm","rb");
  if(!music)return 0;
  memset(&want,0,sizeof(want));
  want.freq=22050;
  want.format=AUDIO_S16LSB;
  want.channels=2;
  want.samples=2048;
  want.callback=audio_callback;
  if(SDL_OpenAudio(&want,&have)!=0){fclose(music);music=NULL;return 0;}
  if(have.freq!=22050||have.format!=AUDIO_S16LSB||have.channels!=2){
    SDL_CloseAudio();fclose(music);music=NULL;return 0;
  }
  music_playing=1;
  SDL_PauseAudio(0);
  return 1;
}
static void stop_audio(void){
  if(music){SDL_CloseAudio();fclose(music);music=NULL;}
}
int main(int argc,char **argv){
  SDL_Window *win;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event ev;
  SDL_Joystick *joy=NULL;
  SDL_GameController *controller=NULL;
  uint32_t previous;
  float angle=.45f,speed=.45f;
  int running=1,paused=0;
  (void)argc;(void)argv;
  /* Required by the official nxdk SDL sample before SDL video initialization. */
  XVideoSetMode(640,480,32,REFRESH_DEFAULT);
  trace_stage("01 - XVideoSetMode OK");
  if(SDL_Init(SDL_INIT_VIDEO)!=0){trace_stage("02 - SDL video init FAILED");trace_stage(SDL_GetError());return 1;}
  trace_stage("02 - SDL video init OK");
  win=SDL_CreateWindow("BORZOI 2001",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_SHOWN);
  if(!win){trace_stage("03 - SDL_CreateWindow FAILED");trace_stage(SDL_GetError());SDL_Quit();return 1;}
  trace_stage("03 - SDL_CreateWindow OK");
  renderer=SDL_CreateRenderer(win,-1,0);
  if(!renderer){trace_stage("04 - SDL_CreateRenderer FAILED");trace_stage(SDL_GetError());SDL_DestroyWindow(win);SDL_Quit();return 1;}
  trace_stage("04 - SDL_CreateRenderer OK");
  texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,W,H);
  if(!texture){trace_stage("05 - SDL_CreateTexture FAILED");trace_stage(SDL_GetError());SDL_DestroyRenderer(renderer);SDL_DestroyWindow(win);SDL_Quit();return 1;}
  trace_stage("05 - SDL_CreateTexture OK");
  create_borzoi();
  trace_stage("06 - Borzoi model ready");
  scene(angle,SDL_GetTicks());
  SDL_UpdateTexture(texture,NULL,screen,W*(int)sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer,texture,NULL,NULL);
  SDL_RenderPresent(renderer);
  trace_stage("07 - first frame rendered");
  /* Keep the first frame on screen while initializing USB and audio. */
  SDL_InitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
  trace_stage("08 - joystick subsystem initialized");
  if(SDL_NumJoysticks()>0){
    if(SDL_IsGameController(0))controller=SDL_GameControllerOpen(0);
    else joy=SDL_JoystickOpen(0);
  }
  trace_stage("09 - controllers initialized");
  if(SDL_InitSubSystem(SDL_INIT_AUDIO)==0){
    trace_stage("10 - audio subsystem initialized");
    if(start_audio())trace_stage("11 - halo.pcm playback started");
    else trace_stage("11 - halo.pcm unavailable or audio failed");
  }else{
    trace_stage("10 - SDL audio init FAILED");
    trace_stage(SDL_GetError());
  }
  previous=SDL_GetTicks();
  while(running){
    uint32_t now=SDL_GetTicks(),dt=now-previous;
    previous=now;
    if(dt>100)dt=100;
    while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT)running=0;
      if(ev.type==SDL_KEYDOWN){
        if(ev.key.keysym.sym==SDLK_ESCAPE)running=0;
        if(ev.key.keysym.sym==SDLK_SPACE)paused=!paused;
        if(ev.key.keysym.sym==SDLK_RIGHT)speed=clampf(speed+.15f,-3,3);
        if(ev.key.keysym.sym==SDLK_LEFT)speed=clampf(speed-.15f,-3,3);
      }
      if(ev.type==SDL_JOYBUTTONDOWN){
        if(ev.jbutton.button==0)paused=!paused;
        if(ev.jbutton.button==6||ev.jbutton.button==7)running=0;
      }
      if(ev.type==SDL_CONTROLLERBUTTONDOWN){
        if(ev.cbutton.button==SDL_CONTROLLER_BUTTON_BACK)running=0;
        if(ev.cbutton.button==SDL_CONTROLLER_BUTTON_A)paused=!paused;
        if(ev.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)speed=clampf(speed+.15f,-3,3);
        if(ev.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_LEFT)speed=clampf(speed-.15f,-3,3);
      }
    }
    if(!paused)angle+=(float)dt*speed/1000.0f;
    scene(angle,now);
    SDL_UpdateTexture(texture,NULL,screen,W*(int)sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,texture,NULL,NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(14);
  }
  stop_audio();
  if(controller)SDL_GameControllerClose(controller);
  if(joy)SDL_JoystickClose(joy);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}