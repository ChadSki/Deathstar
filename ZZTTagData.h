// ZZTTagData.h

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

#ifndef deathstar_ZZTTagData_h
#define deathstar_ZZTTagData_h

#define BITM "mtib"
#define ANTR "rtna"
#define COLL "lloc"
#define MOD2 "2dom"
#define PHYS "syhp"
#define ITMC "cmti"
#define SCNR "rncs"
#define SKY  " yks"
#define MATG "gtam"
#define SBSP "psbs"
#define FONT "tnof"
#define USTR "rtsu"
#define TAGC "cgat"
#define SOUL "luoS"
#define SND  "!dns"
#define DELA "aLeD"
#define DEVC "cved"
#define HUDG "gduh"
#define SHDR "rdhs"
#define SOSO "osos"
#define SCEX "xecs"
#define SMET "tems"
#define SCHI "ihcs"
#define SPLA "alps"
#define SOTR "rtos"
#define SWAT "taws"
#define SGLA "algs"
#define SENV "vnes"
#define LENS "snel"
#define JPT  "!tpj"
#define EFFE "effe"
#define ACTV "vtca"
#define LIGH "hgil"
#define UNHI "ihnu"


typedef struct {
    uint16_t tagTableIndex;
    uint16_t tableIndex;
} __attribute__((packed)) TagID;

typedef struct {
    char padding[0x24];
    uint16_t type;
} __attribute__((packed)) Shader;

typedef struct {
    uint32_t count;
    uint32_t offset;
    uint32_t zero;
} __attribute__((packed)) TagReflexive;

typedef struct {
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
} __attribute__((packed)) HaloMapHeader;

typedef struct {
	uint32_t tagIndexOffset;
	TagID scenarioTag;
	uint32_t mapId;
	uint32_t tagCount;
	uint32_t vertexCount;
	uint32_t vertexOffset;
	uint32_t indicesCount;
	uint32_t vertexSize;
	uint32_t modelSize;
	uint32_t tags;
} __attribute__((packed)) HaloMapIndex;


typedef struct {
	uint32_t classA;
	uint32_t classB;
	uint32_t classC;
	TagID identity;
	uint32_t nameOffset;
	uint32_t dataOffset;
	char padding[0x8];
} __attribute__((packed)) MapTag;

typedef struct {
    char mainClass[4];
    uint32_t nameOffset;
    uint32_t zero;
    TagID tagId;
} __attribute__((packed)) Dependency; //0x10

typedef struct {
    Dependency shader;           //0x0
    char padding[0x10];          //0x10
} __attribute__((packed)) Mod2ShaderDependencies;

typedef struct {
    char padding[0xDC];          //0x0
    TagReflexive mod2Shaders;    //0xDC
} __attribute__((packed)) Mod2Dependencies;

typedef struct {
    char padding[0xC];           //0x0
    Dependency equipment;        //0xC  eqip
} __attribute__((packed)) WeapMagazineMagazineDependencies; //0x1C

typedef struct {
    char padding[0x38];          //0x0
    Dependency reloadingEffect;  //0x38 effe
    Dependency chamberingEffect; //0x48 effe
    char padding1[0xC];          //0x58
    uint32_t magazinesCount;     //0x64
    uint32_t magazinesOffset;    //0x68
    char padding2[0x4];          //0x6C
} __attribute__((packed)) WeapMagazineDependencies; //0x70

typedef struct {
    char padding[0x24];          //0x0
    Dependency firingEffect;     //0x24 effe
    Dependency misfireEffect;    //0x34 effe
    Dependency emptyEffect;      //0x44 effe
    Dependency firingDamage;     //0x54 jpt!
    Dependency misfireDamage;    //0x64 jpt!
    Dependency emptyDamage;      //0x74 jpt!
} __attribute__((packed)) WeapTriggerFiringEffects; //0x84

typedef struct {
    char padding[0x5C];          //0x0
    Dependency chargingEffect;   //0x5C effe
    char padding2[0x28];         //0x6C
    Dependency projectile;       //0x94 proj <-- obje
    char padding3[0x64];         //0xA4
    TagReflexive firingEffect;   //0x108
} __attribute__((packed)) WeapTriggerDependencies; //0x114

enum ResourceType {
    OBJE_TYPE_BITMAP = 0,
    OBJE_TYPE_SOUND = 1
};

typedef struct {
    uint16_t type; //0x0
    char padding[0x2]; //0x2
    TagID name; //0x4
} __attribute__((packed)) ObjeResources; //0x8

