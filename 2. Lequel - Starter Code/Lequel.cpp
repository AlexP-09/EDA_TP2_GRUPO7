/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#include <cmath>
#include <codecvt>
#include <locale>
#include <iostream>

#include "Lequel.h"

using namespace std;

/**
 * @brief Builds a trigram profile from a given text.
 *
 * @param text Vector of lines (Text)
 * @return TrigramProfile The trigram profile
 */
TrigramProfile buildTrigramProfile(const Text &text)
{
    TrigramProfile trigramProfile;
    wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    for (auto line : text)
    {
        if ((line.length() > 0) &&
            (line[line.length() - 1] == '\r'))
            line = line.substr(0, line.length() - 1);

        if (line.length() < 3)
            continue;
        wstring unicodeString = converter.from_bytes(line);
        for (int i = 0; i <= unicodeString.length()-3; i++)
        {
            wstring unicodeTrigram = unicodeString.substr(i, 3);
            string trigram = converter.to_bytes(unicodeTrigram);

            trigramProfile[trigram] += 1.0f;
            //cout << trigram << endl;

        }

    }
  
    return trigramProfile;
}

/**
 * @brief Normalizes a trigram profile.
 *
 * @param trigramProfile The trigram profile.
 */
void normalizeTrigramProfile(TrigramProfile &trigramProfile)
{
    float sum = 0.0f;
    for (const auto& element : trigramProfile) 
    {
        sum += element.second*element.second;
    }
    sum = sqrt(sum);
    if (!sum)
    {
        cout << "divided by zero" << endl;
        return;
    }

    for (auto& element : trigramProfile)
    {
        element.second /= sum;
    }
    return;
}

/**
 * @brief Calculates the cosine similarity between two trigram profiles
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @return float The cosine similarity score
 */
float getCosineSimilarity(TrigramProfile &textProfile, TrigramProfile &languageProfile)
{
    float sum = 0.0f;
    for (const auto& textElement : textProfile)
    {
        for (const auto& languageElement : languageProfile)
        {
            if (textElement.first == languageElement.first)
            {
                sum += textElement.second * languageElement.second;
            }
        }
    }
    return sum;
}

/**
 * @brief Identifies the language of a text.
 *
 * @param text A Text (vector of lines)
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
string identifyLanguage(const Text &text, LanguageProfiles &languages)
{
    TrigramProfile textProfile=buildTrigramProfile(text);
    normalizeTrigramProfile(textProfile);
    string bestLanguage = "";  
    float maxSimilarity = 0.0f; 

    for (auto& languageElement : languages)
    {
        float similarity = getCosineSimilarity(textProfile, languageElement.trigramProfile);
        if (similarity > maxSimilarity) 
        {
            maxSimilarity = similarity;
            bestLanguage = languageElement.languageCode;
        }

    }

    return bestLanguage; 
}   
