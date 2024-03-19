typedef enum ResponseType
{
    DISPATCH,
    ON_SCENE
} ResponseType;

typedef struct ResponseTime
{
    ResponseType type;
    int under_2_mins;
    int mins_3_5;
    int mins_6_10;
    int over_10_mins;
} ResponseTime;

typedef struct Subfield
{
    char *name;
    ResponseTime *responseTimes[2];
} Subfield;

typedef struct CallType
{
    char *name;
    int total_count;
    Subfield *call_total;
    Subfield **subfields;
} CallType;
