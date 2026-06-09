#ifndef STANCEBEAM_ANALYTICS_H
#define STANCEBEAM_ANALYTICS_H
#define DATAROWS 130
#define DATACOLUMN 4
#define RIGHT 0
#define LEFT 1

int setSensorDataForOrientation(double euler[], double mag[], double mag_max_min[]);

void setSensorData(double acc[], double gyro[], double euler[], double mag[], double mag_max_min[]);

int isTappingDetected(double gyroX[]);

int isSwingDetected(double gyroX[], double gyroDRange);

int calculateMaxIndex(double gyroX[]);

int calculateDownSwingIndex(double gyroX[], int max_index);

int calculateFollowThroughIndex(double gyroX[], int max_index);

int calculateLastNegativeIndex(double gyroY[], int max_or_hit_index);

int calculateFirstNegativeIndex(double gyroY[], int max_or_hit_index);

void calculateIndex(double gyroX[]);

double calculateBackLift(double Pitch[], int backlif_start_index, int downswing_start_index,
                         int shot_Type);

void calculateSpeed(double gyroData[DATAROWS], double sweetSpot);

double calculateMISpeed(int max_or_hit_index);

void calculatePower(double accData[DATAROWS], double gyroData[DATAROWS], double weightofbat,
                    double sweetSpot, int dpr);

double calculateMIPower(int max_or_hit_index);

// NEW POWER WITHOUT TIME DEPENDENCY
double calculateMIPowerWithOutTime(int max_or_impact_index, double weightofbat, double sweetSpot);

double calculateRotationX(double gyro[DATAROWS], int start_index, int last_index);

double calculateRotationY(double gyro[DATAROWS], int start_index, int last_index);

double calculateRotationZ(double gyro[DATAROWS], int start_index, int last_index);

void calculatePosition(double accData[DATAROWS]);

int shotAnalysis(double gyroData[DATAROWS], double eulerAngles[DATAROWS], double orientation_App,
                 double orientation_Ana,
                 int downswing_start_index, int max_or_impact_index, int followthrogh_end_index,
                 int userBattingHand, int firmware_Version);

double calculateDownSwing(double gyroData[DATAROWS], double Pitch[], double backlift, int shot_type,
                          int downswing_start_index, int max_or_hit_index);

double
calculateFollowThrough(double gyroData[DATAROWS], double gyroY[], double Pitch[], double backlift,
                       double downswing, int shottype, int max_or_hit_index,
                       int followthrough_end_index, int batting_hand);

//    int       calculateImpactIndex (double  gyroX[] , double gyroY[] ,double gyroZ[], int max_index);
int calculateImpactIndex(double gyroX[], double gyroY[], double gyroZ[], int max_index,
                         double maxSpeed);

// double    calculateEfficiency ( int max_index ,int impact_index );
double calculateEfficiency(int max_index, int impact_index, double maxspeed, double impactspeed);

int kinematics(double acc[], double gyro[], double angle[], double mag[], double mag_max_min[]);

int Orientation(double euler[], double mag[], double mag_max_min[]);

double calculateTimeToImpact(double Time[], int downswing_start_index, int impact_index);

int calculateBackLiftSIndex(double gyroX[], int downswing_start_index);

double calculateBackLiftDirection_Ana(double orientation_Ana,
                                      int backlift_start_index_or_max_or_impact_index,
                                      int userBattingHand);

double calculateBackLiftDirection_App(double orientation_App,
                                      int backlift_start_index_or_max_or_impact_index,
                                      int userBattingHand);

double calculateBackLiftDirectionAverage_Ana(double orientation_Ana,
                                             int backlift_start_index_or_max_or_impact_index,
                                             int userBattingHand);

double calculateBackLiftDirectionAverage_App(double orientation_App,
                                             int backlift_start_index_or_max_or_impact_index,
                                             int userBattingHand);

double calculateBatDirectionforShotType_App(double orientation_App,
                                            int backlift_start_index_or_max_or_impact_index,
                                            int userBattingHand);

double calculateBatDirectionforShotType_Ana(double orientation_Ana,
                                            int backlift_start_index_or_max_or_impact_index,
                                            int userBattingHand);

