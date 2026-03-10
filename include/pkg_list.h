#pragma once
#include "main.h"

// Globals used by the packet-list module (defined in main.cpp)
extern pkgListType *pkgList;
extern bool psramBusy;
extern bool lastHeard_Flag;
extern unsigned long lastHeardTimeout;
extern SemaphoreHandle_t pkgListMutex;

// Packet type classifier
uint16_t pkgType(const char *raw);

// Find functions (linear search)
int pkgList_Find(char *call);
int pkgList_Find(char *call, uint16_t type);
int pkgList_Find(char *call, char *object, uint16_t type);

// Oldest-entry finder
int pkgListOld();

// Sort helpers
void sort(pkgListType a[], int size);
void sortPkgDesc(pkgListType a[], int size);

// Accessor
pkgListType getPkgList(int idx);

// Insert/update entry
int pkgListUpdate(char *call, char *raw, uint16_t type, bool channel, uint16_t audioLvl);
