#include "pkg_list.h"

int pkgList_Find(char *call)
{
    int result = -1;
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    for (int i = 0; i < PKGLISTSIZE; i++)
    {
        if (strstr(pkgList[(int)i].calsign, call) != NULL)
        {
            result = i;
            break;
        }
    }
    xSemaphoreGiveRecursive(pkgListMutex);
    return result;
}

int pkgList_Find(char *call, uint16_t type)
{
    int result = -1;
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    for (int i = 0; i < PKGLISTSIZE; i++)
    {
        if (pkgList[i].type == type)
        {
            if (strstr(pkgList[i].calsign, call) != NULL)
            {
                result = i;
                break;
            }
        }
    }
    xSemaphoreGiveRecursive(pkgListMutex);
    return result;
}

int pkgList_Find(char *call, char *object, uint16_t type)
{
    int result = -1;
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    for (int i = 0; i < PKGLISTSIZE; i++)
    {
        if (pkgList[i].type == type)
        {
            if (strnstr(pkgList[i].calsign, call, strlen(call)) != NULL)
            {
                if (strnstr(pkgList[i].object, object, strlen(object)) != NULL)
                {
                    result = i;
                    break;
                }
            }
        }
    }
    xSemaphoreGiveRecursive(pkgListMutex);
    return result;
}

int pkgListOld()
{
    int i, ret = -1;
    time_t minimum = time(NULL) + 86400; // pkgList[0].time;
    for (i = 0; i < PKGLISTSIZE; i++)
    {
        if (pkgList[(int)i].time < minimum)
        {
            minimum = pkgList[(int)i].time;
            ret = i;
        }
    }
    return ret;
}

void sort(pkgListType a[], int size)
{
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    for (int i = 0; i < (size - 1); i++)
    {
        for (int o = 0; o < (size - (i + 1)); o++)
        {
            if (a[o].time < a[o + 1].time)
            {
                pkgListType t = a[o];
                a[o] = a[o + 1];
                a[o + 1] = t;
            }
        }
    }
    xSemaphoreGiveRecursive(pkgListMutex);
}

void sortPkgDesc(pkgListType a[], int size)
{
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    for (int i = 0; i < (size - 1); i++)
    {
        for (int o = 0; o < (size - (i + 1)); o++)
        {
            if (a[o].pkg < a[o + 1].pkg)
            {
                pkgListType t = a[o];
                a[o] = a[o + 1];
                a[o + 1] = t;
            }
        }
    }
    xSemaphoreGiveRecursive(pkgListMutex);
}

uint16_t pkgType(const char *raw)
{
    uint16_t type = 0;
    char packettype = 0;
    const char *body;
    // int paclen = strlen(raw);
    char *ptr;

    if (*raw == 0)
        return 0;

    packettype = (char)raw[0];
    body = &raw[1];

    switch (packettype)
    {
    case '$': // NMEA
        type |= FILTER_POSITION;
        break;
    case 0x27: /* ' */
    case 0x60: /* ` */
        type |= FILTER_POSITION;
        type |= FILTER_MICE;
        break;
    case '!':
    case '=':
        type |= FILTER_POSITION;
        if (body[18] == '_' || body[10] == '_')
        {
            type |= FILTER_WX;
            break;
        }
    case '/':
    case '@':
        type |= FILTER_POSITION;
        if (body[25] == '_' || body[16] == '_')
        {
            type |= FILTER_WX;
            break;
        }
        if (strchr(body, 'r') != NULL)
        {
            if (strchr(body, 'g') != NULL)
            {
                if (strchr(body, 't') != NULL)
                {
                    if (strchr(body, 'P') != NULL)
                    {
                        type |= FILTER_WX;
                    }
                }
            }
        }
        break;
    case ':':
        if (body[9] == ':' &&
            (memcmp(body + 10, "PARM", 4) == 0 ||
             memcmp(body + 10, "UNIT", 4) == 0 ||
             memcmp(body + 10, "EQNS", 4) == 0 ||
             memcmp(body + 10, "BITS", 4) == 0))
        {
            type |= FILTER_TELEMETRY;
        }
        else
        {
            type |= FILTER_MESSAGE;
        }
        break;
    case '{': // User defind
    case '<': // statcapa
    case '>':
        type |= FILTER_STATUS;
        break;
    case '?':
        type |= FILTER_QUERY;
        break;
    case ';':
        type |= FILTER_OBJECT;
        if (body[35] == '_')
            type |= FILTER_WX;
        break;
    case ')':
        type |= FILTER_ITEM;
        break;
    case '}':
        type |= FILTER_THIRDPARTY;
        ptr = strchr(raw, ':');
        if (ptr != NULL)
        {
            ptr++;
            type |= pkgType(ptr);
        }
        break;
    case 'T':
        type |= FILTER_TELEMETRY;
        break;
    case '#': /* Peet Bros U-II Weather Station */
    case '*': /* Peet Bros U-I  Weather Station */
    case '_': /* Weather report without position */
        type |= FILTER_WX;
        break;
    default:
        type = 0;
        break;
    }
    return type;
}

pkgListType getPkgList(int idx)
{
    pkgListType ret;
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    memset(&ret, 0, sizeof(pkgListType));
    if (idx < PKGLISTSIZE)
        memcpy(&ret, &pkgList[idx], sizeof(pkgListType));
    xSemaphoreGiveRecursive(pkgListMutex);
    return ret;
}