typedef struct {
    uint16_t tagObjectType;      //0x0
    char padding[0x26];          //0x2
    Dependency model;            //0x28 mod2
    Dependency animation;        //0x38 antr
    char padding2[0x28];         //0x48
    Dependency collision;        //0x70 coll
    Dependency physics;          //0x80 phys
    Dependency shader;           //0x90 shdr
    Dependency creationEffect;   //0xA0 effe
    char padding3[0xC0];         //0xB0
    TagReflexive resources;      //0x140
} __attribute__((packed)) ObjeDependencies; //0x17C
typedef struct {
    ObjeDependencies obje;       //0x0
    char padding[0xCC];          //0x17C
    Dependency materialEffects;  //0x248 foot
    Dependency collisionSound;   //0x258 snd!
    char padding2[0x80];         //0x268
    Dependency detonatingEffect; //0x2E8 effe
    Dependency detonationEffect; //0x2F8 effe
} __attribute__((packed)) ItemDependencies; //0x308
typedef struct {
    ItemDependencies item;       //0x0
    char padding[0x8];           //0x308
    Dependency pickupSound;      //0x310 snd!
} __attribute__((packed)) EqipDependencies; //0x320
typedef struct {
    Dependency cameraTrack;      //0x0
    char padding[0x10];          //0x10
} __attribute__((packed)) UnitCameraTrackDependencies; //0x18
typedef struct {
    Dependency weapon;           //0x0
    char padding[0x14];          //0x10
} __attribute__((packed)) UnitWeaponDependencies; //0x24
typedef struct {
    ObjeDependencies obje;       //0x0
    char padding[0xC];           //0x17C
    Dependency integratedLight;  //0x188 ligh
    char padding2[0x5C];         //0x198
    TagReflexive cameraTrack;    //0x1F4
    char padding3[0x4C];         //0x200
    Dependency spawnedActor;     //0x24C actr
    char padding4[0x2C];         //0x25C
    Dependency meleeDamage;      //0x288 jpt!
    char padding5[0x10];         //0x298
    TagReflexive unitHud;        //0x2A8 -- not done yet
    TagReflexive unitDialogue;   //0x2B4 -- not done yet
    char padding6[0x18];         //0x2C0
    TagReflexive weapons;        //0x2D8
} __attribute__((packed)) UnitDependencies;
typedef struct {
    char padding[0x4];           //0x0
    Dependency defaultResult;    //0x4   effe
    char padding1[0x28];         //0x14
    Dependency potentialResult;  //0x3C  effe
    char padding2[0x1C];         //0x4C  effe
    Dependency detonationEffect; //0x68  effe
    char padding3[0x28];         //0x78
} __attribute__((packed)) ProjMaterialResponseDependencies;
typedef struct {
    ObjeDependencies obje;       //0x0
    char padding[0x10];          //0x17C
    Dependency superDetonation;  //0x18C effe
    char padding1[0x10];         //0x19C
    Dependency detonationEffect; //0x1AC effe
    char padding2[0x38];         //0x1BC
    Dependency detonationStarted;//0x1F4 effe
    Dependency flybySound;       //0x204 effe
    Dependency attachedDamage;   //0x214 jpt!
    Dependency impactDamage;     //0x224 jpt!
    char padding3[0xC];          //0x234
    TagReflexive materialRespond;//0x240 effe's
} __attribute__((packed)) ProjDependencies; //incomplete
typedef struct {
    ItemDependencies item;       //0x0
    char padding[0x34];          //0x308
    Dependency readyEffect;      //0x33C effe
    char padding2[0x28];         //0x34C
    Dependency overheatedEffect; //0x374 effe
    Dependency detonationEffect; //0x384 effe
    Dependency meleeDamage;      //0x394 jpt!
    Dependency meleeResponse;    //0x3A4 jpt!
    char padding3[0x8];          //0x3B4
    Dependency firingParams;     //0x3BC actr
    char padding4[0x54];         //0x3CC
    Dependency lightOnEffect;    //0x420 effe
    Dependency lightOffEffect;   //0x430 effe
    char padding5[0x1C];         //0x440
    Dependency fpModel;          //0x45C mod2
    Dependency fpAnimation;      //0x46C antr
    char padding6[0x4];          //0x47C
    Dependency hud;              //0x480 wphi
    Dependency pickupSound;      //0x490 snd!
    Dependency zoomInSound;      //0x4A0 snd!
    Dependency zoomOutSound;     //0x4B0 snd!
    char padding7[0x30];         //0x4C0
    TagReflexive magazines;      //0x4F0
    TagReflexive triggers;       //0x4FC
} __attribute__((packed)) WeapDependencies; //0x508
typedef struct {
    char padding[0x24];
    Dependency dependency;
    char padding1[0x20];
} __attribute__((packed)) ItmcPermutationDependencies;
typedef struct {
    TagReflexive permutation;    //0x0;
} __attribute__((packed)) ItmcDependencies;
typedef struct {
    Dependency sky;              //0x0
} __attribute__((packed)) ScnrSkies; //0x10
typedef struct {
    Dependency shader;           //0x0
    uint32_t type;               //0x10
} __attribute__((packed)) SBSPCollisionMaterialsDependencies;
typedef struct {
    Dependency shader;           //0x0
    char padding[0xF0];          //0x10
} __attribute__((packed)) SBSPLightmapsMaterialsReflexives;
typedef struct {
    char padding[0x14];          //0x0
    TagReflexive materials;      //0x14
} __attribute__((packed)) SBSPLightmapsDependencies;
typedef struct {
    char padding[0xBC];
    TagReflexive collMaterials;  //0xBC
    char padding1[0x54];
    TagReflexive lightmaps;      //0x11C
} __attribute__((packed)) SBSPDependencies;
typedef struct {
    uint32_t fileOffset;         //0x0
    uint32_t tagSize;            //0x4
    uint32_t bspMagic;           //0x8
    uint32_t zero;               //0xC
    Dependency bsp;              //0x10
} __attribute__((packed)) ScnrBSPs;
typedef struct {
    Dependency object;           //0x0   obje <--
    char padding[0x20];          //0x10
} __attribute__((packed)) ScnrPaletteDependency; //0x30
typedef struct {
    char padding[0x50];          //0x0
    Dependency itemCollection;   //0x50
    char padding1[0x30];         //0x60
} __attribute__((packed)) ScnrNetgameItmcDependencies; //0x90
typedef struct {
    char padding[0x3C];          //0x0
    Dependency equipment[0x6];   //0x3C
    char padding1[0x30];         //0x9C
} __attribute__((packed)) ScnrStartingEquipment;
typedef struct {
    Dependency unknown[0x3];     //0x0   sbsp
    TagReflexive skies;          //0x30
    char padding[0x1E0];         //0x3C
    TagReflexive sceneryPalette; //0x21C
    char padding2[0xC];          //0x228
    TagReflexive bipedPalette;   //0x234
    char padding3[0xC];          //0x240
    TagReflexive vehiclePalette; //0x24C
    char padding4[0xC];          //0x258
    TagReflexive equipPalette;   //0x264
    char padding5[0xC];          //0x270
    TagReflexive weaponPalette;  //0x27C
    char padding6[0x18];         //0x288
    TagReflexive machinePalette; //0x2A0
    char padding7[0xC];          //0x2AC
    TagReflexive controlPalette; //0x2B8
    char padding8[0xC];          //0x2C4
    TagReflexive lifiPalette;    //0x2D0
    char padding9[0xC];          //0x2DC
    TagReflexive sscePalette;    //0x2E8
    char padding10[0xC];         //0x2F4
    char padding11[0x84];        //0x300
    TagReflexive netgameItmcs;   //0x384
    TagReflexive startingItmcs;  //0x390
    char padding12[0x18];        //0x39C
    TagReflexive decalPalette;   //0x3B4 //offset is dependency[]
    TagReflexive detailObjects;  //0x3C0 //object palette
    char padding13[0x54];        //0x3CC
    TagReflexive actorPalette;   //0x420 //offset is dependency[]
    char padding14[0x18];
    TagReflexive AIAnimationRef; //0x444
    char padding15[0x154];       //0x450
    TagReflexive BSPs;           //0x5A4
} __attribute__((packed)) ScnrDependencies;

