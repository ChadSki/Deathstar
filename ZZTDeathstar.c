// ZZTDeathstar.c

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
#include "ZZTTagData.h"

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

bool *deprotectedTags; //check for CERTAIN tags

MapTag *tagArray;
uint32_t tagCount;

uint32_t magic;

char *mapdata;
uint32_t mapdataSize;
uint32_t tagdataSize;

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

int saveMap(const char *path, MapData map) {
    FILE *mapFile = fopen(path,"wb");
    if(mapFile) {
        fwrite(map.buffer,1,map.length,mapFile);
        fclose(mapFile);
        return 0;
    }
    return 1;
}

static bool isNulledOut(TagID tag) {
    return (tag.tableIndex == 0 && tag.tagTableIndex == 0) || tag.tagTableIndex > tagCount;
}

static void zteam_changeTagClass(TagID tagId,const char *class) {
    if(isNulledOut(tagId)) return;
    if(deprotectedTags[tagId.tagTableIndex]) return;
    tagArray[tagId.tagTableIndex].classA = *(uint32_t *)(class);
}

static void *translatePointer(uint32_t pointer) { //translates a map pointer to where it points to in Deathstar
    return mapdata + (pointer - magic);
}

static void zteam_deprotectShdr(TagID tagId) {
    if(isNulledOut(tagId)) return; //however, this also means the map is broken
    if(deprotectedTags[tagId.tagTableIndex]) return;
    uint32_t tagClasses[] = {
        *(uint32_t *)&SHDR,
        *(uint32_t *)&SHDR,
        *(uint32_t *)&SHDR,
        *(uint32_t *)&SENV,
        *(uint32_t *)&SOSO,
        *(uint32_t *)&SOTR,
        *(uint32_t *)&SCHI,
        *(uint32_t *)&SCEX,
        *(uint32_t *)&SWAT,
        *(uint32_t *)&SGLA,
        *(uint32_t *)&SMET,
        *(uint32_t *)&SPLA
    };
    void *location = translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
    Shader shdr = *(Shader *)location;
    if(shdr.type < 0x3 || shdr.type > sizeof(tagClasses) / 0x4) return;
    zteam_changeTagClass(tagId, (const char *)&tagClasses[shdr.type]);
    if(shdr.type == SHADER_SCEX) {
        ShaderScexDependencies scex = *(ShaderScexDependencies *)location;
        ShaderShaderLayersDependencies *layers = translatePointer(scex.layers.offset);
        for(uint32_t i=0; i<scex.layers.count; i++) {
            zteam_deprotectShdr(layers[i].shader.tagId);
        }
        zteam_changeTagClass(scex.lensflare.tagId, LENS);
        ShaderSchiMapDependencies *stage4maps = (ShaderSchiMapDependencies *)translatePointer(scex.stage4maps.offset);
        for(uint32_t i=0; i<scex.stage4maps.count;i++) {
            zteam_changeTagClass(stage4maps[i].map.tagId, BITM);
        }
        ShaderSchiMapDependencies *stage2maps = (ShaderSchiMapDependencies *)translatePointer(scex.stage2maps.offset);
        for(uint32_t i=0; i<scex.stage2maps.count;i++) {
            zteam_changeTagClass(stage2maps[i].map.tagId, BITM);
        }
    }
    else if(shdr.type == SHADER_SCHI) {
        ShaderSchiDependencies schi = *(ShaderSchiDependencies *)location;
        ShaderShaderLayersDependencies *layers = translatePointer(schi.layers.offset);
        for(uint32_t i=0; i<schi.layers.count; i++) {
            zteam_deprotectShdr(layers[i].shader.tagId);
        }
        ShaderSchiMapDependencies *maps = translatePointer(schi.maps.offset);
        for(uint32_t i=0; i<schi.maps.count; i++) {
            zteam_changeTagClass(maps[i].map.tagId, BITM);
        }
        zteam_changeTagClass(schi.lensflare.tagId, LENS);
    }
    else if(shdr.type == SHADER_SENV) {
        ShaderSenvDependencies senv = *(ShaderSenvDependencies *)location;
        zteam_changeTagClass(senv.baseMap.tagId, BITM);
        zteam_changeTagClass(senv.bumpMap.tagId, BITM);
        zteam_changeTagClass(senv.illuminationMap.tagId, BITM);
        zteam_changeTagClass(senv.lensFlare.tagId, LENS);
        zteam_changeTagClass(senv.microDetailMap.tagId, BITM);
        zteam_changeTagClass(senv.primaryDetailMap.tagId, BITM);
        zteam_changeTagClass(senv.secondaryDetailMap.tagId, BITM);
        zteam_changeTagClass(senv.reflectionCubeMap.tagId, BITM);
    }
    else if(shdr.type == SHADER_SGLA) {
        ShaderSglaDependencies sgla = *(ShaderSglaDependencies *)location;
        zteam_changeTagClass(sgla.bgTint.tagId, BITM);
        zteam_changeTagClass(sgla.bumpMap.tagId, BITM);
        zteam_changeTagClass(sgla.diffuseDetailMap.tagId, BITM);
        zteam_changeTagClass(sgla.diffuseMap.tagId, BITM);
        zteam_changeTagClass(sgla.reflectionMap.tagId, BITM);
        zteam_changeTagClass(sgla.specularDetailMap.tagId, BITM);
        zteam_changeTagClass(sgla.specularMap.tagId, BITM);
    }
    else if(shdr.type == SHADER_SMET) {
        ShaderSmetDependencies smet = *(ShaderSmetDependencies *)location;
        zteam_changeTagClass(smet.map.tagId, BITM);
    }
    else if(shdr.type == SHADER_SOSO) {
        ShaderSosoDependencies soso = *(ShaderSosoDependencies *)location;
        zteam_changeTagClass(soso.baseMap.tagId, BITM);
        zteam_changeTagClass(soso.detailMap.tagId, BITM);
        zteam_changeTagClass(soso.multiMap.tagId, BITM);
        zteam_changeTagClass(soso.reflectMap.tagId, BITM);
    }
    else if(shdr.type == SHADER_SOTR) {
        ShaderSotrDependencies sotr = *(ShaderSotrDependencies *)location;
        ShaderShaderLayersDependencies *layers = translatePointer(sotr.layers.offset);
        for(uint32_t i=0; i<sotr.layers.count; i++) {
            zteam_deprotectShdr(layers[i].shader.tagId);
        }
        ShaderSotrMapDependencies *maps = translatePointer(sotr.maps.offset);
        for(uint32_t i=0; i<sotr.maps.count; i++) {
            zteam_changeTagClass(maps[i].map.tagId, BITM);
        }
        zteam_changeTagClass(sotr.lensflare.tagId, LENS);
    }
    else if(shdr.type == SHADER_SPLA) {
        ShaderSplaDependencies spla = *(ShaderSplaDependencies *)location;
        zteam_changeTagClass(spla.primaryNoiseMap.tagId, BITM);
        zteam_changeTagClass(spla.secondaryNoiseMap.tagId, BITM);
    }
    else if(shdr.type == SHADER_SWAT) {
        ShaderSwatDependencies swat = *(ShaderSwatDependencies *)location;
        zteam_changeTagClass(swat.baseMap.tagId, BITM);
        zteam_changeTagClass(swat.reflectionMap.tagId, BITM);
        zteam_changeTagClass(swat.rippleMap.tagId, BITM);
    }
}