int pkgListUpdate(char *call, char *raw, uint16_t type, bool channel, uint16_t audioLvl)
{
    xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
    size_t len;
    if (*call == 0)
    {
        xSemaphoreGiveRecursive(pkgListMutex);
        return -1;
    }
    if (*raw == 0)
    {
        xSemaphoreGiveRecursive(pkgListMutex);
        return -1;
    }

    // int start_info = strchr(':',0);

    char callsign[11];
    char object[10];
    size_t sz = strlen(call);
    memset(callsign, 0, 11);
    if (sz > 10)
        sz = 10;
    // strncpy(callsign, call, sz);
    memcpy(callsign, call, sz);

    int i = -1;

    memset(object, 0, sizeof(object));
    if (type & FILTER_ITEM)
    {
        int x = 0;
        char *body = strchr(raw, ':');
        if (body != NULL)
        {
            body += 2;

            for (int z = 0; z < 9 && body[z] != '!' && body[z] != '_'; z++)
            {
                if (body[z] < 0x20 || body[z] > 0x7e)
                {
                    log_d("\titem name has unprintable characters");
                    break; /* non-printable */
                }
                object[x++] = body[z];
            }
        }
        i = pkgList_Find(callsign, object, type);
    }
    else if (type & FILTER_OBJECT)
    {
        int x = 0;
        char *body = strchr(raw, ':');
        // log_d("body=%s",body);
        if (body != NULL)
        {
            body += 2;
            for (int z = 0; z < 9; z++)
            {
                if (body[z] < 0x30 || body[z] > 0x7A)
                {
                    log_d("\tobject name has unprintable characters");
                    break; // non-printable
                }
                object[x++] = body[z];
                // if (raw[i] != ' ')
                //     namelen = i;
            }
        }
        i = pkgList_Find(callsign, object, type);
    }
    else
    {
        i = pkgList_Find(callsign, type);
    }

    if (i > PKGLISTSIZE)
    {
        xSemaphoreGiveRecursive(pkgListMutex);
        return -1;
    }
    if (i > -1)
    { // Found call in old pkg
        if ((channel == 1) || (channel == pkgList[i].channel))
        {
            pkgList[i].time = time(NULL);
            pkgList[i].pkg++;
            pkgList[i].type = type;
            // memcpy(pkgList[i].object,object,sizeof(object));
            if (channel == 0)
            {
                pkgList[i].audio_level = (int16_t)audioLvl;
                //     pkgList[i].rssi = rssi;
                //     pkgList[i].snr = snr;
                //     pkgList[i].freqErr = freqErr;
            }
            else
            {
                pkgList[i].rssi = -140;
                pkgList[i].snr = 0;
                pkgList[i].freqErr = 0;
                pkgList[i].audio_level = 0;
            }
            len = strlen(raw);
            {
                size_t newLen = len + 1;
                char *newRaw;
                if (pkgList[i].raw != NULL)
                    newRaw = (char *)realloc(pkgList[i].raw, newLen);
                else
                    newRaw = (char *)calloc(newLen, sizeof(char));
                if (newRaw)
                {
                    pkgList[i].raw = newRaw;
                    pkgList[i].length = newLen;
                    memset(pkgList[i].raw, 0, pkgList[i].length);
                    memcpy(pkgList[i].raw, raw, len);
                    pkgList[i].raw[len] = 0;
                    log_d("Update: pkgList_idx=%d callsign:%s object:%s", i, callsign, object);
                }
            }
        }
    }
    else
    {
        i = pkgListOld(); // Search free in array
        if (i > PKGLISTSIZE || i < 0)
        {
            xSemaphoreGiveRecursive(pkgListMutex);
            return -1;
        }
        // memset(&pkgList[i], 0, sizeof(pkgListType));
        pkgList[i].channel = channel;
        pkgList[i].time = time(NULL);
        pkgList[i].pkg = 1;
        pkgList[i].type = type;
        if (strlen(object) > 3)
        {
            memcpy(pkgList[i].object, object, 9);
            pkgList[i].object[9] = 0;
        }
        else
        {
            memset(pkgList[i].object, 0, sizeof(pkgList[i].object));
        }
        if (channel == 0)
        {
            pkgList[i].audio_level = (int16_t)audioLvl;
            //     pkgList[i].rssi = rssi;
            //     pkgList[i].snr = snr;
            //     pkgList[i].freqErr = freqErr;
        }
        else
        {
            pkgList[i].rssi = -140;
            pkgList[i].snr = 0;
            pkgList[i].freqErr = 0;
            pkgList[i].audio_level = 0;
        }
        // strcpy(pkgList[i].calsign, callsign);
        memcpy(pkgList[i].calsign, callsign, strlen(callsign));
        len = strlen(raw);
        {
            size_t newLen = len + 1;
            char *newRaw;
            if (pkgList[i].raw != NULL)
                newRaw = (char *)realloc(pkgList[i].raw, newLen);
            else
                newRaw = (char *)calloc(newLen, sizeof(char));
            if (newRaw)
            {
                pkgList[i].raw = newRaw;
                pkgList[i].length = newLen;
                memset(pkgList[i].raw, 0, pkgList[i].length);
                memcpy(pkgList[i].raw, raw, len);
                pkgList[i].raw[len] = 0;
                log_d("New: pkgList_idx=%d callsign:%s object:%s", i, callsign, object);
            }
        }
    }
    lastHeard_Flag = true;
    lastHeardTimeout = millis() + 1000;
    xSemaphoreGiveRecursive(pkgListMutex);
    return i;
}
