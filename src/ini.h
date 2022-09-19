#ifndef _INI_H
#define _INI_H

#include <stdio.h>

typedef struct ini_key_value
{
    char* key;
    char* value;
    struct ini_key_value* next;
    /* data */
} ini_key_value_t;

typedef struct ini_segment
{
    char* name;
    struct ini_segment* next;
    ini_key_value_t* KeyValuePair;
    /* data */
} ini_segment_t;

/// @brief Creates an empty config segment.
/// @return a unnamed config segment.
const ini_segment_t* InitSegment();

/// @brief Creates a named config segment.
/// @param name Name of the segment.
/// @return Returns a named config segment.
const ini_segment_t* InitSegmentWithName(const char* name);

/// @brief Parses a config file.
/// @param file File pointer to parse.
/// @return The root config segment from the file.
const ini_segment_t* ParseIni(const FILE* file);

/// @brief Adds a new config segment, if the name does not already exist.
/// @param rootSegment root of the segments to build the list from.
/// @param name name of the new segment.
/// @return the newly created config segment.
const ini_segment_t* AddSegment(const ini_segment_t** rootSegment, const char* name);

/// @brief Adds a key value pare to a key value list, if the key does not already exist, it will replace the value in that case. each name has to be unique in that segment.
/// @param keyValue key value list.
/// @param key key of the key value.
/// @param value value of the key.
void AddKeyValue(const ini_key_value_t** keyValue, const char* key, const char* value);

/// @brief Finds the segment with a specific name. 
/// @param rootSegment Root config segment.
/// @param name Name of the segment you wish to find.
/// @return the segment with the given name, or null if it does not exist.
const ini_segment_t* GetSegment(const ini_segment_t* rootSegment, const char* name);

/// @brief Finds a key value pair with a given key. 
/// @param segment The segment we wish to look inside.
/// @param key The name of the segment we are looking for.
/// @return The pointer to the given segment or null if it does not exist.
const ini_key_value_t* GetKeyValue(const ini_segment_t* segment, const char* key);

void DeleteSegment(const ini_segment_t* rootSegment, const char* name);
void DeleteKeyValue(const ini_segment_t* segment, const char* key);


void FreeIni(const ini_segment_t* rootSegment);

void printfConfig(const ini_segment_t* rootSegment);

#endif // _INI_H