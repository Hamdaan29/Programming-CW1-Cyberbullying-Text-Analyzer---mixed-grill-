#include "common.h"

// Internal struct for dictionary
typedef struct
{
    char word[WORD_LEN];
    int severity;
} ToxicEntry;

// GLOBAL VARIABLES FOR STOPWORDS AND TOXIC DICTIONARY
char stopwords[MAX_STOPWORDS][WORD_LEN];
ToxicEntry toxicDictionary[MAX_TOXICWORDS];
int stopwordCount = 0;
int toxicCount = 0;

// DICTIONARY FUNCTIONS

int loadStopwords(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Warning: Could not open stopwords file '%s'.\n", filename);
        return 0;
    }
    char line[WORD_LEN];
    stopwordCount = 0;

    // READ FILE LINE BY LINE
    while (fgets(line, sizeof(line), fp) && stopwordCount < MAX_STOPWORDS)
    {
        // remove newline characters manually
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line[len - 1] == '\r')
        {
            line[len - 1] = '\0';
        }

        if (strlen(line) > 0)
        {
            strcpy(stopwords[stopwordCount], line);
            stopwordCount++;
        }
    }
    fclose(fp);
    return 1;
}

// LOAD TOXIC WORDS WITH SEVERITY LEVELS LIKE HELLO,2
int loadToxicWords(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Warning: Could not open toxic words file '%s'.\n", filename);
        return 0;
    }
    char line[WORD_LEN];
    toxicCount = 0;

    // READ FILE LINE BY LINE
    while (fgets(line, sizeof(line), fp) && toxicCount < MAX_TOXICWORDS)
    {
        // Manually remove newline characters
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line[len - 1] == '\r')
        {
            line[len - 1] = '\0';
        }

        if (strlen(line) > 0)
        {
            // SPLIT LINE BY COMMA
            char *wordPart = strtok(line, ",");
            char *severityPart = strtok(NULL, ",");
            if (wordPart)
            {
                strcpy(toxicDictionary[toxicCount].word, wordPart);
                // Convert severity string to integer, default to 1 if missing
                toxicDictionary[toxicCount].severity = severityPart ? atoi(severityPart) : 1;
                toxicCount++;
            }
        }
    }
    fclose(fp);
    return 1;
}

// ADD NEW TOXIC WORD INTERACTIVELY
void addNewToxicWord()
{
    char newWord[WORD_LEN];
    int severity;
    printf("\n=== Add New Toxic Word ===\n");
    printf("Enter word: ");
    scanf("%s", newWord);
    printf("Enter Severity (1=Mild, 2=Moderate, 3=Severe): ");
    scanf("%d", &severity);

    if (toxicCount < MAX_TOXICWORDS) // ADD TO DICTIONARY
    {
        strcpy(toxicDictionary[toxicCount].word, newWord);
        toxicDictionary[toxicCount].severity = severity;
        toxicCount++;
    }

    // APPEND TO FILE FOR FUTURE USE
    FILE *fp = fopen("toxicwords.txt", "a");
    if (fp)
    {
        fprintf(fp, "\n%s,%d", newWord, severity);
        fclose(fp);
        printf("Added '%s' (Level %d) to dictionary.\n", newWord, severity);
    }
    else
    {
        printf("Error: Could not update file.\n");
    }
}

// ANALYSIS LOGIC

int isStopword(const char *word)
{
    for (int i = 0; i < stopwordCount; i++)
    {
        if (strcasecmp(stopwords[i], word) == 0) // CASE-INSENSITIVE COMPARISON
            return 1;
    }
    return 0;
}

// Returns severity level (1-3) if toxic, 0 if safe
int checkToxic(const char *word)
{
    for (int i = 0; i < toxicCount; i++)
    {
        if (strcasecmp(toxicDictionary[i].word, word) == 0)
            return toxicDictionary[i].severity;
    }
    return 0;
}

// Process a single word: update counts and stats
void addWord(TextAnalysis *analysis, const char *word, int toxicLevel)
{
    analysis->totalWords++;
    analysis->totalCharacters += strlen(word);

    if (toxicLevel > 0)
        analysis->toxicCount++;

    // Check if word already exists to update count
    for (int i = 0; i < analysis->wordCount; i++)
    {
        if (strcasecmp(analysis->words[i].word, word) == 0)
        {
            analysis->words[i].count++;
            return;
        }
    }

    // Add new word to the list
    if (analysis->wordCount < MAX_WORDS)
    {
        strcpy(analysis->words[analysis->wordCount].word, word);
        analysis->words[analysis->wordCount].count = 1;
        analysis->words[analysis->wordCount].isToxic = (toxicLevel > 0);
        analysis->words[analysis->wordCount].severity = toxicLevel;
        analysis->wordCount++;
        analysis->uniqueWords++;
    }
}

// Process a line of text: tokenize and analyze words
void processLine(TextAnalysis *analysis, const char *line)
{
    char tempLine[MAX_LINE_LEN];
    strcpy(tempLine, line);

    // Convert to lowercase and replace non-alphanumeric chars with spaces
    for (int i = 0; tempLine[i] != '\0'; i++)
    {
        tempLine[i] = tolower(tempLine[i]);

        // Check if char is NOT a letter (a-z) AND NOT a number (0-9)
        // and not an apostrophe. If so, replace with space.
        if ((tempLine[i] < 'a' || tempLine[i] > 'z') &&
            (tempLine[i] < '0' || tempLine[i] > '9') &&
            tempLine[i] != '\'')
        {
            tempLine[i] = ' ';
        }
    }
    // Tokenize string by spaces
    char *token = strtok(tempLine, " \t\n\r");
    while (token != NULL)
    {
        if (strlen(token) > 0 && !isStopword(token))
        {
            int severity = checkToxic(token);
            addWord(analysis, token, severity);
        }
        token = strtok(NULL, " \t\n\r");
    }
}

