//ZZTMap.h

/*
 
 Copyright (c) 2014, Paul Whitcomb
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Paul Whitcomb nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef deathstar_ZZTMap_h
#define deathstar_ZZTMap_h

typedef enum {
    MAP_OK,
    MAP_INVALID_PATH,
    MAP_INVALID_INDEX_POINTER
} MapError;

struct Vector {
    float x;
    float y;
    float z;
};

typedef enum {
    OBJECT_BIPD = 0x0,
    OBJECT_VEHI = 0x1,
    OBJECT_WEAP = 0x2,
    OBJECT_EQIP = 0x3,
    OBJECT_GARB = 0x4,
    OBJECT_PROJ = 0x5,
    OBJECT_SCEN = 0x6,
    OBJECT_MACH = 0x7,
    OBJECT_CTRL = 0x8,
    OBJECT_LIFI = 0x9,
    OBJECT_PLAC = 0xA,
    OBJECT_SSCE = 0xB,
} TagObjectType;

typedef enum {
    SHADER_UNKN0 = 0x0, //unknown
    SHADER_UNKN1 = 0x1, //unknown
    SHADER_UNKN2 = 0x2, //unknown
    SHADER_SENV = 0x3,
    SHADER_SOSO = 0x4,
    SHADER_SOTR = 0x5,
    SHADER_SCHI = 0x6,
    SHADER_SCEX = 0x7,
    SHADER_SWAT = 0x8,
    SHADER_SGLA = 0x9,
    SHADER_SMET = 0xA,
    SHADER_SPLA = 0xB
} TagShaderType; //0x24

struct TagID {
    uint16_t tagTableIndex;
    uint16_t tableIndex;
};

struct Shader {
    char padding[0x24];
    uint16_t type;
};

struct TagReflexive {
    uint32_t count;
    uint32_t offset;
    uint32_t zero;
};

struct TagDependency {
    char className[4];
    uint32_t metaIndex;
    uint32_t zero;
    struct TagID identity;
};

struct HaloMapHeader {
	uint32_t integrityHead;
	uint32_t version;
	uint32_t length;
	uint32_t zero;
	uint32_t indexOffset;
	uint32_t metaSize;
	char padding[0x8];
	char name[0x20];
	char builddate[0x20];
	uint32_t type;
    char zeroes2[0x198];
    uint32_t integrityFoot;
};

struct HaloMapIndex {
	uint32_t tagIndexOffset;
	struct TagID scenarioTag;
	uint32_t mapId;
	uint32_t tagCount;
	uint32_t vertexCount;
	uint32_t vertexOffset;
	uint32_t indicesCount;
	uint32_t vertexSize;
	uint32_t modelSize;
	uint32_t tags;
};


struct MapTag {
	uint32_t classA;
	uint32_t classB;
	uint32_t classC;
	struct TagID identity;
	uint32_t nameOffset;
	uint32_t dataOffset;
	char padding[0x8];
};

struct MapData {
    char *buffer;
    uint32_t length;
    MapError error;
};

struct Dependency {
    char mainClass[4];
    uint32_t nameOffset;
    uint32_t zero;
    struct TagID tagId;
}; //0x10

struct Mod2ShaderDependencies {
    struct Dependency shader;           //0x0
    char padding[0x10];                 //0x10
};

struct Mod2Dependencies {
    char padding[0xDC];                 //0x0
    struct TagReflexive mod2Shaders;    //0xDC
};

struct WeapMagazineMagazineDependencies {
    char padding[0xC];                  //0x0
    struct Dependency equipment;        //0xC  eqip
}; //0x1C

struct WeapMagazineDependencies {
    char padding[0x38];                 //0x0
    struct Dependency reloadingEffect;  //0x38 effe
    struct Dependency chamberingEffect; //0x48 effe
    char padding1[0xC];                 //0x58
    uint32_t magazinesCount;            //0x64
    uint32_t magazinesOffset;           //0x68
    char padding2[0x4];                 //0x6C
}; //0x70

struct WeapTriggerFiringEffects {
    char padding[0x24];                 //0x0
    struct Dependency firingEffect;     //0x24 effe
    struct Dependency misfireEffect;    //0x34 effe
    struct Dependency emptyEffect;      //0x44 effe
    struct Dependency firingDamage;     //0x54 jpt!
    struct Dependency misfireDamage;    //0x64 jpt!
    struct Dependency emptyDamage;      //0x74 jpt!
}; //0x84

struct WeapTriggerDependencies {
    char padding[0x5C];                 //0x0
    struct Dependency chargingEffect;   //0x5C effe
    char padding2[0x28];                //0x6C
    struct Dependency projectile;       //0x94 proj <-- obje
    char padding3[0x64];                //0xA4
    struct TagReflexive firingEffect;   //0x108
}; //0x114

struct ObjeDependencies {
    uint16_t tagObjectType;             //0x0
    char padding[0x26];                 //0x2
    struct Dependency model;            //0x28 mod2
    struct Dependency animation;        //0x38 antr
    char padding2[0x28];                //0x48
    struct Dependency collision;        //0x70 coll
    struct Dependency physics;          //0x80 phys
    struct Dependency shader;           //0x90 shdr
    struct Dependency creationEffect;   //0xA0 effe
    char padding3[0xCC];                //0xB0
}; //0x17C
struct ItemDependencies {
    struct ObjeDependencies obje;       //0x0
    char padding[0xCC];                 //0x17C
    struct Dependency materialEffects;  //0x248 foot
    struct Dependency collisionSound;   //0x258 snd!
    char padding2[0x80];                //0x268
    struct Dependency detonatingEffect; //0x2E8 effe
    struct Dependency detonationEffect; //0x2F8 effe
}; //0x308
struct EqipDependencies {
    struct ItemDependencies item;       //0x0
    char padding[0x8];                  //0x308
    struct Dependency pickupSound;      //0x310 snd!
}; //0x320
struct UnitCameraTrackDependencies {
    struct Dependency cameraTrack;      //0x0
    char padding[0x10];                 //0x10
}; //0x18
struct UnitWeaponDependencies {
    struct Dependency weapon;           //0x0
    char padding[0x14];                 //0x10
}; //0x24
struct UnitDependencies {
    struct ObjeDependencies obje;       //0x0
    char padding[0xC];                  //0x17C
    struct Dependency integratedLight;  //0x188 ligh
    char padding2[0x5C];                //0x198
    struct TagReflexive cameraTrack;    //0x1F4
    char padding3[0x4C];                //0x200
    struct Dependency spawnedActor;     //0x24C actr
    char padding4[0x2C];                //0x25C
    struct Dependency meleeDamage;      //0x288 jpt!
    char padding5[0x10];                //0x298
    struct TagReflexive unitHud;        //0x2A8 -- not done yet
    struct TagReflexive unitDialogue;   //0x2B4 -- not done yet
    char padding6[0x18];                //0x2C0
    struct TagReflexive weapons;        //0x2D8
};
struct ProjDependencies {
    struct ObjeDependencies obje;       //0x0
    char padding[0x10];                 //0x17C
    struct Dependency superDetonation;  //0x18C effe
}; //incomplete
struct WeapDependencies {
    struct ItemDependencies item;       //0x0
    char padding[0x34];                 //0x308
    struct Dependency readyEffect;      //0x33C effe
    char padding2[0x28];                //0x34C
    struct Dependency overheatedEffect; //0x374 effe
    struct Dependency detonationEffect; //0x384 effe
    struct Dependency meleeDamage;      //0x394 jpt!
    struct Dependency meleeResponse;    //0x3A4 jpt!
    char padding3[0x8];                 //0x3B4
    struct Dependency firingParams;     //0x3BC actr
    char padding4[0x54];                //0x3CC
    struct Dependency lightOnEffect;    //0x420 effe
    struct Dependency lightOffEffect;   //0x430 effe
    char padding5[0x1C];                //0x440
    struct Dependency fpModel;          //0x45C mod2
    struct Dependency fpAnimation;      //0x46C antr
    char padding6[0x4];                 //0x47C
    struct Dependency hud;              //0x480 wphi
    struct Dependency pickupSound;      //0x490 snd!
    struct Dependency zoomInSound;      //0x4A0 snd!
    struct Dependency zoomOutSound;     //0x4B0 snd!
    char padding7[0x30];                //0x4C0
    struct TagReflexive magazines;      //0x4F0
    struct TagReflexive triggers;       //0x4FC
}; //0x508
struct ItmcPermutationDependencies {
    char padding[0x24];
    struct Dependency tagDependency;
    char padding1[0x20];
};
struct ItmcDependencies {
    struct TagReflexive permutation;    //0x0;
};
struct ScnrSkies {
    struct Dependency sky;              //0x0
}; //0x10
struct SBSPCollisionMaterialsDependencies {
    struct TagDependency shader;        //0x0
    uint32_t type;                      //0x10
};
struct SBSPLightmapsMaterialsReflexives {
    struct TagDependency shader;        //0x0
    char padding[0xF0];                 //0x10
};
struct SBSPLightmapsDependencies {
    char padding[0x14];                 //0x0
    struct TagReflexive materials;      //0x14
};
struct SBSPDependencies {
    char padding[0xBC];
    struct TagReflexive collMaterials;  //0xBC
    char padding1[0x54];
    struct TagReflexive lightmaps;      //0x11C
};
struct ScnrBSPs {
    uint32_t fileOffset;                //0x0
    uint32_t tagSize;                   //0x4
    uint32_t bspMagic;                  //0x8
    uint32_t zero;                      //0xC
    struct Dependency bsp;              //0x10
};
struct ScnrPaletteDependency {
    struct Dependency object;           //0x0   obje <--
    char padding[0x20];                 //0x10
}; //0x30
struct ScnrNetgameItmcDependencies {
    char padding[0x50];                 //0x0
    struct Dependency itemCollection;   //0x50
    char padding1[0x30];                //0x60
};//0x90
struct ScnrStartingEquipment {
    char padding[0x3C];                 //0x0
    struct Dependency equipment[0x6];   //0x3C
    char padding1[0x30];                //0x9C
};
struct ScnrDependencies {
    struct Dependency unknown[0x3];     //0x0   sbsp
    struct TagReflexive skies;          //0x30
    char padding[0x1E0];                //0x3C
    struct TagReflexive sceneryPalette; //0x21C
    char padding2[0xC];                 //0x228
    struct TagReflexive bipedPalette;   //0x234
    char padding3[0xC];                 //0x240
    struct TagReflexive vehiclePalette; //0x24C
    char padding4[0xC];                 //0x258
    struct TagReflexive equipPalette;   //0x264
    char padding5[0xC];                 //0x270
    struct TagReflexive weaponPalette;  //0x27C
    char padding6[0x18];                //0x288
    struct TagReflexive machinePalette; //0x2A0
    char padding7[0xC];                 //0x2AC
    struct TagReflexive controlPalette; //0x2B8
    char padding8[0xC];                 //0x2C4
    struct TagReflexive lifiPalette;    //0x2D0
    char padding9[0xC];                 //0x2DC
    struct TagReflexive sscePalette;    //0x2E8
    char padding10[0xC];                //0x2F4
    char padding11[0x84];               //0x300
    struct TagReflexive netgameItmcs;   //0x384
    struct TagReflexive startingItmcs;  //0x390
    char padding12[0x18];               //0x39C
    struct TagReflexive decalPalette;   //0x3B4 //offset is dependency[]
    struct TagReflexive detailObjects;  //0x3C0 //object palette
    char padding13[0x54];               //0x3CC
    struct TagReflexive actorPalette;   //0x420 //offset is dependency[]
    char padding14[0x18];
    struct TagReflexive AIAnimationRef; //0x444
    char padding15[0x154];              //0x450
    struct TagReflexive BSPs;           //0x5A4
};

struct MatgTagCollectionDependencies {
    struct Dependency tag;
};

struct MatgGrenadesDependencies {
    char padding[0x4];                  //0x0
    struct Dependency throwingEffect;   //0x4   effe
    struct Dependency hudInterface;     //0x14
    struct Dependency equipment;        //0x24  eqip
    struct Dependency projectile;       //0x34  proj <--
}; //0x44

struct MatgPlayerInformationDependencies {
    struct Dependency unit;             //0x0   unit <--
    char padding[0xA8];                 //0x10
    struct Dependency spawnEffect;      //0xB8  effe
    char padding1[0x2C];                //0xC8
}; //0xF4


struct MatgMultiplayerInformationDependencies {
    struct Dependency flag;             //0x0   weap <--
    struct Dependency unit;             //0x10  unit <--
    struct TagReflexive vehicles;       //0x20  --TagCollection
    struct Dependency hillShader;       //0x2C  shdr
    struct Dependency flagShader;       //0x3C  shdr
    struct Dependency ball;             //0x4C  weap <--
    struct Dependency sounds;           //0x5C  --TagCollection
    char padding[0x34];                 //0x6C
}; //0xA0

struct MatgDependencies {
    char padding[0xF8];                 //0x0
    struct TagReflexive sounds;         //0xF8  --TagCollection
    struct TagReflexive camera;         //0x104 --TagCollection
    char padding1[0x18];                //0x110
    struct TagReflexive grenades;       //0x128
    struct TagReflexive rasterizerData; //0x134 --incomplete
    struct TagReflexive interfaceBitm;  //0x140 --incomplete
    struct TagReflexive weapons;        //0x14C --TagCollection
    struct TagReflexive powerups;       //0x158 --TagCollection
    struct TagReflexive multiplayerInfo;//0x164 --incomplete
    struct TagReflexive playerInfo;     //0x170
    struct TagReflexive fpInterface;    //0x17C --incomplete
    struct TagReflexive fallingDamage;  //0x188 --incomplete
    struct TagReflexive materials;      //0x194 --incomplete
};


struct MapData openMapAtPath(const char *path);
void saveMap(const char *path, char *map_buffer, uint32_t length);
void zteam_deprotect(char *map_buffer,uint32_t length);
char *name_deprotect(char *map_buffer,uint32_t length, uint32_t *newLength, struct MapData *maps, int map_count);

#endif
