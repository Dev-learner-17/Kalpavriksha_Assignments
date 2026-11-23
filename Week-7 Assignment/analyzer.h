#ifndef ANALYZER_H
#define ANALYZER_H

#include <stddef.h>

#define MAX_TEAM_COUNT 10
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH 50
#define INPUT_BUFFER_SIZE 128
#define ROLE_COUNT 3

#define BATSMAN 1
#define BOWLER 2
#define ALL_ROUNDER 3

#define FIRST_TEAM_ID 1
#define LAST_TEAM_ID 10
#define MIN_PLAYER_ID 1
#define MIN_K_VALUE 1

#define ZERO_VALUE 0
#define ONE_VALUE 1
#define TWO_VALUE 2
#define THREE_VALUE 3
#define HUNDRED_VALUE 100.0f
#define WICKET_WEIGHT 2.0f
#define FLOAT_TOLERANCE 0.0001f

typedef struct
{
    int playerId;
    char name[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    int role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} ODIPlayer;

typedef struct PlayerNode
{
    ODIPlayer player;
    struct PlayerNode *next;
} PlayerNode;

typedef struct RoleNode
{
    PlayerNode *playerNode;
    struct RoleNode *next;
} RoleNode;

typedef struct
{
    int teamId;
    char name[MAX_NAME_LENGTH + 1];
    PlayerNode *players;
    RoleNode *roleHeads[ROLE_COUNT];
    int totalPlayers;
} Team;

typedef struct
{
    RoleNode *current;
} HeapNode;

extern Team teamRecords[MAX_TEAM_COUNT];
extern int totalTeams;

int readInteger(const char *prompt, int *value);
int readNonNegativeInteger(const char *prompt, int *value);
int readFloat(const char *prompt, float *value);
int readString(const char *prompt, char *value, size_t capacity);

int getTeamId(const char *teamName);
int getRoleIndex(int role);
const char *getRoleName(int role);
int getRoleFromName(const char *roleName);
float calculatePerformanceIndex(const ODIPlayer *player);
int playerRanksBefore(const ODIPlayer *first, const ODIPlayer *second);

void initializeTeams(void);
void sortTeamsById(void);
Team *searchTeamById(int teamId);
int isDuplicatePlayerId(int playerId);
int addPlayer(Team *team, ODIPlayer player);
void loadInitialPlayers(void);
void freeMemory(void);

float calculateTeamAverageStrikeRate(const Team *team);
void displayPlayersOfSpecificTeam(void);
void displayTeamsByAverageStrikeRate(void);
void displayTopKPlayersOfTeam(void);
void displayAllPlayersOfRole(void);
void addPlayerToSpecificTeam(void);
void displayMenu(void);

#endif
