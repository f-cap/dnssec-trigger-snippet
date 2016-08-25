/*
 * File:   json_helper.h
 * Author: fcap
 *
 * Created on 18. července 2016, 12:00
 */

#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../vendor/ccan/json/json.h"

typedef struct connection Connection;
typedef struct charChain CharChain;
typedef enum type Type;
typedef struct connectionChain ConnectionChain;
typedef struct assocChain AssocChain;
typedef struct zoneConfig ZoneConfig;
typedef struct localZoneConfig LocalZoneConfig;

extern CharChain *stored_zones;
ConnectionChain *parseConnections(char *input);
void handleErr();
ConnectionChain *newConnectionChain();
Connection *newConnection();
CharChain *newCharChain();
void outOfMemory();
CharChain *charChain_append(CharChain *chain,char *value);
ConnectionChain *connectionChain_append(ConnectionChain *chain,Connection *value);
void printType(JsonNode *node);
void freeCharChain(CharChain *cn);
void freeConnectionChain(ConnectionChain *cn,bool preserveValues);
void freeConnection(Connection *con);
void structPrint(Connection *con);
void charChain_append_double(CharChain **head,char *value);
void freeConnectionChainCell(ConnectionChain *cn);
ConnectionChain *onlyVPN(ConnectionChain *cn);
ConnectionChain *onlyDefault(ConnectionChain *cn);
bool isEmpty(ConnectionChain *cn);
void my_probe_start(CharChain *cn);
bool valueInCharChain(CharChain *cn,char *value);
int charChainLength(CharChain *cn);
bool charChainsEqual(CharChain *cn1,CharChain *cn2);
CharChain *copy_CharChain(CharChain *cn);
Connection *copy_Connection(Connection *cn);
ConnectionChain *copy_ConnectionChain(ConnectionChain *cn);
int main(int argc,char **argv);
int update_global_forwarders(ConnectionChain *inputConnections);
int update_connection_zones(ConnectionChain *inputConnections);
extern CharChain *global_forwarders;
ConnectionChain *noWifi(ConnectionChain *cn);
Connection *getPreferredConnection(Connection *first,Connection *second);
AssocChain *newAssocChain();
void freeAssocChain(AssocChain *cn,bool preserveValues);
AssocChain *assocChain_append(AssocChain *cn,char *zone,Connection *con);
AssocChain *getAssocChainWithZone(AssocChain *cn,char *zone);
AssocChain *getZoneConnectionMapping(ConnectionChain *connections);
ZoneConfig *newZoneConfig();
void freeZoneConfig(ZoneConfig *zn,bool preserveValues);
ZoneConfig *zoneConfig_append(ZoneConfig *what,ZoneConfig *where);
ZoneConfig *getUnboundZoneConfig();
LocalZoneConfig *newLocalZoneConfig();
void freeLocalZoneConfig(LocalZoneConfig *cfg,bool preserveValues);
LocalZoneConfig *localZoneConfig_append(LocalZoneConfig *what,LocalZoneConfig *where);
LocalZoneConfig *getUnboundLocalZoneConfig();
extern char **rfc1918_reverse_zones;
bool isEmptyCharChain(CharChain *cc);
void unbound_local_zones_add(char *zone,char *type);
void unbound_zones_remove(char *zone,char *flush_command);
void freeCharChainCell(CharChain *cc);
void stored_zones_remove_double(CharChain **chain,char *zone);
void stored_zones_remove(char *zone);
char *servers_to_string(CharChain *servers);
void unbound_zones_add(char *zone,CharChain *servers,bool validate);
void unbound_local_zones_remove(char *zone);

enum type {
    VPN,
    WIFI,
    OTHER,
    IGNORE,
	DELIMITER
};

struct assocChain {
	char *zone;
	Connection *connection;
	AssocChain *prev;
	AssocChain *next;
};

struct connection {
    bool default_con;
    CharChain *zones;
    Type type;
    CharChain *servers;
};

struct charChain {
    CharChain *prev;
    char *current;
    CharChain *next;
};

struct connectionChain {
    ConnectionChain *prev;
    Connection *current;
    ConnectionChain *next;
};

struct zoneConfig {
	ZoneConfig *prev;
	ZoneConfig *next;

	char *name;
	CharChain *ips;
	bool secure;
};

struct localZoneConfig {
	LocalZoneConfig *prev;
	LocalZoneConfig *next;

	char *name;
	char *type; // in future the type could be ENUM
};



#endif /* JSON_HELPER_H */
