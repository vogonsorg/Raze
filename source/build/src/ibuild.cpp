
#include "a.h"
#include "baselayer.h"
#include "build.h"
#include "animvpx.h"
#include "clip.h"
#include "scriptfile.h"
#include "common.h"
#include "engine_priv.h"
#include "glsurface.h"
#include "hightile.h"
#include "mdsprite.h"
#include "polymost.h"
#include "pragmas.h"
#include "scriptfile.h"
#include "mmulti.h"
#include "timer.h"

// clip
vec2_t hitscangoal = { (1 << 29) - 1, (1 << 29) - 1 };
int32_t clipmoveboxtracenum = 3;
int32_t engineLoadClipMaps(void) { return {}; }
int clipshape_idx_for_sprite(uspriteptr_t const curspr, int curidx) { return {}; }
int clipinsidebox(vec2_t* vect, int wallnum, int walldist) { return {}; }
int clipinsideboxline(int x, int y, int x1, int y1, int x2, int y2, int walldist) { return {}; }
int32_t clipmovex(vec3_t* pos, int16_t* sectnum, int32_t xvect, int32_t yvect, int32_t const walldist, int32_t const ceildist, int32_t const flordist, uint32_t const cliptype, uint8_t const noslidep) { return {}; }
int32_t clipmove(vec3_t* const pos, int16_t* const sectnum, int32_t xvect, int32_t yvect, int32_t const walldist, int32_t const ceildist, int32_t const flordist, uint32_t const cliptype) { return {}; }
int pushmove(vec3_t* const vect, int16_t* const sectnum, int32_t const walldist, int32_t const ceildist, int32_t const flordist, uint32_t const cliptype, bool clear) { return {}; }
void getzrange(const vec3_t* pos, int16_t sectnum, int32_t* ceilz, int32_t* ceilhit, int32_t* florz, int32_t* florhit, int32_t walldist, uint32_t cliptype) { }
int32_t hitscan(const vec3_t* sv, int16_t sectnum, int32_t vx, int32_t vy, int32_t vz, hitdata_t* hit, uint32_t cliptype) { return {}; }

// defs
int32_t loaddefinitionsfile(const char* fn) { return {}; }

