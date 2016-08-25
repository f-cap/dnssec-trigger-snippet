/*
 * json_helper.c
 *
 *  Created on: 14. 7. 2016
 *      Author: fcap
 */

#include "json_helper.h"
#include <string.h>

CharChain* stored_zones = NULL;

/**
 * Parses input json char* to ConnectionChain
 * @Example //TODO:
 *
 * @param input
 * @return ConnectionChain*
 */
ConnectionChain* parseConnections(char* input) {
    printf(input);
    printf("\n");

    if (!input || !json_validate(input)) {
        handleErr();

    }
    JsonNode *head = json_decode(input);
    //shouldn't be null because we checked if it is valid
    if (head->tag != JSON_OBJECT) {
        handleErr();

    }

    JsonNode *node = head->children.head; // now it should be the first dictionary value e.g. connections

    if (!node || strcmp(node->key, "connections") != 0) { // and also must be array
        handleErr();

    }
    // node is array called connections

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Connection *con;
    ConnectionChain *cons = newConnectionChain();
    JsonNode *connection = node->children.head;

    while (NULL != connection) {
        con = newConnection();
        JsonNode *param = connection->children.head;
        //zpracovani paramu

        while (NULL != param) {
            if (param->tag == JSON_BOOL && strcmp(param->key, "default") == 0) {
                con->default_con = param->bool_;  // hope it copies the value
            } else if (param->tag == JSON_STRING && strcmp(param->key, "type") == 0) {
                if (strcmp(param->string_, "wifi") == 0) {
                    con->type = WIFI;
                } else if (strcmp(param->string_, "vpn") == 0) {
                    con->type = VPN;
                } else if (strcmp(param->string_, "other") == 0) {
                    con->type = OTHER;
                } else {
                    con->type = IGNORE;
                }
            } else if (param->tag == JSON_ARRAY && strcmp(param->key, "zones") == 0) {
                JsonNode *zone = param->children.head;
                CharChain *zn = newCharChain();

                while (NULL != zone) {
                	char *znc = calloc(strlen(zone->string_)+1, sizeof(char));
                	if(NULL == znc)
                		outOfMemory();
                	strcpy(znc, zone->string_);
                    charChain_append(zn, znc);

                    zone = zone->next;
                }
                con->zones = zn;
            } else if (param->tag == JSON_ARRAY && strcmp(param->key, "servers") == 0) {
                JsonNode *server = param->children.head;
                CharChain *sv = newCharChain();

                while (NULL != server) {
                	char *svc = calloc(strlen(server->string_)+1, sizeof(char));
                	if(NULL == svc)
                		outOfMemory();
                	strcpy(svc, server->string_);
                    charChain_append(sv, svc);

                    server = server->next;
                }
                con->servers = sv;
            }

            param = param->next;

        }
        connection = connection->next;
        connectionChain_append(cons, con);
    }

    json_delete(head);
    return cons;
}

void printType(JsonNode *node) {
    if (node->tag == JSON_ARRAY)
        printf("array\n");
    else if (node->tag == JSON_STRING)
        printf("string\n");
    else if (node->tag == JSON_OBJECT)
        printf("object\n");
}

void handleErr() {
    printf("bad json input");
    exit(-1);
}

CharChain* newCharChain() {
    CharChain *cn = calloc(1, sizeof (CharChain));
    if (NULL == cn) {
        outOfMemory();
    }
    return cn;
}

void freeCharChain(CharChain *cn) {
	if(NULL != cn->next) {
		freeCharChain(cn->next);
		//free(cn->next);
	}
	free(cn->current);

	//if(NULL == cn->prev)
		free(cn);
}

ConnectionChain* newConnectionChain() {
    ConnectionChain *cn = calloc(1, sizeof (ConnectionChain));
    if (NULL == cn) {
        outOfMemory();
    }
    return cn;
}

void freeConnectionChain(ConnectionChain *cn, bool preserveValues) { // we're freeing in just one direction
	if(NULL != cn->next) {
		freeConnectionChain(cn->next, preserveValues);
		free(cn->next);
	}

	if(!preserveValues && NULL != cn->current)
		freeConnection(cn->current);

	if(NULL == cn->prev)
			free(cn);
}

Connection* newConnection() {
    Connection *con = calloc(1, sizeof (Connection));
    if (NULL == con) {
        outOfMemory();
    }
    return con;
}

void freeConnection(Connection *con) {
	if(con != NULL) {
		// default_con & type doesn't have to be freed/ mustn't be freed they weren't allocated by malloc
		freeCharChain(con->servers);
		freeCharChain(con->zones);
		free(con);
	}
}

void structPrint(Connection *con) {
	if(con == NULL)
		return;

    if (con->default_con) {
        printf("default: true\n");
    } else {
        printf("default: false\n");
    }
    printf("type: ");
    switch (con->type) {
        case WIFI:
            printf("wifi\n");
            break;

        case VPN:
            printf("vpn\n");
            break;

        default:
        case OTHER:
            printf("other\n");
            break;

    }

    printf("servers: \n");
	for (CharChain *i = con->servers; i != NULL; i = i->next) {
		printf(i->current);
		printf("\n");
	}

    printf("\n");
    printf("zones: \n");
    for (CharChain *i = con->zones; i != NULL; i = i->next) {
    		printf(i->current);
    		printf("\n");
    	}
    printf("\n");
}