double calculateBatDirectionforShotType(double orientation_App, double orientation_Ana,
                                        int backlift_start_index_or_max_or_impact_index,
                                        int userBattingHand, int type_of_Yaw);

int calculateDiffInAEYaw(int max_or_impact_index);

double calculateBatDirectionforShotTypeAverage(double orientation_App, double orientation_Ana,
                                               int max_or_impact_index,
                                               int userBattingHand);

double calculateBatDirectionforShotTypeAverage_App(double orientation_App,
                                                   int backlift_start_index_or_max_or_impact_index,
                                                   int userBattingHand);

double calculateBatDirectionforShotTypeAverage_Ana(double orientation_Ana,
                                                   int backlift_start_index_or_max_or_impact_index,
                                                   int userBattingHand);

double calculateBatFace(double angle[], int backlift_start_index, int batting_hand, double backlift,
                        double backliftDirection);

//    void initPlayer(double batWeight , double batLength ,int gripPosition, double orientation_App, double orientation_Ana , int userHand,int firmwareVersion,int mode);
//    void initPlayer(double batWeight , double batLength ,int gripPosition, double orientation_App, double orientation_Ana , int userHand,int firmwareVersion);
void initPlayer(double batWeight, double batLength, int gripPosition, double orientation_App,
                double orientation_Ana, int userHand, int firmwareVersion, double gryoXThershold,
                int dpr);

int calculateWristBreak(double gyroY[], int downswing_start_index, int followthrough_end_index);

int calculateBallImpactAtBat(int impactIndex);

double calculateBallImpactAtBat_Range_Gyro(int impactIndex, double arr[]);

double calculateBallImpactAtBat_Range_Acc(int impactIndex, double arr[]);

int calculateImpactIndex2(double gyroX[], double gyroY[], double gyroZ[], int max_index);

void calculateUnityData(double backImpactDirApp, double batImpactDirAna, double backliftdirection,
                        double orientation_app, double orientation_ana, int max_or_impact_index,
                        int batting_hand, int firmware_version);

void calculataYawWithTiltCompensation(double Roll_tilt[], double Pitch_tilt[], double Yaw_tilt[],
                                      double mag_X[],
                                      double mag_Y[], double mag_Z[]);

double maxRepeating(double arr[]);

int calculateIndexForYg(double gyroData[], int start_index, int last_index);

int calculateIndexArray(double gyroData[], int indexarr[]);

void calculateRotationXUnity(double XrotUni[DATAROWS], int start_index, int end_index);

void calculateRotationYUnity(double YrotUni[DATAROWS], int start_index, int end_index);

void calculateRotationZUnity(double ZrotUni[DATAROWS], int start_index, int end_index);

void calculateRotationXarrUnity(double XrotArr[DATAROWS]);

void calculateRotationYarrUnity(double YrotArr[DATAROWS], int Ylimitindex);

void calculateRotationZarrUnity(double ZrotArr[DATAROWS], int Zlimitindex);

void calculateAverageAndInsert(double inset_arr[], double data_arr[]);

void calculateAverageAndInsertArray();

int calculateFirstNegativeYgInBack();

void calculateUnityDataPosition(double orientation, int userbatting_hand, int max_or_impactindex,
                                int stop_or_not);

void calculate_five_AverageAndInsert(double insert_arr[], double data_arr[]);

void calculateDiffAndSmooth(double Out_Array[260], double In_Array[260]);

int calculateWristBreakForUnity(double orientation, int userbatting_hand, int max_or_impactindex);

int calculateLastIndexforUnityAfterWristBreak(int wristbreakindex);

void calculateUnityZpositionFilterData(int shot_type);

void calculateUnityZpositionFilterDataTo2(int shot_type, int user_hand);

int calculateStartofZminus2();

double calculateLengthofSweetSpot(double batLength, int gripPostion);

// back lift direction test code
double calculataYawWithTiltCompensation_test(double Roll_tilt, double Pitch_tilt, double mag_X,
                                             double mag_Y,
                                             double mag_Z);

