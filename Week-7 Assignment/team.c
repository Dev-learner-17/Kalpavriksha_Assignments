#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyzer.h"
#include "Players_data.h"

Team teamRecords[MAX_TEAM_COUNT];
int totalTeams = ZERO_VALUE;

int getTeamId(const char *teamName)
{
    if (strcmp(teamName, "India") == ZERO_VALUE)
        return FIRST_TEAM_ID;
    if (strcmp(teamName, "Australia") == ZERO_VALUE)
        return 2;
    if (strcmp(teamName, "England") == ZERO_VALUE)
        return 3;
    if (strcmp(teamName, "Pakistan") == ZERO_VALUE)
        return 4;
    if (strcmp(teamName, "New Zealand") == ZERO_VALUE)
        return 5;
    if (strcmp(teamName, "Bangladesh") == ZERO_VALUE)
        return 6;
    if (strcmp(teamName, "Afghanistan") == ZERO_VALUE)
        return 7;
    if (strcmp(teamName, "West Indies") == ZERO_VALUE)
        return 8;
    if (strcmp(teamName, "South Africa") == ZERO_VALUE)
        return 9;
    if (strcmp(teamName, "Sri Lanka") == ZERO_VALUE)
        return LAST_TEAM_ID;

    return ZERO_VALUE;
}

void initializeTeams(void)
{
    int index;
    int roleIndex;

    totalTeams = teamCount;

    if (totalTeams != MAX_TEAM_COUNT)
    {
        printf("The supplied data must contain exactly %d teams.\n",
               MAX_TEAM_COUNT);
        exit(EXIT_FAILURE);
    }

    for (index = ZERO_VALUE; index < totalTeams; index++)
    {
        teamRecords[index].teamId = getTeamId(teams[index]);
        snprintf(teamRecords[index].name, sizeof(teamRecords[index].name),
                 "%s", teams[index]);
        teamRecords[index].players = NULL;
        teamRecords[index].totalPlayers = ZERO_VALUE;

        for (roleIndex = ZERO_VALUE; roleIndex < ROLE_COUNT; roleIndex++)
            teamRecords[index].roleHeads[roleIndex] = NULL;
    }
}

void sortTeamsById(void)
{
    int firstIndex;
    int secondIndex;

    for (firstIndex = ZERO_VALUE; firstIndex < totalTeams; firstIndex++)
    {
        for (secondIndex = firstIndex + ONE_VALUE;
             secondIndex < totalTeams; secondIndex++)
        {
            if (teamRecords[firstIndex].teamId >
                teamRecords[secondIndex].teamId)
            {
                Team temporaryTeam = teamRecords[firstIndex];
                teamRecords[firstIndex] = teamRecords[secondIndex];
                teamRecords[secondIndex] = temporaryTeam;
            }
        }
    }
}

Team *searchTeamById(int teamId)
{
    int left = ZERO_VALUE;
    int right = totalTeams - ONE_VALUE;

    while (left <= right)
    {
        int middle = left + (right - left) / TWO_VALUE;

        if (teamRecords[middle].teamId == teamId)
            return &teamRecords[middle];

        if (teamRecords[middle].teamId < teamId)
            left = middle + ONE_VALUE;
        else
            right = middle - ONE_VALUE;
    }

    return NULL;
}

float calculateTeamAverageStrikeRate(const Team *team)
{
    PlayerNode *currentPlayer = team->players;
    float totalStrikeRate = 0.0f;
    int eligiblePlayerCount = ZERO_VALUE;

    while (currentPlayer != NULL)
    {
        if (currentPlayer->player.role == BATSMAN ||
            currentPlayer->player.role == ALL_ROUNDER)
        {
            totalStrikeRate += currentPlayer->player.strikeRate;
            eligiblePlayerCount++;
        }

        currentPlayer = currentPlayer->next;
    }

    if (eligiblePlayerCount == ZERO_VALUE)
        return 0.0f;

    return totalStrikeRate / eligiblePlayerCount;
}

void displayTeamsByAverageStrikeRate(void)
{
    Team *sortedTeams[MAX_TEAM_COUNT];
    Team *temporaryTeam;
    int firstIndex;
    int secondIndex;

    for (firstIndex = ZERO_VALUE; firstIndex < totalTeams; firstIndex++)
        sortedTeams[firstIndex] = &teamRecords[firstIndex];

    for (firstIndex = ZERO_VALUE; firstIndex < totalTeams; firstIndex++)
    {
        for (secondIndex = firstIndex + ONE_VALUE;
             secondIndex < totalTeams; secondIndex++)
        {
            if (calculateTeamAverageStrikeRate(sortedTeams[firstIndex]) <
                calculateTeamAverageStrikeRate(sortedTeams[secondIndex]))
            {
                temporaryTeam = sortedTeams[firstIndex];
                sortedTeams[firstIndex] = sortedTeams[secondIndex];
                sortedTeams[secondIndex] = temporaryTeam;
            }
        }
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("---------------------------------------------------------\n");
    printf("%-8s %-20s %-15s %-15s\n",
           "ID", "Team", "Average SR", "Total Players");
    printf("---------------------------------------------------------\n");

    for (firstIndex = ZERO_VALUE; firstIndex < totalTeams; firstIndex++)
    {
        printf("%-8d %-20s %-15.2f %-15d\n",
               sortedTeams[firstIndex]->teamId,
               sortedTeams[firstIndex]->name,
               calculateTeamAverageStrikeRate(sortedTeams[firstIndex]),
               sortedTeams[firstIndex]->totalPlayers);
    }
}
