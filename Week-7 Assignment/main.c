#include <stdio.h>
#include <stdlib.h>
#include "analyzer.h"

void displayMenu(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("             ICC ODI PLAYER PERFORMANCE ANALYZER\n");
    printf("============================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display All Players of a Specific Role Across All Teams\n");
    printf("6. Exit\n");
    printf("============================================================\n");
}

int main(void)
{
    int choice;
    int programRunning = ONE_VALUE;

    initializeTeams();
    sortTeamsById();
    loadInitialPlayers();

    while (programRunning)
    {
        displayMenu();

        if (!readInteger("Enter your choice: ", &choice))
            continue;

        switch (choice)
        {
            case 1:
                addPlayerToSpecificTeam();
                break;
            case 2:
                displayPlayersOfSpecificTeam();
                break;
            case 3:
                displayTeamsByAverageStrikeRate();
                break;
            case 4:
                displayTopKPlayersOfTeam();
                break;
            case 5:
                displayAllPlayersOfRole();
                break;
            case 6:
                programRunning = ZERO_VALUE;
                break;
            default:
                printf("Invalid menu choice.\n");
        }
    }

    freeMemory();
    printf("Thank you for using ICC ODI Player Performance Analyzer.\n");

    return EXIT_SUCCESS;
}