typedef struct {
    Dependency tag;
} __attribute__((packed)) MatgTagCollectionDependencies;

typedef struct {
    char padding[0x4];           //0x0
    Dependency throwingEffect;   //0x4   effe
    Dependency hudInterface;     //0x14
    Dependency equipment;        //0x24  eqip
    Dependency projectile;       //0x34  proj <--
} __attribute__((packed)) MatgGrenadesDependencies; //0x44

typedef struct {
    Dependency unit;             //0x0   unit <--
    char padding[0xA8];          //0x10
    Dependency spawnEffect;      //0xB8  effe
    char padding1[0x2C];         //0xC8
} __attribute__((packed)) MatgPlayerInformationDependencies; //0xF4

typedef struct {
    Dependency flag;             //0x0   weap <--
    Dependency unit;             //0x10  unit <--
    TagReflexive vehicles;       //0x20  --TagCollection
    Dependency hillShader;       //0x2C  shdr
    Dependency flagShader;       //0x3C  shdr
    Dependency ball;             //0x4C  weap <--
    Dependency sounds;           //0x5C  --TagCollection
    char padding[0x34];          //0x6C
} __attribute__((packed)) MatgMultiplayerInformationDependencies; //0xA0

typedef struct {
    char padding[0xF8];          //0x0
    TagReflexive sounds;         //0xF8  --TagCollection
    TagReflexive camera;         //0x104 --TagCollection
    char padding1[0x18];         //0x110
    TagReflexive grenades;       //0x128
    TagReflexive rasterizerData; //0x134 --incomplete
    TagReflexive interfaceBitm;  //0x140 --incomplete
    TagReflexive weapons;        //0x14C --TagCollection
    TagReflexive powerups;       //0x158 --TagCollection
    TagReflexive multiplayerInfo;//0x164 --incomplete
    TagReflexive playerInfo;     //0x170
    TagReflexive fpInterface;    //0x17C --incomplete
    TagReflexive fallingDamage;  //0x188 --incomplete
    TagReflexive materials;      //0x194 --incomplete
} __attribute__((packed)) MatgDependencies;