static void zteam_deprotectMod2(TagID tagId) {
    if(isNulledOut(tagId)) return;
    zteam_changeTagClass(tagId,MOD2);
    Mod2Dependencies model = *( Mod2Dependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
    Mod2ShaderDependencies *shaders = translatePointer(model.mod2Shaders.offset);
    for(uint32_t i=0;i<model.mod2Shaders.count;i++) {
        zteam_deprotectShdr(shaders[i].shader.tagId);
    }
}

static void zteam_deprotectEffe(TagID tagId) {
    if(isNulledOut(tagId)) return;
    if(deprotectedTags[tagId.tagTableIndex]) return;
    zteam_changeTagClass(tagId, EFFE);
    deprotectedTags[tagId.tagTableIndex] = true;
}

static void zteam_deprotectObjectTag(TagID tagId) {
    if(isNulledOut(tagId)) return;
    
    if(deprotectedTags[tagId.tagTableIndex]) return;
    
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
    
    ObjeDependencies object = *( ObjeDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
    
    if(object.tagObjectType > sizeof(tagClasses) / 0x4)
    {
        return; //failed to ID tag
    }
    
    zteam_changeTagClass(tagId,(const char *)&tagClasses[object.tagObjectType]);
    
    deprotectedTags[tagId.tagTableIndex] = true;
    
    zteam_deprotectMod2(object.model.tagId);
    zteam_changeTagClass(object.animation.tagId,ANTR);
    zteam_changeTagClass(object.collision.tagId,COLL);
    zteam_changeTagClass(object.physics.tagId,PHYS);
    zteam_deprotectShdr(object.shader.tagId);
    
    ObjeResources *resources = (ObjeResources *)translatePointer(object.resources.offset);
    for(uint32_t i=0;i<object.resources.count;i++) {
        if(resources[i].type == OBJE_TYPE_BITMAP) {
            zteam_changeTagClass(resources[i].name, BITM);
        }
        else if(resources[i].type == OBJE_TYPE_SOUND) {
            zteam_changeTagClass(resources[i].name, SND);
        }
    }
    
    if(object.tagObjectType == OBJECT_WEAP) {
        WeapDependencies weap = *( WeapDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
        zteam_deprotectMod2(weap.fpModel.tagId);
        zteam_changeTagClass(weap.fpAnimation.tagId, ANTR);
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
        zteam_changeTagClass(unit.integratedLight.tagId,LIGH);
        zteam_changeTagClass(unit.meleeDamage.tagId, JPT);
        zteam_changeTagClass(unit.spawnedActor.tagId, ACTV);
    }
    
    if(object.tagObjectType == OBJECT_PROJ) {
        ProjDependencies proj = *(ProjDependencies *)translatePointer(tagArray[tagId.tagTableIndex].dataOffset);
        zteam_deprotectEffe(proj.superDetonation.tagId);
        zteam_deprotectEffe(proj.detonationEffect.tagId);
        zteam_changeTagClass(proj.attachedDamage.tagId, JPT);
        zteam_changeTagClass(proj.impactDamage.tagId, JPT);
        ProjMaterialResponseDependencies *respond = (ProjMaterialResponseDependencies *)translatePointer(proj.materialRespond.offset);
        for(uint32_t i=0;i<proj.materialRespond.count;i++) {
            zteam_deprotectEffe(respond[i].defaultResult.tagId);
            zteam_deprotectEffe(respond[i].detonationEffect.tagId);
            zteam_deprotectEffe(respond[i].potentialResult.tagId);
        }
    }
}

static void zteam_deprotectObjectPalette(TagReflexive reflexive) {
    ScnrPaletteDependency *palette = (ScnrPaletteDependency *)translatePointer(reflexive.offset);
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
    zteam_changeTagClass(tag,ITMC);
    ItmcPermutationDependencies *itmcPerm = ( ItmcPermutationDependencies *)translatePointer(itmc.permutation.offset);
    for(uint32_t i=0;i<itmc.permutation.count;i++) {
        zteam_deprotectObjectTag(itmcPerm[i].dependency.tagId);
    }
}

static void *translateCustomPointer(uint32_t pointer, uint32_t customMagic, uint32_t offset) {
    return mapdata + offset + (pointer - customMagic);
}

static void zteam_deprotectSBSP(TagID tagId,uint32_t fileOffset, uint32_t bspMagic) {
    if(isNulledOut(tagId))
        return;
    if(deprotectedTags[tagId.tagTableIndex]) return;
    zteam_changeTagClass(tagId, SBSP);
    deprotectedTags[tagId.tagTableIndex] = true;
    SBSPDependencies sbsp = *( SBSPDependencies *)(mapdata + fileOffset);
    SBSPCollisionMaterialsDependencies *materials = ( SBSPCollisionMaterialsDependencies *)translateCustomPointer(sbsp.collMaterials.offset,bspMagic,fileOffset);
    for(uint32_t i=0;i<sbsp.collMaterials.count;i++) {
        zteam_deprotectShdr(materials[i].shader.tagId);
    }
    SBSPLightmapsDependencies *lightmaps = ( SBSPLightmapsDependencies *)translateCustomPointer(sbsp.lightmaps.offset,bspMagic,fileOffset);
    for(uint32_t i=0;i<sbsp.lightmaps.count;i++) {
        SBSPLightmapsMaterialsReflexives *materials = ( SBSPLightmapsMaterialsReflexives *)translateCustomPointer(lightmaps[i].materials.offset, bspMagic, fileOffset);
        for(uint32_t q=0;q<lightmaps[i].materials.count;q++) {
            zteam_deprotectShdr(materials[q].shader.tagId);
        }
    }
}

static bool classCanBeDeprotected(uint32_t class) {
    
    uint32_t tagClasses[] = {
        *(uint32_t *)&DEVC,
        *(uint32_t *)&HUDG,
        *(uint32_t *)&MATG,
        *(uint32_t *)&DELA,
        *(uint32_t *)&SND,
        *(uint32_t *)&SOUL,
        *(uint32_t *)&TAGC,
        *(uint32_t *)&USTR
    };
    for(int i=0;i<sizeof(tagClasses)/4;i++) {
        if(class == tagClasses[i]) return false;
    }
    return true;
}

static bool classAutogeneric(uint32_t class) {
    uint32_t tagClasses[] = {
        *(uint32_t *)&BITM,
        *(uint32_t *)&SBSP,
        *(uint32_t *)&SCNR,
        *(uint32_t *)&ITMC,
        *(uint32_t *)&FONT
    };
    for(int i=0;i<sizeof(tagClasses)/4;i++) {
        if(class == tagClasses[i]) return true;
    }
    return false;
}

#define MATCHING_THRESHOLD 0.7
#define MAX_TAG_NAME_SIZE 0x20

MapData name_deprotect(MapData map, MapData *maps, int map_count) {
    uint32_t length = map.length;
    
    HaloMapHeader *headerOldMap = ( HaloMapHeader *)(map.buffer);
    HaloMapIndex *indexOldMap = (HaloMapIndex *)(map.buffer + headerOldMap->indexOffset);
    tagCount = indexOldMap->tagCount;
    
    char *modded_buffer = calloc(map.length + MAX_TAG_NAME_SIZE * tagCount,0x1);
    
    memcpy(modded_buffer,map.buffer,length);
    
    HaloMapHeader *header = ( HaloMapHeader *)(modded_buffer);
    HaloMapIndex *index = ( HaloMapIndex *)(modded_buffer + header->indexOffset);
    
    mapdata = modded_buffer;
    magic = META_MEMORY_OFFSET - header->indexOffset;
    
    char *names = modded_buffer + length;
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
    
    MapData new_map;
    new_map.buffer = modded_buffer;
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
    
    magic = META_MEMORY_OFFSET - header->indexOffset;
    
    tagArray = ( MapTag *)(translatePointer(index->tagIndexOffset));
    tagCount = index->tagCount;
    
    deprotectedTags = calloc(sizeof(bool) * tagCount,0x1);
    
    mapdataSize = length;
    tagdataSize = length - header->indexOffset;
    
    TagID matgTag;
    matgTag.tableIndex = 0xFFFF;
    matgTag.tagTableIndex = 0xFFFF;
    
    for(int i=0;i<tagCount;i++) {
        char *name = translatePointer(tagArray[i].nameOffset);
        uint32_t class = tagArray[i].classA;
        if(class == *(uint32_t *)&MATG && strcmp(name,"globals\\globals") == 0) {
            matgTag = tagArray[i].identity;
            break;
        }
    }
    if(!isNulledOut(matgTag)) {
        deprotectedTags[matgTag.tagTableIndex] = true;
    }
    
    MapTag scenarioTag = tagArray[index->scenarioTag.tagTableIndex];
    zteam_changeTagClass(index->scenarioTag, SCNR);
    
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
        zteam_changeTagClass(skies[i].sky.tagId, SKY);
    }
    
    ScnrBSPs *bsps = ( ScnrBSPs *)translatePointer(scnrData.BSPs.offset);
    for(uint32_t i=0;i<scnrData.BSPs.count;i++) {
        zteam_deprotectSBSP(bsps[i].bsp.tagId, bsps[i].fileOffset, bsps[i].bspMagic);
    }
    
    ScnrNetgameItmcDependencies *itmcs = ( ScnrNetgameItmcDependencies *)translatePointer(scnrData.netgameItmcs.offset);
    for(uint32_t i=0;i<scnrData.netgameItmcs.count;i++) {
        zteam_deprotectItmc(itmcs[i].itemCollection.tagId);
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