void outOfMemory() {
    printf("OUT OF MEMORY!");
    exit(-1);
}

void charChain_append_double(CharChain **head, char *value) {
	if(*head == NULL) {
		CharChain *chain = newCharChain();
		chain->current = value;
		*head = chain;
		return;
	}

    CharChain *tmp = *head;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    CharChain *vl = newCharChain();
    vl->current = value;
    vl->prev = tmp;
    tmp->next = vl;

}

CharChain* charChain_append(CharChain *chain, char *value) {
	if(chain == NULL) {
		chain = newCharChain();
		chain->current = value;
		return chain;
	}

    CharChain *tmp = chain;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return chain;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    CharChain *vl = newCharChain();
    vl->current = value;
    vl->prev = tmp;
    tmp->next = vl;

    return chain;

}

ConnectionChain* connectionChain_append(ConnectionChain *chain, Connection *value) {
    ConnectionChain *tmp = chain;
    if (NULL == tmp->current) {
        tmp->current = value;
        tmp->prev = NULL;
        tmp->next = NULL;
        return chain;
    }

    while (NULL != tmp->next) {
        tmp = tmp->next;
    }

    ConnectionChain *vl = newConnectionChain();
    vl->current = value;
    vl->prev = tmp;
    vl->next = NULL;
    tmp->next = vl;

    return chain;
}

//char** charChain_to_array(CharChain *chain) {
//    CharChain *tmp = chain;
//    int length = 1; // because we need one for null pointer as an array delimiter
//
//    if (NULL != chain->current) {
//        length++;
//    }
//
//    while (tmp->next) {
//        tmp = tmp->next;
//        length++;
//    }
//
//    char **arry = calloc(length, sizeof (char*));
//    if (NULL == arry) {
//        outOfMemory();
//    }
//
//    tmp = chain;
//    for (int i = 0; i < length - 2; i++) {
//        arry[i] = tmp->current;
//        tmp = tmp->next;
//    }
//
//    return arry;
//}
//
//Connection* connectionChain_to_array(ConnectionChain *chain) {
//    ConnectionChain *tmp = chain;
//    int length = 1; // because we need one for null pointer as an array delimiter
//
//    if (NULL != chain->current) {
//        length++;
//    }
//
//    while (tmp->next) {
//        tmp = tmp->next;
//        length++;
//    }
//
//    Connection *arry = calloc(length, sizeof (Connection*));
//    if (NULL == arry) {
//        outOfMemory();
//    }
//
//    tmp = chain;
//    for (int i = 0; i < length - 2; i++) {
//        arry[i] = *tmp->current;
//        tmp = tmp->next;
//    }
////    Connection *nl = calloc(1, sizeof(Connection));
////    nl->type = DELIMITER;
////    arry[length] = *nl;
//    return arry;
//}

//ConnectionChain* onlyVPN(ConnectionChain *cn) {
//	ConnectionChain *toRet = newConnectionChain();
//
//	for(ConnectionChain *i = cn; i != NULL; i = i->next) {
//		if(i->current->type == VPN) {
//			connectionChain_append(toRet, i->current);
//		}
//	}
//	return toRet;
//}

void freeConnectionChainCell(ConnectionChain *cn) {
	freeConnection(cn->current);
	free(cn);
}

ConnectionChain* onlyVPN(ConnectionChain *cn) {
	ConnectionChain *top = cn;
	ConnectionChain *i = cn;
	while(i != NULL) {
		if(i->current->type != VPN) {
			if(i->prev != NULL)
				i->prev->next = i->next;
			if(i->next != NULL)
				i->next->prev = i->prev;
			if(i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
				if(i->prev == NULL)
					top = i->next;
				ConnectionChain *tmp = i->next;
				freeConnectionChainCell(i);
				i = tmp;
				continue;
			}
			else {
				if(i->current != NULL) {
					freeConnection(i->current);
					i->current = NULL;
				}
			}
		}
		i = i->next;
	}

	return top;
}


ConnectionChain* onlyDefault(ConnectionChain *cn) {
	ConnectionChain *top = cn;
	ConnectionChain *i = cn;

	while(i != NULL) {
		if(!i->current->default_con) {
			if(i->prev != NULL)
				i->prev->next = i->next;
			if(i->next != NULL)
				i->next->prev = i->prev;
			if(i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
				if(i->prev == NULL)
					top = i->next;
				ConnectionChain *tmp = i->next;
				freeConnectionChainCell(i);
				i = tmp;
				continue;
			}
			else {
				if(i->current != NULL) {
					freeConnection(i->current);
					i->current = NULL;
				}
			}
		}
		i = i->next;
	}

	return top;
}