// engine
int32_t g_loadedMapVersion = -1;  // -1: none (e.g. started new)
bool playing_rr;
bool playing_blood;
int32_t rendmode = 0;
int32_t glrendmode = REND_POLYMOST;
int32_t r_rortexture = 0;
int32_t r_rortexturerange = 0;
int32_t r_rorphase = 0;
int32_t mdtims, omdtims;
uint8_t alphahackarray[MAXTILES];
int32_t polymostcenterhoriz = 100;
uint8_t globalr = 255, globalg = 255, globalb = 255;
int16_t pskybits_override = -1;
void (*loadvoxel_replace)(int32_t voxindex) = NULL;
int16_t tiletovox[MAXTILES];
CVAR(Bool, r_usenewaspect, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG);
int32_t newaspect_enable = 0;
int32_t globalflags;
const char* engineerrstr = "No error";
int32_t(*getpalookup_replace)(int32_t davis, int32_t dashade) = NULL;
int16_t yax_bunchnum[MAXSECTORS][2];
int16_t yax_nextwall[MAXWALLS][2];
int32_t xdimen = -1, xdimenscale;
int32_t xyaspect;
int32_t qsetmode = 0;
int32_t(*animateoffs_replace)(int const tilenum, int fakevar) = NULL;
int32_t(*insertsprite_replace)(int16_t sectnum, int16_t statnum) = NULL;
int32_t(*deletesprite_replace)(int16_t spritenum) = NULL;
int32_t(*changespritesect_replace)(int16_t spritenum, int16_t newsectnum) = NULL;
int32_t(*changespritestat_replace)(int16_t spritenum, int16_t newstatnum) = NULL;
void (*initspritelists_replace)(void) = NULL;
int32_t(*loadboard_replace)(const char* filename, char flags, vec3_t* dapos, int16_t* daang, int16_t* dacursectnum) = NULL;
int32_t guniqhudid;
int32_t spritesortcnt;
spriteext_t* spriteext;
spritesmooth_t* spritesmooth;
wallext_t* wallext;
sectortype* sector;
walltype* wall;
spritetype* sprite;
tspriteptr_t tsprite;
uint32_t sectorchanged[MAXSECTORS + M32_FIXME_SECTORS];
uint32_t wallchanged[MAXWALLS + M32_FIXME_WALLS];
uint32_t spritechanged[MAXSPRITES];
int32_t xdim, ydim, numpages;
int32_t yxaspect, viewingrange;
TArray<intptr_t> ylookup;
int32_t rotatesprite_y_offset;
int32_t rotatesprite_yxaspect;
int32_t numyaxbunches;
int16_t headsectbunch[2][YAX_MAXBUNCHES], nextsectbunch[2][MAXSECTORS];
int32_t Numsprites;
int16_t numsectors, numwalls;
int32_t display_mirror;
ClockTicks totalclock, totalclocklock;
int32_t numframes, randomseed;
int16_t sintable[2048];
uint8_t palette[768];
int16_t numshades;
char* palookup[MAXPALOOKUPS];
uint8_t paletteloaded;
char* blendtable[MAXBLENDTABS];
uint8_t blackcol;
int32_t maxspritesonscreen;
char showinvisibility;
int32_t g_visibility, parallaxvisibility;
uint8_t numalphatabs;
vec2_t windowxy1, windowxy2;
TArray<int16_t> startumost, startdmost;
int32_t g_pskyidx;
int32_t pskynummultis;
int32_t* multipskytile;
char parallaxtype;
int32_t parallaxyoffs_override, parallaxyscale_override;
int16_t tailspritefree;
int16_t headspritesect[MAXSECTORS + 1], headspritestat[MAXSTATUS + 1];
int16_t prevspritesect[MAXSPRITES], prevspritestat[MAXSPRITES];
int16_t nextspritesect[MAXSPRITES], nextspritestat[MAXSPRITES];
int automapping;
char show2dsector[(MAXSECTORS + 7) >> 3];
char show2dwall[(MAXWALLS + 7) >> 3];
char show2dsprite[(MAXSPRITES + 7) >> 3];
uint8_t gotpic[(MAXTILES + 7) >> 3];
char gotsector[(MAXSECTORS + 7) >> 3];
uint16_t h_xsize[MAXTILES];
int32_t enginecompatibility_mode;
int32_t nextvoxid;
intptr_t voxoff[MAXVOXELS][MAXVOXMIPS]; // used in KenBuild
int8_t voxreserve[(MAXVOXELS + 7) >> 3];
int8_t voxrotate[(MAXVOXELS + 7) >> 3];
int32_t mdinited;
tile2model_t tile2model[MAXTILES + EXTRATILES];
int32_t mdpause;
voxmodel_t* voxmodels[MAXVOXELS];
void faketimerhandler() { }
int16_t yax_getbunch(int16_t i, int16_t cf) { return {}; }
int16_t yax_getnextwall(int16_t wal, int16_t cf) { return {}; }
int16_t yax_vnextsec(int16_t line, int16_t cf) { return {}; }
void yax_update(int32_t resetstat) { }
int32_t yax_getneighborsect(int32_t x, int32_t y, int32_t sectnum, int32_t cf) { return {}; }
void yax_preparedrawrooms(void) { }
void yax_drawrooms(void (*SpriteAnimFunc)(int32_t, int32_t, int32_t, int32_t, int32_t), int16_t sectnum, int32_t didmirror, int32_t smoothr) { }
void do_insertsprite_at_headofsect(int16_t spritenum, int16_t sectnum) { }
void do_deletespritesect(int16_t deleteme) { }
void do_insertsprite_at_headofstat(int16_t spritenum, int16_t statnum) { }
int32_t insertspritestat(int16_t statnum) { return {}; }
void do_deletespritestat(int16_t deleteme) { }
int32_t insertsprite(int16_t sectnum, int16_t statnum) { return {}; }
int32_t deletesprite(int16_t spritenum) { return {}; }
int32_t changespritesect(int16_t spritenum, int16_t newsectnum) { return {}; }
int32_t changespritestat(int16_t spritenum, int16_t newstatnum) { return {}; }
int32_t lintersect(const int32_t originX, const int32_t originY, const int32_t originZ, const int32_t destX, const int32_t destY, const int32_t destZ, const int32_t lineStartX, const int32_t lineStartY, const int32_t lineEndX, const int32_t lineEndY, int32_t* intersectionX, int32_t* intersectionY, int32_t* intersectionZ) { return {}; }
int32_t rayintersect(int32_t x1, int32_t y1, int32_t z1, int32_t vx, int32_t vy, int32_t vz, int32_t x3, int32_t y3, int32_t x4, int32_t y4, int32_t* intx, int32_t* inty, int32_t* intz) { return {}; }
psky_t* tileSetupSky(int32_t const tilenum) { return {}; }
int32_t enginePreInit(void) { return {}; }
int32_t engineInit(void) { return {}; }
int32_t enginePostInit(void) { return {}; }
void engineUnInit(void) { }
void initspritelists(void) { }
//void set_globalang(fix16_t const ang) { }
int32_t renderDrawRoomsQ16(int32_t daposx, int32_t daposy, int32_t daposz, fix16_t daang, fix16_t dahoriz, int16_t dacursectnum) { return {}; }
int32_t wallvisible(int32_t const x, int32_t const y, int16_t const wallnum) { return {}; }
void renderDrawMasks(void) { }
void renderDrawMapView(int32_t dax, int32_t day, int32_t zoome, int16_t ang) { }
int32_t engineLoadBoard(const char* filename, char flags, vec3_t* dapos, int16_t* daang, int16_t* dacursectnum) { return {}; }
int32_t engineLoadBoardV5V6(const char* filename, char fromwhere, vec3_t* dapos, int16_t* daang, int16_t* dacursectnum) { return {}; }
int32_t videoSetGameMode(char davidoption, int32_t daupscaledxdim, int32_t daupscaledydim, int32_t dabpp, int32_t daupscalefactor) { return {}; }
void videoNextPage(void) { }
int32_t qloadkvx(int32_t voxindex, const char* filename) { return {}; }
void vox_undefine(int32_t const tile) { }
void vox_deinit() { }
int32_t inside(int32_t x, int32_t y, int16_t sectnum) { return {}; }
int32_t getangle(int32_t xvect, int32_t yvect) { return {}; }
int32_t ksqrt(uint32_t num) { return {}; }
int32_t spriteheightofsptr(uspriteptr_t spr, int32_t* height, int32_t alsotileyofs) { return {}; }
int32_t setsprite(int16_t spritenum, const vec3_t* newpos) { return {}; }
int32_t setspritez(int16_t spritenum, const vec3_t* newpos) { return {}; }
int32_t nextsectorneighborz(int16_t sectnum, int32_t refz, int16_t topbottom, int16_t direction) { return {}; }
int32_t cansee(int32_t x1, int32_t y1, int32_t z1, int16_t sect1, int32_t x2, int32_t y2, int32_t z2, int16_t sect2) { return {}; }
void neartag(int32_t xs, int32_t ys, int32_t zs, int16_t sectnum, int16_t ange, int16_t* neartagsector, int16_t* neartagwall, int16_t* neartagsprite, int32_t* neartaghitdist, int32_t neartagrange, uint8_t tagsearch, int32_t(*blacklist_sprite_func)(int32_t)) { }
void dragpoint(int16_t pointhighlight, int32_t dax, int32_t day, uint8_t flags) { }
int32_t lastwall(int16_t point) { return {}; }
int32_t getwalldist(vec2_t const in, int const wallnum, vec2_t* const out) { return {}; }
int32_t getsectordist(vec2_t const in, int const sectnum, vec2_t* const out) { return {}; }
int findwallbetweensectors(int sect1, int sect2) { return {}; }
void updatesector(int32_t const x, int32_t const y, int16_t* const sectnum) { }
void updatesectorz(int32_t const x, int32_t const y, int32_t const z, int16_t* const sectnum) { }
void updatesectorneighbor(int32_t const x, int32_t const y, int16_t* const sectnum, int32_t initialMaxDistance, int32_t maxDistance) { }
void updatesectorneighborz(int32_t const x, int32_t const y, int32_t const z, int16_t* const sectnum, int32_t initialMaxDistance, int32_t maxDistance) { }
void rotatepoint(vec2_t const pivot, vec2_t p, int16_t const daang, vec2_t* const p2) { }
void videoSetCorrectedAspect() { }
void videoSetViewableArea(int32_t x1, int32_t y1, int32_t x2, int32_t y2) { }
void renderSetAspect(int32_t daxrange, int32_t daaspect) { }
void rotatesprite_(int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, uint8_t dapalnum, int32_t dastat, uint8_t daalpha, uint8_t dablend, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2) { }
void videoClearViewableArea(int32_t dacol) { }
void videoClearScreen(int32_t dacol) { }
void renderSetTarget(int16_t tilenume, int32_t xsiz, int32_t ysiz) { }
void renderRestoreTarget(void) { }
void squarerotatetile(int16_t tilenume) { }
void renderPrepareMirror(int32_t dax, int32_t day, int32_t daz, fix16_t daang, fix16_t dahoriz, int16_t dawall, int32_t* tposx, int32_t* tposy, fix16_t* tang) { }
void renderCompleteMirror(void) { }
int32_t sectorofwall(int16_t wallNum) { return {}; }
int32_t getceilzofslopeptr(usectorptr_t sec, int32_t dax, int32_t day) { return {}; }
int32_t getflorzofslopeptr(usectorptr_t sec, int32_t dax, int32_t day) { return {}; }
void getzsofslopeptr(usectorptr_t sec, int32_t dax, int32_t day, int32_t* ceilz, int32_t* florz) { }
void yax_getzsofslope(int sectNum, int playerX, int playerY, int32_t* pCeilZ, int32_t* pFloorZ) { }
void alignceilslope(int16_t dasect, int32_t x, int32_t y, int32_t z) { }
void alignflorslope(int16_t dasect, int32_t x, int32_t y, int32_t z) { }
void setfirstwall(int16_t sectnum, int16_t newfirstwall) { }
void renderSetRollAngle(int32_t rolla) { }