// Analyze file and populate TextAnalysis struct
int analyzeFile(TextAnalysis *analysis, const char *filename, int appendMode)
{

    if (!appendMode)
    {
        analysis->wordCount = 0;
        analysis->totalWords = 0;
        analysis->toxicCount = 0;
        analysis->totalCharacters = 0;
        analysis->uniqueWords = 0;
    }

    // OPEN FILE FOR READING
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return 0;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp))
    {
        processLine(analysis, line);
    }
    fclose(fp);
    return 1;
}

// SORTING & DISPLAY
// BUBBLE SORT IMPLEMENTATION
void bubbleSortWords(TextAnalysis *analysis)
{
    printf("Sorting using Bubble Sort...\n");
    for (int i = 0; i < analysis->wordCount - 1; i++)
    {
        for (int j = 0; j < analysis->wordCount - i - 1; j++)
        {
            if (analysis->words[j].count < analysis->words[j + 1].count)
            {
                WordInfo temp = analysis->words[j];
                analysis->words[j] = analysis->words[j + 1];
                analysis->words[j + 1] = temp;
            }
        }
    }
}

// QUICK SORT IMPLEMENTATION
int compareWords(const void *a, const void *b)
{
    const WordInfo *wordA = (const WordInfo *)a;
    const WordInfo *wordB = (const WordInfo *)b;

    // Sort Descending: If B is bigger than A, B goes first
    if (wordB->count > wordA->count)
    {
        return 1;
    }
    else if (wordB->count < wordA->count)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void quickSortWords(TextAnalysis *analysis)
{
    printf("Sorting using Quick Sort...\n");
    qsort(analysis->words, analysis->wordCount, sizeof(WordInfo), compareWords);
}

const char *getSeverityLabel(int level)
{
    if (level == 1)
        return "MILD";
    if (level == 2)
        return "MODERATE";
    if (level == 3)
        return "SEVERE";
    return "UNKNOWN";
}

// GENERAL DISPLAY FUNCTIONS
void printStatistics(TextAnalysis *analysis)
{
    printf("\n GENERAL WORD STATISTICS \n");
    printf("Total Words:       %d\n", analysis->totalWords);
    printf("Unique Words:      %d\n", analysis->uniqueWords);

    if (analysis->totalWords > 0)
    {
        // CALCULATE LEXICAL DIVERSITY (UNIQUE / TOTAL * 100)
        float diversity = ((float)analysis->uniqueWords / analysis->totalWords) * 100;
        printf("Lexical Diversity: %.2f%%\n", diversity);
        printf("Avg Word Length:   %.2f chars\n", (float)analysis->totalCharacters / analysis->totalWords);
    }
    else
    {
        printf("No words analyzed yet.\n");
    }
}

// TOXIC WORD DISPLAY FUNCTION
void printToxicStatistics(TextAnalysis *analysis)
{
    printf("\n=== TOXIC WORD ANALYSIS ===\n");
    printf("Toxic Words Found: %d\n", analysis->toxicCount);

    if (analysis->totalWords > 0)
    {
        printf("Toxic Percentage:  %.2f%%\n", (float)analysis->toxicCount / analysis->totalWords * 100); // TOXIC PERCENTAGE
    }
    else
    {
        printf("No words analyzed yet.\n");
    }
}

// DISPLAY TOP N WORDS BASED ON FREQUENCY
void ShowTopWords(TextAnalysis *analysis, int limit, int toxicOnly)
{
    if (toxicOnly)
        printf("\n=== TOP %d TOXIC WORDS ===\n", limit);
    else
        printf("\n=== TOP %d WORDS (ALL) ===\n", limit);

    int count = 0;
    for (int i = 0; i < analysis->wordCount && count < limit; i++)
    {
        if (toxicOnly && !analysis->words[i].isToxic)
            continue;

        printf("%d. %s (%d)", count + 1, analysis->words[i].word, analysis->words[i].count);

        if (analysis->words[i].isToxic) // ADD TOXICITY LABEL
        {
            printf(" [%s TOXICITY]", getSeverityLabel(analysis->words[i].severity));
        }
        printf("\n");
        count++;
    }
}
// SAVE ANALYSIS RESULTS TO FILE
void saveResults(TextAnalysis *analysis, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        printf("Error: Could not save file.\n");
        return;
    }

    fprintf(fp, "TEXT ANALYSIS REPORT\n====================\n");
    fprintf(fp, "Total Words: %d\n", analysis->totalWords);
    fprintf(fp, "Toxic Words: %d\n", analysis->toxicCount);
    if (analysis->totalWords > 0)
    {
        float diversity = ((float)analysis->uniqueWords / analysis->totalWords) * 100;
        fprintf(fp, "Lexical Diversity: %.2f%%\n", diversity);
    }

    fprintf(fp, "\nWORD FREQUENCY LIST:\n");
    quickSortWords(analysis); // Sort before saving to ensure order

    for (int i = 0; i < analysis->wordCount; i++)
    {
        fprintf(fp, "%s: %d", analysis->words[i].word, analysis->words[i].count);
        if (analysis->words[i].isToxic)
        {
            fprintf(fp, " [TOXIC LEVEL %d]", analysis->words[i].severity); // ADD TOXICITY INFO
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("Results saved to '%s'.\n", filename);
}