bool isEmpty(ConnectionChain *cn) { // only one direction checking
	if(NULL == cn || (NULL == cn->current && NULL == cn->next))
		return true;
	return false;
}

void my_probe_start(CharChain *cn) {

}

bool valueInCharChain(CharChain *cn, char* value) {
	if(NULL == value) {
		return false;
	}
	for(CharChain *i = cn; NULL != i; i = i->next) {
		if(NULL == i->current)
			return false;
		if(strcmp(i->current, value) == 0)
			return true;
	}

	return false;
}

int charChainLength(CharChain *cn) {
	int len = 0;

	CharChain *tmp = cn;
	while (NULL != tmp) {
		tmp = tmp->next;
		len++;
	}

	return len;
}

bool charChainsEqual(CharChain *cn1, CharChain *cn2) { // not the order of values, just "is it also there?"
	if(NULL == cn1 && NULL == cn2)
		return true;

	if((NULL == cn1 && NULL != cn2) || (NULL == cn2 && NULL != cn1))
		return false;

	if(charChainLength(cn1) != charChainLength(cn2)) { // we pretend every value is unique
		return false;
	}

	for(CharChain *i = cn1; NULL != i->next; i = i->next) {
		if(!valueInCharChain(cn2, i->current))
			return false;
	}

	return true;

}

CharChain* copy_CharChain(CharChain *cn) {
	if(cn == NULL)
		return NULL;

	CharChain *toRet = newCharChain();


	CharChain *nxt = NULL;
	if(cn->next != NULL) {
		nxt = copy_CharChain(cn->next);
	}
	toRet->next = nxt;

	if(cn->current != NULL) {
		char *chr = calloc(strlen(cn->current)+1, sizeof(char));
		if(chr == NULL) {
			outOfMemory();
		}

		strcpy(chr, cn->current);
		toRet->current = chr;
	}

	if(toRet->next != NULL) {
		toRet->next->prev = toRet;
	}
	return toRet;
}

Connection* copy_Connection(Connection* cn) {

	if(cn == NULL) {
		return NULL;
	}
	Connection *toRet = newConnection();


	toRet->default_con = cn->default_con;
	toRet->type = cn->type;
	toRet->servers = copy_CharChain(cn->servers);
	toRet->zones = copy_CharChain(cn->zones);

	return toRet;
}

ConnectionChain* copy_ConnectionChain(ConnectionChain *cn) {

	if(cn == NULL)
		return NULL;

	ConnectionChain *toRet = newConnectionChain();


	ConnectionChain *nxt = NULL;
	if(cn->next != NULL) {
		nxt = copy_ConnectionChain(cn->next);
	}
	toRet->next = nxt;

	if(cn->current != NULL) {
		toRet->current = copy_Connection(cn->current);
	}

	if(toRet->next != NULL) {
		toRet->next->prev = toRet;
	}
	return toRet;
}

int main(int argc, char **argv) {
	//ZoneConfig *zc = getUnboundZoneConfig();
	//LocalZoneConfig *lzc = getUnboundLocalZoneConfig();


	char *input = calloc(1024, sizeof(char));
	if(input == NULL)
		outOfMemory();

	for(int i = 0; i< 5; i++) {
		scanf("%s", input);
		ConnectionChain *connections = parseConnections(input);

		update_global_forwarders(connections);
		update_connection_zones(connections);

		freeConnectionChain(connections, false);
	}
	free(input);
	if(stored_zones != NULL)
		freeCharChain(stored_zones);

	if(global_forwarders != NULL)
		freeCharChain(global_forwarders);
}

CharChain *global_forwarders = NULL;

int update_global_forwarders(ConnectionChain *inputConnections)
{
	ConnectionChain *connections = copy_ConnectionChain(inputConnections);
	bool use_vpn_global_forwarders = true;

    if(use_vpn_global_forwarders) { // note only vpn must return NULL if there's none
    	connections = onlyVPN(connections);
    }
    if(isEmpty(connections)) {
    	if(connections != NULL)
    		freeConnectionChain(connections, false); // no preserve values because it should be empty
    	connections = copy_ConnectionChain(inputConnections);

    	connections = onlyDefault(connections);
    }

    if(isEmpty(connections)) {
    	if(connections != NULL)
    		freeConnectionChain(connections, false);
    	printf("Nic ke zpracovani\n");
    	return EXIT_SUCCESS;
    }

    CharChain *ips = newCharChain();
    for(ConnectionChain *i = connections; NULL != i; i = i->next) { // every ip is in Chain only once after this cycle
    	if(NULL == i->current)
    		continue;

    	for(CharChain *ip = i->current->servers; NULL != ip; ip = ip->next) {
    		if(NULL == ip->current)
    			continue;
    		if(valueInCharChain(ips, ip->current))
    			continue;

    		char *cpy = calloc(strlen(ip->current)+1, sizeof(char));
    		if(cpy == NULL)
    			outOfMemory();
    		strcpy(cpy, ip->current);
    		charChain_append(ips, cpy);
    	}
    }


    if(!charChainsEqual(ips, global_forwarders)) { // compare with previous forwarders
    	if(NULL != global_forwarders)
    		freeCharChain(global_forwarders);
    	global_forwarders = ips;

        my_probe_start(ips);

    } else {
    	freeCharChain(ips);
    	printf("They're same\n");
    }

	freeConnectionChain(connections, false);

	return EXIT_SUCCESS;
}