double calculateBackLiftDirectionAverage_Ana_test(double orientation_Ana,
                                                  int backlift_start_index_or_max_or_impact_index,
                                                  int userBattingHand);

double calculateBackLiftDirection_Ana_test(double orientation_Ana,
                                           int backlift_start_index_or_max_or_impact_index,
                                           int userBattingHand);

// impact direction test code
double calculateBatDirectionforShotType_Ana_Test(double orientation_Ana, double pitch,
                                                 int max_or_impact_index,
                                                 int userBattingHand, int firmware_Version);

double
calculateBatDirectionforShotTypeAverage_Ana_Test_R(double orientation_Ana, int max_or_impact_index,
                                                   int userBattingHand, int firmvare_Version);

double
calculateBatDirectionforShotTypeAverage_Ana_Test_90(double orientation_Ana, int max_or_impact_index,
                                                    int userBattingHand, int firmware_version);

// data for unity is correct or not
int calculateDataForUnityCorrectOrNotForVerticalShots(int shot_type, double Yaw_Unity[],
                                                      double Pitch_Unity[],
                                                      double backliftdirection,
                                                      double downswingdirection, int wristbreak,
                                                      int max_or_impact_index);

// Pitch Value for next index should high in unity
void
calculatePitchFilterNextValueHigh(double insert_arr[], double data_arr[], int max_or_impact_index);

// this function is for get analytics version
void getAnalytics_Version();

// this function is for improve unity Raw values
void calculte_Roll_Array_Unity(double out_arr[], double inp_arr[], int shot_type,
                               int max_or_impactindex);

// this function is to wheather this is impact or not
int calculate_Stop_or_Not(double acc[], double gyro[], double angle[], double orientation_app,
                          double orientation_ana, int battinghand, int maxindex,
                          int firmware_version);

// change straight to on drive
int change_stright_to_on_drive(int shot_type, int impact_index, int wrist_break,
                               double batimpactirection_ana);

// shottype for Pull and hook in case of horizontal shot type only
int shot_analysis_horizontal_pull_or_hook(int vertical_or_horizontal, double Xrotationarray[],
                                          double Yrotationarry[], int downswing_start_index,
                                          int max_or_impact_index, double backliftdirection);

int
shot_analysis_horizontal_cut_pull_hook_using_new_logic(double backliftDirection_at_downswing_starts,
                                                       double orientation_App,
                                                       int max_or_impact_index, int batting_hand);

// horizontal shots coming as vertical bat shots
int horizontal_as_vertical(int horizontal_or_vertical, int downswing_start_index,
                           int max_or_impact_index);

// for my testing in cython
struct shotResult *getStruct(int a);

// firmware changes
double calculateBackLiftDirection_App_Test_NewFirmware(double orientation_App,
                                                       int backlift_start_index_or_max_or_impact_index,
                                                       int userBattingHand);

double calculateBackLiftDirection_App_Test_NewFirmware_Avg(double orientation_App,
                                                           int backlift_start_index_or_max_or_impact_index,
                                                           int userBattingHand);

// for horizontal bat shots only new logic
int getIndexForRollInRange(double arr[], int max_or_impact_index);

// code improvement
double
calculateDirectionForShotType(int batting_hand, double orientation_app_or_ana, double orientation);

double calculateDirectionForBackliftDirection(int batting_hand, double orientation_app_or_ana,
                                              double orientation);

//
double factorForSpeed(double gyroX[], int downswing_start_index, int max_or_impact_index,
                      int followthrough_end_index, double sweetSpot);

//
double calculateImpactAngle(double roll[], int impact_index, int shot_type);

int lastPositiveIndexPitchDuringDownSwing(double Pitch[], int downswing_start_index, int max_index,
                                          int follow_end_index, int shottype, int wrist_break);

double calculateDownswingDirection_App_Test_NewFirmware(double orientation_App,
                                                        int downswing_start_index_or_max_or_impact_index,
                                                        int userBattingHand);

double directionOfBatAtAnyPoint(double Pitch_Orientation, int Bating_Hand, int backlift_start_index,
                                int downswing_start_index, int horizontal_vertical, int shot_type,
                                int first_Negative_in_Y);

