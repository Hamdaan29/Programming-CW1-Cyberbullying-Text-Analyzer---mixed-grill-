#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// SETTINGS
#define MAX_WORDS 30000     // Limit for total unique words
#define WORD_LEN 100        // Max length of a single word
#define MAX_STOPWORDS 2000  // Limit for stopword list
#define MAX_TOXICWORDS 3000 // Limit for toxic dictionary
#define MAX_LINE_LEN 4096   // Buffer size for reading lines

// DATA STRUCTURES
typedef struct
{
    char word[WORD_LEN];
    int count;
    int isToxic;  // 0 = Safe, 1 = Toxic
    int severity; // 1 = Mild, 2 = Moderate, 3 = Severe
} WordInfo;

typedef struct
{
    WordInfo words[MAX_WORDS];
    int wordCount;
    int totalWords;
    int toxicCount;
    int totalCharacters;
    int uniqueWords;
} TextAnalysis;

// FUNCTION DECLARATIONS
int loadStopwords(const char *filename);
int loadToxicWords(const char *filename);
void addNewToxicWord();

// ANALYSIS FUNCTIONS
int analyzeFile(TextAnalysis *analysis, const char *filename, int appendMode);

// DISPLAY FUNCTIONS
void printStatistics(TextAnalysis *analysis);
void printToxicStatistics(TextAnalysis *analysis);

// SORTING AND SAVING FUNCTIONS
void bubbleSortWords(TextAnalysis *analysis);
void quickSortWords(TextAnalysis *analysis);
void ShowTopWords(TextAnalysis *analysis, int limit, int toxicOnly);
void saveResults(TextAnalysis *analysis, const char *filename);

#endif