// hightile
polytint_t hictinting[MAXPALOOKUPS];

// mdsprite
int32_t Ptile2tile(int32_t tile, int32_t palette) { return {}; }
int32_t md_undefinetile(int32_t tile) { return {}; }

// mhk
usermaphack_t g_loadedMapHack;  // used only for the MD4 part
usermaphack_t* usermaphacks;
int32_t num_usermaphacks;
int compare_usermaphacks(const void* a, const void* b) { return {}; }
int32_t engineLoadMHK(const char* filename) { return {}; }

// palette
uint8_t* basepaltable[MAXBASEPALS] = { palette };
uint8_t curbasepal;
uint32_t g_lastpalettesum = 0;
palette_t curpalette[256];			// the current palette, unadjusted for brightness or tint
palette_t curpalettefaded[256];		// the current palette, adjusted for brightness and tint (ie. what gets sent to the card)
palette_t palfadergb = { 0, 0, 0, 0 };
unsigned char palfadedelta = 0;
ESetPalFlags curpaletteflags;
palette_t palookupfog[MAXPALOOKUPS];
int8_t g_noFloorPal[MAXPALOOKUPS];
void (*paletteLoadFromDisk_replace)(void) = NULL;
glblend_t glblend[MAXBLENDTABS];
void palettePostLoadTables(void) { }
void paletteFixTranslucencyMask(void) { }
int32_t paletteLoadLookupTable(FileReader& fp) { return {}; }
void paletteSetupDefaultFog(void) { }
void palettePostLoadLookups(void) { }
FRenderStyle GetBlend(int blend, int def) { return {}; }
float float_trans(uint32_t maskprops, uint8_t blend) { return {}; }
int32_t paletteSetLookupTable(int32_t palnum, const uint8_t* shtab) { return {}; }
void paletteMakeLookupTable(int32_t palnum, const char* remapbuf, uint8_t r, uint8_t g, uint8_t b, char noFloorPal) { }
void paletteSetColorTable(int32_t id, uint8_t const* const table, bool transient) { }
void paletteFreeColorTables() { }
void videoSetPalette(int dabrightness, int dapalid, ESetPalFlags flags) { }
palette_t paletteGetColor(int32_t col) { return {}; }
void videoFadePalette(uint8_t r, uint8_t g, uint8_t b, uint8_t offset) { }

