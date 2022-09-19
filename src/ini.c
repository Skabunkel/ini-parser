#include "ini.h"

#include <string.h>
#include <stdlib.h>

#define COMMENT_TOKEN ';'
#define COMMENT_TOKEN_ALT '#'
#define SEGMENT_START '['
#define SEGMENT_END ']'
#define KEY_VALUE_DEVIDER_ALT ':'
#define KEY_VALUE_DEVIDER '='
#define FIRST_WHITESPACE_TOKE ' '
#define SECOND_WHITESPACE_TOKE '\t'
#define BUFFER_SIZE 4*1024

typedef enum parse_state
{
    ps_none = 0,
    ps_segment = 1,
    ps_keyValue = 2
} parse_state_t;


int compare(const char* a, const char* b)
{
    if(a == b) return 0;

    if(a == NULL)
    {
        return 1;
    }
    if(b == NULL)
    {
        return -1;
    }

    return strcmp(a, b);
}

const ini_segment_t* InitSegment()
{
    ini_segment_t* segment = (ini_segment_t*)malloc(sizeof(ini_segment_t));
    memset(segment, 0, sizeof(ini_segment_t));

    return segment;
}

const ini_segment_t* InitSegmentWithName(const char* name)
{
    ini_segment_t* segment = (ini_segment_t*)InitSegment();

    if(name != NULL)
    {
        int length = strlen(name);
        segment->name = (char*)malloc(length+1);
        memset(segment->name, 0, length+1);
        memcpy(segment->name, name, length);
    }
    
    return segment;
}

const ini_key_value_t* InitKeyValueWithKeyValue(const char* key, const char* value)
{
    ini_key_value_t* keyValuePair = (ini_key_value_t*)malloc(sizeof(ini_key_value_t));
    memset(keyValuePair, 0, sizeof(ini_key_value_t));

    if(key != NULL)
    {
        int length = strlen(key);
        keyValuePair->key = (char*)malloc(length+1);
        memset(keyValuePair->key, 0, length+1);
        memcpy(keyValuePair->key, key, length);
    }
    
    if(value != NULL)
    {
        int length = strlen(value);
        keyValuePair->value = (char*)malloc(length+1);
        memset(keyValuePair->value, 0, length+1);
        memcpy(keyValuePair->value, value, length);
    }

    return keyValuePair;
}

char* trim(char* head, char* tail)
{
    while (head != tail)
    {
        if((*head) == FIRST_WHITESPACE_TOKE || (*head) == SECOND_WHITESPACE_TOKE || (*head) == SEGMENT_START || (*head) == KEY_VALUE_DEVIDER || (*head) == KEY_VALUE_DEVIDER_ALT || (*head) == '\0')
        {
            (*head) = '\0';
        }
        else
        {
            break;
        }
        head++;
    }

    while (head != tail)
    {
        if((*tail) == FIRST_WHITESPACE_TOKE || (*tail) == SECOND_WHITESPACE_TOKE || (*tail) == SEGMENT_END || (*tail) == KEY_VALUE_DEVIDER || (*tail) == KEY_VALUE_DEVIDER_ALT || (*tail) == COMMENT_TOKEN || (*tail) == COMMENT_TOKEN_ALT || (*tail) == '\r' || (*tail) == '\n' || (*tail) == '\0')
        {
            (*tail) = '\0';
        }
        else
        {
            break;
        }
        tail--;
    }
    return head;
}

ini_segment_t* parse_segment(const char* buffer, const int length, const ini_segment_t** segments)
{
    char* head = buffer;
    char* tail = buffer+((length)*sizeof(char));
    
    head = trim(head, tail);

    if(head != tail)
    {
       return AddSegment(segments, head);
    }
}

void parse_key_value(const char* buffer, const int length, const ini_segment_t** segments)
{
    if((*segments) == NULL)
        (*segments) = InitSegment();
    
    char* head = buffer;
    char* tail = head+((length)*sizeof(char));

    char* tailOfName = strchr(buffer, KEY_VALUE_DEVIDER);
    char* headOfValue = tailOfName + sizeof(char);
    
    head = trim(head, tailOfName);
    headOfValue = trim(headOfValue, tail);

    AddKeyValue(&(*segments)->KeyValuePair, head, headOfValue);
}

