#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyzer.h"
#include "Players_data.h"

int getRoleIndex(int role)
{
    return role - ONE_VALUE;
}

const char *getRoleName(int role)
{
    if (role == BATSMAN)
        return "Batsman";
    if (role == BOWLER)
        return "Bowler";
    if (role == ALL_ROUNDER)
        return "All-rounder";

    return "Unknown";
}

int getRoleFromName(const char *roleName)
{
    if (strcmp(roleName, "Batsman") == ZERO_VALUE)
        return BATSMAN;
    if (strcmp(roleName, "Bowler") == ZERO_VALUE)
        return BOWLER;
    if (strcmp(roleName, "All-rounder") == ZERO_VALUE)
        return ALL_ROUNDER;

    return ZERO_VALUE;
}

float calculatePerformanceIndex(const ODIPlayer *player)
{
    if (player->role == BATSMAN)
        return (player->battingAverage * player->strikeRate) /
               HUNDRED_VALUE;

    if (player->role == BOWLER)
        return (player->wickets * WICKET_WEIGHT) +
               (HUNDRED_VALUE - player->economyRate);

    return ((player->battingAverage * player->strikeRate) /
            HUNDRED_VALUE) + (player->wickets * WICKET_WEIGHT);
}

int playerRanksBefore(const ODIPlayer *first, const ODIPlayer *second)
{
    if (first->performanceIndex - second->performanceIndex >
        FLOAT_TOLERANCE)
        return ONE_VALUE;

    if (second->performanceIndex - first->performanceIndex >
        FLOAT_TOLERANCE)
        return ZERO_VALUE;

    return first->playerId < second->playerId;
}

int isDuplicatePlayerId(int playerId)
{
    int teamIndex;
    PlayerNode *currentPlayer;

    for (teamIndex = ZERO_VALUE; teamIndex < totalTeams; teamIndex++)
    {
        currentPlayer = teamRecords[teamIndex].players;

        while (currentPlayer != NULL)
        {
            if (currentPlayer->player.playerId == playerId)
                return ONE_VALUE;

            currentPlayer = currentPlayer->next;
        }
    }

    return ZERO_VALUE;
}

static int insertPlayerInRoleList(Team *team, PlayerNode *playerNode)
{
    int roleIndex = getRoleIndex(playerNode->player.role);
    RoleNode **head = &team->roleHeads[roleIndex];
    RoleNode *newNode = malloc(sizeof(RoleNode));
    RoleNode *current;

    if (newNode == NULL)
        return ZERO_VALUE;

    newNode->playerNode = playerNode;
    newNode->next = NULL;

    if (*head == NULL ||
        playerRanksBefore(&playerNode->player,
                          &(*head)->playerNode->player))
    {
        newNode->next = *head;
        *head = newNode;
        return ONE_VALUE;
    }

    current = *head;

    while (current->next != NULL &&
           !playerRanksBefore(&playerNode->player,
                              &current->next->playerNode->player))
        current = current->next;

    newNode->next = current->next;
    current->next = newNode;

    return ONE_VALUE;
}

int addPlayer(Team *team, ODIPlayer player)
{
    PlayerNode *newNode;
    PlayerNode *current;

    if (team == NULL ||
        team->totalPlayers >= MAX_PLAYERS_PER_TEAM ||
        player.playerId < MIN_PLAYER_ID ||
        isDuplicatePlayerId(player.playerId))
        return ZERO_VALUE;

    player.performanceIndex = calculatePerformanceIndex(&player);
    newNode = malloc(sizeof(PlayerNode));

    if (newNode == NULL)
        return ZERO_VALUE;

    newNode->player = player;
    newNode->next = NULL;

    if (!insertPlayerInRoleList(team, newNode))
    {
        free(newNode);
        return ZERO_VALUE;
    }

    if (team->players == NULL)
        team->players = newNode;
    else
    {
        current = team->players;

        while (current->next != NULL)
            current = current->next;

        current->next = newNode;
    }

    team->totalPlayers++;
    return ONE_VALUE;
}

