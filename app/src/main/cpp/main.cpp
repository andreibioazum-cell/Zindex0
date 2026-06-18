#include <android/native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <vector>
#include <cmath>
#include <time.h>

// --- SIMPLEX NOISE (из noise.c) ---
int p[]={151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};
float grad(int h, float x, float y) { int g = h & 15; float u = g<8?x:y, v = g<4?y:g==12||g==14?x:0; return ((g&1)?-u:u) + ((g&2)?-v:v); }
float simplex2(float x, float y) {
    float n0, n1, n2, s = (x+y)*0.366f; int i = floor(x+s), j = floor(y+s);
    float t = (i+j)*0.211f, X0 = i-t, Y0 = j-t, x0 = x-X0, y0 = y-Y0;
    int i1 = x0>y0?1:0, j1 = x0>y0?0:1;
    float x1 = x0-i1+0.211f, y1 = y0-j1+0.211f, x2 = x0-1.0f+2.0f*0.211f, y2 = y0-1.0f+2.0f*0.211f;
    int ii = i&255, jj = j&255;
    float t0 = 0.5f-x0*x0-y0*y0; if(t0<0) n0=0; else { t0*=t0; n0=t0*t0*grad(p[ii+p[jj]], x0, y0); }
    float t1 = 0.5f-x1*x1-y1*y1; if(t1<0) n1=0; else { t1*=t1; n1=t1*t1*grad(p[ii+i1+p[jj+j1]], x1, y1); }
    float t2 = 0.5f-x2*x2-y2*y2; if(t2<0) n2=0; else { t2*=t2; n2=t2*t2*grad(p[ii+1+p[jj+1]], x2, y2); }
    return 40.0f*(n0+n1+n2);
}

// ШЕЙДЕРЫ
const char* VS = "#version 300 es\nlayout(location=0) in vec3 aP; layout(location=1) in vec3 aN; layout(location=2) in float aI;\nuniform mat4 uM; uniform vec3 uO; uniform int uK; out float vL;\nvoid main(){\n if(((uK >> int(aI)) & 1) == 0) { gl_Position = vec4(2.0); return; }\n gl_Position = uM * vec4(aP + uO, 1.0); vL = dot(aN, normalize(vec3(0.3, 1.0, 0.5))) * 0.4 + 0.6; }";
const char* FS = "#version 300 es\nprecision mediump float; in float vL; uniform vec3 uC; out vec4 fC; void main(){ fC=vec4(uC*vL, 1.0); }";

struct App {
    EGLDisplay disp; EGLSurface surf; EGLContext ctx;
    GLuint prog, vbo, uM, uO, uC, uK;
    float px=64, py=35, pz=64, vy=0;
    uint8_t world[128*64*128], masks[128*64*128];
};

void init_gl(App* s, ANativeWindow* win) {
    s->disp = eglGetDisplay(EGL_DEFAULT_DISPLAY); eglInitialize(s->disp, 0, 0);
    EGLConfig cfg; EGLint n; EGLint attr[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
    eglChooseConfig(s->disp, attr, &cfg, 1, &n);
    s->surf = eglCreateWindowSurface(s->disp, cfg, win, 0);
    EGLint cattr[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    s->ctx = eglCreateContext(s->disp, cfg, 0, cattr); eglMakeCurrent(s->disp, s->surf, s->surf, s->ctx);
    GLuint vS = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vS, 1, &VS, 0); glCompileShader(vS);
    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fS, 1, &FS, 0); glCompileShader(fS);
    s->prog = glCreateProgram(); glAttachShader(s->prog, vS); glAttachShader(s->prog, fS); glLinkProgram(s->prog);
    s->uM=glGetUniformLocation(s->prog,"uM"); s->uO=glGetUniformLocation(s->prog,"uO"); s->uC=glGetUniformLocation(s->prog,"uC"); s->uK=glGetUniformLocation(s->prog,"uK");

    float cV[] = {0,0,0, 1,0,0, 1,1,0, 0,0,0, 1,1,0, 0,1,0, 0,0,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1, 0,1,1, 0,0,0, 0,1,0, 0,1,1, 0,0,0, 0,1,1, 0,0,1, 1,0,0, 1,1,0, 1,1,1, 1,0,0, 1,1,1, 1,0,1, 0,1,0, 1,1,0, 1,1,1, 0,1,0, 1,1,1, 0,1,1, 0,0,0, 1,0,0, 1,0,1, 0,0,0, 1,0,1, 0,0,1};
    float cN[] = {0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1, 0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1, -1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0, 1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0, 0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0, 0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0,0,-1,0};
    std::vector<float> vd;
    for(int i=0; i<8; i++){
        float ox=(i&1)*0.5f, oy=((i>>1)&1)*0.5f, oz=((i>>2)&1)*0.5f;
        for(int v=0; v<36; v++){ vd.push_back(cV[v*3]*0.495f+ox); vd.push_back(cV[v*3+1]*0.495f+oy); vd.push_back(cV[v*3+2]*0.495f+oz); vd.push_back(cN[v*3]); vd.push_back(cN[v*3+1]); vd.push_back(cN[v*3+2]); vd.push_back((float)i); }
    }
    glGenBuffers(1, &s->vbo); glBindBuffer(GL_ARRAY_BUFFER, s->vbo); glBufferData(GL_ARRAY_BUFFER, vd.size()*4, vd.data(), GL_STATIC_DRAW);
    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
}