void set_keyvalue_value(ini_key_value_t* keyvalue, const char* value)
{
    if(keyvalue == NULL)
        return;

    if(compare(value, keyvalue->value) == 0)
        return;
    
    free(keyvalue->value);

    int length = strlen(value);
    keyvalue->value = (char*)malloc(length+1);
    memset(keyvalue->value, 0, length+1);
    memcpy(keyvalue->value, value, length);
}

const ini_segment_t* ParseIni(const FILE* file)
{
    char* buffer = (char*)malloc((BUFFER_SIZE)+1);
    memset(buffer, 0, BUFFER_SIZE);

    int remainder = 0;
    int index = 0;
    char* ofset;
    char* readHead;
    parse_state_t ps = ps_none;

    ini_segment_t* iniRoot = NULL;
    ini_segment_t* iniReader = NULL;

    FILE* fs = file;

    while(fgets(buffer+remainder, (BUFFER_SIZE)-remainder, fs) != NULL)
    {
        readHead = buffer;

        while ((buffer-readHead) < BUFFER_SIZE)
        {
            if(readHead == ofset)
                readHead += 1;

            ofset = strchr(readHead, '\n');

            if(ofset == NULL)
                break;
            

            while ((readHead+index) < ofset)
            {
                switch (readHead[index])
                {
                case COMMENT_TOKEN_ALT:
                case COMMENT_TOKEN:
                        switch (ps)
                        {
                        case ps_keyValue:
                            parse_key_value(readHead, index, &iniReader);
                            ps = ps_none;
                            if(iniRoot == NULL)
                                iniRoot = iniReader;
                            
                            readHead += index;
                            index = 0;
                        default:
                        break;
                        }
                        readHead = ofset;
                        index = 0;
                break;
                case SEGMENT_START:
                    switch (ps)
                    {
                    case ps_none:
                        ps = ps_segment;
                        readHead += index;
                        index = 0;
                    default:
                    break;
                    }
                break;

                case SEGMENT_END:
                    switch (ps)
                    {
                    case ps_segment:
                        iniReader = parse_segment(readHead, index, &iniRoot);
                        ps = ps_none;
                        if(iniRoot == NULL)
                            iniRoot = iniReader;
                        
                        readHead += index;
                        index = 0;
                    default:
                    break;
                    }
                break;

                case KEY_VALUE_DEVIDER_ALT:
                case KEY_VALUE_DEVIDER:
                    switch (ps)
                    {
                    case ps_none:
                        ps = ps_keyValue;
                    default:
                    break;
                    }
                break;
                
                default:
                break;
                }
                index++;
                                
            }

            switch (ps)
            {
            case ps_keyValue:
                parse_key_value(readHead, index, &iniReader);
                if(iniRoot == NULL)
                    iniRoot = iniReader;
            break;
            default:
                
            break;
            }

            readHead += index;
            index = 0;
            
            ps = ps_none;
        }
    }
    free(buffer);
    return iniRoot;
}

const ini_segment_t* AddSegment(const ini_segment_t** rootSegment, const char* name)
{
    if((*rootSegment) == NULL)
    {
        (*rootSegment) = (ini_segment_t*)InitSegmentWithName(name);
        return (*rootSegment);
    }

    ini_segment_t* prev = NULL;
    ini_segment_t* ittirator = (*rootSegment);
    int comparison = 0;

    while (ittirator != NULL)
    {
        prev = ittirator;
        comparison = compare(name, ittirator->name);

        if(comparison == 0)
        {
            return ittirator;
        }

        ittirator = ittirator->next; 
    }
    
    if(ittirator == NULL)
    {
        ittirator = prev;
    }

    ini_segment_t* newSegment = (ini_segment_t*)InitSegmentWithName(name);

    newSegment->next = ittirator->next;
    ittirator->next = newSegment;

    return newSegment;
}