void loadInitialPlayers(void)
{
    int index;
    int loadedCount = ZERO_VALUE;
    ODIPlayer player;
    Team *team;
    int role;

    for (index = ZERO_VALUE; index < playerCount; index++)
    {
        team = searchTeamById(getTeamId(players[index].team));
        role = getRoleFromName(players[index].role);

        if (team == NULL || role == ZERO_VALUE)
        {
            printf("Invalid initial record at index %d.\n", index);
            continue;
        }

        memset(&player, ZERO_VALUE, sizeof(player));
        player.playerId = players[index].id;

        snprintf(player.name, sizeof(player.name), "%s",
                 players[index].name);
        snprintf(player.teamName, sizeof(player.teamName), "%s",
                 players[index].team);

        player.role = role;
        player.totalRuns = players[index].totalRuns;
        player.battingAverage = players[index].battingAverage;
        player.strikeRate = players[index].strikeRate;
        player.wickets = players[index].wickets;
        player.economyRate = players[index].economyRate;

        if (addPlayer(team, player))
            loadedCount++;
        else
            printf("Could not load player ID %d.\n", player.playerId);
    }

    printf("%d initial player records loaded.\n", loadedCount);
}

void addPlayerToSpecificTeam(void)
{
    int teamId;
    int role;
    ODIPlayer player;
    Team *team;

    memset(&player, ZERO_VALUE, sizeof(player));

    if (!readInteger("Enter Team ID: ", &teamId))
        return;

    team = searchTeamById(teamId);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("Team has reached its maximum player limit.\n");
        return;
    }

    if (!readInteger("Enter Player ID: ", &player.playerId))
        return;

    if (player.playerId < MIN_PLAYER_ID ||
        isDuplicatePlayerId(player.playerId))
    {
        printf("Player ID must be positive and unique.\n");
        return;
    }

    if (!readString("Enter Player Name: ", player.name,
                    sizeof(player.name)))
        return;

    if (!readInteger("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ",
                     &role))
        return;

    if (role < BATSMAN || role > ALL_ROUNDER)
    {
        printf("Invalid role.\n");
        return;
    }

    player.role = role;

    if (!readNonNegativeInteger("Enter Total Runs: ", &player.totalRuns))
        return;

    if (!readFloat("Enter Batting Average: ", &player.battingAverage))
        return;

    if (!readFloat("Enter Strike Rate: ", &player.strikeRate))
        return;

    if (!readNonNegativeInteger("Enter Wickets: ", &player.wickets))
        return;

    if (!readFloat("Enter Economy Rate: ", &player.economyRate))
        return;

    snprintf(player.teamName, sizeof(player.teamName), "%s", team->name);

    if (addPlayer(team, player))
        printf("Player added successfully to Team %s.\n", team->name);
    else
        printf("Unable to add player. Check team limit and memory.\n");
}

void freeMemory(void)
{
    int teamIndex;
    int roleIndex;
    PlayerNode *currentPlayer;
    PlayerNode *nextPlayer;
    RoleNode *currentRole;
    RoleNode *nextRole;

    for (teamIndex = ZERO_VALUE; teamIndex < totalTeams; teamIndex++)
    {
        for (roleIndex = ZERO_VALUE; roleIndex < ROLE_COUNT; roleIndex++)
        {
            currentRole = teamRecords[teamIndex].roleHeads[roleIndex];

            while (currentRole != NULL)
            {
                nextRole = currentRole->next;
                free(currentRole);
                currentRole = nextRole;
            }

            teamRecords[teamIndex].roleHeads[roleIndex] = NULL;
        }

        currentPlayer = teamRecords[teamIndex].players;

        while (currentPlayer != NULL)
        {
            nextPlayer = currentPlayer->next;
            free(currentPlayer);
            currentPlayer = nextPlayer;
        }

        teamRecords[teamIndex].players = NULL;
        teamRecords[teamIndex].totalPlayers = ZERO_VALUE;
    }
}
