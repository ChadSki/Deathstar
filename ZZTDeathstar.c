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