void android_main(struct android_app* app) {
    App s; memset(&s, 0, sizeof(s)); app->userData = &s;
    for(int x=0; x<128; x++) for(int z=0; z<128; z++) {
        int h = simplex2(x*0.02f, z*0.02f) * 12 + 20;
        for(int y=0; y<h; y++) { int i=x|(y<<7)|(z<<13); s.world[i]=(y==h-1?1:2); s.masks[i]=255; }
    }
    struct timespec last_t; clock_gettime(CLOCK_MONOTONIC, &last_t);
    while(1) {
        int id, ev; struct android_poll_source* src;
        while((id=ALooper_pollAll(0, 0, &ev, (void**)&src))>=0) { if(src) src->process(app, src); if(app->destroyRequested) return; }
        if(app->window) {
            if(!s.ctx) init_gl(&s, app->window);
            struct timespec now_t; clock_gettime(CLOCK_MONOTONIC, &now_t);
            float dt = (now_t.tv_sec - last_t.tv_sec) + (now_t.tv_nsec - last_t.tv_nsec) / 1e9f;
            last_t = now_t; if(dt > 0.05f) dt = 0.05f;
            s.vy -= 15.0f * dt; s.py += s.vy * dt; if(s.py < 30) { s.py=30; s.vy=0; }
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(s.prog);
            float f = 1.0f / tanf(75.0f * 0.5f * M_PI / 180.0f), aspect = (float)ANativeWindow_getWidth(app->window)/ANativeWindow_getHeight(app->window);
            float m[16] = {f/aspect,0,0,0, 0,f,0,0, 0,0,-1.002f,-1.0f, 0,0,-0.2002f,0}; glUniformMatrix4fv(s.uM, 1, 0, m);
            glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, 0, 28, 0); glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, 0, 28, (void*)12); glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 1, GL_FLOAT, 0, 28, (void*)24); glEnableVertexAttribArray(2);
            int rD = 12;
            for(int x=s.px-rD; x<s.px+rD; x++) for(int z=s.pz-rD; z<s.pz+rD; z++) for(int y=s.py-rD; y<s.py+rD; y++) {
                if(x<0||x>=128||y<0||y>=64||z<0||z>=128) continue;
                int i=x|(y<<7)|(z<<13); if(s.world[i]) {
                    glUniform3f(s.uO, x-s.px, y-s.py, z-s.pz-5.0f);
                    glUniform3f(s.uC, s.world[i]==1?0.4f:0.6f, s.world[i]==1?0.8f:0.6f, s.world[i]==1?0.2f:0.6f);
                    glUniform1i(s.uMask, s.masks[i]); glDrawArrays(GL_TRIANGLES, 0, 36*8);
                }
            }
            eglSwapBuffers(s.disp, s.surf);
        }
    }
}