ConnectionChain* noWifi(ConnectionChain *cn) {
	ConnectionChain *top = cn;
		ConnectionChain *i = cn;
		while(i != NULL) {
			if(i->current->type == WIFI) {
				if(i->prev != NULL)
					i->prev->next = i->next;
				if(i->next != NULL)
					i->next->prev = i->prev;
				if(i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
					if(i->prev == NULL)
						top = i->next;
					ConnectionChain *tmp = i->next;
					freeConnectionChainCell(i);
					i = tmp;
					continue;
				}
				else {
					if(i->current != NULL) {
						freeConnection(i->current);
						i->current = NULL;
					}
				}
			}
			i = i->next;
		}

		return top;
}

Connection* getPreferredConnection(Connection *first, Connection *second) //todo: add IPv6 functionality of default_con
{
	if(second->type == VPN && first->type != VPN) {
		return second;
	}

	if(first->type == VPN && second->type != VPN) {
		return first;
	}

	if(second->default_con && !first->default_con) {
		return second;
	}

	if(first->default_con && !second->default_con) {
		return first;
	}

	return first;
}

AssocChain* newAssocChain() {
	AssocChain *cn = calloc(1, sizeof (AssocChain));

	if(cn == NULL) {
		outOfMemory();
	}

	return cn;
}

void freeAssocChain(AssocChain *cn, bool preserveValues) { // we're freeing in just one direction

	if(NULL != cn->next) {
		freeAssocChain(cn->next, preserveValues);
		free(cn->next);
	}

	if(!preserveValues && NULL != cn->zone)
		free(cn->zone);

	if(!preserveValues && NULL != cn->connection)
		freeConnection(cn->connection);

	if(NULL == cn->prev)
			free(cn);

}

AssocChain* assocChain_append(AssocChain *cn, char *zone, Connection *con)
{
	AssocChain *tmp = cn;
	if (NULL == tmp->zone && NULL == tmp->connection) {
		tmp->zone = zone;
		tmp->connection = con;
		tmp->prev = NULL;
		tmp->next = NULL;
		return cn;
	}

	while (NULL != tmp->next) {
		tmp = tmp->next;
	}

	AssocChain *vl = newAssocChain();
	vl->zone = zone;
	vl->connection = con;
	vl->prev = tmp;
	vl->next = NULL;
	tmp->next = vl;

	return cn;
}

AssocChain* getAssocChainWithZone(AssocChain *cn, char *zone)
{
	for(AssocChain *i = cn; i != NULL; i = i->next) {
		if(i->zone == NULL)
			continue;
		if(strcmp(i->zone, zone) == 0) {
			return i;
		}
	}
	return NULL;
}

AssocChain* getZoneConnectionMapping(ConnectionChain *connections)
{
	AssocChain *result = newAssocChain();
	for(ConnectionChain *i = connections; i != NULL; i = i->next) {
		//TODO:null handle
		for(CharChain *l = i->current->zones; l != NULL; l = l->next) {
			AssocChain *tmp = getAssocChainWithZone(result, l->current);
			if(tmp == NULL) {
				assocChain_append(result, l->current, i->current);
			} else {
				tmp->connection = getPreferredConnection(tmp->connection, i->current);
			}
		}
	}
	return result;
}

ZoneConfig* newZoneConfig()
{
	ZoneConfig *zc = calloc(1, sizeof(ZoneConfig));

	if(zc == NULL)
		outOfMemory();

	return zc;
}

void freeZoneConfig(ZoneConfig *zn, bool preserveValues) {
	if(NULL != zn->next) {
			freeZoneConfig(zn->next, preserveValues);
			free(zn->next);
		}

		if(!preserveValues && NULL != zn->ips)
			freeCharChain(zn->ips);

		if(!preserveValues && NULL != zn->name)
			free(zn->name);

//		if(!preserveValues && NULL != zn->secure)
//			free(zn->secure);

		if(NULL == zn->prev)
				free(zn);
}

ZoneConfig* zoneConfig_append(ZoneConfig *what, ZoneConfig *where) {
	ZoneConfig *tmp = where;
	if (NULL == tmp->name && NULL == tmp->ips) {
		tmp->name = what->name;
		tmp->ips = what->ips;
		tmp->secure = what->secure;
		tmp->prev = NULL;
		tmp->next = NULL;

		freeZoneConfig(what, true);
		return where;
	}

	while (NULL != tmp->next) {
		tmp = tmp->next;
	}

	what->prev = tmp;
	tmp->next = what;

	return where;
}