typedef struct {
    char padding[0xA4]; //0x0
    Dependency baseMap; //0xA4
    char padding1[0x8]; //0xB4
    Dependency multiMap; //0xBC
    char padding2[0x10]; //0xCC
    Dependency detailMap; //0xDC
    char padding3[0x78]; //0xEC
    Dependency reflectMap; //0x164
} __attribute__((packed)) ShaderSosoDependencies;

typedef struct {
    char padding[0xD4]; //0x0
    Dependency primaryNoiseMap; //0xD4
    char padding1[0x38]; //0xE4
    Dependency secondaryNoiseMap; //0x114
} __attribute__((packed)) ShaderSplaDependencies;

typedef struct {
    Dependency shader; //0x0
} __attribute__((packed)) ShaderShaderLayersDependencies;

typedef struct {
    char padding[0x1C]; //0x0
    Dependency map; //0x1C
    char padding1[0x38]; //0x2C
} __attribute__((packed)) ShaderSotrMapDependencies;

typedef struct {
    char padding[0x38]; //0x0
    Dependency lensflare; //0x38
    TagReflexive layers; //0x48
    TagReflexive maps; //0x54
} __attribute__((packed)) ShaderSotrDependencies;

typedef struct {
    char padding[0x4C]; //0x0
    Dependency map; //0x4C
} __attribute__((packed)) ShaderSmetDependencies;

typedef struct {
    char padding[0x6C]; //0x0
    Dependency map; //0x6C
    char padding1[0x60]; //0x7C
} __attribute__((packed)) ShaderSchiMapDependencies;

typedef struct {
    char padding[0x38]; //0x0
    Dependency lensflare; //0x38
    TagReflexive layers; //0x48
    TagReflexive maps; //0x54
} __attribute__((packed)) ShaderSchiDependencies;

typedef struct {
    char padding[0x30]; //0x0
    Dependency lensFlare; //0x30
    char padding1[0x48]; //0x40
    Dependency baseMap; //0x88
    char padding2[0x20]; //0x98
    Dependency primaryDetailMap; //0xB8
    char padding3[0x4]; //0xC8
    Dependency secondaryDetailMap; //0xCC
    char padding4[0x20]; //0xDC
    Dependency microDetailMap; //0xFC
    char padding5[0x1C]; //0x10C
    Dependency bumpMap; //0x128
    char padding6[0x11C]; //0x138
    Dependency illuminationMap; //0x254
    char padding7[0xC0]; //0x264
    Dependency reflectionCubeMap; //0x324
} __attribute__((packed)) ShaderSenvDependencies;

typedef struct {
    char padding[0x38]; //0x0
    Dependency lensflare; //0x38
    TagReflexive layers; //0x48
    TagReflexive stage4maps; //0x54
    TagReflexive stage2maps; //0x60
} __attribute__((packed)) ShaderScexDependencies;

typedef struct {
    char padding[0x4C]; //0x0
    Dependency baseMap; //0x4C
    char padding1[0x40]; //0x5C
    Dependency reflectionMap; //0x9C
    char padding2[0x1C]; //0xAC
    Dependency rippleMap; //0xC8
} __attribute__((packed)) ShaderSwatDependencies;

typedef struct {
    char padding[0x64]; //0x0
    Dependency bgTint; //0x64
    char padding1[0x38]; //0x74
    Dependency reflectionMap; //0xAC
    float padding2; //0xBC
    Dependency bumpMap; //0xC0
    char padding3[0x88]; //0xD0
    Dependency diffuseMap; //0x158
    float padding4; //0x168
    Dependency diffuseDetailMap; //0x16C
    char padding5[0x24]; //0x17C
    Dependency specularMap; //0x190
    float padding6; //0x1B0
    Dependency specularDetailMap; //0x1B4
} __attribute__((packed)) ShaderSglaDependencies;

#endif
