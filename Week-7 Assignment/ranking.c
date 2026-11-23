#include <stdio.h>
#include "analyzer.h"

static void displayPlayerHeader(void)
{
    printf("\n%-8s %-25s %-13s %-9s %-9s %-9s %-8s %-8s %-10s\n",
           "ID", "Name", "Role", "Runs", "Average",
           "SR", "Wickets", "Economy", "Perf.Index");
    printf("-----------------------------------------------------------------------------------------------\n");
}

static void displayPlayer(const ODIPlayer *player)
{
    printf("%-8d %-25s %-13s %-9d %-9.2f %-9.2f %-8d %-8.2f %-10.2f\n",
           player->playerId, player->name, getRoleName(player->role),
           player->totalRuns, player->battingAverage, player->strikeRate,
           player->wickets, player->economyRate, player->performanceIndex);
}

static int readRole(int *role)
{
    if (!readInteger("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ",
                     role))
        return ZERO_VALUE;

    if (*role < BATSMAN || *role > ALL_ROUNDER)
    {
        printf("Invalid role.\n");
        return ZERO_VALUE;
    }

    return ONE_VALUE;
}

void displayPlayersOfSpecificTeam(void)
{
    int teamId;
    int eligiblePlayerCount = ZERO_VALUE;
    float totalStrikeRate = 0.0f;
    PlayerNode *currentPlayer;
    Team *team;

    if (!readInteger("Enter Team ID: ", &teamId))
        return;

    team = searchTeamById(teamId);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    printf("\nPlayers of Team %s\n", team->name);
    displayPlayerHeader();
    currentPlayer = team->players;

    while (currentPlayer != NULL)
    {
        displayPlayer(&currentPlayer->player);

        if (currentPlayer->player.role == BATSMAN ||
            currentPlayer->player.role == ALL_ROUNDER)
        {
            totalStrikeRate += currentPlayer->player.strikeRate;
            eligiblePlayerCount++;
        }

        currentPlayer = currentPlayer->next;
    }

    printf("\nTotal Players: %d\n", team->totalPlayers);

    if (eligiblePlayerCount > ZERO_VALUE)
        printf("Average Batting Strike Rate: %.2f\n",
               totalStrikeRate / eligiblePlayerCount);
    else
        printf("No eligible batting players found.\n");
}

void displayTopKPlayersOfTeam(void)
{
    int teamId;
    int role;
    int requestedCount;
    int displayedCount = ZERO_VALUE;
    Team *team;
    RoleNode *currentNode;

    if (!readInteger("Enter Team ID: ", &teamId))
        return;

    team = searchTeamById(teamId);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    if (!readRole(&role))
        return;

    if (!readInteger("Enter number of players (K): ", &requestedCount))
        return;

    if (requestedCount < MIN_K_VALUE)
    {
        printf("K must be positive.\n");
        return;
    }

    currentNode = team->roleHeads[getRoleIndex(role)];

    printf("\nTop %d %s Players of Team %s\n",
           requestedCount, getRoleName(role), team->name);
    displayPlayerHeader();

    while (currentNode != NULL && displayedCount < requestedCount)
    {
        displayPlayer(&currentNode->playerNode->player);
        displayedCount++;
        currentNode = currentNode->next;
    }

    if (displayedCount == ZERO_VALUE)
        printf("No players found for this role.\n");
}

static int hasHigherHeapPriority(const HeapNode *first,
                                 const HeapNode *second)
{
    return playerRanksBefore(&first->current->playerNode->player,
                             &second->current->playerNode->player);
}

static void swapHeapNodes(HeapNode *first, HeapNode *second)
{
    HeapNode temporaryNode = *first;
    *first = *second;
    *second = temporaryNode;
}

static void heapifyDown(HeapNode heap[], int heapSize, int parentIndex)
{
    int highestIndex = parentIndex;
    int leftChildIndex = (parentIndex * TWO_VALUE) + ONE_VALUE;
    int rightChildIndex = (parentIndex * TWO_VALUE) + TWO_VALUE;

    if (leftChildIndex < heapSize &&
        hasHigherHeapPriority(&heap[leftChildIndex], &heap[highestIndex]))
        highestIndex = leftChildIndex;

    if (rightChildIndex < heapSize &&
        hasHigherHeapPriority(&heap[rightChildIndex], &heap[highestIndex]))
        highestIndex = rightChildIndex;

    if (highestIndex != parentIndex)
    {
        swapHeapNodes(&heap[parentIndex], &heap[highestIndex]);
        heapifyDown(heap, heapSize, highestIndex);
    }
}

static void insertHeapNode(HeapNode heap[], int *heapSize,
                           RoleNode *roleNode)
{
    int currentIndex = *heapSize;

    heap[currentIndex].current = roleNode;
    (*heapSize)++;

    while (currentIndex > ZERO_VALUE)
    {
        int parentIndex = (currentIndex - ONE_VALUE) / TWO_VALUE;

        if (hasHigherHeapPriority(&heap[parentIndex], &heap[currentIndex]))
            break;

        swapHeapNodes(&heap[parentIndex], &heap[currentIndex]);
        currentIndex = parentIndex;
    }
}

static void removeHeapRoot(HeapNode heap[], int *heapSize)
{
    if (*heapSize == ZERO_VALUE)
        return;

    (*heapSize)--;

    if (*heapSize > ZERO_VALUE)
    {
        heap[ZERO_VALUE] = heap[*heapSize];
        heapifyDown(heap, *heapSize, ZERO_VALUE);
    }
}

void displayAllPlayersOfRole(void)
{
    int role;
    int heapSize = ZERO_VALUE;
    int teamIndex;
    HeapNode heap[MAX_TEAM_COUNT];

    if (!readRole(&role))
        return;

    for (teamIndex = ZERO_VALUE; teamIndex < totalTeams; teamIndex++)
    {
        RoleNode *roleHead =
            teamRecords[teamIndex].roleHeads[getRoleIndex(role)];

        if (roleHead != NULL)
            insertHeapNode(heap, &heapSize, roleHead);
    }

    printf("\nAll %s Players Across All Teams\n", getRoleName(role));
    displayPlayerHeader();

    while (heapSize > ZERO_VALUE)
    {
        RoleNode *currentRoleNode = heap[ZERO_VALUE].current;

        displayPlayer(&currentRoleNode->playerNode->player);

        if (currentRoleNode->next != NULL)
        {
            heap[ZERO_VALUE].current = currentRoleNode->next;
            heapifyDown(heap, heapSize, ZERO_VALUE);
        }
        else
            removeHeapRoot(heap, &heapSize);
    }
}