int indexForHorizontHighPitch(double arr[], int backlift_start_index, int downswing_start_index);

int checkYawForBatMovement(double arr[], int HighPitchindex, int batting_hand,
                           double orientation_app_or_ana);

struct playerInfo *getPlayerInfo();

struct playerInfo {
    double batLength;
    double batWeight;
    int gripPosition;
    double orientation_App;
    double orientation_Ana;
    int userBattingHand;
    int firmware_version;
    int mode;
    double gryoXRange;
    int dpr;
};

struct shotResult *getResult();

static struct shotResult {
    double powerMax;
    double powerImpact;
    double powerAtImpact;
    double Power;
    double Speed;
    double speedAtImpact;
    double Efficiency;
    double Backlift;
    double Downswing;
    double Followthroug;
    double timeToImpact;
    double batBackliftDirection;
    double batBackliftDirection_Ana;
    double batBackliftDirection_App;
    double batBackliftDirection_Avg;
    double batBackliftDirection_Test;
    double batbackliftDirection_Test1;
    double batbackliftDirection_Test2;
    double batbackliftDirection_App_NewFirmware_Test;
    double rollAtBacklift_Test;
    double pitchAtBacklift_Test;
    double batdownswingDirection_App_NewFirmware_Test;
    double rollAtDownswing_Test;
    double pitchAtDownswing_Test;
    double batimpactDirection_App_NewFirmware_Test;
    double rollAtImpact_Test;
    double pitchAtImpact_Test;
    double batfollowThroughDirection_App_NewFirmware_Test;
    double rollAtfollowThrough_Test;
    double pitchAtfollowThrough_Test;
    double batImapctDirection;
    double batImapctDirection_Ana;
    double batImapctDirection_App;
    double batImpactDirection_Ana_test_90;
    double batImpactDirection_Ana_test_r;
    double batFace;
    double posX[DATAROWS];
    double posY[DATAROWS];
    double posZ[DATAROWS];
    int maxIndex;
    int impactIndex;
    int backlift_start_index;
    int follow_through_index;
    int unity_last_index;
    int shotType;
    int shotType2;
    int wrist_break_index;
    int vertical_or_horizontal_shot;
    int g_y_negative;
    int g_y_negative_greater;
    double ball_impact_at_bat;
    double PitchUnity[DATAROWS];
    double YawUnity[DATAROWS];
    double RollUnity[DATAROWS];
    double Yaw_tilt[DATAROWS];
    double TimeUnityFilter[260];
    double PitchUnityFilter[260];
    double YawUnityFilter[260];
    double RollUnityFilter[260];
    double posXFilter[260];
    double posYFilter[260];
    double posZFilter[260];
    double Xrotation_validation[130];
    double Yrotation_validation[130];
    double Zrotation_validation[130];
    int unity_data_correct_ornot;
    double Yawat90PitchatDownswing;
    double YawatRealPitchatDownswing;
    double Yaw_tilt_90[DATAROWS];
    double Yaw_tilt_R[DATAROWS];
    double Yaw_tilt_90_R[DATAROWS];
    double YRota_at_impact;
    double Rota_at_downswing;
    double Rota_at_max_or_impact;
    double F_Xg;
    double F_Yg;
    double F_Zg;
    double F_Xa;
    double F_Ya;
    double F_Za;
    double Roll_at_start_backlift;  // for testing purpose
    double Pitch_at_start_backlift; // for testing purpose
    double impact_Angle;
    double impact_Direction;
    double impact_GroundedAir;
    double impact_Pitch;
    double downswing_Direction;

    // Impact R&D
    int impactBucket;
    double DiffXg1;
    double DiffXg2;
    double UpXg3;
    double DiffYa;
    double DiffXa;
    double DiffZa;
    double DiffZg;
    double DiffYg;
} shot;

struct orientationResult *getOrientationResult();

struct orientationResult {
    double Orientation_App;
    double Orientation_Ana;
};

struct codeVersion *getCodeVersion();

struct codeVersion {
    char version[10];
};

#endif // STANCEBEAM_ANALYTICS_H