// polymost
CVAR(Bool, hw_detailmapping, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, hw_glowmapping, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, hw_polygonmode, 0, 0)
CVARD(Bool, hw_animsmoothing, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable model animation smoothing")
CVARD(Bool, hw_hightile, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable hightile texture rendering")
CVARD(Bool, hw_models, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable model rendering")
CVARD(Bool, hw_parallaxskypanning, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable parallaxed floor/ceiling panning when drawing a parallaxing sky")
CVARD(Bool, hw_shadeinterpolate, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable shade interpolation")
CVARD(Float, hw_shadescale, 1.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "multiplier for shading")
bool hw_int_useindexedcolortextures;
CUSTOM_CVARD(Bool, hw_useindexedcolortextures, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "enable/disable indexed color texture rendering")
{
    hw_int_useindexedcolortextures = self;
}
CUSTOM_CVARD(Int, hw_texfilter, TEXFILTER_ON, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "changes the texture filtering settings")
{
    static const char* const glfiltermodes[] =
    {
        "NEAREST",
        "LINEAR",
        "NEAREST_MIPMAP_NEAREST",
        "LINEAR_MIPMAP_NEAREST",
        "NEAREST_MIPMAP_LINEAR",
        "LINEAR_MIPMAP_LINEAR",
        "LINEAR_MIPMAP_LINEAR with NEAREST mag"
    };

    if (self < 0 || self > 6) self = 0;
    else
    {
        gltexapplyprops();
        OSD_Printf("Texture filtering mode changed to %s\n", glfiltermodes[hw_texfilter]);
    }
}
CUSTOM_CVARD(Int, hw_anisotropy, 4, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "changes the OpenGL texture anisotropy setting")
{
    gltexapplyprops();
}
void (*PolymostProcessVoxels_Callback)(void) = NULL;
void gltexapplyprops(void) { }
void uploadpalswaps(int count, int32_t* swaps) { }
void Polymost_prepare_loadboard(void) { }
void PrecacheHardwareTextures(int nTile) { }
void Polymost_Startup() { }

// sdlayer
double g_beforeSwapTime;
GameInterface* gi;
int myconnectindex, numplayers;
int connecthead, connectpoint2[MAXMULTIPLAYERS];
int32_t xres = -1, yres = -1, bpp = 0, bytesperline, refreshfreq = -1;
intptr_t frameplace = 0;
char modechange = 1;
void calc_ylookup(int32_t bpl, int32_t lastyidx) { }
void videoBeginDrawing(void) { }
void videoEndDrawing(void) { }

// voxmodel
voxmodel_t* loadkvxfrombuf(const char* kvxbuffer, int32_t length) { return {}; }
