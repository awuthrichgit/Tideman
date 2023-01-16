#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

bool check_rows(int column);
int check_loser(int loser, int winner);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for(int i = 0; i < candidate_count; i++){
        if(strcmp(name, candidates[i]) == 0){
            //candidates are encoded in ranks by their index
            //value that corresponds to candidates[i] array
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    //iterate through ranks[]
    for(int i = 0; i < candidate_count; i++)
    {
        //iterate through ranks[] after i
        for(int j = i+1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]] += 1;
        }
    }

    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    //iterate through each row of preferences
    for(int i = 0; i < candidate_count; i++)
    {
        //iterate through each column for a row
        for(int j = 0; j < candidate_count; j++)
        {
            //check if the i candidate beat the j candidate
            if(preferences[i][j] > preferences[j][i])
            {
                //update pairs with winner/loser and move to next index
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pair_count+=1;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    //struct to bundle margin of victory for each candidate and
    //their position in pairs array
    typedef struct
    {
        int winSize;
        int originalPosition;
    }
    win_margin;

    //array to store win_margins
    win_margin tally[MAX * (MAX - 1) / 2];

    //populate win_margin struct with data by iteration over pairs[]
    for(int i = 0; i < pair_count; i++)
    {
        tally[i].originalPosition = i;
        tally[i].winSize = preferences[pairs[i].winner][pairs[i].loser] - preferences[pairs[i].loser][pairs[i].winner];
    }

    //sort tally by win_size -> use that to sort pairs array.
    bool sorted = false;
    do
    {
        //perform bubble sort and check if sorted
        int swaps = 0;
        for(int i = 0; i < pair_count-1; i++)
        {

            if(tally[i].winSize < tally[i+1].winSize)
            {
                int tempWin = 0;
                int tempPos = 0;
                tempWin = tally[i].winSize;
                tempPos = tally[i].originalPosition;

                tally[i].winSize = tally[i+1].winSize;
                tally[i].originalPosition = tally[i+1].originalPosition;

                tally[i+1].winSize = tempWin;
                tally[i+1].originalPosition = tempPos;
                swaps++;
            }
        }
        if(swaps == 0)
        {
            sorted = true;
        }
    } while(!sorted);

    //we have sorted tally. sort pairs
    //copy pairs into another array
    pair copy[MAX * (MAX - 1) / 2];
    for(int i = 0; i < pair_count; i++)
    {
        copy[i] = pairs[i];
    }
    //select from copy the pairs values with the greatest difference first based off sorting
    for(int i = 0; i < pair_count; i++)
    {
        pairs[i] = copy[tally[i].originalPosition];
    }

    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{

    //iterate through pairs[]
    for(int i = 0; i < pair_count; i++)
    {
        //select loser of pair
        //check loser's row of locked to see if they're winning against anyone
        //if they are then do the before line again
        //if at any point, the loser you're checking is the winner of the original pair, return false bc that's a cycle!

        if(check_loser(pairs[i].loser, pairs[i].winner) != -2)
        {
            //add a directed edge from the winner to the loser
            locked[pairs[i].winner][pairs[i].loser] = true;
        }

    }


    return;
}

//Check loser function
int check_loser(int loser, int winner)
{
    /*
    Checks if a directed edge exists between two nodes on the graph of locked-pairs
    Param loser: integer representing the loser node to check if they're winning against anyone
    Param winner: integer representing the winner in the original pair of nodes checked
    Returns: int representing the candidate that the loser won against
    */

   if(loser == winner)
   {
    //reveals a cycle where the loser is the winner
        return -2;
   }

    //iterate through locked[][] loser row
    for(int i = 0; i < candidate_count; i++)
    {
    //check if loser currently wins against another node
        if(locked[loser][i] == 1)
        {
             //call check_loser on that loser
             if(check_loser(i, winner) == -2)
             {
                return -2;
             }
        }
     }
    //if wins against no one else
    return loser;

}

// Print the winner of the election
void print_winner(void)
{
    //iterate over the columns of locked[][] and select column with no 1's
    for(int i = 0; i < candidate_count; i++)
    {
        //check rows for each column
        if(check_rows(i))
        {
            printf("%s\n", candidates[i]);
            return;
        }
    }
    return;
}

bool check_rows(int column)
{
    //iterate through the rows in column
        for(int j = 0; j < candidate_count; j++)
        {
            //column remains constant, row changes
            if(locked[j][column] == true)
            {
                return false;
            }
        }
        return true;
}