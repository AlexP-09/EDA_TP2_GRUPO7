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
#include <string>
#include <unordered_map>
#include <map>

#include "raylib.h"
#include "CSVdata.h"
#include "Lequel.h"

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
	const int MAX_TRIGRAMS = 10000;
	int trigramCount = 0;
	bool reachedMaxTrigrams = false;

    for (auto line : text)
    {
        if ((line.length() > 0) &&
            (line[line.length() - 1] == '\r'))
            line = line.substr(0, line.length() - 1);

        if (line.length() < 3)
            continue;

        wstring unicodeString = converter.from_bytes(line);

		// Converts to lowercase
        wstring lowercaseString;
        for (wchar_t c : unicodeString) 
        {
            lowercaseString += std::tolower(c);
        }

        for (int i = 0; i <= lowercaseString.length()-3 && !reachedMaxTrigrams; i++)
        {
            wstring unicodeTrigram = lowercaseString.substr(i, 3);
            string trigram = converter.to_bytes(unicodeTrigram);

            trigramProfile[trigram] += 1.0f;
            //cout << trigram << endl;
            
			// Limit the number of trigrams to avoid excessive memory usage
			if (++trigramCount >= MAX_TRIGRAMS)
				reachedMaxTrigrams = true;
        }

        if (reachedMaxTrigrams)
            break;
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
 
    for (const auto& textElement : textProfile) {

        string trigram = textElement.first;
        float text_frequency = textElement.second;

		// Looks for the trigram in the language profile
        auto found_trigram = languageProfile.find(trigram);

		// If found, multiply the frequencies and add to the sum
        if (found_trigram != languageProfile.end()) {
            float language_frequency = found_trigram->second;  // ej: 0.4
            sum += text_frequency * language_frequency;   // 0.5 * 0.4
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

/**
 * @brief Adds a custom language to the system
 *
 * @param languageCodeNames Reference to the language code names map
 * @param languages Reference to the language profiles
 * @return 1 if language was added successfully
 * @return -1 if there was an error
 */
int addCustomLanguage(map<string, string>& languageCodeNames, LanguageProfiles& languages)
{
    // Get the dropped file path
    FilePathList droppedFiles = LoadDroppedFiles();
    if (droppedFiles.count != 1)
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    const char* filePath = droppedFiles.paths[0];

    // Extract filename without extension as language name
    string fileName = string(filePath);
    size_t lastSlash = fileName.find_last_of("/\\");
    if (lastSlash != string::npos)
        fileName = fileName.substr(lastSlash + 1);

    size_t lastDot = fileName.find_last_of(".");
    if (lastDot != string::npos)
        fileName = fileName.substr(0, lastDot);

    string languageName = fileName;

    // Generate a 3-character language code from the name
    string languageCode;
    if (languageName.length() >= 3)
    {
        languageCode = languageName.substr(0, 3);
        for (char& c : languageCode)
            c = tolower(c);
    }
    else
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    // Check if language code or name already exists
    if (languageCodeNames.find(languageCode) != languageCodeNames.end())
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    for (const auto& entry : languageCodeNames)
    {
        if (entry.second == languageName)
        {
            UnloadDroppedFiles(droppedFiles);
            return -1;
        }
    }

    Text text;
    if (!getTextFromFile(filePath, text))
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    TrigramProfile trigramProfile = buildTrigramProfile(text);

    if (trigramProfile.empty())
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    CSVData trigramCSVData;

    for (const auto& entry : trigramProfile)
    {
        vector<string> row;
        row.push_back(entry.first);
        row.push_back(to_string((int)entry.second));
        trigramCSVData.push_back(row);
    }

    // Save trigram profile to CSV file
    string trigramFilePath = TRIGRAMS_PATH + languageCode + ".csv";
    if (!writeCSV(trigramFilePath, trigramCSVData))
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    // Read existing language codes file
    CSVData existingLanguages;
    if (!readCSV(LANGUAGECODE_NAMES_FILE, existingLanguages))
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    existingLanguages.push_back({ languageCode, languageName });

    // Save updated language codes file
    if (!writeCSV(LANGUAGECODE_NAMES_FILE, existingLanguages))
    {
        UnloadDroppedFiles(droppedFiles);
        return -1;
    }

    languageCodeNames[languageCode] = languageName;

    // Create new language profile and add to languages list
    languages.push_back(LanguageProfile());
    LanguageProfile& newLanguage = languages.back();
    newLanguage.languageCode = languageCode;

    // Convert trigrams to lowercase and normalize
    unordered_map<string, float> tempTrigramProfile;
    for (const auto& entry : trigramProfile)
    {
        string lowercaseTrigram;
        for (char c : entry.first)
            lowercaseTrigram += tolower(c);

        tempTrigramProfile[lowercaseTrigram] += entry.second;
    }

    for (const auto& entry : tempTrigramProfile)
        newLanguage.trigramProfile[entry.first] = entry.second;

    normalizeTrigramProfile(newLanguage.trigramProfile);

    UnloadDroppedFiles(droppedFiles);
    return 1;
}