#include "common.h"

int main()
{
    TextAnalysis analysis;
    char filename[256];
    int choice;
    int fileLoaded = 0; // check if data has been loaded

    // Load Dictionaries at Startup
    loadStopwords("stopwords.txt");
    loadToxicWords("toxicwords.txt");
    printf("Dictionaries loaded successfully.\n");

    analysis.wordCount = 0;
    //  do while loop for menu system so it runs at least once
    do
    {
        // USER INTERFACE
        printf("\n=== CYBERBULLYING TEXT ANALYZER ===\n");
        printf("1. Load text file for analysis\n");
        printf("2. Display general word statistics\n");
        printf("3. Display toxic word analysis\n");
        printf("4. Sort and display top N words\n");
        printf("5. Save results to output file\n");
        printf("6. Add New Toxic Word\n");
        printf("7. Exit program\n");
        printf("Choice: ");

        // Robust Input Handling: check if input is a number
        int result = scanf("%d", &choice);

        // Clear the buffer manually
        while (getchar() != '\n')
            ;

        if (result == 0)
        {
            printf("Invalid input. Please enter a number.\n");
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1: // Load text file
            printf("Enter filename: ");
            fgets(filename, sizeof(filename), stdin);

            // Remove newline character
            int len = strlen(filename);
            if (len > 0 && filename[len - 1] == '\n')
            {
                filename[len - 1] = '\0';
            }

            int append = 0;
            if (fileLoaded)
            {
                char resp;
                printf("Data already loaded. Clear it? (y/n): ");
                scanf("%c", &resp);
                while (getchar() != '\n') // Clear input buffer
                    ;
                if (resp == 'n' || resp == 'N')
                    append = 1;
            }

            // Analyze the file
            if (analyzeFile(&analysis, filename, append))
            {
                printf("File analyzed successfully.\n");
                fileLoaded = 1;
            }
            else
            {
                printf("Error: Could not open '%s'.\n", filename);
            }
            break;

        case 2: // Display general stats
            if (fileLoaded)
                printStatistics(&analysis);
            else
                printf("Please load a file first.\n");
            break;

        case 3: // Display toxic analysis
            if (fileLoaded)
                printToxicStatistics(&analysis);
            else
                printf("Please load a file first.\n");
            break;

        case 4: // Sort and display top N words
            if (fileLoaded)
            {
                int sortType, limit, modeChoice, showToxicOnly;
                printf("\nSort Method:\n1. Bubble Sort\n2. Quick Sort\nChoice: ");
                scanf("%d", &sortType);

                printf("\nDisplay Mode:\n1. Show All\n2. Show TOXIC Only\nChoice: ");
                scanf("%d", &modeChoice);

                // Use simple if/else for readability

                if (modeChoice == 2)
                {
                    showToxicOnly = 1;
                }
                else
                {
                    showToxicOnly = 0;
                }

                printf("How many words to display? ");
                scanf("%d", &limit);

                // Choose sorting algorithm
                if (sortType == 2)
                    // Use Quick Sort for large files
                    quickSortWords(&analysis);
                else
                    bubbleSortWords(&analysis);

                ShowTopWords(&analysis, limit, showToxicOnly); // Display top N words
            }
            else
            {
                printf("Please load a file first.\n");
            }
            break;

        case 5: // Save results
            if (fileLoaded)
            {
                saveResults(&analysis, "analysis_report.txt");
            }
            else
            {
                printf("No data to save. Load a file first.\n");
            }
            break;

        case 6: // dictionary update for toxic words
            addNewToxicWord();
            printf("NOTE: Changes will apply to next analysis.\n");
            break;

        case 7: // Exit program
            printf("Exiting program... Goodbye!\n");
            break;

        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);

    return 0;
}