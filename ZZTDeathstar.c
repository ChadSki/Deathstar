//ZZTDeathstar.c

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
#include <time.h>
#include "ZZTDeathstar.h"

#define META_MEMORY_OFFSET 0x40440000 //Halo CE and Halo PC ONLY

typedef enum {
    false = 0,
    true = 1
} bool;

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
    char className[4];
    uint32_t metaIndex;
    uint32_t zero;
    TagID identity;
} __attribute__((packed)) TagDependency;

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
    char padding3[0xCC];         //0xB0
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
    ObjeDependencies obje;       //0x0
    char padding[0x10];          //0x17C
    Dependency superDetonation;  //0x18C effe
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
    Dependency tagDependency;
    char padding1[0x20];
} __attribute__((packed)) ItmcPermutationDependencies;
typedef struct {
    TagReflexive permutation;    //0x0;
} __attribute__((packed)) ItmcDependencies;
typedef struct {
    Dependency sky;              //0x0
} __attribute__((packed)) ScnrSkies; //0x10
typedef struct {
    TagDependency shader;        //0x0
    uint32_t type;               //0x10
} __attribute__((packed)) SBSPCollisionMaterialsDependencies;
typedef struct {
    TagDependency shader;        //0x0
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

TagID *deprotectedTags;
int deprotectedTagsCount;

 MapTag *tagArray;
uint32_t tagCount;

uint32_t magic;

char *mapdata;
uint32_t mapdataSize;
uint32_t tagdataSize;

static bool isNulledOut(TagID tag) {
    return tag.tagTableIndex == 0xFFFF;
}

MapData openMapAtPath(const char *path) {
    MapData mapData;
    FILE *map = fopen(path,"r");
    if(map)
    {
        fseek(map,0x0,SEEK_END);
        mapData.length = (uint32_t)ftell(map);
        fseek(map,0x0,SEEK_SET);
        mapData.buffer = malloc(mapData.length);
        fread(mapData.buffer,mapData.length,0x1,map);
        fclose(map);
         HaloMapHeader *mapHeader = ( HaloMapHeader *)(mapData.buffer);
        if(mapHeader->indexOffset > mapData.length) {
            mapData.error = MAP_INVALID_INDEX_POINTER;
        }
        else {
            mapData.error = MAP_OK;
        }
    }
    else {
        mapData.error = MAP_INVALID_PATH;
    }
    return mapData;
}

void saveMap(const char *path, MapData map) {
    FILE *mapFile = fopen(path,"wb");
    fwrite(map.buffer,1,map.length,mapFile);
    fclose(mapFile);
}

static void zteam_changeTagClass(TagID tagId,const char *class) {
    if(tagId.tagTableIndex != 0xFFFF)
        tagArray[tagId.tagTableIndex].classA = *(uint32_t *)(class);
}

static void *translatePointer(uint32_t pointer) { //translates a map pointer to where it points to in Deathstar
    return mapdata + (pointer - magic);
}

static void zteam_deprotectShdr(TagID tagId) {
    uint32_t tagClasses[] = {
        *(uint32_t *)&"rdhs",
        *(uint32_t *)&"rdhs",
        *(uint32_t *)&"rdhs",
        *(uint32_t *)&"vnes",
        *(uint32_t *)&"osos",
        *(uint32_t *)&"rtos",
        *(uint32_t *)&"ihcs",
        *(uint32_t *)&"xecs",
        *(uint32_t *)&"taws",
        *(uint32_t *)&"algs",
        *(uint32_t *)&"tems",
        *(uint32_t *)&"alps"
    };
     Shader shdr = *( Shader *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
    if(shdr.type < 0x3 || shdr.type > sizeof(tagClasses) / 0x4)
        return;
    zteam_changeTagClass(tagId, (const char *)&tagClasses[shdr.type]);
}

static void zteam_deprotectMod2(TagID tagId) {
    zteam_changeTagClass(tagId,"2dom");
     Mod2Dependencies model = *( Mod2Dependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
     Mod2ShaderDependencies *shaders = translatePointer(model.mod2Shaders.offset);
    for(uint32_t i=0;i<model.mod2Shaders.count;i++) {
        zteam_deprotectShdr(shaders[i].shader.tagId);
    }
}

static void zteam_deprotectObjectTag(TagID tagId) {
    if(tagId.tagTableIndex > tagCount) return;
    for(int i=0; i<deprotectedTagsCount ;i++) {
        if(deprotectedTags[i].tagTableIndex == tagId.tagTableIndex)
            return;
    }
    uint32_t tagClasses[] = {
        *(uint32_t *)&"dpib",
        *(uint32_t *)&"ihev",
        *(uint32_t *)&"paew",
        *(uint32_t *)&"piqe",
        *(uint32_t *)&"brag",
        *(uint32_t *)&"jorp",
        *(uint32_t *)&"necs",
        *(uint32_t *)&"hcam",
        *(uint32_t *)&"lrtc",
        *(uint32_t *)&"ifil",
        *(uint32_t *)&"calp",
        *(uint32_t *)&"ecss"
    };
    
    deprotectedTags[deprotectedTagsCount] = tagId;
    deprotectedTagsCount++;
    
     ObjeDependencies object = *( ObjeDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
    
    if(object.tagObjectType > sizeof(tagClasses) / 0x4)
    {
        zteam_changeTagClass(tagId,"ejbo");
        return;
    }
    
    zteam_changeTagClass(tagId,(const char *)&tagClasses[object.tagObjectType]);
    
    if(!isNulledOut(object.model.tagId)) {
        zteam_deprotectMod2(object.model.tagId);
    }
    zteam_changeTagClass(object.animation.tagId,"rtna");
    zteam_changeTagClass(object.collision.tagId,"lloc");
    zteam_changeTagClass(object.physics.tagId,"syhp");
    
    if(object.tagObjectType == OBJECT_WEAP) {
         WeapDependencies weap = *( WeapDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
        zteam_changeTagClass(weap.fpModel.tagId, "2dom");
        zteam_changeTagClass(weap.fpAnimation.tagId, "rtna");
         WeapTriggerDependencies *triggers = translatePointer(weap.triggers.offset);
        for(uint32_t i=0;i<weap.triggers.count;i++) {
            zteam_deprotectObjectTag(triggers[i].projectile.tagId);
        }
    }
    if(object.tagObjectType == OBJECT_VEHI || object.tagObjectType == OBJECT_BIPD) {
         UnitDependencies unit = *( UnitDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
         UnitWeaponDependencies *weapons = ( UnitWeaponDependencies *)translatePointer(unit.weapons.offset);
        for(uint32_t i=0;i<unit.weapons.count;i++) {
            zteam_deprotectObjectTag(weapons[i].weapon.tagId);
        }
    }
}

static void zteam_deprotectObjectPalette(TagReflexive reflexive) {
     ScnrPaletteDependency *palette = ( ScnrPaletteDependency *)translatePointer(reflexive.offset);
    for(int i=0;i<reflexive.count;i++) {
        zteam_deprotectObjectTag(palette[i].object.tagId);
    }
}

static void zteam_deprotectMatgObjectTagCollection(TagReflexive reflexive) {
     MatgTagCollectionDependencies *collection = ( MatgTagCollectionDependencies *)translatePointer(reflexive.offset);
    for(int i=0;i<reflexive.count;i++) {
        zteam_deprotectObjectTag(collection[i].tag.tagId);
    }
}

static void zteam_deprotectItmc(TagID tag) {
     ItmcDependencies itmc = *( ItmcDependencies *)translatePointer(tagArray[tag.tagTableIndex].dataOffset);
    zteam_changeTagClass(tag,"cmti");
     ItmcPermutationDependencies *itmcPerm = ( ItmcPermutationDependencies *)translatePointer(itmc.permutation.offset);
    for(uint32_t i=0;i<itmc.permutation.count;i++) {
        zteam_deprotectObjectTag(itmcPerm[i].tagDependency.tagId);
    }
}

static void *translateCustomPointer(uint32_t pointer, uint32_t customMagic, uint32_t offset) {
    if(pointer < 0x40000000) {
        return NULL;
    }
    else if(pointer < 0x40440000 + tagdataSize) {
        return translatePointer(pointer);
    }
    else
        return mapdata + offset + (pointer - customMagic);
}

static void zteam_deprotectSBSP(TagID tagId,uint32_t fileOffset, uint32_t bspMagic) {
    if(tagId.tagTableIndex == 0xFFFF)
        return;
    zteam_changeTagClass(tagId, "psbs");
     SBSPDependencies sbsp = *( SBSPDependencies *)(mapdata + fileOffset);
     SBSPCollisionMaterialsDependencies *materials = ( SBSPCollisionMaterialsDependencies *)translateCustomPointer(sbsp.collMaterials.offset,bspMagic,fileOffset);
    for(uint32_t i=0;i<sbsp.collMaterials.count;i++) {
        zteam_deprotectShdr(materials[i].shader.identity);
    }
     SBSPLightmapsDependencies *lightmaps = ( SBSPLightmapsDependencies *)translateCustomPointer(sbsp.lightmaps.offset,bspMagic,fileOffset);
    for(uint32_t i=0;i<sbsp.lightmaps.count;i++) {
         SBSPLightmapsMaterialsReflexives *materials = ( SBSPLightmapsMaterialsReflexives *)translateCustomPointer(lightmaps[i].materials.offset, bspMagic, fileOffset);
        for(uint32_t q=0;q<lightmaps[i].materials.count;q++) {
            zteam_deprotectShdr(materials[q].shader.identity);
        }
    }
}

static bool classCanBeDeprotected(uint32_t class) {
    
    uint32_t tagClasses[] = {
        *(uint32_t *)&"aLeD",
        *(uint32_t *)&"cved",
        *(uint32_t *)&"aLeD",
        *(uint32_t *)&"ghud",
        *(uint32_t *)&"aLeD",
        *(uint32_t *)&"gtam",
        *(uint32_t *)&"aLeD",
        *(uint32_t *)&"!dns",
        *(uint32_t *)&"luoS",
        *(uint32_t *)&"cgat",
        *(uint32_t *)&"rtsu"
    };
    for(int i=0;i<sizeof(tagClasses)/4;i++) {
        if(class == tagClasses[i]) return false;
    }
    return true;
}

static bool classAutogeneric(uint32_t class) {
    uint32_t tagClasses[] = {
        *(uint32_t *)&"mtib",
        *(uint32_t *)&"psbs",
        *(uint32_t *)&"rncs",
        *(uint32_t *)&"cmti",
        *(uint32_t *)&"tnof"
    };
    for(int i=0;i<sizeof(tagClasses)/4;i++) {
        if(class == tagClasses[i]) return true;
    }
    return false;
}

#define MATCHING_THRESHOLD 0.7

MapData name_deprotect(MapData map, MapData *maps, int map_count) {
    uint32_t length = map.length;
    char *map_buffer = map.buffer;
    
    char *modded_buffer = malloc(length);
    memcpy(modded_buffer,map_buffer,length);
    
    HaloMapHeader *header = ( HaloMapHeader *)(modded_buffer);
    HaloMapIndex *index = ( HaloMapIndex *)(modded_buffer + header->indexOffset);
    
    mapdata = modded_buffer;
    magic = META_MEMORY_OFFSET - header->indexOffset;
    
    char *names = calloc(0x20 * index->tagCount,0x1);
    int namesLength = 0x0;
    
    tagArray = ( MapTag *)(translatePointer(index->tagIndexOffset));
    tagCount = index->tagCount;
    
    for(uint32_t i=0;i<tagCount;i++) {
        if(!classCanBeDeprotected(tagArray[i].classA) || tagArray[i].nameOffset < META_MEMORY_OFFSET || tagArray[i].nameOffset > META_MEMORY_OFFSET + header->indexOffset) {
            continue;
        }
        
        if(strncmp(translatePointer(tagArray[i].nameOffset),"ui\\",0x3) == 0)
            continue;
        
        bool automaticallyGeneric = classAutogeneric(tagArray[i].classA);
        
        bool deprotected = false;
        
        if(!automaticallyGeneric) {
            float best_match = 0.0;
            for(int map=0;map<map_count;map++) {
                if(maps[map].error != MAP_OK) continue;
                //TODO: add fuzzy tag comparison
            }
            if(best_match != 0.0) {
                deprotected = false;
            }
        }
        
        if(!deprotected)
            sprintf(names + namesLength, "deathstar\\tag_%u", i);
        
        int newname_length = (int)strlen(names + namesLength);
        
        tagArray[i].nameOffset = length + namesLength + magic;
        
        namesLength += newname_length + 0x1;
    }
    
    uint32_t new_length = length + namesLength;
    
    header->length = new_length;
    header->metaSize = new_length - header->indexOffset;
    
    char *new_buffer = calloc(length + namesLength,0x1);
    memcpy(new_buffer, modded_buffer, length);
    memcpy(new_buffer + length, names, namesLength);
    
    free(modded_buffer);
    
    MapData new_map;
    new_map.buffer = new_buffer;
    new_map.length = new_length;
    new_map.error = MAP_OK;
    
    return new_map;
}

MapData zteam_deprotect(MapData map)
{
    MapData new_map;
    
    new_map.buffer = malloc(map.length);
    new_map.length = map.length;
    new_map.error = MAP_OK;
    
    memcpy(new_map.buffer,map.buffer,map.length);
    
    mapdata = new_map.buffer;
    uint32_t length = new_map.length;
    
    HaloMapHeader *header = ( HaloMapHeader *)(new_map.buffer);
    HaloMapIndex *index = ( HaloMapIndex *)(new_map.buffer + header->indexOffset);
    
    deprotectedTags = calloc(sizeof(TagID) * index->tagCount,0x1);
    
    magic = META_MEMORY_OFFSET - header->indexOffset;
    
    tagArray = ( MapTag *)(translatePointer(index->tagIndexOffset));
    tagCount = index->tagCount;
    
    mapdataSize = length;
    tagdataSize = length - header->indexOffset;
    
    MapTag scenarioTag = tagArray[index->scenarioTag.tagTableIndex];
    deprotectedTags[deprotectedTagsCount] = scenarioTag.identity;
    deprotectedTagsCount++;
    zteam_changeTagClass(index->scenarioTag,"rncs");
    
    ScnrDependencies scnrData = *( ScnrDependencies *)translatePointer(scenarioTag.dataOffset);
    
    zteam_deprotectObjectPalette(scnrData.sceneryPalette);
    zteam_deprotectObjectPalette(scnrData.bipedPalette);
    zteam_deprotectObjectPalette(scnrData.equipPalette);
    zteam_deprotectObjectPalette(scnrData.vehiclePalette);
    zteam_deprotectObjectPalette(scnrData.weaponPalette);
    zteam_deprotectObjectPalette(scnrData.machinePalette);
    zteam_deprotectObjectPalette(scnrData.controlPalette);
    zteam_deprotectObjectPalette(scnrData.lifiPalette);
    zteam_deprotectObjectPalette(scnrData.sscePalette);
    
    ScnrSkies *skies = ( ScnrSkies *)translatePointer(scnrData.skies.offset);
    for(uint32_t i=0;i<scnrData.skies.count;i++) {
        zteam_changeTagClass(skies[i].sky.tagId, " yks");
    }
    
    ScnrBSPs *bsps = ( ScnrBSPs *)translatePointer(scnrData.BSPs.offset);
    for(uint32_t i=0;i<scnrData.BSPs.count;i++) {
        zteam_deprotectSBSP(bsps[i].bsp.tagId, bsps[i].fileOffset, bsps[i].bspMagic);
    }
    
    ScnrNetgameItmcDependencies *itmcs = ( ScnrNetgameItmcDependencies *)translatePointer(scnrData.netgameItmcs.offset);
    for(uint32_t i=0;i<scnrData.netgameItmcs.count;i++) {
        zteam_deprotectItmc(itmcs[i].itemCollection.tagId);
    }
    
    TagID matgTag;
    matgTag.tagTableIndex = 0xFFFF;
    
    for(int i=0;i<tagCount;i++) {
        char *name = translatePointer(tagArray[i].nameOffset);
        uint32_t class = tagArray[i].classA;
        if(class == *(uint32_t *)&"gtam" && strcmp(name,"globals\\globals") == 0) {
            matgTag = tagArray[i].identity;
            break;
        }
    }
    
    if(!isNulledOut(matgTag)) {
        MatgDependencies matg = *( MatgDependencies *)(translatePointer(tagArray[matgTag.tagTableIndex].dataOffset));
        zteam_deprotectMatgObjectTagCollection(matg.weapons);
        zteam_deprotectMatgObjectTagCollection(matg.powerups);
        
        MatgPlayerInformationDependencies *playerInfo = translatePointer(matg.playerInfo.offset);
        for(uint32_t i=0;i<matg.playerInfo.count;i++) {
            zteam_deprotectObjectTag(playerInfo[i].unit.tagId);
        }
        
        MatgMultiplayerInformationDependencies *multiplayerInfo = translatePointer(matg.multiplayerInfo.offset);
        for(uint32_t i=0;i<matg.multiplayerInfo.count;i++) {
            zteam_deprotectObjectTag(multiplayerInfo[i].unit.tagId);
            zteam_deprotectObjectTag(multiplayerInfo[i].flag.tagId);
            zteam_deprotectObjectTag(multiplayerInfo[i].ball.tagId);
            zteam_deprotectMatgObjectTagCollection(multiplayerInfo[i].vehicles);
        }
    }
    
    free(deprotectedTags);
    
    return new_map;
}