ZoneConfig* getUnboundZoneConfig()
{
	// what if too many requests like at probing for example?
	FILE *unbound = popen("unbound-control status", "r");
	pclose(unbound); //maybe the EXIT_SUCCESS condition would be great here to determine if unbound is running correctly

	unbound = popen("unbound-control list_forwards", "r");
	// subprocess.check_output(["unbound-control", "list_forwards"]).decode() -- decode is missing and is not implemented here

	char *buf = NULL;
	char word_buf[512] = {0};
	int word_buf_pos = 0;
	size_t len = 0;

	ZoneConfig *zoneConfig = newZoneConfig();
	ZoneConfig *tmp = newZoneConfig();
	CharChain *chtmp = newCharChain();

	int pos_in_fields = 1;

	while(getline(&buf, &len, unbound) != -1) { //deprecated comment: sometimes some chars are missing don't know why
		for(int i = 0; i < len; i++) {
			if((buf[i] == ' ' || buf[i] == '\n') && word_buf_pos > 0) {
				if(pos_in_fields == 1) {
					char *nm = calloc(word_buf_pos + 1, sizeof(char));
					if(nm == NULL)
						outOfMemory();
					strncpy(nm, word_buf, word_buf_pos-1); // we don't want a dot at the end of a string
					tmp->name = nm;

					pos_in_fields++;
					word_buf_pos = 0;

				} else if(pos_in_fields == 2 || pos_in_fields == 3) {
					// should be "IN" or "FORWARD" value we don't want them so skip them
					pos_in_fields++;
					word_buf_pos = 0;

				} else if(pos_in_fields == 4) {
					if(strncmp(word_buf, "+i", 2) == 0) {
						tmp->secure = false;
					} else {
						tmp->secure = true;
						char *ip = calloc(word_buf_pos + 1, sizeof(char));
						if(ip == NULL) {
							outOfMemory();
						}
						strncpy(ip, word_buf, word_buf_pos);

						charChain_append(chtmp, ip);
						//means its ip address
					}

					pos_in_fields++;
					word_buf_pos = 0;
				} else if(pos_in_fields > 4) { // pos is greater
					char *ip = calloc(word_buf_pos + 1, sizeof(char));
					if(ip == NULL) {
						outOfMemory();
					}
					strncpy(ip, word_buf, word_buf_pos);

					charChain_append(chtmp, ip);

					pos_in_fields++;
					word_buf_pos = 0;

				}
			} else if(buf[i] == ' ' && word_buf_pos == 0) { // ignore leading spaces if any
				//do nothing
			} else {
				word_buf[word_buf_pos] = *(buf+i); // copy the char value
				word_buf_pos++;
			}
			if(buf[i] == '\n') {
				tmp->ips = chtmp;
				chtmp = newCharChain();
				zoneConfig_append(tmp, zoneConfig);
				tmp = newZoneConfig();

				pos_in_fields = 1;
				word_buf_pos = 0;
				break;
			}
		}
	}

	// za predpokladu, ze cely vypis konci novym radkem jinak mazeme posledni zaznam nikoliv prazdnou bunku pripravenou k plneni
	freeZoneConfig(tmp, false); //values may be null
	if(buf != NULL)
		free(buf);
	freeCharChain(chtmp);
	pclose(unbound);

	return zoneConfig;
}

LocalZoneConfig* newLocalZoneConfig()
{
	LocalZoneConfig *cn = calloc(1, sizeof (LocalZoneConfig));
	    if (NULL == cn) {
	        outOfMemory();
	    }
	    return cn;
}

void freeLocalZoneConfig(LocalZoneConfig *cfg, bool preserveValues)
{
	if(NULL != cfg->next) {
		freeLocalZoneConfig(cfg->next, preserveValues);
		free(cfg->next);
	}

	if(!preserveValues && NULL != cfg->name)
		free(cfg->name);

	if(!preserveValues && NULL != cfg->type)
		free(cfg->type);

	if(NULL == cfg->prev)
			free(cfg);
}

LocalZoneConfig* localZoneConfig_append(LocalZoneConfig *what, LocalZoneConfig *where)
{
	LocalZoneConfig *tmp = where;
		if (NULL == tmp->name && NULL == tmp->type) {
			tmp->name = what->name;
			tmp->type = what->type;
			tmp->prev = NULL;
			tmp->next = NULL;

			freeLocalZoneConfig(what, true);
			return where;
		}

		while (NULL != tmp->next) {
			tmp = tmp->next;
		}

		what->prev = tmp;
		tmp->next = what;

		return where;
}