void AddKeyValue(const ini_key_value_t** keyValue, const char* key, const char* value)
{
    if((*keyValue) == NULL)
    {
        (*keyValue) = (ini_key_value_t*)InitKeyValueWithKeyValue(key, value);
        return;
    }

    ini_key_value_t* prev;
    ini_key_value_t* ittirator = (*keyValue);
    int comparison = 0;

    while (ittirator != NULL)
    {   
        prev = ittirator;
        comparison = compare(key, ittirator->key);
        if(comparison == 0)
        {
            set_keyvalue_value(ittirator, value);
            return;
        }
        
        ittirator = ittirator->next; 
    }

    if(ittirator == NULL)
        ittirator = prev;

    ini_key_value_t* newKeuValue = (ini_key_value_t*)InitKeyValueWithKeyValue(key, value);
    newKeuValue->next = ittirator->next;
    ittirator->next = newKeuValue;
}


const ini_segment_t* GetSegment(const ini_segment_t* rootSegment, const char* name)
{
    ini_segment_t* itterator = (ini_segment_t*)rootSegment;
    while (itterator != NULL)
    {
        if(compare(name, itterator->name) == 0)
        {
            break;
        }

        itterator = itterator->next;
    }
    return itterator;
}
const ini_key_value_t* GetKeyValue(const ini_segment_t* segment, const char* key)
{
    ini_key_value_t* itterator = (ini_key_value_t*)segment->KeyValuePair;
    while (itterator != NULL)
    {
        if(compare(key, itterator->key) == 0)
        {
            break;
        }
        itterator = itterator->next;
    }
    return itterator;
}

void DeleteKeyValue_Impl(ini_key_value_t* keyValue)
{
    free(keyValue->key);
    free(keyValue->value);
    free(keyValue);
}

void DeleteSegment_Impl(ini_segment_t* segment)
{   
    free(segment->name);
    ini_key_value_t* itterator = (ini_key_value_t*)segment->KeyValuePair;
    ini_key_value_t* next;

    while (itterator != NULL)
    {
        next = itterator->next;
        DeleteKeyValue_Impl(itterator);
        itterator = next;
    }

    free(segment); 
}

void DeleteSegment(const ini_segment_t* rootSegment, const char* name)
{
    ini_segment_t* prevSegment = NULL;
    ini_segment_t* itterator = (ini_segment_t*)rootSegment;
    while (itterator != NULL)
    {
        if(compare(name, itterator->name) == 0)
        {
            if(prevSegment != NULL)
            {
                prevSegment->next = itterator->next;
            }
            DeleteSegment_Impl(itterator);
            return;
        }
        prevSegment = itterator;
        itterator = itterator->next;
    }
}

void DeleteKeyValue(const ini_segment_t* segment, const char* key)
{
    ini_key_value_t* prev = NULL;
    ini_key_value_t* itterator = (ini_key_value_t*)segment->KeyValuePair;
    while (itterator != NULL)
    {
        if(compare(key, itterator->key) == 0)
        {
            if(prev != NULL)
            {
                prev->next = itterator->next;
            }
            DeleteKeyValue_Impl(itterator);
            return;
        }
        prev = itterator;
        itterator = itterator->next;
    }
}


void FreeIni(const ini_segment_t* rootSegment)
{
    ini_segment_t* next = NULL;
    ini_segment_t* itterator = (ini_segment_t*)rootSegment;
    while (itterator != NULL)
    {
        next = itterator->next;
        DeleteSegment_Impl(itterator);
        itterator = next;
    }
}

void printfkeyValue(const ini_segment_t* segment)
{
    if(segment->KeyValuePair == NULL)
        return;

    ini_key_value_t* itterator = (ini_key_value_t*)segment->KeyValuePair;
    while (itterator != NULL)
    {
        if(itterator->key != NULL)
        {
            printf("%s = %s\r\n", itterator->key, itterator->value);
        }
        itterator = itterator->next;
    }
}

void printfConfig(const ini_segment_t* rootSegment)
{
    ini_segment_t* itterator = (ini_segment_t*)rootSegment;
    while (itterator != NULL)
    {
        if(itterator->name != NULL)
        {
            printf("[%s]\r\n", itterator->name);
        }
        printfkeyValue(itterator);
        itterator = itterator->next;
    }
}

