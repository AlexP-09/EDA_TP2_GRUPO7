/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#ifndef LEQUEL_H
#define LEQUEL_H

#include <list>
#include <map>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Text.h"

using namespace std;

const string LANGUAGECODE_NAMES_FILE = "resources/languagecode_names_es.csv";
const string TRIGRAMS_PATH = "resources/trigrams/";

// TrigramProfile: map of trigram -> frequency
// typedef std::map<std::string, float> TrigramProfile;
// Using unordered_map for better performance
typedef std::unordered_map<std::string, float> TrigramProfile;

// TrigramList: list of trigrams
typedef std::list<std::string> TrigramList;

struct LanguageProfile
{
    std::string languageCode;
    TrigramProfile trigramProfile;
};

typedef std::list<LanguageProfile> LanguageProfiles;

// Functions
TrigramProfile buildTrigramProfile(const Text &text);
void normalizeTrigramProfile(TrigramProfile &trigramProfile);
float getCosineSimilarity(TrigramProfile &textProfile, TrigramProfile &languageProfile);
std::string identifyLanguage(const Text &text, LanguageProfiles &languages);

int addCustomLanguage(map<string, string>& languageCodeNames, LanguageProfiles& languages);


#endif