LocalZoneConfig* getUnboundLocalZoneConfig()
{
	// what if too many requests like at probing for example?
	FILE *unbound = popen("unbound-control status", "r");
	pclose(unbound); //maybe the EXIT_SUCCESS condition would be great here to determine if the unbound is running correctly

	unbound = popen("unbound-control list_local_zones", "r");
	// subprocess.check_output(["unbound-control", "list_forwards"]).decode() -- decode is missing and is not implemented here

	char *buf = NULL;
	char word_buf[512] = {0};
	int word_buf_pos = 0;
	size_t len = 0;


	LocalZoneConfig *localZoneConfig = newLocalZoneConfig();
	LocalZoneConfig *tmp = newLocalZoneConfig();

	int pos_in_fields = 1;

	while(getline(&buf, &len, unbound) != -1) {//deprecated comment: sometimes some chars are missing don't know why
		for(int i = 0; i < len; i++) {
			if((buf[i] == ' ' || buf[i] == '\n') && word_buf_pos > 0) {
				if(pos_in_fields == 1) {
					char *nm = calloc(word_buf_pos + 1, sizeof(char));
					if(nm == NULL)
						outOfMemory();
					strncpy(nm, word_buf, word_buf_pos-1); // we don't want a dot at the end of a string
					tmp->name = nm;

					pos_in_fields++;
					word_buf_pos = 0;

				} else if(pos_in_fields == 2) {
					char *tp = calloc(word_buf_pos + 1, sizeof(char));
					if(tp == NULL)
						outOfMemory();
					strncpy(tp, word_buf, word_buf_pos);
					tmp->type = tp;

					pos_in_fields++;
					word_buf_pos = 0;

				}
			} else if(buf[i] == ' ' && word_buf_pos == 0) {
				// do nothing
			} else {
				word_buf[word_buf_pos] = *(buf+i); // hope it copies char value
				word_buf_pos++;
			}

			if(buf[i] == '\n') {
				localZoneConfig_append(tmp, localZoneConfig);
				tmp = newLocalZoneConfig();

				pos_in_fields = 1;
				word_buf_pos = 0;
				break;
			}
		}
	}

	// za predpokladu, ze cely vypis konci novym radkem jinak mazeme zaznam nikoliv prazdnou bunku pripravenou k plneni
	freeLocalZoneConfig(tmp, false); //values may be null
	pclose(unbound);
	if(buf != NULL)
		free(buf);
	return localZoneConfig;
}

char **rfc1918_reverse_zones = (char *[]) {"c.f.ip6.arpa", "d.f.ip6.arpa", "168.192.in-addr.arpa", "16.172.in-addr.arpa", "17.172.in-addr.arpa", "18.172.in-addr.arpa", "19.172.in-addr.arpa", "20.172.in-addr.arpa", "21.172.in-addr.arpa", "22.172.in-addr.arpa", "23.172.in-addr.arpa", "24.172.in-addr.arpa", "25.172.in-addr.arpa", "26.172.in-addr.arpa", "27.172.in-addr.arpa", "28.172.in-addr.arpa", "29.172.in-addr.arpa", "30.172.in-addr.arpa", "31.172.in-addr.arpa", "10.in-addr.arpa"}; // hope every string ends with NULL

bool isEmptyCharChain(CharChain *cc) {
	if(NULL == cc || (NULL == cc->current && NULL == cc->next))
		return true;
	return false;
}


void unbound_local_zones_add(char *zone, char *type)
{
	int len = 27;
	len += strlen(zone);
	len ++; // one space
	len += strlen(type);
	len ++; //string delimiter
	char *command = calloc(len, sizeof(char));
	if(command == NULL)
		outOfMemory();
	strcpy(command, "unbound-control local_zone ");
	strcat(command, zone);
	strcat(command, " ");
	strcat(command, type);
	FILE *ubd = popen(command, "r");
	//we can read any answer here
	pclose(ubd);
	free(command);
}

void unbound_zones_remove(char *zone, char *flush_command)
{
	int len = 31;
	len += strlen(zone);
	len ++; //string delimiter
	char *command = calloc(len, sizeof(char));
	if(command == NULL)
		outOfMemory();
	strcpy(command, "unbound-control forward_remove ");
	strcat(command, zone);
	FILE *ubd = popen(command, "r");
	//we can read any answer here
	pclose(ubd);
	free(command);

	len = 16;
	len += strlen(flush_command);
	len ++; // one space
	len += strlen(zone);
	len ++; //string delimiter
	command = calloc(len, sizeof(char));
	if(command == NULL)
		outOfMemory();

	strcpy(command, "unbound-control ");
	strcat(command, flush_command);
	strcat(command, " ");
	strcat(command, zone);

	ubd = popen(command, "r");
	// here we can read any answer
	pclose(ubd);
	free(command);
	ubd = popen("unbound-control flush_requestlist", "r");
	pclose(ubd);
}

void freeCharChainCell(CharChain *cc)
{
	free(cc->current);
	free(cc);
}

void stored_zones_remove_double(CharChain **chain, char* zone) { //removes only the first occurrence
	for(CharChain **i = chain; *i != NULL; ) {
		CharChain *entry = *i;

		if(strcmp(entry->current, zone) == 0) {
			*i = entry->next;
			freeCharChainCell(entry);
			return;
		}

		i = &entry->next;
	}
}

void stored_zones_remove(char *zone)
{
	CharChain *i = stored_zones;
	while(i != NULL) {
		if(i->current == NULL) {
			i = i->next;
			continue;
		}
		if(strcmp(i->current, zone) == 0) {
			if(i->prev != NULL)
				i->prev->next = i->next;
			if(i->next != NULL)
				i->next->prev = i->prev;
			if(i->next != NULL || i->prev != NULL) {// means that cell isn't the only one in chain
				CharChain *tmp = i->next;  //todo: use new ** concept
//				if(i->prev != NULL && i->prev->prev == NULL)
//					stored_zones = i->prev;
				freeCharChainCell(i);
				i = tmp;
				continue;
			}
			else {
				if(i->current != NULL) {
					free(i->current);
					i->current = NULL;
				}
			}
		}
		i = i->next;
	}
}

char* servers_to_string(CharChain *servers)
{
	int len = 0;

	for(CharChain *tmp = servers; tmp != NULL; tmp = tmp->next) {
		if(tmp->current != NULL) {
			len += strlen(tmp->current);
			len ++; // one space
		}
	} // includes string delimiter because there isn't a space after the last word

	char *toRet = calloc(len, sizeof(char));
	if(toRet == NULL)
		outOfMemory();

	for(CharChain *tmp = servers; tmp != NULL; tmp = tmp->next) {
		if(tmp->prev == NULL) {
			strcpy(toRet, tmp->current);

		} else {
			strcat(toRet, tmp->current);
		}

		if(tmp->next != NULL) {
			strcat(toRet, " ");
		}
	}
	return toRet;
}

void unbound_zones_add(char *zone, CharChain *servers, bool validate)
{
	int len = 28;
	len += 3; // for "+i " if any
	len += strlen(zone);
	len ++; // one space
	char *srvs = servers_to_string(servers); // don't forget to deallocate this
	len += strlen(srvs);
	len ++; //string delimiter
	char *command = calloc(len, sizeof(char));
	if(command == NULL)
		outOfMemory();
	strcpy(command, "unbound-control forward_add ");
	if(!validate) {
		strcat(command, "+i ");
	}
	strcat(command, zone);
	strcat(command, " ");
	strcat(command, srvs);
	FILE *ubd = popen(command, "r");
	//we can read any answer here
	pclose(ubd);
	free(command);
	free(srvs);
}

void unbound_local_zones_remove(char *zone)
{
	int len = 34;
	len += strlen(zone);
	len ++; // string delimiter

	char *command = calloc(len, sizeof(char));

	if(command == NULL)
		outOfMemory();

	strcpy(command, "unbound-control local_zone_remove ");
	strcat(command, zone);

	FILE *ubd = popen(command, "r");
	pclose(ubd);
	free(command);
}

int update_connection_zones(ConnectionChain *inputConnections)
{
	// static temporary config
	bool use_private_address_range = true;
	bool add_wifi_provided_zone = false;
	bool validate_connection_provided_zones = false;
	bool keep_positive_answers = false;

	char *flush_command;
	if(keep_positive_answers)
		flush_command = "flush_negative";
	else
		flush_command = "flush_zone";

	// we copy inputConnections to keep them untouched for another usage somewhere else and for correct deallocation
	ConnectionChain *connections = copy_ConnectionChain(inputConnections);

    if(!add_wifi_provided_zone) { // what if there's only wifi on list?
    	connections = noWifi(connections);
    }

    AssocChain *mappedConnections = getZoneConnectionMapping(connections);
    ZoneConfig *unbound_zones = getUnboundZoneConfig();
    bool in = false;
	bool not_in = true;


	// Remove any zones managed by dnssec-trigger that are no longer valid.

    //for(CharChain *c = stored_zones; c != NULL; c = c->next) {
	CharChain *c = stored_zones;
	while(c != NULL) {
		// leave zones that are provided by some connection
    	for(AssocChain *cn = mappedConnections; cn != NULL; cn = cn->next) {
    		if(cn->zone == NULL || c->current == NULL)
    			continue;
    		if(strcmp(cn->zone, c->current) == 0) {
    			in = true;
    			break;
    		}
    	}

    	if(in) {
    		in = false;
    		c = c->next;
    		continue;
    	}
    	// ---------------------

    	for(int i = 0; i < 20; i++) { // 20 is number of records in rfc array //todo: do it dynamic use arrayDelimiter to determine the end of the array
    		char *zone = (char *)*(rfc1918_reverse_zones + i);

    		if(c->current == NULL || zone == NULL)
    			continue;
    		if(strcmp(zone, c->current) == 0) {
    			// if zone is private address range reverse zone and we are configured to use them, leave it
    			if(use_private_address_range) {
    				in = true;
    				break;
    			} else {
    				// otherwise add Unbound local zone of type 'static' like Unbound does and remove it later
    				unbound_local_zones_add(c->current, "static");
    				// how about putting here break too?
    			}
    		}
    	}

    	if(in) {
    		in = false;
    		c = c->next;
    		continue;
    	}
    	// --------------------
    	// Remove all zones that are not in connections except OR
    	// are private address ranges reverse zones and we are NOT
    	// configured to use them

    	for(ZoneConfig *ubd_zn = unbound_zones; ubd_zn != NULL; ubd_zn = ubd_zn->next) {
    		if(ubd_zn->name == NULL || c->current == NULL)
    			continue;
    		if(strcmp(ubd_zn->name, c->current) == 0) {
    			unbound_zones_remove(c->current, flush_command);
    			// how about putting here break too?
    		}
    	}
    	CharChain *tmp = c->next; // now there's just one danger, we've removed next cell, but this case shouldn't appear because we should have been iterated on the same position which will be deleted
    	stored_zones_remove_double(&stored_zones, c->current); // danger situations we've removed next, we've removed current
    	c = tmp;
    }
	// ------------------------
	// Install all zones coming from connections except those installed
	// by other means than dnssec-trigger-script.
    for(AssocChain *cn = mappedConnections; cn != NULL; cn = cn->next) {
    	// Reinstall a known zone or install a new zone.
		for(CharChain *stored = stored_zones; stored != NULL; stored = stored->next) {
			if(cn->zone == NULL || stored->current == NULL)
				continue;
			if(strcmp(cn->zone, stored->current) == 0) {
				in = true;
				break;
			}
		}

		if(!in) {
			for(ZoneConfig *ubd_zn = unbound_zones; ubd_zn != NULL; ubd_zn = ubd_zn->next) {
				if(ubd_zn->name == NULL || cn->zone == NULL)
					continue;
				if(strcmp(cn->zone, ubd_zn->name) == 0) {
					not_in = false;
					break;
				}
			}
		}

		if(in || not_in) {
			unbound_zones_add(cn->zone, cn->connection->servers, validate_connection_provided_zones); // maybe should ensure every server is there only once in chain
			char *zn = calloc(strlen(cn->zone)+1, sizeof(char));
			if(zn == NULL)
				outOfMemory();
			strcpy(zn, cn->zone);
			charChain_append_double(&stored_zones, zn);
		}
		in = false;
		not_in = true;
	}
	// ---------------------------

    in = false;
	not_in = true;

	// Configure forward zones for reverse name resolution of private addresses.
	// RFC1918 zones will be installed, except those already provided by connections
	// and those installed by other means than by dnssec-trigger-script.
	// RFC19118 zones will be removed if there are no global forwarders.
    if(use_private_address_range) {
    	for(int i = 0; i < 20; i++) { //todo: use arrayDelimiter to determine the end of the array
    		char *zone = (char *)*(rfc1918_reverse_zones + i);
			// Ignore a connection provided zone as it's been already processed.
    		for(AssocChain *con = mappedConnections; con != NULL; con = con->next) {
    			if(zone == NULL || con->zone == NULL)
    				continue;
    			if(strcmp(zone, con->zone) == 0) {
    				continue;
    			}
    		}

    		if(!isEmptyCharChain(global_forwarders)) {
    			// Reinstall a known zone or install a new zone.
				for(CharChain *stored = stored_zones; stored != NULL; stored = stored->next) {
					if(zone == NULL || stored->current == NULL)
						continue;
					if(strcmp(zone, stored->current) == 0) {
						in = true;
						break;
					}
				}

				not_in = true;
				if(!in) {
					for(ZoneConfig *ubd_zn = unbound_zones; ubd_zn != NULL; ubd_zn = ubd_zn->next) {
						if(zone == NULL || ubd_zn->name == NULL)
							continue;
						if(strcmp(zone, ubd_zn->name) == 0) {
							not_in = false;
							break;
						}
					}
				}

				if(in || not_in) {
					unbound_zones_add(zone, global_forwarders, false);
					char *zn = calloc(strlen(zone)+1, sizeof(char));
					if(zn == NULL)
						outOfMemory();
					strcpy(zn, zone);
					charChain_append_double(&stored_zones, zn);
					unbound_local_zones_remove(zone);
				}
				in = false;
				not_in = true;

    		} else {
    			// There are no global forwarders, therefore remove the zone
    			//CharChain *stored = stored_zones;
    			for(CharChain *stored = stored_zones; stored != NULL; stored = stored->next) {
    			//while(stored != NULL) {
    				if(zone == NULL || stored->current == NULL) {
    					continue;
    				}

					if(strcmp(zone, stored->current) == 0) {
						//CharChain *tmp = stored_zones->next;
							stored_zones_remove_double(&stored_zones, zone);
						//	stored = tmp;
						//stored_zones_remove(zone);
						break;
					}
				}

				for(ZoneConfig *ubd_zn = unbound_zones; ubd_zn != NULL; ubd_zn = ubd_zn->next) {
					if(zone == NULL || ubd_zn->name == NULL)
						continue;
					if(strcmp(zone, ubd_zn->name) == 0) {
						unbound_zones_remove(zone, flush_command);
						break;
					}
				}
				unbound_local_zones_add(zone, "static");

    		}
    	}
    }

    freeAssocChain(mappedConnections, true); // preserve values because for mapping we used values from other chain and didn't copy them
    freeZoneConfig(unbound_zones, false);
    freeConnectionChain(connections, false);




	return EXIT_SUCCESS;
}


