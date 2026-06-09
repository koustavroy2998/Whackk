#include <cstdio>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <cstring>

#include "analytics.h"
#include <jni.h>

/******************** HERE WE ARE ASSIGNING MEMORY AT RUN TIME FOR STRUCTURES LIKE INITPLAYER,SHOTRESULT **************************/
static struct playerInfo *player = 0;
static struct shotResult *result = 0;
static struct orientationResult *orientation = 0;
static struct codeVersion *version = 0;

struct shotResult *getResult() {
    if (result == nullptr) {
        result = (struct shotResult *) malloc(sizeof(struct shotResult));
    }
    return result;
}

struct orientationResult *getOrientationResult() {
    if (orientation == 0) {
        orientation = (struct orientationResult *) malloc(sizeof(struct orientationResult));
    }
    return orientation;
}

struct codeVersion *getCodeVersion() {
    if (version == 0) {
        version = (struct codeVersion *) malloc(sizeof(struct codeVersion));
    }
    return version;
}

// void initPlayer(double batWeight , double batLength ,int gripPosition, double orientation_App, double orientation_Ana , int userHand,int firmwareVersion,int mode)
void initPlayer(double batWeight, double batLength, int gripPosition, double orientation_App,
                double orientation_Ana, int userHand, int firmwareVersion, double gryoXThershold,
                int dpr) {
    if (player == 0) {
        player = (struct playerInfo *) malloc(sizeof(struct playerInfo));
    }
    player->batLength = (batLength != NAN && batLength >= 64.135 && batLength <= 96.5) ? batLength
                                                                                       : 87.5; // is in centimeter from app
    player->batWeight = (batWeight != NAN && batWeight >= 0.5 && batWeight <= 1.4) ? batWeight
                                                                                   : 0.8;      // is in kg from the app
    player->gripPosition = (gripPosition != NAN && gripPosition >= 0 && gripPosition <= 2)
                           ? gripPosition : 1;
    player->orientation_Ana = (orientation_Ana != NAN && orientation_Ana >= 0 &&
                               orientation_Ana <= 360) ? orientation_Ana : 180;
    player->orientation_App = (orientation_App != NAN && orientation_App >= 0 &&
                               orientation_App <= 360) ? orientation_App : 180;
    player->userBattingHand = (userHand != NAN && userHand >= 0 && userHand <= 180) ? userHand : 1;
    player->firmware_version = (firmwareVersion != NAN && firmwareVersion >= 1 &&
                                firmwareVersion <= 3) ? firmwareVersion : 1;
    player->gryoXRange = (gryoXThershold != NAN && gryoXThershold >= 350 && gryoXThershold <= 1000)
                         ? gryoXThershold : 500;
    player->dpr = (dpr != NAN && dpr >= 50 && dpr <= 100) ? dpr : 50;
    //    player->mode                = (mode != NAN && mode >= 0 && mode <= 3)? mode:0;
}

struct playerInfo *getPlayerInfo(void) {
    return player;
}
/*********************************************************************************************************************************/
/********************************HERE WE ARE DECLARING GLOBAL VARIABLES TO WHICH WE CAN USE THROUGOUT THE PROGRAM *****************/
double Ta[DATAROWS], Tg[DATAROWS], Xa[DATAROWS], Ya[DATAROWS], Za[DATAROWS], Xg[DATAROWS], Yg[DATAROWS], Zg[DATAROWS],
        Pitch[DATAROWS], Roll[DATAROWS], Yaw[DATAROWS], Yaw_T[DATAROWS], Pfac[DATAROWS], Vg[DATAROWS], Va[DATAROWS], Aacc[DATAROWS],
        accData[DATAROWS][DATACOLUMN], gyroData[DATAROWS][DATACOLUMN], angleData[DATAROWS][
        DATACOLUMN - 1], magData[DATAROWS][DATACOLUMN - 1],
        Xpos[DATAROWS], Ypos[DATAROWS], Zpos[DATAROWS], Pitch_change, XaL[DATAROWS], YaL[DATAROWS], ZaL[DATAROWS], PitchX_U[DATAROWS],
        YawY_U[DATAROWS], YawY_U_Avg[DATAROWS], RollZ_U[DATAROWS], angleData_O[DATAROWS][
        DATACOLUMN - 1], magData_O[DATAROWS][DATACOLUMN -
                                             1], Pitch_O_App[DATAROWS], Roll_O_App[DATAROWS], Yaw_O_App[DATAROWS], Yaw_O_Ana[DATAROWS], Mag_X_O[DATAROWS], Mag_Y_O[DATAROWS], Mag_Z_O[DATAROWS], Xm[DATAROWS], Ym[DATAROWS], Zm[DATAROWS], mag_max_X, mag_max_Y, mag_max_Z, mag_min_X, mag_min_Y, mag_min_Z, Xrotation_unity[DATAROWS], Yrotation_unity[DATAROWS], Zrotation_unity[DATAROWS], Time_U_Filter[
        2 * DATAROWS], PitchX_U_Filter[2 * DATAROWS], PitchX_U_Filter_High_Value[
        2 * DATAROWS], RollZ_U_Filter[2 * DATAROWS], YawY_U_Filter[
        2 * DATAROWS], YawY_U_Filter_With_diff[2 * DATAROWS], Xpos_Filter[
        2 * DATAROWS], Ypos_Filter[2 * DATAROWS], Zpos_Filter[2 * DATAROWS], Xpos_Unity_acc[
        2 * DATAROWS], Ypos_Unity_acc[2 * DATAROWS], Zpos_Unity_acc[2 * DATAROWS], Zpos_Unity_acc2[
        2 * DATAROWS], Zpos_Unity_acc3[2 *
                                       DATAROWS], sweetSpotLenth = 0, backliftdirectiontest1 = 0, backliftdirectiontest2 = 0, Yaw_T_90[DATAROWS], Yaw_T_90_R[DATAROWS], Yaw_T_R[DATAROWS], Yrota_at_impact = 0, rota_at_downswing = 0, rota_at_max_or_impact = 0, f_gx = 0, f_gy = 0, f_gz = 0, f_ax = 0, f_ay = 0, f_az = 0;
int i = 0, j = 0, SI = 0, MI = 0, FTI = 0, BSI = 0, II = 0, shotType = 0, gyro_y_negative = 0, gyro_y_negative_greater = 0, Yindex[DATAROWS], Zindex[DATAROWS], YindexLimit = 0, ZindexLimit = 0, SIU = 0, SHV = 0, WBI_UN = 0, PST = 0, ULI = 0, count = 0, SON = 0, IisG = 0;

// New Parameters Added to Improve and Do R&D on Impact Issue
// we have four buckets for impact
int impactBucket = 0;

double DiffXg1 = 0, DiffXg2 = 0, UpXg3 = 0, DiffYa = 0, DiffXa = 0, DiffZa = 0, DiffZg = 0, DiffYg = 0;

// SHV is shot horizontal or vertical , vertical == 1,
// SIU is downswing start for unity
// shotType is just for testing the shots using
// Ta[DATAROWS] is array of time for accelero data
// Tg[DATAROWS] is array of time for gyro data
// Xa[DATAROWS] is array of acceleration in X direction of acclero data
// Xg[DATAROWS] is array of  angular velocity in X direction of gyro data
// Ya[DATAROWS] is array of acceleration in Y direction of accelero data
// Yg[DATAROWS] is array of angular velocity  in Y direction of gyro data
// Za[DATAROWS] is array of acceleration in Z direction of accelero data
// Zg[DATAROWS] is array of angular velocity  in Z direction of gyro data
// Vavg[DATAROWS] is array of  resultant velocity
// Aacc[DATAROWS] is array of  resultant acceleration
// Pfac[DATAROWS] is array of  Power factor
// MI is the index value where maximum rotation in X direction is found using gyro
// SI is  the index value where  down swing start
// FTI is the index till  where  we have to calculate follow-through
// BSI is the index from where backlift starts
/***************************************************************************************************************************/
const char *getfield(char *line, int num) {
    const char *tok;
    for (tok = strtok(line, ",");
         tok && *tok;
         tok = strtok(NULL, ",\n")) {
        if (!--num) {
            return tok;
        }
    }
    return NULL;
}

// int main ()
//{
//     int x,y;
//     static double   mag_max_min[] = {33,90,-13,-51,-6,-88};
//     initPlayer(1.11,87,1,6,21,1,3,400,50);
//     //printf("hey my name is arif ahmad..");
//     //    printf("swingNum,powerFactor,efficiency,backliftAngle,downswingAngle,followthroughAngle,impactAngle,batImpactDirection,impactGroundedOrAir,maxSpeed    ,impactSpeed,maxSpeedIndex,impactSpeedIndex,timeToImpact,swingType,verticalHorizontalShot,backliftDirection,batFace,downswingDirection \n");
//     char buf[100];
//     int file = 1;
//     //     int arr[] = {1,38,42,46,52};
//	printf("swingNum,MaxSpeed,hitindex,impactBucket,DiffXg1,DiffXg2,UpXg3,DiffZg,DiffYg,DiffXa,DiffYa,DiffZa \n");
//     if(file==1)
//     {
//         for(int x = 1; x <13 ; x++)
//         {
//             printf("%d, \t ",x);
//             //snprintf(buf,100 , "/Users/apple/desktop/data/day6/Somya_session2/%d.csv", x);
//             snprintf(buf,100 , "/Users/apple/desktop/data/Impact Issue/KIOC/Ran/140/%d.csv",x);
//			FILE* stream = fopen(buf, "r");
//             i = 0 ;
//             char line[2048];
//             while (fgets(line, 2048, stream))
//             {
//                 char* tmp = strdup(line);
//                 char* tmp1 = strdup(line);
//                 char* tmp2 = strdup(line);
//                 char* tmp3 = strdup(line);
//                 char* tmp4= strdup(line);
//                 char* tmp5 = strdup(line);
//                 char* tmp6 = strdup(line);
//                 char* tmp7 = strdup(line);
//                 char* tmp8 = strdup(line);
//                 char* tmp9 = strdup(line);
//                 char* tmp10 = strdup(line);
//                 char* tmp11 = strdup(line);
//                 char* tmp12 = strdup(line);
//
//
//                 if(i > 1 && i < 132)
//                 {
//////                    //                  for mobile file save
////                                     Ta[i-2] = atof(getfield(tmp, 4));
////                                     Xa[i-2] = atof(getfield(tmp1, 5));
////                                     Ya[i-2] = atof(getfield(tmp2, 6));
////                                     Za[i-2] = atof(getfield(tmp3, 7));
////                                     Xg[i-2] = atof(getfield(tmp4, 9));
////                                     Yg[i-2] = atof(getfield(tmp5, 10));
////                                     Zg[i-2] = atof(getfield(tmp6, 11));
////                                     Pitch[i-2] = atof(getfield(tmp7, 13));
////                                     Roll[i-2] = atof(getfield(tmp8,12));
////                                     Yaw[i-2] = atof(getfield(tmp9,14));
////                                     Xm[i-2] = atof(getfield(tmp10, 26));
////                                     Ym[i-2] = atof(getfield(tmp11, 27));
////                                     Zm[i-2] = atof(getfield(tmp12, 28));
//
//
//                    Ta[i-2] = atof(getfield(tmp, 1));
//                    Xa[i-2] = atof(getfield(tmp1, 2));
//                    Ya[i-2] = atof(getfield(tmp2, 3));
//                    Za[i-2] = atof(getfield(tmp3, 4));
//                    Xg[i-2] = atof(getfield(tmp4, 5));
//                    Yg[i-2] = atof(getfield(tmp5, 6));
//                    Zg[i-2] = atof(getfield(tmp6, 7));
//                    Pitch[i-2] = atof(getfield(tmp7, 8));
//                    Roll[i-2] = atof(getfield(tmp8,9));
//                    Yaw[i-2] = atof(getfield(tmp9,10));
//                    Xm[i-2] = atof(getfield(tmp10, 11));
//                    Ym[i-2] = atof(getfield(tmp11, 12));
//                    Zm[i-2] = atof(getfield(tmp12, 13));
//                    //                 //printf(" %d %f %f %f %f %f %f %f %f %f %f %f %f %f  \n",i-2,Ta[i],Xa[i],Ya[i],Za[i],Xg[i],Yg[i],Zg[i],Pitch[i],Roll[i],Yaw[i],Xm[i],Ym[i],Zm[i]);
//                }
//                i++;
//                // NOTE strtok clobbers tmp
//                free(tmp);
//                free(tmp1);
//                free(tmp2);
//                free(tmp3);
//                free(tmp4);
//                free(tmp5);
//                free(tmp6);
//                free(tmp7);
//                free(tmp8);
//                free(tmp9);
//                free(tmp10);
//                free(tmp11);
//                free(tmp12);
//
//            }
//            double acc[520],gyro[520],euler[390],mag[390];
//            for(i = 0 ; i < 130 ;i++){
//                //printf("%f %f %f %f %f %f %f %f %f %f  \n",Ta[i],Xa[i],Ya[i],Za[i],Xg[i],Yg[i],Zg[i],Roll[i],Pitch[i],Yaw[i]);
//                acc[i] = Ta[i];
//                acc[i+130] = Xa[i];
//                acc[i+260] = Ya[i];
//                acc[i+ 390 ] = Za[i];
//
//                gyro[i] = Ta[i];
//                gyro[i+130] = Xg[i];
//                gyro[i+260] = Yg[i];
//                gyro[i+ 390 ] = Zg[i];
//
//                euler[i] = Yaw[i];
//                euler[i+130] = Pitch[i];
//                euler[i+260] = Roll[i];
//
//                mag[i] = Xm[i];
//                mag[i+130] = Ym[i];
//                mag[i+260] = Zm[i];
//
//            }
//            y = kinematics(acc,gyro,euler,mag,mag_max_min);
//        }
//
//    }
//    else
//    {
//        double acc[] = {};
//        double gyro[] = {};
//        double euler[] ={};
//        double mag[] = {};
//        y = kinematics(acc,gyro,euler,mag,mag_max_min);
//    }
//    getAnalytics_Version();
//    getStruct(1);
//}

// Analytics Version Number
void getAnalytics_Version(void) {
    const char ver[] = "1.1.12";
    struct codeVersion *ourVersion = getCodeVersion();
    strcpy(ourVersion->version, ver);
}
/* ************************************************************* HERE FROM ANDROID APP WE ARE GETTING DATA IN STRING FORMAT ************* */
/*
 As we are getting data from Android App,  accelerometer, one array of X - Y - Z acceleration  respectively.
 gyroscope, one array  X - Y - Z angular Accleration respectively.
 Euler angles, one array  Yaw - Pitch - Roll respectively.
 Magnetometer data, one array of X - Y - Z respectively.
 setSensorData is used in Kinematics function.
 setSensorData is a function to set the sensor data like we are getting raw data from sensor as acceleration , angular
 velocity, and fusion data like raw pitch ,roll,yaw so here we  storing data to array of X acceleration , array of Y
 acceleration , array of Z acceleration, array of X angular velocity, array of Y angular velocity , array of Z angular
 velocity ,array of Pitch , array of roll, Pitch yaw
 */
void
setSensorData(double acc[], double gyro[], double euler[], double mag[], double mag_max_min[]) {
    mag_max_X = mag_max_min[0]; // will improve this part of code later, values of magneto min max should pass
    mag_min_X = mag_max_min[3]; // at the time of user info.
    mag_max_Y = mag_max_min[1];
    mag_min_Y = mag_max_min[4];
    mag_max_Z = mag_max_min[2];
    mag_min_Z = mag_max_min[5];
    for (i = 0; i < DATAROWS; i++) {
        accData[i][0] = acc[i];
        accData[i][1] = acc[i + 130];
        accData[i][2] = acc[i + 260];
        accData[i][3] = acc[i + 390];

        gyroData[i][0] = gyro[i];
        gyroData[i][1] = gyro[i + 130];
        gyroData[i][2] = gyro[i + 260];
        gyroData[i][3] = gyro[i + 390];

        angleData[i][0] = euler[i];
        angleData[i][1] = euler[i + 130];
        angleData[i][2] = euler[i + 260];

        magData[i][0] = mag[i];
        magData[i][1] = mag[i + 130];
        magData[i][2] = mag[i + 260];

        Ta[i] = accData[i][0];
        Xa[i] = accData[i][1];
        Ya[i] = accData[i][2];
        Za[i] = accData[i][3];

        Tg[i] = gyroData[i][0];
        Xg[i] = gyroData[i][1];
        Yg[i] = gyroData[i][2];
        Zg[i] = gyroData[i][3];

        Pitch[i] = angleData[i][1];
        Roll[i] = angleData[i][2];
        Yaw[i] = angleData[i][0];

        Xm[i] = magData[i][0];
        Ym[i] = magData[i][1];
        Zm[i] = magData[i][2];

        // remove gravity factor, or linear acceleration
        XaL[i] = Xa[i] + cos(Pitch[i]) * sin(Roll[i]);
        YaL[i] = Ya[i] + sin(Pitch[i]);
        ZaL[i] = Za[i] - cos(Pitch[i]) * cos(Roll[i]);
        // printf("%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",i,Xa[i],Ya[i],Za[i],Xg[i],Yg[i],Zg[i],Yaw[i],Pitch[i],Roll[i],Xm[i],Ym[i],Zm[i]);
    }
}

/*
 setSensorDataForOrientation is used in Orientation function to calculate orientation with tilt compensated yaw formula
 provided by mythreyi maam.
 */
int setSensorDataForOrientation(double euler[], double mag[], double mag_max_min[]) {
    int count = 0, result = 1;
    mag_max_X = mag_max_min[0];
    mag_min_X = mag_max_min[3];
    mag_max_Y = mag_max_min[1];
    mag_min_Y = mag_max_min[4];
    mag_max_Z = mag_max_min[2];
    mag_min_Z = mag_max_min[5];

    for (i = 0; i < DATAROWS; i++) {
        angleData_O[i][0] = euler[i];
        angleData_O[i][1] = euler[i + 130];
        angleData_O[i][2] = euler[i + 260];
        magData_O[i][0] = mag[i];
        magData_O[i][1] = mag[i + 130];
        magData_O[i][2] = mag[i + 260];
        Pitch_O_App[i] = angleData_O[i][1];
        Roll_O_App[i] = angleData_O[i][2];
        Yaw_O_App[i] = angleData_O[i][0];

        Mag_X_O[i] = magData_O[i][0];
        Mag_Y_O[i] = magData_O[i][1];
        Mag_Z_O[i] = magData_O[i][2];
        if (Mag_X_O[i] == 0 && Mag_Y_O[i] == 0 &&
            Mag_Z_O[i] == 0) // to count whether data is courrpt or not
        {
            count++;
        }
    }
    if (count > 75) {
        result = 0;
    }

    return result;
}

int calculateIndexArray(double gyroData[], int indexarr[]) {
    int startindex = 0, index = 0, indexlimit1 = 1;
    indexarr[0] = 0;
    while (startindex < 129) {
        index = calculateIndexForYg(gyroData, startindex, 130);
        indexarr[indexlimit1] = index;
        indexlimit1++;
        startindex = index;
    }

    return indexlimit1;
}

int calculateIndexForYg(double gyroData[], int start_index, int last_index) {
    int m, index = 0;
    m = start_index;
    if (m < 130) {
        if (gyroData[m] < 0) {
            for (m = start_index; m < last_index; m++) {
                if (gyroData[m] > 0 && gyroData[m + 1] > 0 && gyroData[m + 2] > 0) {
                    index = m;
                    break;
                } else {
                    index = m;
                }
            }
        } else if (gyroData[m] >= 0) {
            for (m = start_index; m < last_index; m++) {
                if (gyroData[m] < 0 && gyroData[m + 1] < 0 && gyroData[m + 2] < 0) {
                    index = m;
                    break;
                } else {
                    index = m;
                }
            }
        }
    }
    return index;
}

int calculateFirstNegativeYgInBack() {
    int index = SI;
    for (i = SI; i > BSI; i--) {
        if (Yg[i] < 0 && Yg[i - 1] < 0 && Yg[i - 2] < 0) {
            index = i;
            break;
        }
    }
    return index;
}

int calculateFirstNegativeYgInBackforTesting() {
    int index = SI;
    for (i = SI; i > 2; i--) {
        if (Yg[i] < 0 && Yg[i - 1] < 0 && Yg[i - 2] < 0) {
            index = i;
            break;
        }
    }
    return index;
}

/*******************************************************************************************************************************/

/***************** TILT COMPENSATION FORMULA IMPLIMENTATION *********************************************************/
void calculataYawWithTiltCompensation(double Roll_tilt[], double Pitch_tilt[], double Yaw_Tilt[],
                                      double mag_X[], double mag_Y[],
                                      double mag_Z[]) {
    double Pitch_Tilt[130], Roll_Tilt[130], Mag_X_Tilt[130], Mag_Y_Tilt[130], Mag_Z_Tilt[130], Avg_Mag[130],
            Xh[130], Yh[130], mx[130], my[130], mz[130];
    /*This Algorithm is Provided by Mytheri Maam. in this algorithm here is we are doing first calibration
     ,and there is combination of hard and soft iron is used  */
    for (i = 0; i < DATAROWS; i++) {
        Pitch_Tilt[i] = Pitch_tilt[i] * M_PI / 180; // converting degree to radian
        Roll_Tilt[i] = Roll_tilt[i] * M_PI / 180;

        Mag_X_Tilt[i] = ((mag_X[i] - mag_min_X) / (mag_max_X - mag_min_X)) * 2 - 1;
        Mag_Y_Tilt[i] = ((mag_Y[i] - mag_min_Y) / (mag_max_Y - mag_min_Y)) * 2 - 1;
        Mag_Z_Tilt[i] = ((mag_Z[i] - mag_min_Z) / (mag_max_Z - mag_min_Z)) * 2 - 1;
        mx[i] = Mag_Y_Tilt[i];
        my[i] = -Mag_X_Tilt[i];
        mz[i] = Mag_Z_Tilt[i];

        Avg_Mag[i] = sqrtf(mx[i] * mx[i] + my[i] * my[i] + mz[i] * mz[i]);
        mx[i] = mx[i] / Avg_Mag[i];
        my[i] = my[i] / Avg_Mag[i];
        mz[i] = mz[i] / Avg_Mag[i];

        Xh[i] = mx[i] * cos(Pitch_Tilt[i]) + mz[i] * sin(Pitch_Tilt[i]);
        Yh[i] = mx[i] * sin(Pitch_Tilt[i]) * sin(Roll_Tilt[i]) + my[i] * cos(Roll_Tilt[i]) -
                mz[i] * sin(Roll_Tilt[i]) * cos(Pitch_Tilt[i]);

        if (Xh[i] > 0 && Yh[i] >= 0) {
            Yaw_Tilt[i] = atan(Yh[i] / Xh[i]) * 180 / 3.14;
        } else if (Xh[i] < 0) {
            Yaw_Tilt[i] = (180 + atan(Yh[i] / Xh[i]) * 180 / 3.14);
        } else if (Xh[i] > 0 && Yh[i] <= 0) {
            Yaw_Tilt[i] = (360 + atan(Yh[i] / Xh[i]) * 180 / 3.14);
        } else if (Xh[i] == 0 && Yh[i] < 0) {
            Yaw_Tilt[i] = (90);
        } else if (Xh[i] == 0 && Yh[i] >= 0) {
            Yaw_Tilt[i] = (270);
        }
    }
}

double calculataYawWithTiltCompensation_test(double Roll_tilt, double Pitch_tilt, double mag_X,
                                             double mag_Y,
                                             double mag_Z) {
    double Yaw_Tilt = 0, Pitch_Tilt = 0, Roll_Tilt = 0, Mag_X_Tilt = 0, Mag_Y_Tilt = 0, Mag_Z_Tilt = 0, Avg_Mag = 0,
            Xh = 0, Yh = 0, mx = 0, my = 0, mz = 0;
    /*This Algorithm is Provided by Mytheri Maam. in this algorithm here is we are doing first calibration
     ,and there is combination of hard and soft iron is used  */
    // printf("%f %f %f \n",mag_X,mag_Y,mag_Z);

    Pitch_Tilt = Pitch_tilt * M_PI / 180; // converting degree to radian
    Roll_Tilt = Roll_tilt * M_PI / 180;

    Mag_X_Tilt = ((mag_X - mag_min_X) / (mag_max_X - mag_min_X)) * 2 - 1;
    Mag_Y_Tilt = ((mag_Y - mag_min_Y) / (mag_max_Y - mag_min_Y)) * 2 - 1;
    Mag_Z_Tilt = ((mag_Z - mag_min_Z) / (mag_max_Z - mag_min_Z)) * 2 - 1;

    mx = Mag_Y_Tilt;
    my = -Mag_X_Tilt;
    mz = Mag_Z_Tilt;

    Avg_Mag = sqrtf(mx * mx + my * my + mz * mz);
    mx = mx / Avg_Mag;
    my = my / Avg_Mag;
    mz = mz / Avg_Mag;

    Xh = mx * cos(Pitch_Tilt) + mz * sin(Pitch_Tilt);
    Yh = mx * sin(Pitch_Tilt) * sin(Roll_Tilt) + my * cos(Roll_Tilt) -
         mz * sin(Roll_Tilt) * cos(Pitch_Tilt);

    if (Xh > 0 && Yh >= 0) {
        Yaw_Tilt = atan(Yh / Xh) * 180 / 3.14;
    } else if (Xh < 0) {
        Yaw_Tilt = (180 + atan(Yh / Xh) * 180 / 3.14);
    } else if (Xh > 0 && Yh <= 0) {
        Yaw_Tilt = (360 + atan(Yh / Xh) * 180 / 3.14);
    } else if (Xh == 0 && Yh < 0) {
        Yaw_Tilt = (90);
    } else if (Xh == 0 && Yh >= 0) {
        Yaw_Tilt = (270);
    }

    return Yaw_Tilt;
}

/************************************************************************************************************************************/
/******************************** FUNTION TO COUNT MAXIMUM OCCURANCE OF ANY NUMBER IN AN ARRAY ****************************/
double maxRepeating(double arr[]) {
    // Iterate though input array, for every element
    // arr[i], increment arr[arr[i]%k] by k
    double res = 0;
    int k = 130, arr2[DATAROWS];
    for (i = 0; i < DATAROWS; i++) {
        arr2[i] = round(arr[i]);
        arr2[arr2[i] % k] += k;
    }
    // Find index of the maximum repeating element
    int max = arr2[0], result = 0;
    for (i = 1; i < DATAROWS; i++) {
        if (arr2[i] > max) {
            max = arr2[i];
            result = i;
        }
    }
    // Uncomment this code to get the original array back
    for (i = 0; i < DATAROWS; i++) {
        arr2[i] = arr2[i] % k;
    }
    // Return index of the maximum element
    res = (double) arr[result];
    return res;
}

/**********************************************************************************************************************************/
/***************************************************FUNTION TO CALCULATE ORIENTATION ***********************************/
int Orientation(double euler[], double mag[], double mag_max_min[]) {
    struct orientationResult *ourOrientation = getOrientationResult();
    double orientation_App, orientation_Ana;
    int index = setSensorDataForOrientation(euler, mag, mag_max_min);
    calculataYawWithTiltCompensation(Roll_O_App, Pitch_O_App, Yaw_O_Ana, Mag_X_O, Mag_Y_O, Mag_Z_O);
    orientation_App = maxRepeating(Yaw_O_App);
    orientation_Ana = maxRepeating(Yaw_O_Ana);
    ourOrientation->Orientation_Ana = orientation_Ana;
    ourOrientation->Orientation_App = orientation_App;
    return index;
}
/*******************************************************************************************************************/
/*
 calculateMaxIndex is a function for calculating max index , in that function we are passing the array of angular velocity
 in X direction and we are getting maximum index for maximum angular speed in X direction.
 */
int calculateMaxIndex(double gyroX[]) {
    double max = gyroX[0];
    int index = 0;
    for (j = 0; j < DATAROWS; j++) {
        if (gyroX[j] > max) {
            max = gyroX[j];
            index = j;
        }
    }
    return index;
}

/*
 calculateBackLiftIndex is a function for calculating backlift start  index , in that function we are passing the array of
 angular velocity in X direction and downswing start index and then checking for any positive value from downswing start
 index till start index if we get any then we are breaking out the loop we are getting maximum index for maximum angular
 speed in X direction.
 */
int calculateBackLiftSIndex(double gyroX[], int downswing_start_index) {
    int index = 0;
    for (j = downswing_start_index - 1; j > 0; j--) {
        if (gyroX[j] > 0) {
            index = j;
            break;
        }
    }
    return index;
}

/*
 calculateDownswingIndex is a function for calculating index  where downswing starts, in that function we are passing array of
 angular speed in X direction and maximum angular speed index then we are checking for first negative value of angular speed
 form max index to starting index of array .
 */

int calculateDownSwingIndex(double gyroX[], int max_index) {
    int index = 0;
    for (j = max_index; j > 0; j--) {
        if (gyroX[j] < 0) {
            if (gyroX[j - 1] < 0 && gyroX[j - 2] < 0 && gyroX[j - 3] < 0 && gyroX[j - 4] < 0) {
                index = j;
                break;
            }
        }
    }
    return index + 1;
}

/*
 calculateFollowThroughIndex is a function for calculating index where followThrough ends , in that function we are passing
 array of angular speed in X direction and maximum angular speed index  then we are checking for first negative value of
 angular speed form max index to end index of array .
 */

int calculateFollowThroughIndex(double gyroX[], int max_index) {
    int index = 0;
    for (j = max_index; j < DATAROWS; j++) {
        if (gyroX[j] < 0) {
            if (gyroX[j + 1] < 0 && gyroX[j + 2] < 0 && gyroX[j + 3] < 0 && gyroX[j + 4] < 0) {
                index = j;
                break;
            }
        } else if (gyroX[j] < 0 && j > 125) {
            index = j;
            break;
        }
    }
    if (index == 0) {
        index = 130;
    }
    return index - 1;
}

int calculateFirstNegativeIndex(double gyroY[], int max_or_hit_index) {
    int index = 0;
    for (j = max_or_hit_index; j <= FTI; j++) {
        if (gyroY[j] < 0) {
            if (gyroY[j + 1] < 0 && gyroY[j + 2] < 0 && gyroY[j + 3] < 0 && gyroY[j + 4] < 0) {
                index = j;
                break;
            }
        }
    }
    return index + 1;
}

int calculateLastNegativeIndex(double gyroY[], int max_or_hit_index) {
    int index = 129, f_neg_index;
    f_neg_index = calculateFirstNegativeIndex(gyroY, max_or_hit_index);
    for (j = f_neg_index; j <= 129; j++) {
        if (gyroY[j] > 0) {
            if (gyroY[j + 1] > 0 && gyroY[j + 2] > 0 && gyroY[j + 3] > 0) {
                index = j;
                break;
            }
        }
    }
    return index - 1;
}

/*
 calculateIndex is a function for calculating all important index and assigns their values to global variable like MI,SI,FTI
 in that function we are passing an array of angular velocity in X direction.
 */

void calculateIndex(double gyroX[]) {
    MI = calculateMaxIndex(gyroX);
    SI = calculateDownSwingIndex(gyroX, MI);
    FTI = calculateFollowThroughIndex(gyroX, MI);
    BSI = calculateBackLiftSIndex(gyroX, SI);
}

/*
 calculateBackLift  is a function for calculating BackLift in that function we are passing an array of in Pitch and downswing
 start index(first positive in gyro X data). and for BackLift we are taking average of BackLift at First positive till 5 value
 in back word direction.
 */

double calculateBackLift(double Pitch[], int backlift_start_index, int downswing_start_index,
                         int shot_Type) {
    double backlift = 0;
    int diff = downswing_start_index - backlift_start_index;
    if (diff > 5) // this condition is  added for corrupt data (shot played before 3 sec interval)
    {
        if (shot_Type == 1) { // in case of vertical bat shots
            for (i = downswing_start_index; i > downswing_start_index - 5; i--) {
                if (Pitch[i] <=
                    -90) // if pitch is less  than -90 means -120 -130 then add it to 360
                {
                    backlift = backlift + 180 + (180 + Pitch[i]);
                } else if (Pitch[i] > -90 && Pitch[i] <
                                             0) // if pitch greater than -90 and less than 0 menas betwen 0 to -90 then add it to 180
                {
                    backlift = backlift + (180 - Pitch[i]);
                } else {
                    backlift = backlift + Pitch[i];
                }
            }
            backlift = backlift / 5;
        } else { // in case of horizontal bat shots
            int x = 0, index = 0, index2 = 0;
            //            printf("YES \n");
            if (Pitch[backlift_start_index] <= -90) {
                backlift = 360 + Pitch[backlift_start_index] -
                           Xrotation_unity[downswing_start_index - 1];
            } else if (Pitch[backlift_start_index] > -90 && Pitch[backlift_start_index] < 0) {
                backlift = 180 - Pitch[backlift_start_index] -
                           Xrotation_unity[downswing_start_index - 1];
            } else {
                backlift = Pitch[backlift_start_index] - Xrotation_unity[downswing_start_index - 1];
                //                printf(" %f \n",Pitch[backlift_start_index]);
            }

            for (x = downswing_start_index; x > BSI; x--) {
                if (Yg[x] < 0) {
                    index = x;
                    // printf("YES \n");
                    break;
                }
            }
            for (x = downswing_start_index; x > BSI; x--) {
                // printf("%f %f \n",Yg[x],Yrotation_unity[x]);
                if (Yg[x] > 800 && Yrotation_unity[x] > 40) {
                    for (i = x; i > BSI; i--) {
                        if (Yg[i] < 0) {
                            index2 = i;
                            // printf("YES \n");
                            break;
                        }
                    }
                }
            }
            if (index > BSI) {
                backlift = backlift - Yrotation_unity[downswing_start_index];
                // printf(" Backlift %f \n",backlift);
            }

            if (index2 >
                0) // this conditon is added , when downswing in Y axis start before X axis,for this we are calculating the index2
            {
                if (Pitch[index2] <= -90) {
                    backlift = 360 + Pitch[index2];
                } else if (Pitch[index2] > -90 && Pitch[index2] < 0) {
                    backlift = 180 - Pitch[index2];
                } else {
                    backlift = Pitch[index2];
                }
                // printf("backlift is %f %f \n",backlift,Pitch[index2]);
            }
        }
    } else {
        if (Pitch[downswing_start_index] < 0) {
            backlift = backlift + 180 + (180 + Pitch[downswing_start_index]);
        } else {
            backlift = backlift + Pitch[downswing_start_index];
        }
    }
    // conditions for data should not go less thean zero and greater than 270
    if (backlift < 0) {
        backlift = -backlift;
    }

    if (backlift > 270) {
        backlift = 270;
    }

    return backlift;
}

double calculateLengthofSweetSpot(double batLength, int gripPostion) {
    double loc = 0, sweetSpot = 0;
    batLength = batLength / 100; // to convert from cm to meter
    loc = 0.15544 *
          batLength;   // 150/965 = 0.15544 and 150 is length of sweetspot from bottom of the bat
    if (gripPostion == 0) {
        sweetSpot = 0.66666667 * batLength - loc + 0.75 * batLength / 9;
    } else if (gripPostion == 1) {
        sweetSpot = 0.66666667 * batLength - loc + 1 * batLength / 9;
    } else if (gripPostion == 2) {
        sweetSpot = 0.66666667 * batLength - loc + 1.25 * batLength / 9;
    } else {
        sweetSpot = 0.6;
    }
    return sweetSpot;
}

/*
 calculateSpeed  is a function for calculating Speed in that function we are passing Gyroscope data.
 and for Speed   V = r*w r is the length of the sweetSpot.and speed is calculated on filtered data and for filtration
 we have applied Low Pass filter to reduce peaks in data.
 */

void calculateSpeed(double gyroData[DATAROWS], double sweetSpot) {
    double Xvelg[DATAROWS], Yvelg[DATAROWS], Zvelg[DATAROWS], XgF[DATAROWS], XgF2[DATAROWS], YgF[DATAROWS], ZgF[DATAROWS];
    /***************** LOW PASS FILTER ****************************/
    int CUTOFF = 30, SAMPLE_RATE = 100;
    double RC = 1.0 / (CUTOFF * 2 * 3.14);
    double dt = 1.0 / SAMPLE_RATE;
    double alpha = dt / (RC + dt);
    // code to remove peaks from gyro X data some random values

    XgF2[0] = Xg[0];
    XgF2[1] = Xg[1];
    for (i = 2; i < DATAROWS; i++) {
        if (fabs(Xg[i] - Xg[i - 1]) > 500 || fabs(Xg[i] - Xg[i + 1]) > 500) {
            XgF2[i] = (fabs(Xg[i - 2]) + fabs(Xg[i - 1]) + fabs(Xg[i]) + fabs(Xg[i + 1]) +
                       fabs(Xg[i + 2])) / 5;
        } else {
            XgF2[i] = Xg[i];
        }
        //        printf("%f \n",XgF2[i]);
    }

    XgF[0] = Xg[0];
    YgF[0] = Yg[0];
    ZgF[0] = Zg[0];
    for (i = 1; i < DATAROWS; i++) {
        XgF[i] = XgF2[i - 1] + (alpha * (XgF2[i] - XgF[i - 1]));
        YgF[i] = Yg[i - 1] + (alpha * (Yg[i] - YgF[i - 1]));
        ZgF[i] = Zg[i - 1] + (alpha * (Zg[i] - ZgF[i - 1]));
        //        printf("%f \n",XgF[i]);
        //        printf("%f \n ",)
    }
    /*************************************************************/
    for (i = 0; i < DATAROWS; i++) {
        Xvelg[i] = sweetSpot * XgF[i] * 3.14159 / 180;
        Yvelg[i] = sweetSpot * YgF[i] * 3.14159 / 180;
        Zvelg[i] = sweetSpot * ZgF[i] * 3.14159 / 180;
        Vg[i] = sqrtf(Xvelg[i] * Xvelg[i]); // to calculate linear velocity
        //        printf(" %d %f %f %f %f \n",i,Xg[i],Vg[i]*18/5,XgF2[i],XgF[i]);
    }
}

double factorForSpeed(double gyroX[], int downswing_start_index, int max_or_impact_index,
                      int followthrough_end_index, double sweetSpot) {
    double fac = 0, sum = 0, fsum = 0;
    int count = 0;
    for (i = downswing_start_index; i < followthrough_end_index; i++) {
        if (gyroX[i] > 1800) {
            count = count + 1;
        }
    }
    if (count > 2) {
        if ((max_or_impact_index - downswing_start_index) > 5) {
            for (i = max_or_impact_index - 5; i < max_or_impact_index; i++) {
                fsum = fabs(gyroX[i] - gyroX[i - 1]);
                if (fsum > 150) {
                    fsum = 150;
                }
                sum = sum + fsum;
            }
            fac = sweetSpot * (sum / 5) * (18 / 5) * (3.14159 / 180) * (count - 1);
        }
    }
    return fac;
}

/*
 calculateMISpeed is a function for  calculating Speed at max index or at   index in which we are passing
 either max index or impact index and we are getting speed in Km/Hr.
 */
double calculateMISpeed(int max_or_impact_index) {
    double speed = Vg[max_or_impact_index] * 18 / 5;
    // printf("%f \n",speed);
    return speed;
}

/*
 calculatePower is a function for  calculating Power factor in that we passing array Accelerometer  data and Gyroscope data
 an for power we are using P = m * a * v and in that formula we are using resultant acceleration.

 */

void calculatePower(double accData[DATAROWS], double gyroData[DATAROWS], double weightofbat,
                    double sweetSpot, int dpr) {
    double XgF[DATAROWS];
    Pfac[0] = 0;
    // sweetSpot is in meter thats why we are converting it to mili-meter by multiplying by 1000
    double momentOfInertia =
            0.08333 * weightofbat * sweetSpot * sweetSpot * 1000 * 1000 + weightofbat * 481 * 481;
    // printf("%f \n",momentOfInertia);
    // code to remove random high value data
    XgF[0] = 0;
    for (i = 1; i < DATAROWS; i++) {
        if (fabs(Xg[i] - Xg[i - 1]) > 500) {
            XgF[i] = (fabs(Xg[i - 2]) + fabs(Xg[i - 1]) + fabs(Xg[i]) + fabs(Xg[i + 1]) +
                      fabs(Xg[i + 2])) / 5;
        } else {
            XgF[i] = Xg[i];
        }
    }
    if (SI - BSI > 5) // if data is correct ,
    {
        for (i = 1; i < DATAROWS; i++) {
            if (XgF[i] > 0 && XgF[i] < 700) {
                /******** Power is calculated using P = moment Of inertia * Alpha * Angular Velocity and unit of power will be watt ******/
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                               fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                               fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            } else if (XgF[i] > 700 && XgF[i] < 800) {
                if (abs(SI - i) < 15) {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia *
                                   (fabs(fabs(XgF[i])) / ((Tg[i] - Tg[SI]) / 2 + 30)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI] + 30)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                } else {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                }
            } else if (XgF[i] > 800 && XgF[i] < 1200) {
                if (abs(SI - i) < 22) {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia *
                                   (fabs(fabs(XgF[i])) / ((Tg[i] - Tg[SI]) / 2 + 44)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI] + 44)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                } else {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                }
            } else if (XgF[i] > 1200 && XgF[i] < 1600) {
                if (abs(SI - i) < 30) {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia *
                                   (fabs(fabs(XgF[i])) / ((Tg[i] - Tg[SI]) / 2 + 60)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI] + 60)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                } else {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                }
            } else if (XgF[i] > 1600 && XgF[i] < 2000) {
                if (abs(SI - i) < 38) {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia *
                                   (fabs(fabs(XgF[i])) / ((Tg[i] - Tg[SI]) / 2 + 100)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI] + 100)) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                } else {
                    if (dpr == 50) {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    } else {
                        Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                                   fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                    }
                }
            } else {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI]) / 2) *
                               fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i])) / (Tg[i] - Tg[SI])) *
                               fabs(XgF[i]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        }
    } else if (BSI == SI - 1) // this part is added for corrupt data shot taken before 3 seconds
    {
        if (XgF[1] > 0 && XgF[1] < 100) {
            for (i = 0; i < DATAROWS; i++) {
                /******** Power is calculated using P = moment Of inertia * Alpha * Angular Velocity and unit of power will be watt ******/
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI]) / 2) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI])) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 100 && XgF[1] < 200) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 15)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 15)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 200 && XgF[1] < 300) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 37)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 37)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 300 && XgF[1] < 400) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 50)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 50)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 400 && XgF[1] < 600) {

            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 62)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 62)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 600 && XgF[1] < 800) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 72)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 72)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 800 && XgF[1] < 1000) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 82)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 82)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 1000 && XgF[1] < 1200) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 92)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 92)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 1200 && XgF[1] < 1400) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 102)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 102)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else if (XgF[1] > 1400 && XgF[1] < 1600) {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 112)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 112)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }
            }
        } else {
            for (i = 0; i < DATAROWS; i++) {
                if (dpr == 50) {
                    Pfac[i] = (momentOfInertia *
                               (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 120)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                } else {
                    Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 120)) *
                               fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
                }

                // printf("%d %d %f \n ",i,MI,Pfac[i]/1.7);
            }
        }
    } else {
        for (i = 0; i < DATAROWS; i++) {
            if (dpr == 50) {
                Pfac[i] =
                        (momentOfInertia * (fabs(fabs(XgF[i + 1])) / ((Tg[i] - Tg[SI]) / 2 + 120)) *
                         fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
            } else {
                Pfac[i] = (momentOfInertia * (fabs(fabs(XgF[i + 1])) / (Tg[i] - Tg[SI] + 120)) *
                           fabs(XgF[i + 1]) * 3.14 * 3.14) / (1000 * 180 * 180);
            }
        }
    }
}

/*
 calculateMIPower  is a function for  calculating Power at max index or at impact index in which we are passing
 either max index or impact index.
 */
double calculateMIPower(int max_or_impact_index) {
    double power = 0;
    power = fabs(Pfac[max_or_impact_index] / 1.7);
    //	printf("%f \n",power);
    return power;
}

double calculateMIPowerWithOutTime(int max_or_impact_index, double weightofbat, double sweetSpot) {
    double power = 0, pfac[130], xgF[130];
    pfac[0] = 0;
    // sweetSpot is in meter thats why we are converting it to mili-meter by multiplying by 1000
    double momentOfInertia =
            0.08333 * weightofbat * sweetSpot * sweetSpot * 1000 * 1000 + weightofbat * 481 * 481;
    // printf("%f \n",momentOfInertia);
    // code to remove random high value data
    xgF[0] = 0;
    for (i = 1; i < DATAROWS; i++) {
        if (fabs(Xg[i] - Xg[i - 1]) > 500) {
            xgF[i] = (fabs(Xg[i - 2]) + fabs(Xg[i - 1]) + fabs(Xg[i]) + fabs(Xg[i + 1]) +
                      fabs(Xg[i + 2])) / 5;
        } else {
            xgF[i] = Xg[i];
        }
        //
        // printf("%f \n",XgF[i]);
    }
    for (i = 0; i < DATAROWS - 1; i++) {
        pfac[i + 1] =
                (momentOfInertia * (fabs(fabs(xgF[i + 1]) / (37 * 7))) * fabs(xgF[i + 1]) * 3.14 *
                 3.14) / (1000 * 180 * 180);
        //		printf("%d %f \n",i,pfac[i]);
    }
    power = fabs(pfac[max_or_impact_index]);
    //	printf("%d  %d   %f %f  \t",max_or_impact_index - SI,SI,xgF[max_or_impact_index],Tg[max_or_impact_index] - Tg[SI]);
    //	printf("%f \t",xgF[max_or_impact_index]);
    return power;
}

/*************************************** ROTATION IS CALCULATED **************************************/
/*
 FOR INTIGRATION, FORMULA IS TAKEN FROM MATLAB DOCUMENTATION CURVE FITTING TOOL, HOW TO CALCULATE P1 AND P2.
 */
/*
 calculateRotation  is a function for  calculating rotation , in that function we are passing Gyroscope data and start index
 and end index and for rotation first we are integrating the Gyroscope data. and for integration firstly we are getting the
 Values of constants (like  P1 P2 ) and then putting the value in Final equation for rotation.
 */
double calculateRotationX(double gyroData[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double Xrot[DATAROWS], T[DATAROWS], XT[DATAROWS], SXT = 0, SX = 0, ST = 0, DST = 0, P1 = 0, P2 = 0, rotation;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        XT[i] = Xg[i] * T[i];
        SXT = SXT + XT[i];
        SX = SX + Xg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }
    P1 = ((diff * SXT) - (SX * ST)) / ((diff * DST - (ST * ST)));
    P2 = (SX - P1 * ST) / diff;
    for (i = start_index; i <= end_index; i++) {
        Xrot[i] = ((P1 * T[i] * T[i]) / 2) + P2 * T[i];
    }
    rotation = Xrot[end_index];
    return rotation;
}

double calculateRotationY(double gyroData[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double Yrot[DATAROWS], T[DATAROWS], YT[DATAROWS], SY = 0, SYT = 0, ST = 0, DST = 0, P3 = 0, P4 = 0, rotation;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        YT[i] = Yg[i] * T[i];
        SYT = SYT + YT[i];
        SY = SY + Yg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }
    P3 = ((diff * SYT) - (SY * ST)) / ((diff * DST - (ST * ST)));
    P4 = (SY - P3 * ST) / diff;
    for (i = start_index; i <= end_index; i++) {
        Yrot[i] = ((P3 * T[i] * T[i]) / 2) + P4 * T[i];
    }
    rotation = Yrot[end_index - 1];
    return rotation;
}

double calculateRotationZ(double gyroData[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double Zrot[DATAROWS], T[DATAROWS], ZT[DATAROWS], SZT = 0, SZ = 0, ST = 0, DST = 0, P5 = 0, P6 = 0, rotation;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        ZT[i] = Zg[i] * T[i];
        SZT = SZT + ZT[i];
        SZ = SZ + Zg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }

    P5 = ((diff * SZT) - (SZ * ST)) / ((diff * DST - (ST * ST)));
    P6 = (SZ - P5 * ST) / diff;

    for (i = start_index; i <= end_index; i++) {
        Zrot[i] = ((P5 * T[i] * T[i]) / 2) + P6 * T[i];
    }
    rotation = Zrot[end_index];
    return rotation;
}

/* These two function is for calculate rotation for Horizontal bat shots */
void calculateRotationXUnity(double XrotUni[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double T[DATAROWS], XT[DATAROWS], SXT = 0, SX = 0, ST = 0, DST = 0, P1 = 0, P2 = 0;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        XT[i] = Xg[i] * T[i];
        SXT = SXT + XT[i];
        SX = SX + Xg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }
    P1 = ((diff * SXT) - (SX * ST)) / ((diff * DST - (ST * ST)));
    P2 = (SX - P1 * ST) / diff;
    for (i = start_index; i <= end_index; i++) {
        XrotUni[i] = ((P1 * T[i] * T[i]) / 2) + P2 * T[i];
    }
}

void calculateRotationYUnity(double YrotUni[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double T[DATAROWS], YT[DATAROWS], SY = 0, SYT = 0, ST = 0, DST = 0, P3 = 0, P4 = 0;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        YT[i] = Yg[i] * T[i];
        SYT = SYT + YT[i];
        SY = SY + Yg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }
    P3 = ((diff * SYT) - (SY * ST)) / ((diff * DST - (ST * ST)));
    P4 = (SY - P3 * ST) / diff;
    for (i = start_index; i <= end_index; i++) {
        YrotUni[i] = ((P3 * T[i] * T[i]) / 2) + P4 * T[i];
    }
}

void calculateRotationZUnity(double ZrotUni[DATAROWS], int start_index, int end_index) {
    int diff = end_index - start_index;
    double T[DATAROWS], ZT[DATAROWS], SZ = 0, SZT = 0, ST = 0, DST = 0, P5 = 0, P6 = 0;
    for (i = start_index; i <= end_index; i++) {
        T[i] = (Tg[i] - Tg[start_index]) / 1000;
    }
    for (i = start_index; i <= end_index; i++) {
        ZT[i] = Zg[i] * T[i];
        SZT = SZT + ZT[i];
        SZ = SZ + Zg[i];
        ST = ST + T[i];
        DST = DST + T[i] * T[i];
    }
    P5 = ((diff * SZT) - (SZ * ST)) / ((diff * DST - (ST * ST)));
    P6 = (SZ - P5 * ST) / diff;
    for (i = start_index; i <= end_index; i++) {
        ZrotUni[i] = ((P5 * T[i] * T[i]) / 2) + P6 * T[i];
    }
}

void calculateRotationXarrUnity(double XrotArr[DATAROWS]) {
    calculateRotationXUnity(XrotArr, BSI, SI - 1);
    calculateRotationXUnity(XrotArr, SI, FTI);
}

void calculateRotationYarrUnity(double YrotArr[DATAROWS], int Ylimitindex) {
    int x = 0;
    while (x < Ylimitindex - 1) {
        calculateRotationYUnity(YrotArr, Yindex[x], Yindex[x + 1] - 1);
        x++;
    }
}

void calculateRotationZarrUnity(double ZrotArr[DATAROWS], int Zlimitindex) {
    int x = 0;
    while (x < Zlimitindex - 1) {
        calculateRotationZUnity(ZrotArr, Zindex[x], Zindex[x + 1] - 1);
        x++;
    }
}

/******************************************************************************************/
/**************************************************************************************************************************/
/*
 in that function we are calculating displacement  in three direction x, y ,z.  for displacement we are applying double
 integration. and for integration first we are calculating the constants P1 P2 and so on.
 */

void calculatePosition(double accData[DATAROWS]) {
    double T[DATAROWS], XT[DATAROWS], YT[DATAROWS], ZT[DATAROWS], SXT = 0, SYT = 0, SZT = 0, SX = 0, SY = 0,
            SZ = 0, ST = 0, DST = 0, P1 = 0, P2 = 0, P3 = 0, P4 = 0, P5 = 0, P6 = 0;
    for (i = 0; i < DATAROWS; i++) {
        T[i] = (Ta[i] - Ta[0]) / 1000;
    }
    for (i = 0; i < DATAROWS; i++) {
        XT[i] = XaL[i] * T[i];
        YT[i] = YaL[i] * T[i];
        ZT[i] = ZaL[i] * T[i];

        SXT = SXT + XT[i];
        SYT = SYT + YT[i];
        SZT = SZT + ZT[i];

        SX = SX + XaL[i];
        SY = SY + YaL[i];
        SZ = SZ + ZaL[i];
        ST = ST + T[i];

        DST = DST + T[i] * T[i];
    }
    P1 = ((DATAROWS * SXT) - (SX * ST)) / ((DATAROWS * DST - (ST * ST)));
    P2 = (SX - P1 * ST) / DATAROWS;
    P3 = ((DATAROWS * SYT) - (SY * ST)) / ((DATAROWS * DST - (ST * ST)));
    P4 = (SY - P3 * ST) / DATAROWS;
    P5 = ((DATAROWS * SZT) - (SZ * ST)) / ((DATAROWS * DST - (ST * ST)));
    P6 = (SZ - P5 * ST) / DATAROWS;
    for (i = 0; i < DATAROWS; i++) {
        Xpos[i] = ((T[i] * T[i] * (3 * P2 + P1 * T[i])) / 6) * 9.81;
        Ypos[i] = ((T[i] * T[i] * (3 * P4 + P3 * T[i])) / 6) * 9.81;
        Zpos[i] = ((T[i] * T[i] * (3 * P6 + P5 * T[i])) / 6) * 9.81;
    }
}

double calculateEfficiency(int max_index, int impact_index, double maxspeed, double impactspeed) {
    double efficiency = 0, power = 0, speed = 0, speedatimpact = 0, spedfactor = 0;
    // speed = Vg[max_index] * 18 / 5;
    //  speedatimpact = Vg[impact_index] * 18 /5;
    speed = round(maxspeed);
    speedatimpact = round(impactspeed);
    if (speed < speedatimpact) {
        speed = speedatimpact;
    }
    power = Pfac[impact_index] / Pfac[max_index];
    if (speedatimpact == speed) {
        spedfactor = (speedatimpact / speed) * 100;
        power = 1;
    } else {
        if (speed > 0 && speed <= 20) {
            spedfactor = (speedatimpact / speed) * 70;
        } else if (speed > 20 && speed <= 30) {
            spedfactor = (speedatimpact / speed) * 80;
        } else if (speed > 30 && speed <= 40) {
            spedfactor = (speedatimpact / speed) * 90;
        } else {
            spedfactor = (speedatimpact / speed) * 100;
        }
    }
    if (power > 1) {
        power = 1;
    }
    if (impact_index > 0) {
        efficiency = ((power) * 100 + (spedfactor)) / 2;
    }
    return efficiency;
}

/*
 calculateFollowThrough is a function for calculating the Follow Through ,in that function we are passing array of Pitch ,
 value of back-lift, value of downswing and short-type , Follow Through will be different for different types of shot.
 and for follow through  we are checking values of pitch.
 */

double
calculateFollowThrough(double gyroData[DATAROWS], double gyroY[], double Pitch[], double backlift,
                       double downswing, int shot_type, int max_or_impact_index,
                       int followthrough_end_index, int batting_hand) {
    double followThrough = 0, rotation = 0;
    int first_neg_index = 0, last_neg_index = 0;
    first_neg_index = calculateFirstNegativeIndex(gyroY, max_or_impact_index);
    last_neg_index = calculateLastNegativeIndex(gyroY, max_or_impact_index);
    rotation = calculateRotationY(gyroData, first_neg_index, last_neg_index);
    //    printf("%d  %d %d %d \n",max_or_impact_index,II,WBI_UN,shot_type);

    //    printf("%d %d \n",max_or_impact_index,followthrough_end_index);

    //    printf(" ShotType %d \n",shot_type);
    //    printf("Rotation %f \n",rotation);

    if (shot_type == 11 && rotation >
                           -40) // if shot type is 11(on drive) and rotation in Y is greater than -40 then we will
        // calculate follow through according to shot type  1 and 3.
    {

        if (Pitch[max_or_impact_index] > 0) {
            if (Pitch[followthrough_end_index] >
                0) // if both max and follow through pitch is postive then subtract
            {
                if (Pitch[max_or_impact_index] > Pitch[followthrough_end_index]) {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                } else {
                    followThrough = 0;
                }
            } else {
                followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
            }
        } else {
            if (Pitch[followthrough_end_index] > 0) {
                followThrough = 0;
            } else {
                if (Pitch[max_or_impact_index] < Pitch[followthrough_end_index]) {
                    followThrough = 0;
                } else {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                }
            }
        }

        if (followThrough < 0) {
            followThrough = 0;
        }
    } else if (shot_type == 21 || shot_type == 22 || shot_type == 23) {
        if (Pitch[max_or_impact_index] > 0) {
            if (Pitch[followthrough_end_index] >
                0) // if both max and follow through pitch is postive then subtract
            {
                if (Pitch[max_or_impact_index] > Pitch[followthrough_end_index]) {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                } else {
                    followThrough = 0;
                }
            } else {
                followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
            }
        } else {
            if (Pitch[followthrough_end_index] > 0) {
                followThrough = 0;
            } else {
                if (Pitch[max_or_impact_index] < Pitch[followthrough_end_index]) {
                    followThrough = 0;
                } else {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                }
            }
        }
        if (followThrough < 0) {
            followThrough = 0;
        }
    } else if (shot_type == 3 || shot_type == 1 || shot_type == 7) //  with wrist break.
    {
        if (WBI_UN == 0) {

            if (Pitch[max_or_impact_index] > 0) {
                if (Pitch[followthrough_end_index] >
                    0) // if both max and follow through pitch is postive then subtract
                {
                    if (Pitch[max_or_impact_index] > Pitch[followthrough_end_index]) {
                        followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                    } else {
                        followThrough = 0;
                    }
                } else {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                }
            } else {
                if (Pitch[followthrough_end_index] > 0) {
                    followThrough = 0;
                } else {
                    if (Pitch[max_or_impact_index] < Pitch[followthrough_end_index]) {
                        followThrough = 0;
                    } else {
                        followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                    }
                }
            }
            if (followThrough < 0) {
                followThrough = 0;
            }
        } else {
            if (batting_hand == 1) {
                int index = 129;
                for (i = WBI_UN + 1; i < 129; i++) {
                    if (Yg[i] > 0) {
                        index = i;
                        break;
                    }
                }
                followThrough = (Xrotation_unity[WBI_UN] - Xrotation_unity[max_or_impact_index]) -
                                Yrotation_unity[index -
                                                1]; // for right handed batsman Yrotaion will be negative in case of wrist break
                // for this reason we are subtracting from X rotation  means adding
            } else {
                int index = 129;
                for (i = WBI_UN + 1; i < 129; i++) {
                    if (Yg[i] < 0) {
                        index = i;
                        break;
                    }
                }
                followThrough = (Xrotation_unity[WBI_UN] - Xrotation_unity[max_or_impact_index]) +
                                Yrotation_unity[index -
                                                1]; // for right handed batsman Yrotaion will be positive  in case of wrist break
                // for this reason we are adding  from X rotation
            }
        }
    } else if (shot_type == 11 || shot_type == 8) {
        if (WBI_UN == 0) // if there any wrist break then only we add y negative rotation.
        {
            if (Pitch[max_or_impact_index] > 0) {
                if (Pitch[followthrough_end_index] >
                    0) // if both max and follow through pitch is postive then subtract
                {
                    if (Pitch[max_or_impact_index] > Pitch[followthrough_end_index]) {
                        followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                    } else {
                        followThrough = 0;
                    }
                } else {
                    followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                }
            } else {
                if (Pitch[followthrough_end_index] > 0) {
                    followThrough = 0;
                } else {
                    if (Pitch[max_or_impact_index] < Pitch[followthrough_end_index]) {
                        followThrough = 0;
                    } else {
                        followThrough = Pitch[max_or_impact_index] - Pitch[followthrough_end_index];
                    }
                }
            }
            if (followThrough < 0) {
                followThrough = 0;
            }
        } else {
            if (batting_hand == 1) {
                //              followThrough = Pitch[WBI_UN] - rotation;

                followThrough = -rotation; // not adding the pitch
                //                printf("Follow through %f %f \n",followThrough,Pitch[WBI_UN]);
            } else {
                //                followThrough = Pitch[WBI_UN] + rotation;
                followThrough = rotation; // not adding the pitch
            }
        }
    } else if (shot_type == 5 || shot_type == 9 || shot_type == 10 || shot_type == 13 ||
               shot_type == 33) {
        if (WBI_UN == 0) {
            //            followThrough = calculateRotationX(Xg,max_or_impact_index,followthrough_end_index);
            followThrough =
                    Xrotation_unity[followthrough_end_index] - Xrotation_unity[max_or_impact_index];
        } else {
            if (batting_hand == 1) {
                int index = 129;
                for (i = WBI_UN + 1; i < 129; i++) {
                    if (Yg[i] > 0) {
                        index = i;
                        break;
                    }
                }
                followThrough = Xrotation_unity[WBI_UN] - Yrotation_unity[index - 1];
            } else {
                int index = 129;
                for (i = WBI_UN + 1; i < 129; i++) {
                    if (Yg[i] < 0) {
                        index = i;
                        break;
                    }
                }
                followThrough = Xrotation_unity[WBI_UN] + Yrotation_unity[index - 1];
            }
        }
    } else {
        followThrough = calculateRotationX(gyroData, max_or_impact_index, followthrough_end_index);
    }
    // conditions for data should not go less thean zero and greater than 270
    if (followThrough < 0) {
        followThrough = -followThrough;
    }
    if (followThrough > 270) {
        followThrough = 270;
    }
    //    printf("%f %f %f  %d \n",followThrough,Pitch[max_or_impact_index],Pitch[followthrough_end_index],WBI_UN);
    //    printf("FOllow Through %f \n",followThrough);
    return followThrough;
}

/*
 calculateDownswing is a function for calculating the Downswing ,in that function we are passing array of Pitch , value of
 back-lift, either max index or impact index  and short-type ,Downswing  will be different for different types of shot.
 */

double calculateDownSwing(double gyroData[DATAROWS], double Pitch[], double backlift, int shot_type,
                          int downswing_start_index, int max_or_impact_index) {
    double Downswing = 0;
    //    printf(" shot_type %d \n",shot_type);
    if (shot_type == 1 || shot_type == 3 || shot_type == 11 || shot_type == 23 || shot_type == 21 ||
        shot_type == 22 || shot_type == 8 || shot_type == 7) {
        Downswing = backlift - Pitch[max_or_impact_index];
        // printf(" Downsing %f \n",Downswing);
    } else if (shot_type == 5 || shot_type == 9 || shot_type == 10 || shot_type == 33) {
        //        printf("%f \n",Yrotation_unity[max_or_impact_index] );
        if (Yrotation_unity[max_or_impact_index] >
            0) // if Yrotaion at start or downswing is postive then only we adding it to
        {
            Downswing = calculateRotationX(Xg, downswing_start_index, max_or_impact_index) +
                        Yrotation_unity[max_or_impact_index];
        } else {
            double factorRotaion = 0;
            for (i = max_or_impact_index; i > downswing_start_index; i--) {
                if (Yrotation_unity[i] > 0) {
                    factorRotaion = Yrotation_unity[i];
                    break;
                }
            }
            Downswing = calculateRotationX(Xg, downswing_start_index, max_or_impact_index) +
                        factorRotaion;
        }
    } else {
        if (Pitch[max_or_impact_index] > 0) {
            Downswing = backlift - Pitch[max_or_impact_index];
        } else {
            if (Yrotation_unity[max_or_impact_index] >
                0) // if Yrotaion at start or downswing is postive then only we adding it to
            {
                Downswing = calculateRotationX(Xg, downswing_start_index, max_or_impact_index) +
                            Yrotation_unity[max_or_impact_index];
            } else {
                Downswing = calculateRotationX(Xg, downswing_start_index, max_or_impact_index);
            }
        }
    }
    // conditions for data should not go less thean zero and greater than 270
    if (Downswing < 0) {
        Downswing = -Downswing;
    }

    if (Downswing > 270) {
        Downswing = 270;
    }
    //    printf("%f  %d \n",Downswing,max_or_impact_index,);
    return Downswing;
}

/*
 isTappingDetected is to remove tapping data if tapping = 0 means tapping and 1 means no tapping.
 */

int isTappingDetected(double gyroX[]) {
    int flagup = 0, flagdown = 0;
    int tapping = 0;
    for (i = 0; i < DATAROWS; i++) {
        if (gyroX[i] > 350 && flagup == flagdown) {
            flagup++;
        } else if (gyroX[i] < 0 && flagup != flagdown) {
            flagdown++;
        }
    }
    if (flagup <= 3 && flagdown <= 3) {
        tapping = 1;
    }

    return tapping;
}

/*
 isSwingDetected is to remove fake swing  data if swing  = 0 means fake swing and 1 means real  tapping.
 in that we are checking for gyro X value  from First Positive to Last Positive is grater than 400 and count is
 grater than  4 than swing is real and one more condition we are checking for gyroX greater than 650 and count is
 greater than one and also swing is real.
 */

int isSwingDetected(double gyroX[], double gyroDRange) {
    int swing = 0, count = 0, count2 = 0, count3 = 0;
    int lowCount = 0;
    double decRange = 0;
    if (gyroDRange < 500) {
        lowCount = 2;
        decRange = 150;
    }

    for (i = SI; i <= FTI; i++) {
        if (gyroX[i] > 400) {
            count++;
        }
    }
    //	printf("%d \t",count);
    for (i = SI; i <= FTI; i++) {
        if (gyroX[i] > 650) {
            count2++;
        }
    }
    for (i = SI; i <= FTI; i++) {
        if (gyroX[i] > 500 - decRange) // for testing
        {
            count3++;
        }
    }

    if (count > 4 - lowCount) {
        //		printf("Yes \t");
        if (count3 > 3 || fabs(Pitch[SI]) >
                          60) // this condition is added to remove fake swings (passing the ball by bat)
        {
            swing = 1;
        }
    } else if (count2 >= 1 && count2 <= 3 && (FTI - MI) > 5 && fabs(Pitch[SI]) > 60) {
        swing = 1;
    } else if (count2 > 3) {
        swing = 1;
    }
    //	printf("%d \n",swing);
    return swing;
}

/*
 calculateImpactIndex is to calculate Impact. in then first we are checking for dip in gyro X  value.

 */

int calculateImpactIndex(double gyroX[], double gyroY[], double gyroZ[], int max_index,
                         double maxSpeed) {
    int lg = 0, hg = 0, ay = 0, ay1 = 0, az = 0, firstflag = 0, firstflag2 = 0, secondflag = 0, hitindex = 0, k;
    j = max_index - 10;
    int j2 = FTI -
             3;           // according to new impact logic we have to check for first flag should set on
    double impactdiffvalue = 0; // this variable added if impact is at FTI  and value is greater than 2000 than its a impact
    double min = 0, data = 0;
    DiffXg1 = 0, DiffXg2 = 0, UpXg3 = 0, DiffYa = 0, DiffXa = 0, DiffZa = 0, DiffZg = 0, DiffYg = 0;
    while (j < FTI - 3) // flag one should check before FTI - 3 index
    {
        if (firstflag == 0 && secondflag == 0) {
            if (gyroX[j + 1] - gyroX[j] < -150) {
                //				printf("%d \t",firstflag);
                DiffXg1 = gyroX[j + 1] - gyroX[j];
                firstflag = 1;
                hg = j;
                break;
            } else {
                data = gyroX[j + 1] - gyroX[j];
                if (data < min) {
                    min = data;
                    DiffXg1 = min;
                    DiffXg2 = min;
                    DiffYa = Ya[j + 1] - Ya[j];
                    DiffXa = (Xa[j + 1]) - (Xa[j]);
                    DiffZg = gyroZ[j] - gyroZ[j - 1];
                    DiffYg = gyroY[j] - gyroY[j - 1];
                    DiffZa = Za[j] - Za[j - 1];
                }
            }
        }
        j++;
    }
    while (j2 <= FTI && FTI < 125) // flag one should check before FTI  index
    {
        if (firstflag2 == 0 && firstflag == 0 && secondflag == 0) {
            if (gyroX[j2 + 1] - gyroX[j2] < -150) {
                firstflag2 = 1;
                DiffXg2 = gyroX[j2 + 1] - gyroX[j2];
                hg = j2;
                impactdiffvalue = fabs(gyroX[j2 + 1] - gyroX[j2]);
                break;
            } else {
                data = gyroX[j2 + 1] - gyroX[j2];
                if (data < min) {
                    min = data;
                    DiffXg2 = min;
                    DiffYa = Ya[j2 + 1] - Ya[j2];
                    DiffXa = (Xa[j2 + 1]) - (Xa[j2]);
                    DiffZg = gyroZ[j2] - gyroZ[j2 - 1];
                    DiffYg = gyroY[j2] - gyroY[j2 - 1];
                    DiffZa = Za[j2] - Za[j2 - 1];
                }
            }
        }
        j2++;
    }
    //	printf("FirstFlag %d \n",firstflag2);
    for (i = hg; i < FTI; i++) {
        if ((gyroX[i + 1] - gyroX[i]) > 70 && firstflag == 1) {
            secondflag = 1;
            lg = i + 1;
            UpXg3 = gyroX[i + 1] - gyroX[i];
            break;
        } else {
            data = gyroX[i + 1] - gyroX[i];
            if (min < data) {
                min = data;
                UpXg3 = min;
            }
        }
    }
    //	printf("SecondFlag %d \n",secondflag);
    if (lg - hg < 10 && lg - hg > 1) {
        hitindex = hg;
        DiffYa = Ya[hitindex + 1] - Ya[hitindex];
        DiffXa = (Xa[hitindex + 1]) - (Xa[hitindex]);
        DiffZg = gyroZ[hitindex] - gyroZ[hitindex - 1];
        DiffYg = gyroY[hitindex] - gyroY[hitindex - 1];
        DiffZa = Za[hitindex] - Za[hitindex - 1];
        impactBucket = 1;
        //		printf("First Flag %d \n",hitindex);
    } else if (firstflag == 1 && secondflag == 0) {
        //        printf("YES \n");
        k = hg;
        int l = hg - 2;
        if (hitindex == 0) { // this part is added to improve impact with accelerometer data -Yg
            while (l < hg + 3) {
                if (fabs((Ya[l + 1]) - (Ya[l])) > 5) {
                    ay1 = l;
                    DiffYa = Ya[l + 1] - Ya[l];
                    break;
                } else {
                    data = fabs((Ya[l + 1]) - (Ya[l]));
                    if (min < data) {
                        min = data;
                        DiffYa = min;
                    }
                }
                l++;
            }
            //			printf(" %d \n",ay1);
            if (ay1 >
                0) // this condition is added to improve impact index with validation of Xa diff
            {
                int m = ay1 - 2;
                while (m < ay1 + 3) {
                    if (fabs((Xa[m + 1]) - (Xa[m])) > 5) {

                        DiffYa = Ya[l + 1] - Ya[l];
                        DiffXa = (Xa[m + 1]) - (Xa[m]);
                        DiffZg = gyroZ[m] - gyroZ[m - 1];
                        DiffYg = gyroY[m] - gyroY[m - 1];
                        DiffZa = Za[m] - Za[m - 1];
                        impactBucket = 2;
                        hitindex = hg;
                        //						printf(" First Flag 2 %d \n",hitindex);
                        break;
                    }
                    {
                        DiffYa = Ya[l + 1] - Ya[l];
                        DiffXa = (Xa[m + 1]) - (Xa[m]);
                        DiffZg = gyroZ[m] - gyroZ[m - 1];
                        DiffYg = gyroY[m] - gyroY[m - 1];
                        DiffZa = Za[m] - Za[m - 1];
                    }
                    m++;
                }
            }
        }
        // this condition added to improve the impact in case of slow speed shots where first flag is set
        if (maxSpeed < 30) {
            l = hg - 2;
            //			printf("%d  L \n",l);
            if (UpXg3 > 30) // if up data then we are checking for only one flag to be true
            {

                while (l < hg + 3) {
                    //					printf("%f  %d \n",Ya[l+1] - Ya[l],hg+3);
                    if (fabs(Ya[l + 1] - Ya[l]) > 5 || fabs(Xa[l + 1] - Xa[l]) > 5 ||
                        fabs(Za[l + 1] - Za[l]) > 5) {

                        hitindex = hg;
                        //						printf("This is an impact");
                        break;
                    }
                    l++;
                }
            } else {
                while (l < hg + 3) {
                    if ((fabs(Ya[l + 1] - Ya[l]) > 5 && fabs(Xa[l + 1] - Xa[l]) > 5) ||
                        (fabs(Ya[l + 1] - Ya[l]) > 5 && fabs(Za[l + 1] - Za[l]) > 5) ||
                        (fabs(Za[l + 1] - Za[l]) > 5 && fabs(Xa[l + 1] - Xa[l]) > 5)) {
                        hitindex = hg;
                        break;
                    }
                    l++;
                }
            }
        } else if (maxSpeed > 30 && maxSpeed < 40) {
            l = hg - 2;
            //			printf("%d  L \n",l);
            if (UpXg3 > 30) // if up data then we are checking for only one flag to be true
            {

                while (l < hg + 3) {
                    //					printf("%f  %d \n",Ya[l+1] - Ya[l],hg+3);
                    if (fabs(Ya[l + 1] - Ya[l]) > 10 || fabs(Xa[l + 1] - Xa[l]) > 10 ||
                        fabs(Za[l + 1] - Za[l]) > 10) {

                        hitindex = hg;
                        //						printf("This is an impact");
                        break;
                    }
                    l++;
                }
            } else {
                while (l < hg + 3) {
                    if ((fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10) ||
                        (fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Za[l + 1] - Za[l]) > 10) ||
                        (fabs(Za[l + 1] - Za[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10)) {
                        hitindex = hg;
                        break;
                    }
                    l++;
                }
            }
        }
    } else if (firstflag2 == 1 && secondflag == 0) {
        //		printf("YES \n");
        k = hg;
        int l = hg - 2;
        if (hitindex == 0) { // this part is added to improve impact with accelerometer data -Yg
            while (l < hg + 3) {
                if (fabs((Ya[l + 1]) - (Ya[l])) > 9) {
                    ay1 = l;
                    DiffYa = Ya[l + 1] - Ya[l];
                    break;
                } else {
                    data = fabs((Ya[l + 1]) - (Ya[l]));
                    if (min < data) {
                        min = data;
                        DiffYa = min;
                    }
                }
                l++;
            }
            if (ay1 >
                0) // this condition is added to improve impact index with validation of Xa diff
            {
                int m = ay1 - 2;
                while (m < ay1 + 3) {
                    if (fabs((Xa[m + 1]) - (Xa[m])) > 10 || fabs(gyroZ[m] - gyroZ[m - 1]) > 1000) {
                        hitindex = hg;
                        DiffYa = Ya[l + 1] - Ya[l];
                        DiffXa = (Xa[m + 1]) - (Xa[m]);
                        DiffZg = gyroZ[m] - gyroZ[m - 1];
                        DiffYg = gyroY[m] - gyroY[m - 1];
                        DiffZa = Za[m] - Za[m - 1];
                        impactBucket = 3;

                        //						printf(" My hit index %d \n",hitindex);
                        break;
                    }
                    {
                        DiffYa = Ya[l + 1] - Ya[l];
                        DiffXa = (Xa[m + 1]) - (Xa[m]);
                        DiffZg = gyroZ[m] - gyroZ[m - 1];
                        DiffYg = gyroY[m] - gyroY[m - 1];
                        DiffZa = Za[m] - Za[m - 1];
                    }
                    m++;
                }
                if (hitindex == 0) {
                    if (impactdiffvalue > 2000) {
                        hitindex = hg;
                    }
                }
            }
            //			printf("%d \n",hitindex);
        }
        // this condition added to improve the impact in case of slow speed shots where first flag is set
        if (maxSpeed < 30) {
            l = hg - 2;
            //
            if (Pitch[hg] > 0) {
                while (l < hg + 3) {
                    if (fabs(Ya[l + 1] - Ya[l]) > 5 || fabs(Xa[l + 1] - Xa[l]) > 5 ||
                        fabs(Za[l + 1] - Za[l]) > 5) {
                        hitindex = hg;
                        break;
                    }
                    l++;
                }
            } else {
                while (l < hg + 3) {
                    if ((fabs(Ya[l + 1] - Ya[l]) > 5 && fabs(Xa[l + 1] - Xa[l]) > 5) ||
                        (fabs(Ya[l + 1] - Ya[l]) > 5 && fabs(Za[l + 1] - Za[l]) > 5) ||
                        (fabs(Za[l + 1] - Za[l]) > 5 && fabs(Xa[l + 1] - Xa[l]) > 5)) {
                        hitindex = hg;
                        break;
                    }
                    l++;
                }
            }
        } else if (maxSpeed > 30) {
            l = hg - 2;
            //
            while (l < hg + 3) {
                if ((fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10) ||
                    (fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Za[l + 1] - Za[l]) > 10) ||
                    (fabs(Za[l + 1] - Za[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10)) {
                    hitindex = hg;
                    break;
                }
                l++;
            }
        }
    }

    // this condition is added to remove fake impact in case of high bat speed shots
    if (impactBucket == 2 && maxSpeed > 40) {
        k = hg;
        int l = hg - 2;
        while (l < hg + 3) {
            if ((fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10) ||
                (fabs(Ya[l + 1] - Ya[l]) > 10 && fabs(Za[l + 1] - Za[l]) > 10) ||
                (fabs(Za[l + 1] - Za[l]) > 10 && fabs(Xa[l + 1] - Xa[l]) > 10)) {
                hitindex = hg;
                break;
            } else // if condtion is not satisfied then hitindex = 0
            {
                hitindex = 0;
            }
            l++;
        }
    }
    //	printf("Hitindex %d %f \n",hitindex,Xg[hitindex]);
    if (hitindex == 0) // this is new logic added on saimon data
    {
        //		printf("Yes \n");
        if (FTI - max_index >
            10) // we have to check 10 index back and 10 index forward of max index , if greater than  MI + 10 , else FTI tak
        {
            for (i = max_index - 10; i < max_index + 10; i++) {
                if (fabs(Ya[i] - Ya[i - 1]) > 8) {
                    ay = i;
                    DiffYa = Ya[i] - Ya[i - 1];
                    break;
                } else {
                    data = fabs((Ya[i + 1]) - (Ya[i]));
                    if (min < data) {
                        min = data;
                        DiffYa = min;
                    }
                }
            }
            if (ay > 0) {
                for (i = ay - 3; i < ay + 3; i++) {
                    if (fabs(Za[i] - Za[i - 1]) > 10 || fabs((Xa[i] - Xa[i - 1])) > 10) {
                        az = i;

                        DiffXa = Xa[i] - Xa[i - 1];
                        DiffZa = Za[i] - Za[i - 1];
                        break;
                    }
                }
                if (az > 0) {
                    for (i = az - 3; i < az + 3; i++) {
                        if (fabs(gyroZ[i] - gyroZ[i - 1]) > 1000 ||
                            fabs(gyroY[i] - gyroY[i - 1]) > 1000) {
                            DiffZg = gyroZ[i] - gyroZ[i - 1];
                            DiffYg = gyroY[i] - gyroY[i - 1];
                            hitindex = i;
                            impactBucket = 4;
                            break;
                        }
                    }
                }
            }
            //			printf("%d impact index \n",hitindex);
        } else {
            for (i = max_index - 10; i <= FTI; i++) {
                if (fabs(Ya[i] - Ya[i - 1]) > 8) {
                    ay = i;
                    DiffYa = Ya[i] - Ya[i - 1];
                    break;
                }
            }
            if (ay > 0) {
                for (i = ay - 3; i < ay + 3; i++) {
                    if (fabs(Za[i] - Za[i - 1]) > 10 || fabs((Xa[i] - Xa[i - 1])) > 10) {
                        az = i;
                        DiffXa = Xa[i] - Xa[i - 1];
                        DiffZa = Za[i] - Za[i - 1];
                        break;
                    }
                }
                if (az > 0) {
                    for (i = az - 3; i < az + 3; i++) {
                        if (fabs(gyroZ[i] - gyroZ[i - 1]) > 1000 ||
                            fabs(gyroY[i] - gyroY[i - 1]) > 1000) {
                            DiffZg = gyroZ[i] - gyroZ[i - 1];
                            DiffYg = gyroY[i] - gyroY[i - 1];
                            impactBucket = 5;
                            hitindex = i;
                            break;
                        }
                    }
                }
            }
        }
        //		printf("%d impact index \n",hitindex);
    }

    if (hitindex < SI + 3) {
        // impact index should be greater than the downswing start index
        hitindex = 0;
    }
    //	printf("%d %f  \n",hitindex,Xg[hitindex]);
    // printf("%d,%d,%f,%f,%f,%f,%f,%f,%f,%f \n",hitindex,impactBucket,DiffXg1,DiffXg2,UpXg3,DiffZg,DiffYg,DiffXa,DiffYa,DiffZa);
    return hitindex;
}

/**************** THIS FUNTION IS ONE MORE CONDITON TO CHECK IMPACT ********************/
/*not validated yet */
int calculateImpactIndex2(double gyroX[], double gyroY[], double gyroZ[], int max_index) {
    int hitindex = 0, ay = 0, az = 0, ax = 0;
    for (i = max_index - 10; i < FTI + 5; i++) {
        if (fabs(Ya[i] - Ya[i - 1]) > 10) {
            ay = i;
            break;
        }
    }
    if (ay > 0) {
        for (i = ay - 3; i < ay + 3; i++) {
            if ((fabs(Za[i] - Za[i - 1]) > 10) || (fabs(Xa[i] - Xa[i - 1]) > 10)) {
                az = i;
                break;
            }
        }
        if (az > 0) {
            for (i = az - 3; i < az + 3; i++) {
                if (fabs(gyroZ[i] - gyroZ[i - 1]) > 500 || fabs(gyroY[i] - gyroY[i - 1]) > 500) {
                    hitindex = i;
                    break;
                }
            }
        }
    }

    if (hitindex < SI) {
        // impact index should be greater than the downswing start index
        hitindex = 0;
    }
    //	printf("%d \n",hitindex);
    return hitindex;
}
/******************************************************************************************/

/************************ Wrist break implementation ***************************/
/* not validated yet */
int calculateWristBreak(double gyroY[], int downswing_start_index, int followthrough_end_index) {
    int x = 0, y = 0;
    for (i = downswing_start_index; i < followthrough_end_index; i++) {
        if (Yg[i] < 0) {
            x++;
            if (Yg[i] < -200) {
                y++;
            }
        }
    }
    gyro_y_negative = x;
    gyro_y_negative_greater = y;
    return 1;
}
/*****************************************************************************/
/******************************************* BALL IMPACT AT BAT TOWARD LEFT OR RIGHT *******************/
/* not validated yet */

double calculateBallImpactAtBat_Range_Gyro(int impactIndex, double arr[]) {
    int index = 0;
    double max = 0, sum = 0;
    for (i = impactIndex - 2; i < impactIndex + 3; i++) {
        // printf("%f %f \n",arr[i],arr[i-1]);
        sum = fabs(arr[i] - arr[i - 1]);
        if (sum > max) {
            max = sum;
        }
    }
    if (sum >= 0 && sum < 1000) {
        index = 0;
    } else if (sum >= 1000 && sum < 2000) {
        index = 1;
    } else if (sum >= 2000 && sum < 3000) {
        index = 2;
    } else if (sum >= 3000 && sum < 4000) {
        index = 3;
    } else {
        index = 4;
    }
    return max;
}

double calculateBallImpactAtBat_Range_Acc(int impactIndex, double arr[]) {
    int index = 0;
    double max = 0, sum = 0;
    for (i = impactIndex - 2; i < impactIndex + 3; i++) {
        sum = fabs(arr[i] - arr[i - 1]);
        if (sum > max) {
            max = sum;
        }
    }
    if (sum >= 0 && sum < 10) {
        index = 0;
    } else if (sum >= 10 && sum < 20) {
        index = 1;
    } else if (sum >= 20 && sum < 30) {
        index = 2;
    } else if (sum >= 30 && sum < 40) {
        index = 3;
    } else {
        index = 4;
    }
    return max;
}

int calculateBallImpactAtBat(int impactIndex) {
    f_gx = calculateBallImpactAtBat_Range_Gyro(impactIndex, Xg);
    f_gy = calculateBallImpactAtBat_Range_Gyro(impactIndex, Yg);
    f_gz = calculateBallImpactAtBat_Range_Gyro(impactIndex, Zg);
    f_ax = calculateBallImpactAtBat_Range_Acc(impactIndex, Xa);
    f_ay = calculateBallImpactAtBat_Range_Acc(impactIndex, Ya);
    f_az = calculateBallImpactAtBat_Range_Acc(impactIndex, Za);
    return 0;
}

/*
 typeOfShot is a function to calculate its an unorthodox shot or not , 0 is for unorthodox and 1 is for drives like straight drive,
 cover drive, on drive .
 1 is for vertical bat shot.
 2 is for horizontal bar shot.
 */

int typeOfShot(int downswing_start_index, int max_or_impact_index, int followthrough_end_index) {
    int index = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    for (i = downswing_start_index;
         i <= max_or_impact_index; i++) // counting Positive and Negative pitch
    {
        if (Pitch[i] > 0) {
            x1++;
        } else {
            y1++;
        }
    }
    //   printf("%d %d \n",y1,x1);
    if (x1 - y1 >= 0) // if diff is more than zero or equal to zero
    {
        if (y1 > 0) // if y1 is greater then Zero
        {
            for (i = max_or_impact_index;
                 i < followthrough_end_index; i++) // again counting the pitch positive
            {
                if (Pitch[i] > 0) {
                    x2++;
                } else {
                    y2++;
                }
            }
            double sum = (Roll[max_or_impact_index - 2] + Roll[max_or_impact_index - 1] +
                          Roll[max_or_impact_index] +
                          Roll[max_or_impact_index + 1] + Roll[max_or_impact_index + 2]) /
                         5; // taking avg of the Roll at impact or max
            if (x2 - y2 > 0 && sum > 45) {
                index = 0;
            } else {
                index = 1;
            }
        } else if (y1 == 0) {
            // if((Roll[max_or_impact_index-2] + Roll[max_or_impact_index-1] + Roll[max_or_impact_index] +
            //   Roll[max_or_impact_index+1] + Roll[max_or_impact_index+2])/5  > 45)// taking Avg and  comparing with roll and should be greater than 45
            //  {
            //    index = 0;
            //  }
            //  else
            //  {
            //    index = 1;
            //  }

            for (i = downswing_start_index + 1; i < max_or_impact_index; i++) {
                if ((Pitch[i - 1] - Pitch[i]) >= -20 && (Pitch[i - 1] - Pitch[i]) < 50) {
                    index = 1;
                } else {
                    index = 0;
                    break;
                }
            }
        }
    } else {
        index = 0;
    }
    //    printf("%d \n",index);
    if (index == 1) // this condition is added for some pull shots coming as vertical bat shots
    {
        index = horizontal_as_vertical(1, downswing_start_index, max_or_impact_index);
    }
    //    printf("%d \n",index);
    return index;
}

int horizontal_as_vertical(int horizontal_or_vertical, int downswing_start_index,
                           int max_or_impact_index) {
    int index = 1, index2 = 0;
    if (horizontal_or_vertical == 1) {
        // printf("YES \n");
        for (i = downswing_start_index; i < max_or_impact_index; i++) {
            if (fabs(fabs(Pitch[i]) - fabs(Pitch[i - 1])) > 50) {
                int k = i, m;
                double sum = 0;
                if (fabs(fabs(Pitch[i + 1]) - fabs(Pitch[i])) > 50 ||
                    fabs(fabs(Pitch[i + 2]) - fabs(Pitch[i + 1])) > 50) {
                    index2 = 1;
                }
                for (m = k - 2; m < k + 3; m++) {
                    sum = sum + fabs(Roll[m]);
                }
                sum = sum / 5;
                if (sum > 60 && index2 == 0) {
                    index = 0;
                }
                break;
            }
        }
    }
    return index;
}

double calculateBatDirectionforShotType(double orientation_App, double orientation_Ana,
                                        int max_or_impact_index,
                                        int userBattingHand, int type_of_yaw) {

    // double orientation2 = Yaw_T[max_or_impact_index + 1] + Roll[max_or_impact_index], backliftdirection = 0;
    // double orientation2 = Yaw_T[max_or_impact_index + 1] , backliftdirection = 0;
    double orientation2 = 0, backliftdirection = 0;
    if (SHV == 1) {
        if (type_of_yaw ==
            1) // if difference between yaw analytics is greater than 100 then use sensor fusion yaw.
        {
            orientation2 = Yaw[max_or_impact_index];
            orientation_Ana = orientation_App;
        } else {
            orientation2 = Yaw_T[max_or_impact_index];
        }
    } else {
        orientation2 = Yaw[max_or_impact_index];
    }
    if (userBattingHand == 1) {
        if (SHV == 1) {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            }
        } else {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            }
        }
    } else {
        if (SHV == 1) {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            }
        } else {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            }
        }
    }
    // this code is not required for now this is for pitch change , will implement later.
    /*
     if(userBattingHand == 1)
     { //------- limit for pitch change 110 to 150
     if(Pitch_change > 150 && orientation <= 180)
     {
     backliftdirection = 180 + backliftdirection;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection = backliftdirection + 180 ;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = backliftdirection - 180 ;
     }
     }
     else
     {
     if (Pitch_change > 150 && orientation <= 180 )
     {
     backliftdirection = - ( 180 + backliftdirection );
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection =  -( backliftdirection + 180 ) ;
     }
     else if(Pitch_change > 110 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = -( backliftdirection - 180 );
     }
     }
     */

    return backliftdirection;
}

int calculateDiffInAEYaw(int max_or_impact_index) {
    int index = 0;
    for (i = 0; i < 5; i++) {
        if (fabs(Yaw_T[max_or_impact_index - 2 + i] - Yaw_T[max_or_impact_index - 1 + i]) > 100 &&
            fabs(Yaw_T[max_or_impact_index - 2 + i] - Yaw_T[max_or_impact_index - 1 + i]) < 180) {
            index = 1;
        }
    }
    return index;
}

double calculateBatDirectionforShotTypeAverage(double orientation_App, double orientation_Ana,
                                               int max_or_impact_index,
                                               int userBattingHand) {
    double direction = 0;
    int type_of_yaw = 0;
    type_of_yaw = calculateDiffInAEYaw(max_or_impact_index);
    for (i = 0; i < 3; i++) {
        direction = direction + calculateBatDirectionforShotType(orientation_App, orientation_Ana,
                                                                 max_or_impact_index + i - 2,
                                                                 userBattingHand, type_of_yaw);
    }
    direction = direction / 3;
    return direction;
}

double calculateBatDirectionforShotType_App(double orientation_App, int max_or_impact_index,
                                            int userBattingHand) {

    // double orientation2 = Yaw_T[max_or_impact_index + 1] + Roll[max_or_impact_index], backliftdirection = 0;
    // double orientation2 = Yaw_T[max_or_impact_index + 1] , backliftdirection = 0;
    double orientation2 = 0, backliftdirection = 0;
    orientation2 = Yaw[max_or_impact_index];
    // if(SHV == 1)
    //  {
    //    orientation2 = Yaw_T[max_or_impact_index + 1];
    // }
    // else
    //{
    //   orientation2 = Yaw[max_or_impact_index + 1];
    // }
    if (userBattingHand == 1) {
        if (SHV == 1) {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            }
        } else {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = (orientation2 - orientation_App) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_App;
                }
            }
        }
    } else {
        if (SHV == 1) {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            }
        } else {
            if (orientation_App <= 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            } else if (orientation_App > 180) {
                if (fabs(orientation2 - orientation_App) >= 180) {
                    backliftdirection = -((orientation2 - orientation_App) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_App);
                }
            }
        }
    }
    // this code is not required for now this is for pitch change , will implement later.
    /*
     if(userBattingHand == 1)
     { //------- limit for pitch change 110 to 150
     if(Pitch_change > 150 && orientation <= 180)
     {
     backliftdirection = 180 + backliftdirection;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection = backliftdirection + 180 ;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = backliftdirection - 180 ;
     }
     }
     else
     {
     if (Pitch_change > 150 && orientation <= 180 )
     {
     backliftdirection = - ( 180 + backliftdirection );
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection =  -( backliftdirection + 180 ) ;
     }
     else if(Pitch_change > 110 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = -( backliftdirection - 180 );
     }
     }
     */

    return backliftdirection;
}

double calculateBatDirectionforShotTypeAverage_App(double orientation_App, int max_or_impact_index,
                                                   int userBattingHand) {
    double direction = 0;
    for (i = 0; i < 3; i++) {
        direction = direction + calculateBatDirectionforShotType_App(orientation_App,
                                                                     max_or_impact_index + i - 2,
                                                                     userBattingHand);
    }
    direction = direction / 3;

    return direction;
}

double calculateBatDirectionforShotType_Ana(double orientation_Ana, int max_or_impact_index,
                                            int userBattingHand) {

    // double orientation2 = Yaw_T[max_or_impact_index + 1] + Roll[max_or_impact_index], backliftdirection = 0;
    // double orientation2 = Yaw_T[max_or_impact_index + 1] , backliftdirection = 0;
    double orientation2 = 0, backliftdirection = 0;
    orientation2 = Yaw_T[max_or_impact_index];
    // if(SHV == 1)
    //  {
    //    orientation2 = Yaw_T[max_or_impact_index + 1];
    // }
    // else
    //{
    //   orientation2 = Yaw[max_or_impact_index + 1];
    // }
    if (userBattingHand == 1) {
        if (SHV == 1) {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            }
        } else {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) - 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = (orientation2 - orientation_Ana) + 360;
                } else {
                    backliftdirection = orientation2 - orientation_Ana;
                }
            }
        }
    } else {
        if (SHV == 1) {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            }
        } else {
            if (orientation_Ana <= 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) - 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            } else if (orientation_Ana > 180) {
                if (fabs(orientation2 - orientation_Ana) >= 180) {
                    backliftdirection = -((orientation2 - orientation_Ana) + 360);
                } else {
                    backliftdirection = -(orientation2 - orientation_Ana);
                }
            }
        }
    }
    // this code is not required for now this is for pitch change , will implement later.
    /*
     if(userBattingHand == 1)
     { //------- limit for pitch change 110 to 150
     if(Pitch_change > 150 && orientation <= 180)
     {
     backliftdirection = 180 + backliftdirection;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection = backliftdirection + 180 ;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = backliftdirection - 180 ;
     }
     }
     else
     {
     if (Pitch_change > 150 && orientation <= 180 )
     {
     backliftdirection = - ( 180 + backliftdirection );
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection =  -( backliftdirection + 180 ) ;
     }
     else if(Pitch_change > 110 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = -( backliftdirection - 180 );
     }
     }
     */
    return backliftdirection;
}

double calculateBatDirectionforShotTypeAverage_Ana(double orientation_Ana, int max_or_impact_index,
                                                   int userBattingHand) {
    double direction = 0;
    for (i = 0; i < 3; i++) {
        direction = direction + calculateBatDirectionforShotType_Ana(orientation_Ana,
                                                                     max_or_impact_index + i - 2,
                                                                     userBattingHand);
    }
    direction = direction / 3;
    return direction;
}

/*
 shotAnalysis to calculate type of shot 1 is for straight drive , 2 for on drive ,3 for cover drive. */

int shotAnalysis(double gyroData[DATAROWS], double eulerAngles[DATAROWS],
                 double orientation_App, double orientation_Ana, int downswing_start_index,
                 int max_or_impact_index, int followthrogh_end_index,
                 int userBattingHand, int firmware_version) {
    int shot_t = 0, type_of_shot = 2;
    double r1, r2, r, batdirectionAtImpactorMax = 0, batdirectionAtImpactorMax2 = 0, batdirectionAtImpactorMax90 = 0, batdirectionAtImpactorMaxR = 0;
    type_of_shot = SHV;
    /*   Due to new logic Implementation */
    batdirectionAtImpactorMaxR = calculateBatDirectionforShotTypeAverage_Ana_Test_R(orientation_App,
                                                                                    max_or_impact_index,
                                                                                    userBattingHand,
                                                                                    firmware_version);
    batdirectionAtImpactorMax90 = calculateBatDirectionforShotTypeAverage_Ana_Test_90(
            orientation_App, max_or_impact_index, userBattingHand, firmware_version);

    if (type_of_shot == 1) // in case of vetical bat shots use actuat pitch and Zero roll
    {
        // printf("%f \n",orientation_App);
        if (firmware_version == 3) {
            batdirectionAtImpactorMax = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                    orientation_App, max_or_impact_index, userBattingHand, firmware_version);
            //            printf("batdirectionAtImpactorMax %f \n",batdirectionAtImpactorMax);
        } else {
            batdirectionAtImpactorMax = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                    orientation_Ana, max_or_impact_index, userBattingHand, firmware_version);
        }
    } else // in case of horizontal bat shots use this logic
    {
        if (batdirectionAtImpactorMaxR >= 0 && batdirectionAtImpactorMax90 >= 0) {
            batdirectionAtImpactorMax = batdirectionAtImpactorMaxR;
        } else if (batdirectionAtImpactorMaxR < 0 && batdirectionAtImpactorMax90 < 0) {
            batdirectionAtImpactorMax = batdirectionAtImpactorMax90;
        } else if (batdirectionAtImpactorMax90 < -70 && batdirectionAtImpactorMaxR > 0) {
            batdirectionAtImpactorMax = batdirectionAtImpactorMax90;
        } else if (batdirectionAtImpactorMax90 > 0 && batdirectionAtImpactorMaxR < 0) {
            batdirectionAtImpactorMax = batdirectionAtImpactorMax90;
        } else {
            batdirectionAtImpactorMax = batdirectionAtImpactorMaxR;
        }
    }

    // printf("%f \n",batdirectionAtImpactorMax);
    // batdirectionAtImpactorMax = calculateBatDirectionforShotTypeAverage(orientation_App,orientation_Ana , max_or_impact_index ,userBattingHand );
    // type_of_shot = typeOfShot(downswing_start_index,max_or_impact_index,followthrogh_end_index);

    r1 = Roll[downswing_start_index];
    r2 = Roll[followthrogh_end_index];
    //    printf("%f \n",batdirectionAtImpactorMax);
    if (type_of_shot == 1) {
        shot_t = 20;
        if (userBattingHand == 1) {
            if (batdirectionAtImpactorMax <= 100 && batdirectionAtImpactorMax >= 15) {
                shot_t = 3;
            } else if (batdirectionAtImpactorMax <= 179 && batdirectionAtImpactorMax > 100) {
                shot_t = 7;
            } else if (batdirectionAtImpactorMax < 15 && batdirectionAtImpactorMax >= -15) {
                shot_t = 1;
            } else if (batdirectionAtImpactorMax < -15 && batdirectionAtImpactorMax >= -50) {
                shot_t = 11;
            } else if (batdirectionAtImpactorMax < -50 && batdirectionAtImpactorMax >= -179) {
                shot_t = 8;
            }
        } else {
            if (batdirectionAtImpactorMax <= 100 && batdirectionAtImpactorMax >= 15) {
                shot_t = 3;
            } else if (batdirectionAtImpactorMax <= 179 && batdirectionAtImpactorMax > 100) {
                shot_t = 7;
            } else if (batdirectionAtImpactorMax <= 15 && batdirectionAtImpactorMax >= -15) {
                shot_t = 1;
            } else if (batdirectionAtImpactorMax < -15 && batdirectionAtImpactorMax >= -70) {
                shot_t = 11;
            } else if (batdirectionAtImpactorMax < -50 && batdirectionAtImpactorMax >= -179) {
                shot_t = 8;
            }
        }
        if (userBattingHand == 1) {
            r = r2 - r1;
            if (r < -30) {
                shotType = 3;
            } else if (r > -30 && r < 0) {
                shotType = 1;
            } else if (r > 5) {
                shotType = 11;
            }
        } else {
            r = r2 - r1;
            if (r > 5) {
                shotType = 3;
            } else if (r > -30 && r < 0) {
                shotType = 1;
            } else if (r < -30) {
                shotType = 11;
            }
        }
    } else {
        if (userBattingHand == 1) {
            if (batdirectionAtImpactorMax <= 100 && batdirectionAtImpactorMax >= 0) {
                shot_t = 5;
            } else if (batdirectionAtImpactorMax <= 179 && batdirectionAtImpactorMax > 100) {
                shot_t = 13;
            } else if (batdirectionAtImpactorMax < 0 && batdirectionAtImpactorMax >= -50) {
                shot_t = 10;
            } else if (batdirectionAtImpactorMax < -50 && batdirectionAtImpactorMax >= -179) {
                shot_t = 9;
            }
        } else {
            if (batdirectionAtImpactorMax <= 100 && batdirectionAtImpactorMax >= 0) {
                shot_t = 5;
            } else if (batdirectionAtImpactorMax <= 179 && batdirectionAtImpactorMax > 100) {
                shot_t = 13;
            } else if (batdirectionAtImpactorMax < 0 && batdirectionAtImpactorMax >= -50) {
                shot_t = 10;
            } else if (batdirectionAtImpactorMax < -50 && batdirectionAtImpactorMax >= -179) {
                shot_t = 9;
            }
        }
    }
    if (shot_t == 20) {
        batdirectionAtImpactorMax2 = calculateBatDirectionforShotTypeAverage(orientation_App,
                                                                             orientation_Ana,
                                                                             max_or_impact_index,
                                                                             userBattingHand);
        if (batdirectionAtImpactorMax2 > 30 && batdirectionAtImpactorMax2 <= 179) {
            shot_t = 21;
        } else if (batdirectionAtImpactorMax2 < -30 && batdirectionAtImpactorMax2 >= -179) {
            shot_t = 23;
        } else if (batdirectionAtImpactorMax2 <= 30 && batdirectionAtImpactorMax2 >= -30) {
            shot_t = 22;
        }
    } else if (shot_t == 0) {
        batdirectionAtImpactorMax2 = calculateBatDirectionforShotTypeAverage(orientation_App,
                                                                             orientation_Ana,
                                                                             max_or_impact_index,
                                                                             userBattingHand);
        if (batdirectionAtImpactorMax2 > 30 && batdirectionAtImpactorMax2 <= 179) {
            shot_t = -1;
        } else if (batdirectionAtImpactorMax2 < -30 && batdirectionAtImpactorMax2 >= -179) {
            shot_t = -3;
        } else if (batdirectionAtImpactorMax2 <= 30 && batdirectionAtImpactorMax2 >= -30) {
            shot_t = -2;
        }
    }
    // this part is added for unity (extra shot type is added because cover with wrist break in going in horizontal bat
    // shot
    if (shot_t == 5) {
        double batdirectionAtImpactorMax3 = calculateBatDirectionforShotTypeAverage(orientation_App,
                                                                                    orientation_Ana,
                                                                                    max_or_impact_index,
                                                                                    userBattingHand);
        if (batdirectionAtImpactorMax3 >= 30 && batdirectionAtImpactorMax3 <= 60) {
            shot_t = 33;
        }
    }
    //    printf(" YES YS %d \n",shot_t);
    return shot_t;
}

int change_stright_to_on_drive(int shot_type, int impact_index, int wrist_break,
                               double batimpactirection_ana) {
    int shot = 1;
    //    printf("%f \n",batimpactirection_ana);
    if (shot_type == 1 && impact_index > 0 && wrist_break > 0) {
        if (wrist_break <= impact_index - 3) {
            if (batimpactirection_ana < 0 && batimpactirection_ana >= -15) {
                shot = 11;
            }
        }
    }
    return shot;
}

int calculateWristBreakForUnity(double orientation, int userbatting_hand, int max_or_impactindex) {
    // int shot_type_2 =shotAnalysis(Xg,Yg,orientation,SI,max_or_impactindex,FTI,userbatting_hand),;
    int shot_type_2 = PST;
    int index = 0, index2 = 0, index3 = 0, index4 = 0, count = 0, count2 = 0;
    if (userbatting_hand == 1) {
        if (SHV == 1) {
            if (shot_type_2 == 3 || shot_type_2 == 11 || shot_type_2 == 8 || shot_type_2 == 1 ||
                shot_type_2 == 7) {
                for (i = max_or_impactindex; i <= FTI; i++) {
                    if (Yg[i] < 0 && Yg[i + 1] < 0 && Yg[i + 2] < 0) {
                        index2 = i;
                        int j = 0;
                        for (j = index2; j > SI; j--) {
                            if (Yg[j] > 0 && Yg[j - 1] > 0) {
                                index2 = j;
                                //                                printf(" %f %d HYE\n",Yg[j],index2);
                                break;
                            }
                        }
                        for (i = index2; i <= FTI; i++) {
                            if (Yg[i] < -200) {
                                count++;
                                if (Yg[i] < -400) {
                                    count2++;
                                }
                            } else if (Yg[i] > 0 &&
                                       Yg[i + 1] > 0) // here checking for where Yg negative ends.
                            {
                                index4 = i; // if negative Yg is till FTI then this is wrist break else not.
                            }
                        }
                        //                        printf("%d %d %d %d  \n",count,count2,index2,FTI);
                        if (count > 6 && index2 < FTI - 10 && index4 == 0) {
                            index = index2;
                        } else if (count2 > 4 && index2 < FTI - 5 && index4 == 0) {
                            index = index2;
                        }
                    }
                    //                    printf(" Yes %d %d %d %f \n",index,max_or_impactindex,FTI,Xg[max_or_impactindex]);
                }
                if (index < FTI && index4 ==
                                   0) // if wrist break is less than FTI then we have to chek and Yg value grater than -400 means -500
                {
                    index3 = index;
                    for (i = index; i < FTI; i++) {
                        if (Yg[i] < -400) {
                            index = index3;
                            break;
                        } else {
                            index = 0;
                        }
                    }
                }
            }
        } else {
            if (shot_type_2 == 33) {
                for (i = max_or_impactindex; i < FTI; i++) {
                    if (Yg[i] < 0 && Yg[i + 1] < 0 && Yg[i + 2] < 0) {
                        index2 = i;
                        int j = 0;
                        for (j = index2; j > SI; j--) {
                            if (Yg[j] > 0) {
                                index2 = j;
                                break;
                            }
                        }
                        for (i = index2; i <= FTI; i++) {
                            if (Yg[i] < -200) {
                                count++;
                                if (Yg[i] < -400) {
                                    count2++;
                                }
                            } else if (Yg[i] > 0 && Yg[i + i] > 0) {
                                index4 = i;
                            }
                        }
                        if (count > 6 && index2 < FTI - 10 && index4 == 0) {
                            index = index2;
                        } else if (count2 > 4 && index2 < FTI - 7 && index4 == 0) {
                            index = index2;
                        }
                    }
                }
            }
        }
    } else {
        if (SHV == 1) {
            if (shot_type_2 == 3 || shot_type_2 == 11 || shot_type_2 == 8 || shot_type_2 == 1 ||
                shot_type_2 == 7) {
                for (i = max_or_impactindex; i < FTI; i++) {
                    if (Yg[i] > 0 && Yg[i + 1] > 0 && Yg[i + 2] > 0) {
                        index2 = i;
                        int j = 0;
                        for (j = index2; j > SI; j--) {
                            if (Yg[j] < 0) {
                                index2 = j;
                                break;
                            }
                        }
                        for (i = index2; i <= FTI; i++) {
                            if (Yg[i] > 200) {
                                count++;
                                if (Yg[i] > 400) {
                                    count2++;
                                }
                            } else if (Yg[i] < 0 &&
                                       Yg[i + 1] < 0) // here checking for where Yg negative ends.
                            {
                                index4 = i; // if negative Yg is till FTI then this is wrist break else not.
                            }
                        }
                        if (count >= 6 && index2 < FTI - 8 && index4 == 0) {
                            index = index2;
                        } else if (count2 > 4 && index2 < FTI - 7 && index4 == 0) {
                            index = index2;
                        }
                    }
                }
                if (index < FTI && index4 ==
                                   0) // if wrist break is less than FTI then we have to chek and Yg value grater than -400 means -500
                {
                    index3 = index;
                    for (i = index; i < FTI; i++) {
                        if (Yg[i] > 400) {
                            index = index3;
                            break;
                        } else {
                            index = 0;
                        }
                    }
                }
            }
        } else {
            if (shot_type_2 == 33) {
                for (i = max_or_impactindex; i < FTI; i++) {
                    if (Yg[i] > 0 && Yg[i + 1] > 0 && Yg[i + 2] > 0) {
                        index2 = i;
                        int j = 0;
                        for (j = index2; j > SI; j--) {
                            if (Yg[j] < 0) {
                                index2 = j;
                                break;
                            }
                        }
                        for (i = index2; i <= FTI; i++) {
                            if (Yg[i] > 200) {
                                count++;
                                if (Yg[i] > 400) {
                                    count2++;
                                }
                            } else if (Yg[i] < 0 && Yg[i + i] < 0) {
                                index4 = i;
                            }
                        }
                        if (count > 6 && index2 < FTI - 10 && index4 == 0) {
                            index = index2;
                        } else if (count2 > 4 && index2 < FTI - 7 && index4 == 0) {
                            index = index2;
                        }
                    }
                }
            }
        }
    }
    return index;
}

int calculateLastIndexforUnityAfterWristBreak(int wristbreakindex) {
    int index = 130;
    for (i = wristbreakindex + 1; i < 130; i++) {
        if (Yg[i] > 0 && i > FTI) {
            index = i;
            break;
        } else if (Yg[i] > 0 && i < FTI) {
            index = FTI;
            break;
        }
    }

    return index;
}

void calculateUnityDataPosition(double orientation, int userbatting_hand, int max_or_impactindex,
                                int stop_or_not) {
    int diff2 = FTI - BSI, diff3 = SI - BSI;
    double sum_Xa1 = 0, factX1 = 0, sum_Xa2 = 0, factX2 = 0, sum_Xa3 = 0, factX3 = 0, sum_Ya1 = 0, factY1 = 0, sum_Ya2 = 0, factY2 = 0,
            sum_Ya3 = 0, factY3 = 0, sum_Za1 = 0, factZ1 = 0, sum_Za2 = 0, factZ2 = 0, sum_Za3 = 0, factZ3 = 0;
    calculateAverageAndInsert(Xpos_Filter,
                              Xa); // first taking the average of two number and insert in between of these two,
    calculateAverageAndInsert(Ypos_Filter, Ya);
    calculateAverageAndInsert(Zpos_Filter, Za);
    // int shotType = typeOfShot(SI,max_or_impactindex,FTI);
    int shot_type = SHV; // for horizontal or vertical
    // int shot_type_2 = shotAnalysis(Xg,Yg,orientation,SI,max_or_impactindex,FTI,userbatting_hand);
    int shot_type_2 = PST;
    if (userbatting_hand == 1) {
        if (shot_type == 1) {
            /*if(shot_type_2 == 3 && WBI_UN != 0)
             {
             int indexY = 2 * max_or_impactindex - 1,indexX = 2 * max_or_impactindex - 1;
             for( i = 2 * BSI + 1 ;i < 2 * SI + 1; i++)
             {
             sum_Ya1 = sum_Ya1 + Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
             if(Zpos_Filter[i] < 0 )
             {
             sum_Za1 = sum_Za1 - Zpos_Filter[i];
             }
             else
             {
             sum_Za1 = sum_Za1 + Zpos_Filter[i];
             }
             }
             for( i = 2 * SI +1 ;i < indexY ;i++)
             {
             sum_Ya2 = sum_Ya2 + Ypos_Filter[i];
             if(YawY_U_Filter[i-2*BSI-1] > 170 )
             {
             indexY = i;
             break;
             }
             }
             for( i = indexY ; i < 2 * FTI - 1 ; i++)
             {
             sum_Ya3 = sum_Ya3 + Ypos_Filter[i];
             }
             for( i = 2 * SI + 1 ; i < indexX ;i++)
             {
             if(Ypos_Filter[i] < 0)
             {
             sum_Xa2 = sum_Xa2 - Ypos_Filter[i];
             }
             else
             {
             sum_Xa2 = sum_Xa2 + Ypos_Filter[i];
             }

             if(Zpos_Filter[i] < 0 )
             {
             sum_Za2 = sum_Za2 - Zpos_Filter[i];
             }
             else
             {
             sum_Za2 = sum_Za2 + Zpos_Filter[i];
             }

             if(YawY_U_Filter[i-2*BSI-1] < -90 )
             {
             indexX = i;
             break;
             }
             }
             for( i = indexX ; i < 2 * FTI - 1 ; i++)
             {
             if(Ypos_Filter[i] < 0)
             {
             sum_Xa3 = sum_Xa3 - Ypos_Filter[i];
             }
             else
             {
             sum_Xa3 = sum_Xa3 + Ypos_Filter[i];
             }

             if(Zpos_Filter[i] < 0 )
             {
             sum_Za3 = sum_Za3 - Zpos_Filter[i];
             }
             else
             {
             sum_Za3 = sum_Za3 + Zpos_Filter[i];
             }
             }
             if(sum_Xa2 < 0)
             {
             sum_Xa2 = - sum_Xa2;   // if sum is negative,making it positive
             }
             if(sum_Xa3 < 0)
             {
             sum_Xa3 = - sum_Xa3;   // if sum is negative,making it positive
             }
             if(sum_Ya1<0)
             {
             sum_Ya1= - sum_Ya1;
             }
             if(sum_Ya2<0)
             {
             sum_Ya2 = - sum_Ya2;
             }
             if(sum_Ya3<0)
             {
             sum_Ya3 = - sum_Ya3;
             }
             if(sum_Za1<0)
             {
             sum_Za1 = - sum_Za1;
             }
             if(sum_Za2<0)
             {
             sum_Za2 = - sum_Za2;
             }
             factX2 = sum_Xa2 / (40*( ( indexX ) - ( 2*SI - 1)));
             factX3 = sum_Xa3 / (40*( ( 2*FTI-1) - (indexX)));
             factY1 = sum_Ya1 / (20*( ( 2*SI-1) - ( 2*BSI - 1)));
             factY2 = sum_Ya2 / (20*( ( indexY ) - ( 2*SI - 1)));
             factY3 = sum_Ya3 / (10*( ( 2*FTI-1) - (indexY)));
             if(sum_Za1 / 40 > 10)
             {
             factZ1 = 10 / ( ( 2*SI-1) - ( 2*BSI - 1));
             }
             else
             {
             factZ1 = sum_Za1 / (40*( ( 2*SI-1) - ( 2*BSI - 1)));
             }
             factZ2 = sum_Za2 / (40*( ( indexX ) - ( 2*SI - 1)));
             factZ3 = sum_Za3 / (20*( ( 2*FTI-1) - (indexX)));
             Xpos_Unity_acc[0] = 0;
             Ypos_Unity_acc[0] = 0;
             Zpos_Unity_acc[0] = 0;
             //printf("%f %f %f \n",factY1,factY2,factY3);
             for( i = 1 ; i < 2 * 129 -1 ; i++)
             {
             if( i + 2*BSI-1 <= 2*SI-1)
             {
             Ypos_Unity_acc[i] =  Ypos_Unity_acc[i-1] + factY1; // in Ypos_Unity_acc adding the factZ1
             }
             else if(i + 2*BSI-1> 2*SI-1 && i + 2*BSI-1<= indexY  )
             {
             Ypos_Unity_acc[i] = Ypos_Unity_acc[i-1] - factY2;
             }
             else if(i + 2*BSI-1 > indexY && i + 2*BSI-1 <= 2*FTI-1 )
             {
             Ypos_Unity_acc[i] = Ypos_Unity_acc[i-1] + factY3;//
             }
             else if(i + 2*BSI-1 > 2*FTI-1 && i + 2*BSI-1 <= 2 * 129-1 )
             {
             Ypos_Unity_acc[i] =  Ypos_Unity_acc[i-1];
             }
             }
             for( i = 1 ; i < 2 * 129 -1 ; i++)
             {
             if( i + 2*BSI-1 <= 2*SI-1 )
             {
             Xpos_Unity_acc[i] =  Xpos_Unity_acc[i-1] + factX1; // in Ypos_Unity_acc adding the factZ1
             Zpos_Unity_acc[i] =  Zpos_Unity_acc[i-1] + factZ1;
             }
             else if(i + 2*BSI-1> 2*SI-1 && i + 2*BSI-1<= indexX  )
             {
             Xpos_Unity_acc[i] = Xpos_Unity_acc[i-1] - factX2;
             Zpos_Unity_acc[i] =  Zpos_Unity_acc[i-1] - factZ2;
             }
             else if(i + 2*BSI-1 > indexX && i + 2*BSI-1 <= 2*FTI-1 )
             {
             //Xpos_Unity_acc[i] = Xpos_Unity_acc[i-1] + factX3;// made changes on 17th december ishwinder sir testing purpose
             Xpos_Unity_acc[i] =   Xpos_Unity_acc[i-1];
             Zpos_Unity_acc[i] =  Zpos_Unity_acc[i-1] + factZ3;
             if(Zpos_Unity_acc[i] > 10)
             {
             Zpos_Unity_acc[i] = 10;
             }
             }
             else if(i + 2*BSI-1 >= 2*FTI-1 && i + 2*BSI-1 <= 2*129-1 )
             {
             Xpos_Unity_acc[i] = Xpos_Unity_acc[i-1] ;
             Zpos_Unity_acc[i] =  Zpos_Unity_acc[i-1];
             if(Zpos_Unity_acc[i] > 10)
             {
             Zpos_Unity_acc[i] = 10;
             }
             }
             }
             }*/
            // else
            //{
            for (i = 2 * BSI - 1; i < 2 * FTI - 1; i++) {
                if (i < 2 * SI - 1) {
                    sum_Ya1 = sum_Ya1 +
                              Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
                    sum_Za1 = sum_Za1 + Zpos_Filter[i];
                } else if (i > 2 * SI - 1 && i < 2 * max_or_impactindex - 1) {
                    sum_Ya2 = sum_Ya2 +
                              Ypos_Filter[i]; // sum from downswing start to max or impact index
                    sum_Za2 = sum_Za2 + Zpos_Filter[i];
                } else if (i > 2 * max_or_impactindex - 1 && i < 2 * FTI - 1) {
                    sum_Ya3 = sum_Ya3 +
                              Ypos_Filter[i]; // sum from max or impact index to followthrough index
                    sum_Xa3 = sum_Xa3 + Xpos_Filter[i];
                }
            }
            if (sum_Xa3 < 0) {
                sum_Xa3 = -sum_Xa3; // if sum is negative,making it positive
            }
            if (sum_Ya1 < 0) {
                sum_Ya1 = -sum_Ya1;
            }
            if (sum_Ya2 < 0) {
                sum_Ya2 = -sum_Ya2;
            }
            if (sum_Ya3 < 0) {
                sum_Ya3 = -sum_Ya3;
            }
            if (sum_Za1 < 0) {
                sum_Za1 = -sum_Za1;
            }
            if (sum_Za2 < 0) {
                sum_Za2 = -sum_Za2;
            }
            factX3 = sum_Xa3 / (20 * ((2 * FTI - 1) -
                                      (2 * max_or_impactindex - 1))); // calculating the factor
            // printf("%f \n",factX3);
            if (sum_Ya1 / 20 > 10) {
                factY1 = 10 / (((2 * SI - 1) - (2 * BSI - 1)));
            } else {
                factY1 = sum_Ya1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
            }
            factY2 = sum_Ya2 / (20 * ((2 * max_or_impactindex - 1) - (2 * SI - 1)));
            factY3 = sum_Ya3 / (10 * ((2 * FTI - 1) - (2 * max_or_impactindex - 1)));
            factZ1 = sum_Za1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
            factZ2 = sum_Za2 / (10 * ((2 * max_or_impactindex - 1) - (2 * SI - 1)));
            if (stop_or_not ==
                1) // this is added for stops only, if that is stop then we are dividing it with 40 in normal we are dividing it by 10
            {
                factZ2 = factZ2 / 4;
            }
            if (FTI - max_or_impactindex <
                15) // this condition is added for sudden change in Zdisplacement data with few index
            {
                factY3 = factY3 / 2;
            }
            Xpos_Unity_acc[0] = 0;
            Ypos_Unity_acc[0] = 0;
            Zpos_Unity_acc[0] = 0;
            for (i = 1; i < 2 * diff2 - 1; i++) {
                Xpos_Unity_acc[i] = 0;
                if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                    Ypos_Unity_acc[i] =
                            Ypos_Unity_acc[i - 1] + factZ1; // in Ypos_Unity_acc adding the factZ1
                    Zpos_Unity_acc[i] =
                            Zpos_Unity_acc[i - 1] + factY1; // in Zpos Unity_acc adding the factY1
                    // printf(" %d %f  \n",i,Zpos_Unity_acc[i]);
                } else if (i + 2 * BSI - 1 > 2 * SI - 1 &&
                           i + 2 * BSI - 1 <= 2 * max_or_impactindex - 1) {
                    Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] - factY2;
                    Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] - factZ2;
                    // printf(" %d %f  \n",i,Zpos_Unity_acc[i]);
                } else if (i + 2 * BSI - 1 > 2 * max_or_impactindex - 1 &&
                           i + 2 * BSI - 1 < 2 * FTI - 1) {
                    if (shot_type_2 == 11 || shot_type_2 == 8 || shot_type == 1 || shot_type == 3) {
                        if (WBI_UN == 0) // this change made only for testing purpose
                        {
                            Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] - factX3;
                        } else {
                            Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1];
                        }
                    } else {
                        if (WBI_UN == 0) {
                            Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] - factX3;
                        } else {
                            Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] + factX3;
                        }
                    }
                    Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1];
                    // Zpos_Unity_acc[i] = Zpos_Unity_acc[i-1] + factY3;
                    if (WBI_UN ==
                        0) // in case of wrist break and all type of shot we are reducing the factor in followthrough
                    {
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factY3;
                        if (Zpos_Unity_acc[i] > 10) {
                            Zpos_Unity_acc[i] = 10;
                        }
                        // printf(" %d %f  \n",i,Zpos_Unity_acc[i]);
                    } else {
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factY3 / 3;
                        if (Zpos_Unity_acc[i] > 10) {
                            Zpos_Unity_acc[i] = 10;
                        }
                    }
                }
            }
            //}
        } else {
            if (shot_type_2 == 33) {
                int indexY = 2 * FTI - 1, indexX = 2 * FTI - 1;
                for (i = 2 * BSI + 1; i < 2 * SI + 1; i++) {
                    sum_Ya1 = sum_Ya1 +
                              Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
                    if (Zpos_Filter[i] < 0) {
                        sum_Za1 = sum_Za1 - Zpos_Filter[i];
                    } else {
                        sum_Za1 = sum_Za1 + Zpos_Filter[i];
                    }
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Ya2 = sum_Ya2 + Ypos_Filter[i];
                    if (YawY_U_Filter[i - 2 * BSI - 1] > 170) {
                        indexY = i;
                        break;
                    }
                }
                for (i = indexY; i < 2 * FTI - 1; i++) {
                    sum_Ya3 = sum_Ya3 + Ypos_Filter[i];
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Xa2 = sum_Xa2 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za2 = sum_Za2 - Zpos_Filter[i];
                    } else {
                        sum_Za2 = sum_Za2 + Zpos_Filter[i];
                    }
                    if (YawY_U_Filter[i - 2 * BSI - 1] < -90) {
                        indexX = i;
                        break;
                    }
                }

                for (i = indexX; i < 2 * FTI - 1; i++) {
                    sum_Xa3 = sum_Xa3 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za3 = sum_Za3 - Zpos_Filter[i];
                    } else {
                        sum_Za3 = sum_Za3 + Zpos_Filter[i];
                    }
                }
                if (sum_Xa2 < 0) {
                    sum_Xa2 = -sum_Xa2; // if sum is negative,making it positive
                }
                if (sum_Xa3 < 0) {
                    sum_Xa3 = -sum_Xa3; // if sum is negative,making it positive
                }
                if (sum_Ya1 < 0) {
                    sum_Ya1 = -sum_Ya1;
                }
                if (sum_Ya2 < 0) {
                    sum_Ya2 = -sum_Ya2;
                }
                if (sum_Ya3 < 0) {
                    sum_Ya3 = -sum_Ya3;
                }
                if (sum_Za1 < 0) {
                    sum_Za1 = -sum_Za1;
                }
                if (sum_Za2 < 0) {
                    sum_Za2 = -sum_Za2;
                }
                factX2 = sum_Xa2 / (40 * ((indexX) - (2 * SI - 1)));
                factX3 = sum_Xa3 / (40 * ((2 * FTI - 1) - (indexX)));
                factY1 = sum_Ya1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
                factY2 = sum_Ya2 / (20 * ((indexY) - (2 * SI - 1)));
                factY3 = sum_Ya3 / (10 * ((2 * FTI - 1) - (indexY)));
                factZ1 = sum_Za1 / (40 * ((2 * SI - 1) - (2 * BSI - 1)));
                factZ2 = sum_Za2 / (40 * ((indexX) - (2 * SI - 1)));
                factZ3 = sum_Za3 / (40 * ((2 * FTI - 1) - (indexX)));
                Xpos_Unity_acc[0] = 0;
                Ypos_Unity_acc[0] = 0;
                Zpos_Unity_acc[0] = 0;
                for (i = 1; i < 2 * 129 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] +
                                            factY1; // in Ypos_Unity_acc adding the factZ1
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexY) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] - factY2;
                    } else if (i + 2 * BSI - 1 > indexY && i + 2 * BSI - 1 <= 2 * FTI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] + factY3;
                    } else if (i + 2 * BSI - 1 > 2 * FTI - 1 && i + 2 * BSI - 1 <= 2 * 129 - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1];
                    }
                }
                for (i = 1; i < 2 * 129 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] +
                                            factX1; // in Ypos_Unity_acc adding the factZ1
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ1;
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexX) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] - factX2;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] - factZ2;
                    } else if (i + 2 * BSI - 1 > indexX && i + 2 * BSI - 1 <= 2 * FTI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] + factX3;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ3;
                    } else if (i + 2 * BSI - 1 >= 2 * FTI - 1 && i + 2 * BSI - 1 <= 2 * 129 - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1];
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1];
                    }
                }
            } else {
                int indexY = 2 * FTI - 1, indexX = 2 * FTI - 1;
                for (i = 2 * BSI + 1; i < 2 * SI + 1; i++) {
                    sum_Ya1 = sum_Ya1 +
                              Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
                    if (Zpos_Filter[i] < 0) {
                        sum_Za1 = sum_Za1 - Zpos_Filter[i];
                    } else {
                        sum_Za1 = sum_Za1 + Zpos_Filter[i];
                    }
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Ya2 = sum_Ya2 + Ypos_Filter[i];
                    if (YawY_U_Filter[i - 2 * BSI - 1] > 179) {
                        indexY = i;
                        break;
                    }
                }
                for (i = indexY; i < 2 * FTI - 1; i++) {
                    sum_Ya3 = sum_Ya3 + Ypos_Filter[i];
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Xa2 = sum_Xa2 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za2 = sum_Za2 - Zpos_Filter[i];
                    } else {
                        sum_Za2 = sum_Za2 + Zpos_Filter[i];
                    }
                    if (YawY_U_Filter[i - 2 * BSI - 1] < -90) {
                        indexX = i;
                        break;
                    }
                }
                for (i = indexX; i < 2 * FTI - 1; i++) {
                    sum_Xa3 = sum_Xa3 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za3 = sum_Za3 - Zpos_Filter[i];
                    } else {
                        sum_Za3 = sum_Za3 + Zpos_Filter[i];
                    }
                }
                if (sum_Xa2 < 0) {
                    sum_Xa2 = -sum_Xa2; // if sum is negative,making it positive
                }
                if (sum_Xa3 < 0) {
                    sum_Xa3 = -sum_Xa3; // if sum is negative,making it positive
                }
                if (sum_Ya1 < 0) {
                    sum_Ya1 = -sum_Ya1;
                }
                if (sum_Ya2 < 0) {
                    sum_Ya2 = -sum_Ya2;
                }
                if (sum_Ya3 < 0) {
                    sum_Ya3 = -sum_Ya3;
                }
                if (sum_Za1 < 0) {
                    sum_Za1 = -sum_Za1;
                }
                if (sum_Za2 < 0) {
                    sum_Za2 = -sum_Za2;
                }
                factX2 = sum_Xa2 / (40 * ((indexX) - (2 * SI - 1)));
                factX3 = 2 * sum_Xa3 / (40 * ((2 * FTI - 1) - (indexX)));
                factY1 = sum_Ya1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
                factY2 = sum_Ya2 / (20 * ((indexY) - (2 * SI - 1)));
                factY3 = sum_Ya3 / (10 * ((2 * FTI - 1) - (indexY)));
                factZ1 = sum_Za1 / (40 * ((2 * SI - 1) - (2 * BSI - 1)));
                factZ2 = sum_Za2 / (40 * ((indexX) - (2 * SI - 1)));
                factZ3 = sum_Za3 / (40 * ((2 * FTI - 1) - (indexX)));
                Xpos_Unity_acc[0] = 0;
                Ypos_Unity_acc[0] = 0;
                Zpos_Unity_acc[0] = 0;
                for (i = 1; i < 2 * diff2 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] +
                                            factY1; // in Ypos_Unity_acc adding the factZ1
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexY) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] - factY2;
                    } else if (i + 2 * BSI - 1 > indexY && i + 2 * BSI - 1 < 2 * FTI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] + factY3;
                    }
                }
                for (i = 1; i < 2 * diff2 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] +
                                            factX1; // in Ypos_Unity_acc adding the factZ1
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ1;
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexX) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] - factX2;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] - factZ2;
                    } else if (i + 2 * BSI - 1 > indexX && i + 2 * BSI - 1 < 2 * FTI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] + factX3;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ3;
                    }
                }
            }
        }
    } else {
        if (shot_type == 1) {
            if (shot_type_2 == 3 && WBI_UN != 0) {
                int indexY = 2 * FTI - 1, indexX = 2 * FTI - 1;
                for (i = 2 * BSI + 1; i < 2 * SI + 1; i++) {
                    sum_Ya1 = sum_Ya1 +
                              Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
                    if (Zpos_Filter[i] < 0) {
                        sum_Za1 = sum_Za1 - Zpos_Filter[i];
                    } else {
                        sum_Za1 = sum_Za1 + Zpos_Filter[i];
                    }
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Ya2 = sum_Ya2 + Ypos_Filter[i];
                    if (YawY_U_Filter[i - 2 * BSI - 1] > 170) {
                        indexY = i;
                        break;
                    }
                }
                for (i = indexY; i < 2 * FTI - 1; i++) {
                    sum_Ya3 = sum_Ya3 + Ypos_Filter[i];
                }
                for (i = 2 * SI + 1; i < 2 * FTI - 1; i++) {
                    sum_Xa2 = sum_Xa2 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za2 = sum_Za2 - Zpos_Filter[i];
                    } else {
                        sum_Za2 = sum_Za2 + Zpos_Filter[i];
                    }
                    if (YawY_U_Filter[i - 2 * BSI - 1] < -90) {
                        indexX = i;
                        break;
                    }
                }
                for (i = indexX; i < 2 * FTI - 1; i++) {
                    sum_Xa3 = sum_Xa3 + Ypos_Filter[i];
                    if (Zpos_Filter[i] < 0) {
                        sum_Za3 = sum_Za3 - Zpos_Filter[i];
                    } else {
                        sum_Za3 = sum_Za3 + Zpos_Filter[i];
                    }
                }
                if (sum_Xa2 < 0) {
                    sum_Xa2 = -sum_Xa2; // if sum is negative,making it positive
                }
                if (sum_Xa3 < 0) {
                    sum_Xa3 = -sum_Xa3; // if sum is negative,making it positive
                }
                if (sum_Ya1 < 0) {
                    sum_Ya1 = -sum_Ya1;
                }
                if (sum_Ya2 < 0) {
                    sum_Ya2 = -sum_Ya2;
                }
                if (sum_Ya3 < 0) {
                    sum_Ya3 = -sum_Ya3;
                }
                if (sum_Za1 < 0) {
                    sum_Za1 = -sum_Za1;
                }
                if (sum_Za2 < 0) {
                    sum_Za2 = -sum_Za2;
                }
                factX2 = sum_Xa2 / (40 * ((indexX) - (2 * SI - 1)));
                factX3 = sum_Xa3 / (40 * ((2 * FTI - 1) - (indexX)));
                factY1 = sum_Ya1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
                factY2 = sum_Ya2 / (20 * ((indexY) - (2 * SI - 1)));
                factY3 = sum_Ya3 / (10 * ((2 * FTI - 1) - (indexY)));
                factZ1 = sum_Za1 / (40 * ((2 * SI - 1) - (2 * BSI - 1)));
                factZ2 = sum_Za2 / (40 * ((indexX) - (2 * SI - 1)));
                factZ3 = sum_Za3 / (40 * ((2 * FTI - 1) - (indexX)));
                Xpos_Unity_acc[0] = 0;
                Ypos_Unity_acc[0] = 0;
                Zpos_Unity_acc[0] = 0;
                for (i = 1; i < 2 * 129 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] +
                                            factY1; // in Ypos_Unity_acc adding the factZ1
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexY) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] - factY2;
                    } else if (i + 2 * BSI - 1 > indexY && i + 2 * BSI - 1 <= 2 * FTI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] + factY3;
                    } else if (i + 2 * BSI - 1 > 2 * FTI - 1 && i + 2 * BSI - 1 <= 2 * 129 - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1];
                    }
                }
                for (i = 1; i < 2 * 129 - 1; i++) {
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] +
                                            factX1; // in Ypos_Unity_acc adding the factZ1
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ1;
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 && i + 2 * BSI - 1 <= indexX) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] - factX2;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] - factZ2;
                    } else if (i + 2 * BSI - 1 > indexX && i + 2 * BSI - 1 <= 2 * FTI - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1] + factX3;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factZ3;
                    } else if (i + 2 * BSI - 1 >= 2 * FTI - 1 && i + 2 * BSI - 1 <= 2 * 129 - 1) {
                        Xpos_Unity_acc[i] = Xpos_Unity_acc[i - 1];
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1];
                    }
                }
            } else {
                for (i = 2 * BSI - 1; i < 2 * FTI - 1; i++) {
                    if (i < 2 * SI - 1) {
                        sum_Ya1 = sum_Ya1 +
                                  Ypos_Filter[i]; // sum from backlift start to downswing start on filtered data
                        sum_Za1 = sum_Za1 + Zpos_Filter[i];
                    } else if (i > 2 * SI - 1 && i < 2 * max_or_impactindex - 1) {
                        sum_Ya2 = sum_Ya2 +
                                  Ypos_Filter[i]; // sum from downswing start to max or impact index
                        sum_Za2 = sum_Za2 + Zpos_Filter[i];
                    } else if (i > 2 * max_or_impactindex - 1 && i < 2 * FTI - 1) {
                        sum_Ya3 = sum_Ya3 +
                                  Ypos_Filter[i]; // sum from max or impact index to followthrough index
                        sum_Xa3 = sum_Xa3 + Xpos_Filter[i];
                    }
                }
                if (sum_Xa3 < 0) {
                    sum_Xa3 = -sum_Xa3; // if sum is negative,making it positive
                }
                if (sum_Ya1 < 0) {
                    sum_Ya1 = -sum_Ya1;
                }
                if (sum_Ya2 < 0) {
                    sum_Ya2 = -sum_Ya2;
                }
                if (sum_Ya3 < 0) {
                    sum_Ya3 = -sum_Ya3;
                }
                if (sum_Za1 < 0) {
                    sum_Za1 = -sum_Za1;
                }
                if (sum_Za2 < 0) {
                    sum_Za2 = -sum_Za2;
                }
                factX3 = sum_Xa3 / (10 * ((2 * FTI - 1) -
                                          (2 * max_or_impactindex - 1))); // calculating the factor
                factY1 = sum_Ya1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
                factY2 = sum_Ya2 / (20 * ((2 * max_or_impactindex - 1) - (2 * SI - 1)));
                factY3 = sum_Ya3 / (10 * ((2 * FTI - 1) - (2 * max_or_impactindex - 1)));
                factZ1 = sum_Za1 / (20 * ((2 * SI - 1) - (2 * BSI - 1)));
                factZ2 = sum_Za2 / (10 * ((2 * max_or_impactindex - 1) - (2 * SI - 1)));
                // printf("%f \n",factZ2);

                Xpos_Unity_acc[0] = 0;
                Ypos_Unity_acc[0] = 0;
                Zpos_Unity_acc[0] = 0;
                for (i = 1; i < 2 * diff2 - 1; i++) {
                    Xpos_Unity_acc[i] = 0;
                    if (i + 2 * BSI - 1 <= 2 * SI - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] +
                                            factZ1; // in Ypos_Unity_acc adding the factZ1
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] +
                                            factY1; // in Zpos Unity_acc adding the factY1
                    } else if (i + 2 * BSI - 1 > 2 * SI - 1 &&
                               i + 2 * BSI - 1 <= 2 * max_or_impactindex - 1) {
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1] - factY2;
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] - factZ2;
                    } else if (i + 2 * BSI - 1 > 2 * max_or_impactindex - 1 &&
                               i + 2 * BSI - 1 < 2 * FTI - 1) {
                        if (shot_type_2 == 11 || shot_type_2 == 8) {
                            Xpos_Unity_acc[i] = -(Xpos_Unity_acc[i - 1] + factX3);
                        } else {
                            Xpos_Unity_acc[i] = -(Xpos_Unity_acc[i - 1] - factX3);
                        }
                        Ypos_Unity_acc[i] = Ypos_Unity_acc[i - 1];
                        Zpos_Unity_acc[i] = Zpos_Unity_acc[i - 1] + factY3;
                        if (Zpos_Unity_acc[i] > 10) {
                            Zpos_Unity_acc[i] = 10;
                        }
                    }
                }
            }
        }
    }
}

int calculateStartofZminus2() {
    int index = 0;
    for (i = 0; i < 2 * (FTI - BSI) - 4; i++) {
        if (Zpos_Unity_acc[i] < -1.1) {
            index = i;
            break;
        }
    }
    return index;
}

void calculateUnityZpositionFilterDataTo2(int shot_type, int user_hand) {
    int index = calculateStartofZminus2();
    // printf("%d index \n",index);
    if (user_hand == 1) {
        if (index > 0) {
            for (i = 0; i < index; i++) {
                // if z is less  than -1.8  means -3 or -4 than make is -1.8
                Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
            }
            for (i = index; i < 2 * 129 - 1; i++) {
                if (Zpos_Unity_acc[i] < -1.1) {
                    Zpos_Unity_acc3[i] = -1.1;
                } else {
                    Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
                }
            }
        } else {
            for (i = 0; i < 2 * 129 - 1; i++) {
                Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
            }
        }
    } else {
        if (index > 0) {
            for (i = 0; i < index; i++) {
                // if z is less  than -2  means -3 or -4 than make is -2
                Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
            }
            for (i = index; i < 2 * 129 - 1; i++) {
                if (Zpos_Unity_acc[i] < -1.1) {
                    Zpos_Unity_acc3[i] = -1.1;
                } else {
                    Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
                }
            }
        } else {
            for (i = 0; i < 2 * 129 - 1; i++) {
                Zpos_Unity_acc3[i] = Zpos_Unity_acc[i];
            }
        }
    }
}

void calculateUnityZpositionFilterData(int shot_type) {
    double fac_z = 0;
    int index = calculateStartofZminus2();
    if (index > 0) {
        fac_z = (Zpos_Unity_acc3[index - 10] + 1.1) / 10;
    }
    if (shot_type == 3 && WBI_UN != 0) {
        if (index > 0) {
            for (i = 0; i < index - 10; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
            }
            for (i = index - 10; i < index; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc2[i - 2] - fac_z;
            }
            for (i = index; i < 2 * 129 - 1; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
            }
        } else {
            for (i = 0; i < 2 * 129 - 1; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
            }
        }
    } else {
        if (index > 0) {
            for (i = 0; i < index - 10; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
                // printf(" 1 ---%f \n",Zpos_Unity_acc2[i]);
            }
            for (i = index - 10; i < index; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc2[i - 2] - fac_z;
                // printf("2 ---%f \n",Zpos_Unity_acc2[i]);
            }
            for (i = index; i < 2 * FTI - 1; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
                // printf("3 ---%f \n",Zpos_Unity_acc2[i]);
            }
        } else {
            for (i = 0; i < 2 * FTI - 1; i++) {
                Zpos_Unity_acc2[i] = Zpos_Unity_acc3[i];
            }
        }
    }
}

/* 1.
 *
 *
 *
 *
 */
void calculateUnityData(double batImpactDirApp, double batImpactDirAna, double backliftdirection,
                        double orientation_app, double orientation_Ana, int max_or_impactindex,
                        int batting_hand, int firmware_version) {
    int index = 0, diff2 = FTI - BSI, diff3 = SI - BSI, breakpoint, shot_type, shot_type2;
    double previousVal = 0, fac, previousPitch = 0;
    breakpoint = calculateFirstNegativeYgInBack();
    shot_type = SHV;
    shot_type2 = PST;
    // shot type 1 is for vertical bat shots
    // printf("%f %f \n",batImpactDirAna,batImpactDirApp);
    if (shot_type == 1) {
        double Yaw_U_vali[130];
        if (firmware_version == 3) {
            for (i = 0; i < 130; i++) {
                // Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                Yaw_U_vali[i] = Yaw[i] - orientation_app;
                if (Yaw_U_vali[i] < -180) {
                    Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                } else if (Yaw_U_vali[i] > 180) {
                    Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                }
                //                printf(" %d %f  %f \n",i,Yaw_U_vali[i],orientation_app);
            }
        } else {
            if (batImpactDirApp > -15 && batImpactDirApp <=
                                         15) // if from both sensor fusion and analytics yaw impact direction is same then use sensor
            {                                                   // fusion use else analytics yaw
                if (batImpactDirAna > -15 && batImpactDirAna <= 15) {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                } else {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                }
            } else if (batImpactDirApp > 15 && batImpactDirApp <= 100) {
                if (batImpactDirAna > 15 && batImpactDirAna <= 100) {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                    }
                } else {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                }
            } else if (batImpactDirApp > 100 && batImpactDirApp <= 179) {
                if (batImpactDirAna > 100 && batImpactDirAna <= 179) {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw[i] - orientation_app;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                } else {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                }
            } else if (batImpactDirApp > -50 && batImpactDirApp <= -15) {

                if (batImpactDirAna > -50 && batImpactDirAna <= -15) {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw[i] - orientation_app;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                    }
                } else {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f %f \n",Yaw_U_vali[i],Yaw_T[i]);
                    }
                }
            } else if (batImpactDirApp > -179 && batImpactDirApp <= -50) {
                if (batImpactDirAna > -179 && batImpactDirAna <= -50) {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw[i] - orientation_app;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                } else {
                    for (i = 0; i < 130; i++) {
                        Yaw_U_vali[i] = Yaw_T[i] - orientation_Ana;
                        if (Yaw_U_vali[i] < -180) {
                            Yaw_U_vali[i] = Yaw_U_vali[i] + 360;
                        } else if (Yaw_U_vali[i] > 180) {
                            Yaw_U_vali[i] = 360 - Yaw_U_vali[i];
                        }
                        // printf("%f \n",Yaw_U_vali[i]);
                    }
                }
            }
        }
        static double RollZ_U2[130]; // local varible due to currupt values in roll value data
        double fact1 = 0;
        double fact1_1 = 0;
        int first_negative = 0;
        for (i = 0; i < 130; i++) // here we are checking index for Yrotaiton negative in backlift
        {
            if (Yindex[i] > BSI) //  that index will be greater than Backlift start index
            {
                first_negative = i;
                break;
            }
        }

        if (firmware_version == 3) {
            fact1 = ((Yaw_U_vali[SI - 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 1]) -
                     (Yaw_U_vali[BSI - 1] + Yaw_U_vali[BSI] + Yaw_U_vali[BSI + 1])) /
                    (3 * (SI - BSI));
            //            printf("%f \n",fact1);
        } else {
            if (BSI != SI - 1) {
                if (batting_hand ==
                    1) // we are calculatig factor from start of backlift direction to downswing start index
                {
                    fact1 = (-(Yaw_U_vali[SI - 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 1]) -
                             (-backliftdirection * 3)) / (3 * (SI - BSI));
                    fact1_1 = (((Yrotation_unity[Yindex[first_negative] - 1] +
                                 Yrotation_unity[SI - 1]) - (-backliftdirection)) / (SI - BSI));
                } else {
                    fact1 = ((Yaw_U_vali[SI - 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 1]) -
                             (backliftdirection * 3)) / (3 * (SI - BSI));
                    fact1_1 = (((Yrotation_unity[Yindex[first_negative] - 1] +
                                 Yrotation_unity[SI - 1]) - (backliftdirection)) / (SI - BSI));
                }
                if (fact1 > 0) {
                    if (fact1 > fact1_1) {
                        fact1 = fact1_1;
                    }
                } else if (fact1 < 0) {
                    if (fact1_1 < 0) {
                        if (fact1 < fact1_1) {
                            fact1 = fact1_1;
                        }
                    }
                }
                // to normalize yaw calculating the factor
                // fact1 = ((Yaw_U_vali[SI-1] + Yaw_U_vali[SI]+ Yaw_U_vali[SI+1]) - (Yaw_U_vali[BSI -1 ] + Yaw_U_vali[BSI] + Yaw_U_vali[BSI+1])) / (3 *(SI-BSI ));
            } else {
                if (batting_hand ==
                    1) // we are calculatig factor from start of backlift direction to downswing start index
                {
                    fact1 = ((Yaw_U_vali[SI - 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 1]) -
                             (-backliftdirection * 3)) / (3 * (SI - BSI));
                } else {
                    fact1 = ((Yaw_U_vali[SI - 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 1]) -
                             (backliftdirection * 3)) / (3 * (SI - BSI));
                }
            }
        }

        double fact2 = (((Yaw_U_vali[FTI - 1]) + (Yaw_U_vali[FTI - 2]) + (Yaw_U_vali[FTI - 3])) -
                        ((Yaw_U_vali[max_or_impactindex - 1]) + (Yaw_U_vali[max_or_impactindex]) +
                         (Yaw_U_vali[max_or_impactindex + 1]))) / (3 * (FTI - max_or_impactindex));
        //        printf("%f \n",fact2);

        if (FTI - max_or_impactindex <
            15) // this condition is added to reduce the factor , index differece is less and factor is high so sudden change will be there in yaw data , to over come this situation we are divinding the fact2 by 2.
        {
            fact2 = fact2 / 2;
            //            printf("%f \n",fact2);
        }
        if (batting_hand == 1) {
            YawY_U[0] = -backliftdirection; // in unity bat Yaw should starts form backlift direction and
            // for right handed batsman we to reverse the backliftdirection
        } else {
            YawY_U[0] = backliftdirection;
        }
        /*** this part is added to normalize pitch during backlif *****************/
        if (Pitch[BSI] < -145) {
            PitchX_U[0] = -(Pitch[BSI] + 180) - 90;
            if (PitchX_U[0] > 179) {
                PitchX_U[0] = -360 + PitchX_U[0];
            }
        } else {
            PitchX_U[0] = 90 - Pitch[BSI];
            if (PitchX_U[0] > 179) {
                PitchX_U[0] = -360 + PitchX_U[0];
            }
        }

        if (Pitch[SI] < -145) {
            PitchX_U[SI - BSI] = -(Pitch[SI] + 180) - 90;
            if (PitchX_U[SI - BSI] > 179) {
                PitchX_U[SI - BSI] = -360 + PitchX_U[SI - BSI];
            }
        } else {
            PitchX_U[SI - BSI] = 90 - Pitch[SI];
            if (PitchX_U[SI - BSI] > 179) {
                PitchX_U[SI - BSI] = -360 + PitchX_U[SI - BSI];
            }
        }

        double factPitch = (PitchX_U[SI - BSI] - PitchX_U[0]) / (SI - BSI);
        int res = (max_or_impactindex - SI) / 2;
        int rem = (max_or_impactindex - SI) % 2;

        double fact_11 =
                ((Yaw_U_vali[SI + res + 1] + Yaw_U_vali[SI + res] + Yaw_U_vali[SI + res - 1]) -
                 (Yaw_U_vali[SI + 1] + Yaw_U_vali[SI] + Yaw_U_vali[SI + 2])) / (3 * res);

        double fact_12 = ((Yaw_U_vali[SI + 2 * res + rem - 1] + Yaw_U_vali[SI + 2 * res + rem] +
                           Yaw_U_vali[SI + 2 * res + rem + 1]) -
                          (Yaw_U_vali[SI + res - 1] + Yaw_U_vali[SI + res] +
                           Yaw_U_vali[SI + res + 1])) / (3 * (res + rem));
        //        printf("%f %f \n",fact_11,fact_12);

        // this is a check if Yaw in backlift is going more than 70 then we are reducing it to half
        double checkfor90 = YawY_U[0] + (SI - BSI) * fact1;
        //        printf("%f  %f %f  \n",checkfor90,fact1,YawY_U[0]);
        if (checkfor90 > 70 || checkfor90 < -70) {
            fact1 = fact1 / 5;
        }
        if (checkfor90 < -70) {
            fact1 = fact1 / 5;
        }

        /*****************************************************************************/
        for (i = 0; i < diff2; i++) {
            if (i + BSI <= SI) {

                YawY_U[i + 1] = YawY_U[i] + fact1; //

                PitchX_U[i + 1] = PitchX_U[i] + factPitch;
                if (batting_hand == 1) {
                    RollZ_U2[i] = -Roll[i +
                                        BSI]; // in case of right handed batsman  we are not changing the ROll Values
                } else {
                    RollZ_U2[i] = -Roll[i +
                                        BSI]; // in case of left handed batsman , we are reversing the  Roll value
                }
                //                printf(" %d %f %f  %f \n",i,YawY_U[i],PitchX_U[i],fact1);
            } else if (i + BSI > SI && i + BSI <= max_or_impactindex) {

                if (shot_type2 == 11 || shot_type2 == 1 || shot_type2 == 8 || shot_type2 == 7) {

                    if (i + BSI > SI && i + BSI <= SI +
                                                   res) // breaking downswing to max or impact into three buckets
                    {
                        YawY_U[i] = YawY_U[i - 1] + fact_11;
                    } else if (i + BSI > SI + res && i + BSI <= SI + 2 * res + rem) {
                        YawY_U[i] = YawY_U[i - 1] + fact_12;
                    }

                    //                    double fact_11 = ((Yaw_U_vali[max_or_impactindex - 1] + Yaw_U_vali[max_or_impactindex] +
                    //                                       Yaw_U_vali[max_or_impactindex - 2]) - (Yaw_U_vali[SI - 1]+ Yaw_U_vali[ SI ]+Yaw_U_vali[SI + 1]))
                    //                    /(3*(max_or_impactindex - SI));
                    //
                    //                    YawY_U[i] = YawY_U[i-1] + fact_11;
                    //
                } else if (shot_type2 == 3) {
                    //                    double fact_11 = ((Yaw_U_vali[max_or_impactindex-1] + Yaw_U_vali[max_or_impactindex] +
                    //                                       Yaw_U_vali[max_or_impactindex-2]) - (Yaw_U_vali[SI-1] + Yaw_U_vali[SI] + Yaw_U_vali[SI+1]))
                    //                    /(3*(max_or_impactindex-SI));

                    if (i + BSI < SI + 10) // this is hardcoded values
                    {
                        //                      YawY_U[i] = YawY_U[i-1] + fact_11 / 2;// chnaged factor only for cover drives

                        if (i + BSI > SI && i + BSI <= SI +
                                                       res) // breaking downswing to max or impact into three buckets
                        {
                            YawY_U[i] = YawY_U[i - 1] + fact_11 / 2;
                        } else if (i + BSI > SI + res && i + BSI <= SI + 2 * res + rem) {
                            YawY_U[i] = YawY_U[i - 1] + fact_12 / 2;
                        }
                        // printf(" O %f \n",YawY_U[i]);
                    } else {
                        // YawY_U[i] = YawY_U[i-1] + fact_11;
                        if (i + BSI > SI && i + BSI <= SI +
                                                       res) // breaking downswing to max or impact into three buckets
                        {
                            YawY_U[i] = YawY_U[i - 1] + fact_11;
                        } else if (i + BSI > SI + res && i + BSI <= SI + 2 * res + rem) {
                            YawY_U[i] = YawY_U[i - 1] + fact_12;
                        }
                        // printf(" N %f \n",YawY_U[i]);
                    }
                } else {
                    YawY_U[i] = Yaw_T[i + BSI] - orientation_Ana;
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    } else if (YawY_U[i] > 180) {
                        YawY_U[i] = 360 - YawY_U[i];
                    }
                }

                RollZ_U2[i] = Roll[i + BSI];
                if (Pitch[i + BSI] < -145) {
                    PitchX_U[i] = -(Pitch[i + BSI] + 180) - 90;
                    if (PitchX_U[i] > 179) {
                        PitchX_U[i] = -360 + PitchX_U[i];
                    }
                } else {
                    PitchX_U[i] = 90 - Pitch[i + BSI];
                    if (PitchX_U[i] > 179) {
                        PitchX_U[i] = -360 + PitchX_U[i];
                    }
                }
            } else if (i + BSI > max_or_impactindex && i + BSI < FTI) {
                if (shot_type2 == 3) {
                    if (IisG ==
                        1) // this condition is for if impact is greater than FTI then we are not changing the yaw
                    {
                        YawY_U[i] = YawY_U[i - 1];
                    } else {
                        YawY_U[i] = YawY_U[i - 1] + fact2;
                    }
                    RollZ_U2[i] = Roll[i + BSI];
                } else if (shot_type2 == 11 || shot_type2 == 8 || shot_type2 == 1 ||
                           shot_type2 == 7) {
                    if (IisG ==
                        1) // this condition is for if impact is greater than FTI then we are not changing the yaw
                    {
                        YawY_U[i] = YawY_U[i - 1];
                    } else {
                        YawY_U[i] = YawY_U[i - 1] + fact2;
                    }
                    RollZ_U2[i] = Roll[i + BSI];
                } else {
                    YawY_U[i] = YawY_U[i - 1] + fact2;
                    RollZ_U2[i] = Roll[i + BSI];
                }

                if (Pitch[i + BSI] < -145) {
                    PitchX_U[i] = -(Pitch[i + BSI] + 180) - 90;
                    if (PitchX_U[i] > 179) {
                        PitchX_U[i] = -360 + PitchX_U[i];
                    }
                } else {
                    PitchX_U[i] = 90 - Pitch[i + BSI];
                    if (PitchX_U[i] > 179) {
                        PitchX_U[i] = -360 + PitchX_U[i];
                    }
                }
            }
        }
        calculate_five_AverageAndInsert(YawY_U_Avg, YawY_U);
        calculte_Roll_Array_Unity(RollZ_U, RollZ_U2, shot_type, max_or_impactindex);
        calculateAverageAndInsertArray();
    }
        // shot type other than 1 is for Horizontal  bat shots
    else {
        if (shot_type2 == 33) {
            int x = 0, index2 = SI;
            double previousPitch = 0, previousPitch1 = 0, previousPitch2 = 0, previousPitch3 = 0, PitchX_U2[130];
            if (Pitch[BSI] < 0) {
                PitchX_U2[0] = 360 + Pitch[BSI];
                previousPitch = PitchX_U2[0];
            } else {
                PitchX_U2[0] = Pitch[BSI];
                previousPitch = PitchX_U2[0];
            }
            PitchX_U[0] = 90 - PitchX_U2[0];
            for (x = SI; x > BSI; x--) {
                if (Yg[x] < 0) {
                    index2 = x;
                    break;
                }
            }
            for (i = 1; i < 130 - BSI; i++) {
                if (index2 > BSI && i + BSI <= index2) {
                    PitchX_U2[i] = previousPitch - Xrotation_unity[i + BSI];
                    previousPitch1 = PitchX_U2[i];
                    PitchX_U[i] = 90 - PitchX_U2[i];
                } else if (i + BSI > index2 && i + BSI <= SI) {
                    PitchX_U2[i] = previousPitch1 - Yrotation_unity[i + BSI];
                    previousPitch2 = PitchX_U2[i];
                    PitchX_U[i] = 90 - PitchX_U2[i];
                } else if (i + BSI > SI && i + BSI <= WBI_UN && WBI_UN != 0) {
                    PitchX_U2[i] = previousPitch2 - Xrotation_unity[i + BSI];
                    previousPitch3 = PitchX_U2[i];
                    PitchX_U[i] = 90 - PitchX_U2[i];
                    if (PitchX_U[i] > 180) {
                        PitchX_U[i] = PitchX_U[i] - 360;
                    }
                } else if (i + BSI > WBI_UN && i + BSI < 129) {
                    PitchX_U2[i] = previousPitch3 + Yrotation_unity[i + BSI];
                    PitchX_U[i] = 90 - PitchX_U2[i];
                    if (PitchX_U[i] > 180) {
                        PitchX_U[i] = PitchX_U[i] - 360;
                    }
                } else if (i + BSI >= 129 && i + BSI < 130) {
                    PitchX_U2[i] = PitchX_U2[i - 1];
                }
            }
            YawY_U[0] = Yaw_T[BSI] - orientation_Ana;
            YawY_U[breakpoint - BSI] = Yaw_T[breakpoint] - orientation_Ana;
            if (YawY_U[0] < -180) {
                YawY_U[0] = 360 + YawY_U[0];
            } else if (YawY_U[0] > 180) {
                YawY_U[0] = 360 - YawY_U[0];
            }
            if (YawY_U[breakpoint - BSI] < -180) {
                YawY_U[breakpoint - BSI] = 360 + YawY_U[breakpoint - BSI];
            } else if (YawY_U[breakpoint - BSI] > 180) {
                YawY_U[breakpoint - BSI] = 360 - YawY_U[breakpoint - BSI];
            }
            RollZ_U[0] = Roll[BSI];
            fac = (YawY_U[breakpoint - BSI] - YawY_U[0]) / (breakpoint - BSI);
            for (i = 1; i < 130; i++) {
                if (Yrotation_unity[i + BSI] == 0 &&
                    i + BSI >= breakpoint) { // last negative index during backlift.
                    index = i;
                    break; // breaking the loop here,and after that in unity data manuplation we will use
                    // Xrotaion_unity and Yrotaion_unity.
                } else if (breakpoint == SI && i + BSI == SI - 10) {
                    index = i;
                    break;
                } else {
                    YawY_U[i] = YawY_U[i - 1] + fac;
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    }
                    RollZ_U[i] = Roll[i + BSI];
                    index = i;
                }
            }
            for (i = index; i < SI - BSI; i++) {
                YawY_U[i] = YawY_U[index - 1] -
                            (Xrotation_unity[i + BSI] - Xrotation_unity[i + BSI - 1]);
                if (YawY_U[i] < -180) {
                    YawY_U[i] = YawY_U[i] + 360;
                }
                RollZ_U[i] = Roll[i + BSI];
                index = i;
            }
            for (i = index + 1; i <= FTI - BSI; i++) {
                if (i < WBI_UN - BSI) {
                    YawY_U[i] = YawY_U[index - 1] - Xrotation_unity[i + BSI] / 2;
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    } else if (YawY_U[i] > 180) {
                        YawY_U[i] = 360 - YawY_U[i];
                    }
                } else {
                    YawY_U[i] = YawY_U[i - 1];
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    } else if (YawY_U[i] > 180) {
                        YawY_U[i] = 360 - YawY_U[i];
                    }
                }
                if (i < max_or_impactindex) {
                    RollZ_U[i] = Roll[i + BSI];
                } else if (i == max_or_impactindex) {
                    if (Roll[i + BSI] < 0) {
                        RollZ_U[i] = Roll[i];
                    } else {
                        double roll_fac = (Roll[i + BSI - 5] + Roll[i + BSI]) / 5;
                        int k = 0;
                        RollZ_U[i - 4] = Roll[i + BSI - 5] - roll_fac;
                        for (k = 3; k >= 0; k--) {
                            RollZ_U[i - k] = RollZ_U[i - k - 1] - roll_fac;
                        }
                    }
                } else {
                    if (Roll[i + BSI] > 0) {
                        RollZ_U[i] = -Roll[i + BSI];
                    } else {
                        RollZ_U[i] = Roll[i + BSI];
                    }
                }
            }
            for (i = FTI - BSI; i < 130 - BSI; i++) {
                YawY_U[i] = YawY_U[i - 1];
                if (Roll[i + BSI] > 0) {
                    RollZ_U[i] = -Roll[i + BSI];
                } else {
                    RollZ_U[i] = Roll[i + BSI];
                }
            }
            for (i = 0; i < 130 - BSI; i++) {
                YawY_U_Avg[i] = YawY_U[i];
            }
            calculate_five_AverageAndInsert(YawY_U_Avg, YawY_U);
        } else if (shot_type2 == 10 || shot_type2 == 9) {
            //  code is for smoothing the Yaw Values
            YawY_U[0] = -backliftdirection;

            if (YawY_U[0] < -180) {
                YawY_U[0] = 360 + YawY_U[0];
            } else if (YawY_U[0] > 180) {
                YawY_U[0] = 360 - YawY_U[i];
            }

            if (Pitch[BSI] < 0) {
                // if pitch is negative means like -176  first add 180 and then negate 90.
                PitchX_U[0] = -(180 + Pitch[BSI]) - 90;
            } else {
                PitchX_U[0] = 90 - Pitch[BSI];
            }
            RollZ_U[0] = Roll[BSI];
            // from back lift start to back lift end we are calculating pitch roll yaw same as horizontal bat shots.
            for (i = 1; i < diff3; i++) {
                if (Yrotation_unity[i + BSI] == 0 && i + BSI >= breakpoint) {
                    // if Y rotation is positive means bat start coming downwards ,here we are
                    // checking one more condition that in Y index1 means Y[0] = 0,Y[1] is the
                    // last negative index during back lift.
                    index = i;
                    break;
                    // breaking the loop here,and after that in unity data manipulation we will use
                    // X rotation_unity and Y rotation_unity.
                } else if (breakpoint == SI && i + BSI == SI - 10) {
                    index = i;
                    break;
                } else {
                    if (Pitch[i + BSI] < 0) {
                        // if pitch is negative means like -176  first add 180 and then negate 90.
                        PitchX_U[i] = -(180 + Pitch[i + BSI]) - 90;
                    } else {
                        PitchX_U[i] = 90 - Pitch[i + BSI];
                    }
                    YawY_U[i] = YawY_U[0] - Xrotation_unity[i + BSI];
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    } else if (YawY_U[i] > 180) {
                        YawY_U[i] = 360 - YawY_U[i];
                    }
                    RollZ_U[i] = Roll[i + BSI];
                    index = i;
                }
            }
            previousVal = PitchX_U[index - 1];
            for (i = index; i < SI - BSI; i++) {
                if (Yrotation_unity[i + BSI] == 0) {
                    // we are checking  sign change in Y rotation data here either positive to negative
                    //  or negative to positive,then storing the previous value of pitch .
                    previousVal = PitchX_U[i - 1];
                }
                PitchX_U[i] = previousVal + Yrotation_unity[i + BSI];
                YawY_U[i] = YawY_U[0] - Xrotation_unity[i + BSI];
                if (YawY_U[i] < -180) {
                    YawY_U[i] = YawY_U[i] + 360;
                } else if (YawY_U[i] > 180) {
                    YawY_U[i] = 360 - YawY_U[i];
                }
                RollZ_U[i] = Roll[i + BSI];
                index = i;
            }
            // previousVal = PitchX_U[index -1];// this part is removed
            for (i = index + 1; i <= FTI - BSI; i++) {
                if (Yrotation_unity[i + BSI] == 0) {
                    // we are checking  sign change in Y rotation data here either positive to negative
                    //  or negative to positive,then storing the previous value of pitch .
                    previousVal = PitchX_U[i - 1];
                }
                PitchX_U[i] = previousVal + Yrotation_unity[i + BSI];
                if (i < FTI) {
                    YawY_U[i] = YawY_U[index - 1] - 1.5 * Xrotation_unity[i + BSI];
                } else {
                    YawY_U[i] = YawY_U[i - 1];
                }
                if (YawY_U[i] < -180) {
                    YawY_U[i] = YawY_U[i] + 360;
                } else if (YawY_U[i] > 180) {
                    YawY_U[i] = 360 - YawY_U[i];
                }
                RollZ_U[i] = Roll[i + BSI];
            }
            for (i = 0; i < diff2; i++) {
                YawY_U_Avg[i] = YawY_U[i];
            }
        } else {
            //  code is for smoothing the Yaw Values
            YawY_U[0] = Yaw_T[BSI] - orientation_Ana;
            YawY_U[breakpoint - BSI] = Yaw_T[breakpoint] - orientation_Ana;
            if (YawY_U[0] < -180) {
                YawY_U[0] = 360 + YawY_U[0];
            } else if (YawY_U[0] > 180) {
                YawY_U[0] = 360 - YawY_U[0];
            }
            if (YawY_U[breakpoint - BSI] < -180) {
                YawY_U[breakpoint - BSI] = 360 + YawY_U[breakpoint - BSI];
            } else if (YawY_U[breakpoint - BSI] > 180) {
                YawY_U[breakpoint - BSI] = 360 - YawY_U[breakpoint - BSI];
            }
            if (Pitch[BSI] < 0) {
                // if pitch is negative means like -176  first add 180 and then negate 90.
                PitchX_U[0] = -(180 + Pitch[BSI]) - 90;
            } else {
                PitchX_U[0] = 90 - Pitch[BSI];
            }
            RollZ_U[0] = Roll[BSI];
            fac = (YawY_U[breakpoint - BSI] - YawY_U[0]) / (breakpoint - BSI);
            // from back lift start to back lift end we are calculating pitch roll yaw same as horizontal bat shots.

            for (i = 1; i < diff3; i++) {
                if (Yrotation_unity[i + BSI] == 0 && i + BSI >= breakpoint) {
                    // if Y rotation is positive means bat start coming downwards ,here we are
                    // checking one more condition that in Y index1 means Y[0] = 0,Y[1] is the
                    // last negative index during back lift.
                    index = i;
                    break;
                    // breaking the loop here,and after that in unity data manipulation we will use
                    // X rotation_unity and Y rotation_unity.
                } else if (breakpoint == SI && i + BSI == SI - 10) {
                    index = i;
                    break;
                } else {
                    if (Pitch[i + BSI] < 0) {
                        // if pitch is negative means like -176  first add 180 and then negate 90.
                        PitchX_U[i] = -(180 + Pitch[i + BSI]) - 90;
                    } else {
                        PitchX_U[i] = 90 - Pitch[i + BSI];
                    }
                    YawY_U[i] = YawY_U[i - 1] + fac;
                    if (YawY_U[i] < -180) {
                        YawY_U[i] = YawY_U[i] + 360;
                    }
                    RollZ_U[i] = Roll[i + BSI];
                    index = i;
                }
            }
            previousVal = PitchX_U[index - 1];
            for (i = index; i < SI - BSI; i++) {
                if (Yrotation_unity[i + BSI] == 0) {
                    // we are checking  sign change in Y rotation data here either positive to negative
                    //  or negative to positive,then storing the previous value of pitch .
                    previousVal = PitchX_U[i - 1];
                }
                PitchX_U[i] = previousVal + Yrotation_unity[i + BSI];
                YawY_U[i] =
                        YawY_U[i - 1] - (Xrotation_unity[i + BSI] - Xrotation_unity[i + BSI - 1]);
                if (YawY_U[i] < -180) {
                    YawY_U[i] = YawY_U[i] + 360;
                } else if (YawY_U[i] > 180) {
                    YawY_U[i] = 360 - YawY_U[i];
                }
                RollZ_U[i] = Roll[i + BSI];
                index = i;
            }
            // previousVal = PitchX_U[index -1];// this part is removed
            for (i = index + 1; i <= FTI - BSI; i++) {
                if (Yrotation_unity[i + BSI] == 0) {
                    // we are checking  sign change in Y rotation data here either positive to negative
                    //  or negative to positive,then storing the previous value of pitch .
                    previousVal = PitchX_U[i - 1];
                }
                PitchX_U[i] = previousVal + Yrotation_unity[i + BSI];
                if (i < FTI) {
                    YawY_U[i] = YawY_U[index - 1] - Xrotation_unity[i + BSI];
                } else {
                    YawY_U[i] = YawY_U[i - 1];
                }
                if (YawY_U[i] < -180) {
                    YawY_U[i] = YawY_U[i] + 360;
                } else if (YawY_U[i] > 180) {
                    YawY_U[i] = 360 - YawY_U[i];
                }
                RollZ_U[i] = Roll[i + BSI];
            }
            for (i = 0; i < diff2; i++) {
                YawY_U_Avg[i] = YawY_U[i];
            }
        }
        calculateAverageAndInsertArray();
    }
}

int calculateDataForUnityCorrectOrNotForVerticalShots(int shot_type, double Yaw_Unity[],
                                                      double Pitch_Unity[],
                                                      double backliftdirection,
                                                      double downswingdirection, int wristbreak,
                                                      int max_or_impact_index) {
    int index = 0;
    // printf("%f \n",backliftdirection);
    if (shot_type == 1) {
        // printf("%f  %d \n",backliftdirection,wristbreak);
        if (wristbreak == 0) {
            if (backliftdirection > -80 && backliftdirection < 80) {

                for (i = 0; i < 2 * (FTI - BSI) - 4; i++) // checking the courrpt data till FTI
                {
                    if (fabs(Yaw_Unity[i] - Yaw_Unity[i + 1]) > 90) {
                        index = 0;
                        break;
                    } else if (fabs(fabs(Pitch_Unity[i]) - fabs(Pitch_Unity[i + 1])) > 30) {
                        index = 0;
                        break;
                    } else {
                        index = 1;
                    }
                }
                if (index == 1) // this condition is added to remove unity dependency from APP
                {
                    index = FTI - 2;
                }
                //                printf("index %d \n",index);
            } else {
                if (downswingdirection < 40 && downswingdirection > -40) {
                    for (i = 0;
                         i < 2 * (FTI - BSI - SI) - 4; i++) // In case of more backlift direction
                    {
                        if (fabs(Yaw_Unity[i] - Yaw_Unity[i + 1]) > 90) {
                            index = 0;
                            break;
                        } else if (fabs(fabs(Pitch_Unity[i]) - fabs(Pitch_Unity[i + 1])) > 30) {
                            index = 0;
                            break;
                        } else {
                            index = 1;
                        }
                    }
                    if (index == 1) // this condition is added to remove unity dependency from APP
                    {
                        index = FTI - SI - 2;
                    }
                }
            }
        } else if (wristbreak != 0) // in case of wrist break
        {
            if (backliftdirection > -80 && backliftdirection < 80) {
                for (i = 0; i < 2 * (FTI - BSI) - 4; i++) {
                    if (fabs(Yaw_Unity[i] - Yaw_Unity[i + 1]) > 90) {
                        index = 0;
                        break;
                    } else if (fabs(fabs(Pitch_Unity[i]) - fabs(Pitch_Unity[i + 1])) > 20) {
                        index = 0;
                        break;
                    } else {
                        if (wristbreak > max_or_impact_index) {
                            index = 1;
                        } else if (max_or_impact_index >= wristbreak &&
                                   max_or_impact_index - wristbreak <= 8) {
                            index = 1;
                        }
                    }
                }
            }

            if (index == 1) {
                if (wristbreak > max_or_impact_index) {
                    index = wristbreak;
                    //                    printf("Yes %d \n",index);
                } else if (wristbreak == max_or_impact_index) {
                    index = max_or_impact_index + 4;
                } else {
                    index = max_or_impact_index + 4;
                }
            }
        }
    }
    //    printf("%d \n",index);
    return index;
}

void calculate_five_AverageAndInsert(double insert_arr[], double data_arr[]) {
    int k = 0;
    for (i = 0; i < FTI - BSI; i++) {
        // printf("%f \n",data_arr[i]);
        if (i <= 2) {
            insert_arr[i] = data_arr[i];
        } else if (i > 2 && i < FTI - BSI - 2) {
            double sum = 0;
            insert_arr[i] = (data_arr[i - 2] + data_arr[i - 1] + data_arr[i] + data_arr[i + 1] +
                             data_arr[i + 2]) / 5;
        } else if (i >= FTI - BSI - 2 && i <= FTI - BSI) {
            insert_arr[i] = data_arr[i];
        } else if (i >= FTI - BSI && i <= 130 - BSI) {
            insert_arr[i] = data_arr[i];
        }
    }
}

void calculateAverageAndInsert(double inset_arr[], double data_arr[]) {
    int j = 0;
    int limit = 2 * DATAROWS;
    for (i = 0; i < limit; i++) {
        if (i % 2 == 0) {
            inset_arr[i] = data_arr[j];
            j++;
        } else {
            if (data_arr[j - 1] < 0 && data_arr[j] > 0) {
                inset_arr[i] = fabs(fabs(data_arr[j]) - fabs(data_arr[j - 1])) / 2;
            } else if (data_arr[j - 1] > 0 && data_arr[j] < 0) {
                inset_arr[i] = (data_arr[j - 1] - data_arr[j]) / 2;
            } else {
                inset_arr[i] = (data_arr[j - 1] + data_arr[j]) / 2;
            }
        }
        // printf(" %d %f %f \n",i,data_arr[i],inset_arr[i]);
    }
}

void calculateAverageAndInsertArray() {
    calculateAverageAndInsert(Time_U_Filter, Tg);
    calculateAverageAndInsert(PitchX_U_Filter, PitchX_U);
    calculateAverageAndInsert(RollZ_U_Filter, RollZ_U);
    calculateAverageAndInsert(YawY_U_Filter, YawY_U_Avg);
}

void
calculatePitchFilterNextValueHigh(double insert_arr[], double data_arr[], int max_or_impact_index) {
    int limit = 2 * DATAROWS;
    for (i = 0; i < limit; i++) {
        if (i > 2 * ((SI + 3) - BSI) - 1 && i < 2 * (FTI -
                                                     BSI)) // first  we were checking from max_or_impact now we are from Downswing start + 3 , 3 is added if backlift and downswing start index is same
        {
            if (data_arr[i] < insert_arr[i - 1] && data_arr[i] > 0) {
                insert_arr[i] = insert_arr[i - 1];
            } else {
                insert_arr[i] = data_arr[i];
            }
        } else {
            insert_arr[i] = data_arr[i];
        }
    }
}

void calculateDiffAndSmooth(double Out_Array[260], double In_Array[260]) {
    int diff = 2 * (FTI - BSI) - 1;
    Out_Array[0] = In_Array[0];
    Out_Array[1] = In_Array[1];
    i = 2;
    while (i > 1 && i < diff - 2) {
        if (fabs(In_Array[i] - In_Array[i - 1]) > 30 && fabs(In_Array[i] - In_Array[i - 1]) < 100) {
            double fact = (In_Array[i + 2] - In_Array[i - 2]) / 5;
            int k = 0;
            for (k = 0; k < 5; k++) {
                Out_Array[i - 2] = In_Array[i - 2] + k * fact;
                //                printf("%f %f \n",In_Array[i],Out_Array[i]);
                i++;
            }
        } else {
            Out_Array[i] = In_Array[i];
            //            printf("%f %f \n",In_Array[i],Out_Array[i]);
            i++;
        }
    }
    while (i >= diff - 2 && i < diff) {
        Out_Array[i] = In_Array[i];
        //       printf(" NO %f %f \n",In_Array[i],Out_Array[i]);
        i++;
    }
    while (i >= diff && i < 260 - BSI) {
        Out_Array[i] = In_Array[i];
        //        printf(" YES %f %f \n",In_Array[i],Out_Array[i]);

        i++;
    }
}

void calculte_Roll_Array_Unity(double out_arr[], double inp_arr[], int shot_type,
                               int max_or_impactindex) {
    if (shot_type == 1) // if vertical
    {
        for (i = 0; i < 130; i++) {
            if (i == max_or_impactindex - BSI) {
                if (Roll[i + BSI] < 0) {
                    out_arr[i] = inp_arr[i];
                } else {
                    double roll_fac = (Roll[i + BSI - 5] - Roll[i + BSI]) / 5;
                    int k = 0;
                    out_arr[i - 4] = Roll[i + BSI - 5] - roll_fac;
                    for (k = 3; k >= 0; k--) {
                        out_arr[i - k] = out_arr[i - k - 1] - roll_fac;
                    }
                }
            } else {
                if (Roll[i + BSI] > 0) {
                    out_arr[i] = inp_arr[i];
                } else {
                    out_arr[i] = inp_arr[i];
                }
            }
        }
    }
}

double calculateTimeToImpact(double Time[], int downswing_start_index, int impact_index) {
    // printf("%d %d \n",downswing_start_index,impact_index);
    double time = (Time[impact_index] - Time[downswing_start_index]);
    return time;
}

double calculateBackLiftDirection_Ana(double orientation_Ana,
                                      int backlift_start_index_or_max_or_impact_index,
                                      int userBattingHand) {
    double orientation2 = Yaw_T[backlift_start_index_or_max_or_impact_index];
    double pitch = Pitch[backlift_start_index_or_max_or_impact_index], backliftdirection = 0;
    // double roll  = Roll[backlift_start_index_or_max_or_impact_index];
    backliftdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_Ana,
                                                               orientation2);
    if (-90 < backliftdirection && backliftdirection < 90) {
        orientation2 = orientation2;
    } else {
        if (135 < pitch && pitch < 180) {
            if (orientation2 > 180) {
                orientation2 = orientation2 - 180;
            } else {
                orientation2 = orientation2 + 180;
            }
        }
    }
    if (Roll[backlift_start_index_or_max_or_impact_index] > -20 &&
        Roll[backlift_start_index_or_max_or_impact_index] < 20) {
        orientation2 = orientation2 - Roll[backlift_start_index_or_max_or_impact_index] / 2;
    } else {
        orientation2 = orientation2 - Roll[backlift_start_index_or_max_or_impact_index];
    }

    backliftdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_Ana,
                                                               orientation2);
    Pitch_change = backliftdirection;
    /*
     if(userBattingHand == 1)
     {
     if ( backliftdirection > 150 && orientation_Ana > 180)
     {//------------limit for pitch_change increased form 110 to 150
     backliftdirection = backliftdirection - 180;
     }
     else if(backliftdirection > 150 && orientation_Ana <= 180)
     {
     backliftdirection = 180 - backliftdirection ;
     }
     }
     else
     {
     if (backliftdirection > 150 && orientation_Ana > 180)
     {
     backliftdirection = -( backliftdirection - 180);
     }
     else if(backliftdirection > 150 && orientation_Ana <= 180)
     {
     backliftdirection = -(180 - backliftdirection ) ;
     }
     }
     */
    // printf("%f \n",backliftdirection);
    return backliftdirection;
}

double calculateBackLiftDirection_App(double orientation_App,
                                      int backlift_start_index_or_max_or_impact_index,
                                      int userBattingHand) {
    double orientation2 = Yaw[backlift_start_index_or_max_or_impact_index];
    double pitch = Pitch[backlift_start_index_or_max_or_impact_index], backliftdirection = 0;
    // double roll  = Roll[backlift_start_index_or_max_or_impact_index];
    backliftdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_App,
                                                               orientation2);
    if (-90 < backliftdirection && backliftdirection < 90) {
        orientation2 = orientation2;
    } else {
        if (135 < pitch && pitch < 180) {
            if (orientation2 > 180) {
                orientation2 = orientation2 - 180;
            } else {
                orientation2 = orientation2 + 180;
            }
        }
    }
    if (Roll[backlift_start_index_or_max_or_impact_index] > -20 &&
        Roll[backlift_start_index_or_max_or_impact_index] < 20) {
        orientation2 = orientation2 - Roll[backlift_start_index_or_max_or_impact_index] / 2;
    } else {
        orientation2 = orientation2 - Roll[backlift_start_index_or_max_or_impact_index];
    }
    backliftdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_App,
                                                               orientation2);
    Pitch_change = backliftdirection;
    /*
     if(userBattingHand == 1)
     {
     if ( backliftdirection > 150 && orientation_App > 180)
     {//------------limit for pitch_change increased form 110 to 150
     backliftdirection = backliftdirection - 180;
     }
     else if(backliftdirection > 150 && orientation_App<= 180)
     {
     backliftdirection = 180 - backliftdirection ;
     }
     }
     else
     {
     if (backliftdirection > 150 && orientation_App > 180)
     {
     backliftdirection = -( backliftdirection - 180);
     }
     else if(backliftdirection > 150 && orientation_App <= 180)
     {
     backliftdirection = -(180 - backliftdirection ) ;
     }
     }
     */
    return backliftdirection;
}

double calculateDownswingDirection_App_Test_NewFirmware(double orientation_App,
                                                        int downswing_start_index_or_max_or_impact_index,
                                                        int userBattingHand) {
    double orientation2 = Yaw[downswing_start_index_or_max_or_impact_index], downswingdirection = 0, roll = Roll[downswing_start_index_or_max_or_impact_index],
            pitch = Pitch[downswing_start_index_or_max_or_impact_index];
    downswingdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_App,
                                                                orientation2);
    //	printf("%f %f %f \n",downswingdirection,orientation2,orientation_App);
    if (roll >= 0 && roll < 10) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 5;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 7;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 10;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 10 && roll < 20) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 5;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 7;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 10;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 20 && roll < 30) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 10;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 17;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 20;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 30 && roll < 40) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 10;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 15;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 30;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 40 && roll < 50) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 10;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 15;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 50 && roll < 60) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 15;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 17;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 60 && roll < 70) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 17;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 19;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 27;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 70 && roll < 80) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 20;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 30;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= 80 && roll < 90) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 30;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 40;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 50;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -10 && roll < 0) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 7;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 10;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -20 && roll < -10) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 7;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 10;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -30 && roll < -20) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 7;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 10;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -40 && roll < -30) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 5;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 10;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 17;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 20;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -50 && roll < -40) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 10;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 15;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 30;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -60 && roll < -50) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 10;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 15;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -70 && roll < -60) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection + 15;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection + 17;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection + 25;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection + 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -80 && roll < -70) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 17;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 19;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 27;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    } else if (roll >= -90 && roll < -80) {
        if (pitch >= 0 && pitch <= 130) {
            downswingdirection = downswingdirection - 20;
        } else if (pitch > 130 && pitch <= 140) {
            downswingdirection = downswingdirection - 25;
        } else if (pitch > 140 && pitch <= 150) {
            downswingdirection = downswingdirection - 30;
        } else if (pitch > 150 && pitch <= 160) {
            downswingdirection = downswingdirection - 40;
        } else {
            downswingdirection = downswingdirection;
        }
    }
    return downswingdirection;
}

double calculateBackLiftDirection_App_Test_NewFirmware(double orientation_App,
                                                       int backlift_start_index_or_max_or_impact_index,
                                                       int userBattingHand) {
    double orientation2 = Yaw[backlift_start_index_or_max_or_impact_index], backliftdirection = 0, roll = Roll[backlift_start_index_or_max_or_impact_index],
            pitch = Pitch[backlift_start_index_or_max_or_impact_index];
    //    printf("yes Yaw is  : %f  %f\n",orientation2,roll);
    //    double roll  = Roll[backlift_start_index_or_max_or_impact_index];
    //	printf(" ! %f \n",pitch);

    backliftdirection = calculateDirectionForBackliftDirection(userBattingHand, orientation_App,
                                                               orientation2);
    // printf("%f\n",backliftdirection);
    if (roll >= 0 && roll < 10) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 5;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 7;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 10;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 13;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 20;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 10 && roll < 20) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 5;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 10;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 15;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 20;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 25;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 30;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 20 && roll < 30) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 10;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 15;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 20;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 35;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 40;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 45;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 30 && roll < 40) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 15;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 25;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 35;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 40;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 45;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 50;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 40 && roll < 50) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 20;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 35;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 40;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 45;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 50;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 55;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 50 && roll < 60) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 30;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 40;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 45;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 50;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 55;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 60;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 60 && roll < 70) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 35;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 45;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 50;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 55;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 60;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 65;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 70 && roll < 80) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 50;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 55;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 60;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 70;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 75;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 80;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= 80 && roll <= 90) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection - 70;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection - 75;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection - 80;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection - 85;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection - 90;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection - 95;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -10 && roll < 0) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 5;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 7;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 10;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 12;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 15;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -20 && roll < -10) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 10;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 15;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 20;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 25;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 30;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -30 && roll < -20) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 5;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 10;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 20;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 35;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 40;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 45;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -40 && roll < -30) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 10;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 20;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 30;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 40;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 45;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 50;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -50 && roll < -40) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 15;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 25;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 40;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 45;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 50;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 55;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -60 && roll < -50) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 20;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 30;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 45;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 50;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 55;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 60;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -70 && roll < -60) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 25;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 35;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 50;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 55;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 60;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 65;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -80 && roll < -70) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 40;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 45;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 45;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 50;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 55;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 65;
        } else {
            backliftdirection = backliftdirection;
        }
    } else if (roll >= -90 && roll < -80) {
        if (pitch >= 0 && pitch <= 120) {
            backliftdirection = backliftdirection + 50;
        } else if (pitch > 120 && pitch <= 140) {
            backliftdirection = backliftdirection + 55;
        } else if (pitch > 140 && pitch <= 150) {
            backliftdirection = backliftdirection + 60;
        } else if (pitch > 150 && pitch <= 160) {
            backliftdirection = backliftdirection + 65;
        } else if (pitch > 160 && pitch <= 180) {
            backliftdirection = backliftdirection + 70;
        } else if (pitch > -180 && pitch <= -155) {
            backliftdirection = backliftdirection + 75;
        } else {
            backliftdirection = backliftdirection;
        }
    }
    // printf(" backlift direction is  %f \n",backliftdirection);
    return backliftdirection;
}

double calculateBackLiftDirectionAverage_Ana(double orientation_Ana,
                                             int backlift_start_index_or_max_or_impact_index,
                                             int userBattingHand) {
    double orientation3 = 0;
    if (backlift_start_index_or_max_or_impact_index == 0) {
        for (i = 0; i < 3; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_Ana(orientation_Ana,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i, userBattingHand);
        }
        orientation3 = orientation3 / 3;
    } else if (backlift_start_index_or_max_or_impact_index == 1) {
        for (i = 0; i < 4; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_Ana(orientation_Ana,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i - 1, userBattingHand);
        }
        orientation3 = orientation3 / 4;
    } else {
        for (i = 0; i < 5; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_Ana(orientation_Ana,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i - 2, userBattingHand);
        }
        orientation3 = orientation3 / 5;
    }
    return orientation3;
}

double calculateBackLiftDirectionAverage_App(double orientation_App,
                                             int backlift_start_index_or_max_or_impact_index,
                                             int userBattingHand) {
    double orientation3 = 0;
    if (backlift_start_index_or_max_or_impact_index == 0) {
        for (i = 0; i < 3; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_App(orientation_App,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i, userBattingHand);
        }
        orientation3 = orientation3 / 3;
    } else if (backlift_start_index_or_max_or_impact_index == 1) {
        for (i = 0; i < 4; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_App(orientation_App,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i - 1, userBattingHand);
        }
        orientation3 = orientation3 / 4;
    } else {
        for (i = 0; i < 5; i++) {
            orientation3 = orientation3 + calculateBackLiftDirection_App(orientation_App,
                                                                         backlift_start_index_or_max_or_impact_index +
                                                                         i - 2, userBattingHand);
        }
        orientation3 = orientation3 / 5;
    }
    return orientation3;
}

double calculateBackLiftDirection_App_Test_NewFirmware_Avg(double orientation_App,
                                                           int backlift_start_index_or_max_or_impact_index,
                                                           int userBattingHand) {
    double orientation3 = 0;
    if (backlift_start_index_or_max_or_impact_index == 0) {
        for (i = 0; i < 3; i++) {
            orientation3 =
                    orientation3 + calculateBackLiftDirection_App_Test_NewFirmware(orientation_App,
                                                                                   backlift_start_index_or_max_or_impact_index +
                                                                                   i,
                                                                                   userBattingHand);
            // printf("%f \n",orientation3);
        }
        orientation3 = orientation3 / 3;
    } else if (backlift_start_index_or_max_or_impact_index == 1) {
        for (i = 0; i < 4; i++) {
            orientation3 =
                    orientation3 + calculateBackLiftDirection_App_Test_NewFirmware(orientation_App,
                                                                                   backlift_start_index_or_max_or_impact_index +
                                                                                   i - 1,
                                                                                   userBattingHand);
        }
        orientation3 = orientation3 / 4;
    } else {
        for (i = 0; i < 5; i++) {
            orientation3 =
                    orientation3 + calculateBackLiftDirection_App_Test_NewFirmware(orientation_App,
                                                                                   backlift_start_index_or_max_or_impact_index +
                                                                                   i - 2,
                                                                                   userBattingHand);
            // printf(" yes %f \n",orientation3);
        }
        orientation3 = orientation3 / 5;
    }
    return orientation3;
}

double calculateBatFace(double angle[], int backlift_start_index, int batting_hand, double backlift,
                        double backliftDirection) {
    double batface = 0, flag = 0;
    //    printf("%d \n",backlift_start_index);
    if (Pitch[backlift_start_index] > 0) {
        if ((Pitch[backlift_start_index]) > 170) {
            // if high Pitch then Roll condition
            if (fabs(Roll[backlift_start_index]) > 30) {
                batface = backliftDirection;
                flag = 1;
            }
        }
    } else {
        if (Pitch[backlift_start_index] >= -180 && Pitch[backlift_start_index] <= -170) {
            // if high Pitch then Roll condition
            if (fabs(Roll[backlift_start_index]) > 30) {
                batface = backliftDirection;
                flag = 1;
            }
        } else if (Pitch[backlift_start_index] > -170 && Pitch[backlift_start_index] <= -160) {
            // if high Pitch then Roll condition
            if (fabs(Roll[backlift_start_index]) > 20) {
                batface = backliftDirection;
                flag = 1;
            }
        } else if (Pitch[backlift_start_index] > -160 && Pitch[backlift_start_index] <= -150) {
            // if high Pitch then Roll condition
            if (fabs(Roll[backlift_start_index]) > 10) {
                batface = backliftDirection;
                flag = 1;
            }
        } else if (Pitch[backlift_start_index] > -150 && Pitch[backlift_start_index] <= -130) {
            batface = backliftDirection;
            flag = 1;
        }
    }
    if (flag == 0) {
        if (batting_hand == 1) {
            batface = Roll[backlift_start_index];
        } else {
            batface = -Roll[backlift_start_index];
        }
    }
    if (batface == -0 || (batface >= -5 && batface <=
                                           5)) // if batface is in range of -5 to 5 , we will show bat face as -5
    {
        batface = 0;
    }
    if (batface > 90) {
        batface = 90;
    }
    if (batface < -90) {
        batface = -90;
    }
    //   printf("%f \n",batface);
    return batface;
}

double calculateBackLiftDirectionAverage_Ana_test(double orientation_Ana,
                                                  int backlift_start_index_or_max_or_impact_index,
                                                  int userBattingHand) {
    double act_backliftdirection = 0;
    if (backlift_start_index_or_max_or_impact_index == 0) {
        for (i = 0; i < 5; i++) {

            act_backliftdirection = act_backliftdirection +
                                    calculateBackLiftDirection_Ana_test(orientation_Ana,
                                                                        backlift_start_index_or_max_or_impact_index +
                                                                        i, userBattingHand);
        }
        act_backliftdirection = act_backliftdirection / 5;
    } else if (backlift_start_index_or_max_or_impact_index == 1) {
        for (i = 0; i < 5; i++) {

            act_backliftdirection = act_backliftdirection +
                                    calculateBackLiftDirection_Ana_test(orientation_Ana,
                                                                        backlift_start_index_or_max_or_impact_index -
                                                                        1 + i, userBattingHand);
        }
        act_backliftdirection = act_backliftdirection / 5;
    } else {
        for (i = 0; i < 5; i++) {

            act_backliftdirection = act_backliftdirection +
                                    calculateBackLiftDirection_Ana_test(orientation_Ana,
                                                                        backlift_start_index_or_max_or_impact_index -
                                                                        2 + i, userBattingHand);
        }
        act_backliftdirection = act_backliftdirection / 5;
    }
    // printf("%f \n",act_backliftdirection);
    return act_backliftdirection;
}

double calculateBackLiftDirection_Ana_test(double orientation_Ana,
                                           int backlift_start_index_or_max_or_impact_index,
                                           int userBattingHand) {
    double orientation3 = 0, orientation3_2 = 0, backliftdirection = 0, backliftdirection2 = 0, act_backliftdirection = 0, orientation2 = 0,
            orientation2_2 = 0, pitch_avg = 0;

    orientation3 = calculataYawWithTiltCompensation_test(0, 90,
                                                         Xm[backlift_start_index_or_max_or_impact_index],
                                                         Ym[backlift_start_index_or_max_or_impact_index],
                                                         Zm[backlift_start_index_or_max_or_impact_index]);

    orientation3_2 = calculataYawWithTiltCompensation_test(0,
                                                           Pitch[backlift_start_index_or_max_or_impact_index],
                                                           Xm[backlift_start_index_or_max_or_impact_index],
                                                           Ym[backlift_start_index_or_max_or_impact_index +
                                                              i],
                                                           Zm[backlift_start_index_or_max_or_impact_index]);

    pitch_avg = Pitch[backlift_start_index_or_max_or_impact_index];

    orientation2 = orientation3;
    orientation2_2 = orientation3_2;
    // printf("%f %f \n",orientation2,orientation2_2);
    if (userBattingHand == 1) {
        if (orientation_Ana <= 180) {
            if (fabs(orientation2 - orientation_Ana) >= 180) {
                backliftdirection = 360 - orientation2 + orientation_Ana;

                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = 360 - orientation2_2 + orientation_Ana;
                } else {
                    backliftdirection2 = orientation_Ana - orientation2_2;
                }
            } else {
                backliftdirection = orientation_Ana - orientation2;
                // printf("%f %f \n",backliftdirection,fabs(orientation2 - orientation_Ana));
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = 360 - orientation2_2 + orientation_Ana;
                } else {
                    backliftdirection2 = orientation_Ana - orientation2_2;
                }
            }
        } else if (orientation_Ana >= 180) {
            if (fabs(orientation2 - orientation_Ana) >= 180) {
                backliftdirection = orientation_Ana - orientation2 - 360;
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = orientation_Ana - orientation2_2 - 360;
                } else {
                    backliftdirection2 = orientation_Ana - orientation2_2;
                }
            } else {
                backliftdirection = orientation_Ana - orientation2;
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = orientation_Ana - orientation2_2 - 360;
                } else {
                    backliftdirection2 = orientation_Ana - orientation2_2;
                }
            }
        }
    } else {
        if (orientation_Ana <= 180) {
            if (fabs(orientation2 - orientation_Ana) >= 180) {
                backliftdirection = -(360 - orientation2 + orientation_Ana);
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = -(360 - orientation2_2 + orientation_Ana);
                } else {
                    backliftdirection2 = -(orientation_Ana - orientation2_2);
                }
            } else {
                backliftdirection = -(orientation_Ana - orientation2);
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = -(360 - orientation2_2 + orientation_Ana);
                } else {
                    backliftdirection2 = -(orientation_Ana - orientation2_2);
                }
            }
        } else if (orientation_Ana > 180) {
            if (fabs(orientation2 - orientation_Ana) >= 180) {
                backliftdirection = -(orientation_Ana - orientation2 - 360);
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = -(orientation_Ana - orientation2_2 - 360);
                } else {
                    backliftdirection2 = -(orientation_Ana - orientation2_2);
                }
            } else {
                backliftdirection = -(orientation_Ana - orientation2);
                if (fabs(orientation2_2 - orientation_Ana) >= 180) {
                    backliftdirection2 = -(orientation_Ana - orientation2_2 - 360);
                } else {
                    backliftdirection2 = -(orientation_Ana - orientation2_2);
                }
            }
        }
    }
    backliftdirectiontest1 = backliftdirection;
    backliftdirectiontest2 = backliftdirection2;

    // printf("%f %f %f  \n",backliftdirectiontest1,backliftdirectiontest2,pitch_avg);
    if (pitch_avg >= 0 && pitch_avg <= 110) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection2;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index] / 2;
            }
        }
    } else if (pitch_avg > 110 && pitch_avg <= 130) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index] / 2;
            }
        }
    } else if (pitch_avg > 130 && pitch_avg <= 140) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection - 10;
                // printf("%f \n",act_backliftdirection);
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index] / 2;
            }
        }
    } else if (pitch_avg > 140 && pitch_avg <= 150) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection - 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index];
            }
        }
    } else if (pitch_avg > 150 && pitch_avg <= 160) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection - 25;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index];
            }
        }
    } else if (pitch_avg > 160 && pitch_avg <= 170) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection - 30;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index];
            }
        }
    } else if (pitch_avg > 170 && pitch_avg <= 180) {
        if (Roll[backlift_start_index_or_max_or_impact_index] >= 0) {
            if ((Roll[backlift_start_index_or_max_or_impact_index]) <= 40) {
                act_backliftdirection = backliftdirection - 35;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 40 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 50) {
                act_backliftdirection = backliftdirection + 5;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 50 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 60) {
                act_backliftdirection = backliftdirection + 10;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 60 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 70) {
                act_backliftdirection = backliftdirection + 12;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 70 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 80) {
                act_backliftdirection = backliftdirection + 15;
            } else if ((Roll[backlift_start_index_or_max_or_impact_index]) > 80 &&
                       (Roll[backlift_start_index_or_max_or_impact_index]) <= 90) {
                act_backliftdirection = backliftdirection + 20;
            }
        } else {
            if (Roll[backlift_start_index_or_max_or_impact_index] >= -30) {
                act_backliftdirection = backliftdirection;
            } else if (Roll[backlift_start_index_or_max_or_impact_index] < -30) {
                act_backliftdirection =
                        backliftdirection + Roll[backlift_start_index_or_max_or_impact_index];
            }
        }
    } else {
        act_backliftdirection = backliftdirection;
    }
    return act_backliftdirection;
}

double calculateBatDirectionforShotType_Ana_Test(double orientation_Ana, double pitch,
                                                 int max_or_impact_index,
                                                 int userBattingHand, int firmware_Version) {

    // double orientation2 = Yaw_T[max_or_impact_index + 1] + Roll[max_or_impact_index], backliftdirection = 0;
    // double orientation2 = Yaw_T[max_or_impact_index + 1] , backliftdirection = 0;
    // printf("%f %f \n",orientation_Ana,pitch);
    double orientation2 = 0, backliftdirection = 0;
    if (firmware_Version == 3) {
        if (SHV == 1) {
            orientation2 = Yaw[max_or_impact_index + 1];
        } else {
            orientation2 = Yaw[max_or_impact_index + 1];
        }
    } else {
        if (pitch < -80 && pitch > -120) {
            orientation2 = calculataYawWithTiltCompensation_test(0, -pitch, Xm[max_or_impact_index],
                                                                 Ym[max_or_impact_index],
                                                                 Zm[max_or_impact_index]);
        } else {
            orientation2 = calculataYawWithTiltCompensation_test(0, pitch, Xm[max_or_impact_index],
                                                                 Ym[max_or_impact_index],
                                                                 Zm[max_or_impact_index]);
        }
    }

    // printf("%f %f %f  \n",orientation2,Yaw[max_or_impact_index],Yaw_T[max_or_impact_index]);
    // printf("%f %f %f %f %f \n",orientation2,Xm[max_or_impact_index],Ym[max_or_impact_index],Zm[max_or_impact_index],pitch);

    backliftdirection = calculateDirectionForShotType(userBattingHand, orientation_Ana,
                                                      orientation2);

    // this code is not required for now this is for pitch change , will implement later.
    /*
     if(userBattingHand == 1)
     { //------- limit for pitch change 110 to 150
     if(Pitch_change > 150 && orientation <= 180)
     {
     backliftdirection = 180 + backliftdirection;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection = backliftdirection + 180 ;
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = backliftdirection - 180 ;
     }
     }
     else
     {
     if (Pitch_change > 150 && orientation <= 180 )
     {
     backliftdirection = - ( 180 + backliftdirection );
     }
     else if(Pitch_change > 150 && orientation > 180  && backliftdirection < 0 )
     {
     backliftdirection =  -( backliftdirection + 180 ) ;
     }
     else if(Pitch_change > 110 && orientation > 180  && backliftdirection > 0 )
     {
     backliftdirection = -( backliftdirection - 180 );
     }
     }
     */
    // printf("%f \n",backliftdirection);
    return backliftdirection;
}

double
calculateBatDirectionforShotTypeAverage_Ana_Test_R(double orientation_Ana, int max_or_impact_index,
                                                   int userBattingHand, int firmware_Version) {

    // printf("%f \n",orientation_Ana);
    double direction = 0;
    for (i = 0; i < 3; i++) {
        direction = direction + calculateBatDirectionforShotType_Ana_Test(orientation_Ana,
                                                                          Pitch[max_or_impact_index +
                                                                                i - 2],
                                                                          max_or_impact_index + i -
                                                                          2, userBattingHand,
                                                                          firmware_Version);
        // printf("%f \n",direction);
    }
    direction = direction / 3;
    return direction;
}

double
calculateBatDirectionforShotTypeAverage_Ana_Test_90(double orientation_Ana, int max_or_impact_index,
                                                    int userBattingHand, int firmware_version) {
    double direction = 0;
    for (i = 0; i < 3; i++) {
        direction = direction + calculateBatDirectionforShotType_Ana_Test(orientation_Ana, 90,
                                                                          max_or_impact_index + i -
                                                                          2, userBattingHand,
                                                                          firmware_version);
    }
    direction = direction / 3;
    return direction;
}

int calculate_Stop_or_Not(double acc[], double gyro[], double angle[], double orientation_app,
                          double orientation_ana, int battinghand, int maxindex,
                          int firmware_version) {
    int res = 1, shv = 0, short_k = 0;
    double backlift = 0, downswing = 0;
    shv = typeOfShot(SI, maxindex, FTI);
    short_k = shotAnalysis(gyro, angle, orientation_app, orientation_ana, SI, maxindex, FTI,
                           battinghand, firmware_version);
    //    printf("WHAR SHot %d \n",short_k);
    backlift = calculateBackLift(Pitch, BSI, SI, shv);
    downswing = calculateDownSwing(gyro, Pitch, backlift, short_k, SI, maxindex);
    if (shv == 1) {
        if (backlift > downswing) {
            for (i = FTI - 1; i > FTI - 4; i--) {
                if (Pitch[i] < -10) // if pitch at fti is less than -10 means that is an stop
                {
                    res = 0;
                    break;
                }
            }
        }
    }
    return res;
}

int shot_analysis_horizontal_pull_or_hook(int vertical_or_horizontal, double Xrotationarray[],
                                          double Yrotationarry[], int downswing_start_index,
                                          int max_or_impact_index, double backliftdiretion) {
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0, y3 = 0, y4 = 0, X1 = 0, X2 = 0;
    int shot_type = 0;
    if (vertical_or_horizontal == 0) {

        x1 = Xrotationarray[downswing_start_index - 1];
        x2 = Xrotationarray[max_or_impact_index];
        for (i = 0; i < 10; i++) {
            if (downswing_start_index < Yindex[i] && Yindex[i] > 0) {
                y1 = Yrotation_unity[Yindex[i - 1] - 1];
                break;
            }
        }
        y2 = Yrotation_unity[downswing_start_index - 1];
        for (i = 0; i < 10; i++) {
            if (max_or_impact_index < Yindex[i] && Yindex[i] > 0) {
                y3 = Yrotation_unity[Yindex[i - 1] - 1];
                if (y3 == y1) {
                    y3 = 0;
                }
                break;
            }
        }
        y4 = Yrotation_unity[max_or_impact_index];
        Yrota_at_impact = y4;
        X1 = sqrtf(x1 * x1 + (fabs(y2) - fabs(y1)) * (fabs(y2) - fabs(y1)));
        rota_at_downswing = X1;
        X2 = sqrtf(
                x2 * x2 + (fabs(y4) - (fabs(y3) - fabs(y2))) * (fabs(y4) - (fabs(y3) - fabs(y2))));
        rota_at_max_or_impact = X2;
    }
    // printf("%f %f  \n",X2,X1);
    if ((X2 - X1) >= 50) {
        shot_type = 10;
    } else {
        shot_type = 13;
    }
    return shot_type;
}

int
shot_analysis_horizontal_cut_pull_hook_using_new_logic(double backliftDirection_at_downswing_starts,
                                                       double orientation_App,
                                                       int max_or_impact_index, int batting_hand) {
    int shot_type = 0;
    double orientation2 = Yaw[max_or_impact_index], backliftdirection = 0;

    double pitch_at_max_or_impact = Pitch[max_or_impact_index];
    double roll_at_max_or_impact = Roll[max_or_impact_index];
    int rollIndex = getIndexForRollInRange(Roll, max_or_impact_index);
    if (rollIndex > 0) // new Logic
    {

        backliftdirection = Yaw[rollIndex] - orientation_App;
        //		printf("%f \n",backliftdirection);
        if (backliftdirection < 0) {
            backliftdirection = backliftdirection + 360;
        }

        if (batting_hand != 1) {
            backliftdirection = 360 - backliftdirection;
        }
        //        printf("%f %d %d \n",backliftdirection,rollIndex,max_or_impact_index);
        if (backliftdirection > 145 && backliftdirection <= 170) {
            if (rollIndex - max_or_impact_index < 15) {
                shot_type = 10;
            } else {
                shot_type = 5;
            }
        } else if (backliftdirection > 170 && backliftdirection < 200) {
            if (rollIndex - max_or_impact_index < 10) {
                shot_type = 10;
            } else {
                shot_type = 5;
            }
        } else if (backliftdirection <= 145) {
            shot_type = 10;
        } else if (backliftdirection >= 200) {
            shot_type = 5;
        }
    } else { // old Logic

        // printf(" pitch %f  %f  \n",pitch_at_max_or_impact,roll_at_max_or_impact);
        /*
         if(-180 <= pitch_at_max_or_impact && pitch_at_max_or_impact<= -160)
         {
         if(40 <= roll_at_max_or_impact && roll_at_max_or_impact <= 90)
         {
         orientation2 = orientation2 - (180 + 1.5 * roll_at_max_or_impact);
         }
         else if(30 <= roll_at_max_or_impact && roll_at_max_or_impact < 40)
         {
         orientation2 = orientation2 - (180 + 2 * roll_at_max_or_impact);
         }
         else if(0 <= roll_at_max_or_impact && roll_at_max_or_impact < 30)
         {
         orientation2 = orientation2 - (180 + 2.8 * roll_at_max_or_impact);
         }
         else
         {
         orientation2 = orientation2 + 0;
         }
         }
         else if(-160 < pitch_at_max_or_impact && pitch_at_max_or_impact<= -140)
         {
         if(20 <= roll_at_max_or_impact && roll_at_max_or_impact <= 90)
         {
         orientation2 = orientation2 - (180 + 1.5 * roll_at_max_or_impact);
         }
         else if(0 <= roll_at_max_or_impact && roll_at_max_or_impact < 20)
         {
         orientation2 = orientation2 - (180 + 2.8 * roll_at_max_or_impact);
         }
         else
         {
         orientation2 = orientation2 + 0;
         }
         }
         else if(-140 < pitch_at_max_or_impact && pitch_at_max_or_impact<= -120)
         {
         if(0 <= roll_at_max_or_impact && roll_at_max_or_impact <= 90)
         {
         orientation2 = orientation2 - (180 + roll_at_max_or_impact);
         }
         else
         {
         orientation2 = orientation2 + 0;
         }
         }
         else if(-120 < pitch_at_max_or_impact && pitch_at_max_or_impact<= -80)
         {
         if(0 <= roll_at_max_or_impact && roll_at_max_or_impact <= 90)
         {
         orientation2 = orientation2 - roll_at_max_or_impact;
         }
         else
         {
         orientation2 = orientation2 + 0;
         }
         }
         else if(-80 < pitch_at_max_or_impact && pitch_at_max_or_impact<= -60)
         {
         if(0 <= roll_at_max_or_impact && roll_at_max_or_impact <= 90)
         {
         orientation2 = orientation2 - roll_at_max_or_impact;
         //printf("%f  yes   \n",orientation2);
         }
         else
         {
         orientation2 = orientation2 + 0;
         }
         }
         if(orientation2 < 0){ // if orientation is negative then

         orientation2 = orientation2 + 360;
         //printf("%f orienta",orientation2);
         }

         */

        backliftdirection = calculateDirectionForShotType(batting_hand, orientation_App,
                                                          orientation2);

        /*
         if(roll_at_max_or_impact>=0 && roll_at_max_or_impact < 10)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 5;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 7;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 10;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=10 && roll_at_max_or_impact < 20)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 5;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 10;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 15;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 20;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=20 && roll_at_max_or_impact < 30)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 10;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 15;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 20;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 35;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=30 && roll_at_max_or_impact < 40)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 15;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 25;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 35;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 40;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=40 && roll_at_max_or_impact < 50)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 20;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 35;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 40;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 45;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=50 && roll_at_max_or_impact < 60)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 30;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 40;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 45;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 50;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=60 && roll_at_max_or_impact < 70)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 35;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 45;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 50;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 55;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=70 && roll_at_max_or_impact < 80)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 50;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 55;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 60;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 70;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=80 && roll_at_max_or_impact <= 90)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection + 70;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection + 75;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection + 80;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection + 85;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }

         else if(roll_at_max_or_impact>= -10 && roll_at_max_or_impact < 0)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 5;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 7;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 10;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=-20 && roll_at_max_or_impact < -10)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection ;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 10;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 15;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 20;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>= -30 && roll_at_max_or_impact < -20)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 5;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 10;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 20;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 35;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>= -40 && roll_at_max_or_impact < -30)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 10;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 20;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 30;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 40;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>= -50 && roll_at_max_or_impact < -40)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 15;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 25;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 40;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 45;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>= -60 && roll_at_max_or_impact < -50)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 20;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 30;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 45;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 50;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=-70 && roll_at_max_or_impact < -60)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 25;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 35;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 50;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 55;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>=-80 && roll_at_max_or_impact < -70)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 40;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 45;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 45;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 50;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         else if(roll_at_max_or_impact>= -90 && roll_at_max_or_impact < -80)
         {
         if(pitch_at_max_or_impact >= 0 && pitch_at_max_or_impact <= 120)
         {
         backliftdirection = backliftdirection - 50;
         }
         else if(pitch_at_max_or_impact > 120 && pitch_at_max_or_impact <= 140)
         {
         backliftdirection = backliftdirection - 55;
         }
         else if(pitch_at_max_or_impact > 140 && pitch_at_max_or_impact <= 150 )
         {
         backliftdirection = backliftdirection - 60;
         }
         else if(pitch_at_max_or_impact > 150 && pitch_at_max_or_impact <= 160 )
         {
         backliftdirection = backliftdirection - 65;
         }
         else
         {
         backliftdirection = backliftdirection;
         }
         }
         */
        // printf("%f \n",backliftdirection);
        if (backliftdirection <= 100 && backliftdirection >= 40) {
            shot_type = 5;
        } else if (backliftdirection <= 179 && backliftdirection > 100) {
            shot_type = 13;
        } else if (backliftdirection < 40 && backliftdirection >= -50) {
            shot_type = 10;
        } else if (backliftdirection < -50 && backliftdirection >= -179) {
            shot_type = 9;
        }
    }
    // printf("%d \n",shot_type);
    return shot_type;
}

int getIndexForRollInRange(double arr[], int max_or_impact_index) {
    int index = 0;
    for (i = max_or_impact_index; i < 130; i++) {
        if (arr[i] >= -15 && arr[i] <= 15 && Pitch[i] > 70) {
            //			printf("%f %f \n",arr[i],Pitch[i]);
            index = i;
            break;
        }
    }
    return index;
}

// this is for testing purpose only
double calculateAvg(double arr[], int start_index, int end_index) {
    double result = 0;
    if (start_index > end_index) {
        return result;
    }
    for (i = start_index; i < end_index; i++) {
        result = result + arr[i];
    }

    return result / (end_index - start_index);
}

double calculateDirectionForBackliftDirection(int batting_hand, double orientation_app_or_ana,
                                              double orientation) {
    double backliftdirection = 0;
    if (batting_hand == 1) {
        if (orientation_app_or_ana <= 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = 360 - orientation + orientation_app_or_ana;
            } else {
                backliftdirection = orientation_app_or_ana - orientation;
            }
        } else if (orientation_app_or_ana > 180) {

            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = (orientation_app_or_ana - orientation) - 360;
            } else {
                backliftdirection = orientation_app_or_ana - orientation;
                // printf(" backlift direction is  %f \n",backliftdirection);
            }
        }
    } else {
        if (orientation_app_or_ana <= 180) {

            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -(360 - orientation + orientation_app_or_ana);
            } else {
                backliftdirection = -(orientation_app_or_ana - orientation);
            }
        } else if (orientation_app_or_ana > 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -(orientation_app_or_ana - orientation - 360);
            } else {
                backliftdirection = -(orientation_app_or_ana - orientation);
            }
        }
    }
    //    printf("%f \n",backliftdirection);
    return backliftdirection;
}

double calculateDirectionForBackliftDirectionTest(int batting_hand, double orientation_app_or_ana,
                                                  double orientation) {
    double backliftdirection = 0;
    if (batting_hand == 1) {
        if (orientation_app_or_ana <= 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = 360 - orientation + orientation_app_or_ana;
            } else {
                backliftdirection = orientation_app_or_ana - orientation;
            }
        } else if (orientation_app_or_ana > 180) {

            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = (orientation_app_or_ana - orientation) - 360;
            } else {
                backliftdirection = orientation_app_or_ana - orientation;
                // printf(" backlift direction is  %f \n",backliftdirection);
            }
        }
    } else {

        if (orientation_app_or_ana <= 180) {
            //            printf("YES YES \n");

            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -(360 - orientation + orientation_app_or_ana);
            } else {
                backliftdirection = -(orientation_app_or_ana - orientation);
            }
        } else if (orientation_app_or_ana > 180) {
            //            printf("Yes %f \t",orientation_app_or_ana);
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -(orientation_app_or_ana - orientation - 360);
            } else {
                backliftdirection = -(orientation_app_or_ana - orientation);
            }
        }
    }
    //    printf("%f \n",backliftdirection);
    return backliftdirection;
}

double
calculateDirectionForShotType(int batting_hand, double orientation_app_or_ana, double orientation) {
    double backliftdirection = 0;
    if (batting_hand == 1) {
        if (orientation_app_or_ana <= 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = (orientation - orientation_app_or_ana) - 360;
            } else {
                backliftdirection = orientation - orientation_app_or_ana;
            }
        } else if (orientation_app_or_ana > 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = (orientation - orientation_app_or_ana) + 360;
            } else {
                backliftdirection = orientation - orientation_app_or_ana;
            }
        }
    } else {
        if (orientation_app_or_ana <= 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -((orientation - orientation_app_or_ana) - 360);
            } else {
                backliftdirection = -(orientation - orientation_app_or_ana);
            }
        } else if (orientation_app_or_ana > 180) {
            if (fabs(orientation - orientation_app_or_ana) >= 180) {
                backliftdirection = -((orientation - orientation_app_or_ana) + 360);
            } else {
                backliftdirection = -(orientation - orientation_app_or_ana);
            }
        }
    }
    return backliftdirection;
}

double calculateImpactAngle(double roll[], int impact_index, int shot_type) {
    double impactAngle = 0;
    double sum = 0;
    if (shot_type == 1 || shot_type == 0) {
        for (i = impact_index - 6; i < impact_index - 3; i++) {
            sum = sum + fabs(roll[i]);
        }
        impactAngle = sum / 3;
    }

    if (roll[impact_index - 1] < 0) {
        impactAngle = -impactAngle;
    }

    return impactAngle;
}

int kinematics(double acc[], double gyro[], double angle[], double mag[], double mag_max_min[]) {
    struct playerInfo *ourPlayer = getPlayerInfo();
    struct shotResult *ourResult = getResult();
    double speed = 0, satimpact = 0, backlift = 0, downswing = 0, time = 0,
            power = 0, powerAtImpact = 0, efficiency = 0, ballimpactatbat = 0, followthroug = 0,
            backliftdirection_ana = 0, backliftdirection_app = 0, backliftdirection_app_test_newFirmware = 0, downswingdirection_app_test_newFirmware = 0, impactormaxdirection_app_test_newFirmware = 0, followthrougdirection_app_test_newFirmware = 0, backliftdirection_avg = 0, backliftdirection_test = 0,
            batface = 0, maxorimpactdirection_ana = 0, maxorimpactdirection_app = 0, maxorimpactdirection = 0, maxorimpactdirection_90 = 0, maxorimpactdirection_r = 0, impact_angle = 0, impact_direction = 0, impact_groundedorair = 0, powerMax = 0, powerImpact = 0;
    int short_k = 0, wristbreak = 0, result = 1, diff = 0, d_diff = 0, shot_type_for_h_v = 0, max_or_impact = 0;
    setSensorData(acc, gyro, angle, mag, mag_max_min);
    calculataYawWithTiltCompensation(Roll, Pitch, Yaw_T, Xm, Ym, Zm);
    for (i = 0; i < 130; i++) {
        Yaw_T_R[i] = calculataYawWithTiltCompensation_test(0, Pitch[i], Xm[i], Ym[i], Zm[i]);
        Yaw_T_90[i] = calculataYawWithTiltCompensation_test(0, 90, Xm[i], Ym[i], Zm[i]);
        Yaw_T_90_R[i] = calculataYawWithTiltCompensation_test(Roll[i], 90, Xm[i], Ym[i], Zm[i]);
    }
    calculateIndex(Xg);
    YindexLimit = calculateIndexArray(Yg, Yindex);
    ZindexLimit = calculateIndexArray(Zg, Zindex);
    calculateRotationXarrUnity(Xrotation_unity);
    calculateRotationYarrUnity(Yrotation_unity, YindexLimit);
    calculateRotationZarrUnity(Zrotation_unity, ZindexLimit);
    //    printf("%f \n",Yrotation_unity[SI]);
    sweetSpotLenth = calculateLengthofSweetSpot(ourPlayer->batLength, ourPlayer->gripPosition);
    int tapping = isTappingDetected(Xg);
    if (tapping == 1) {
        int swing = isSwingDetected(Xg, player->gryoXRange);
        if (swing == 1) {
            calculateSpeed(Xg, sweetSpotLenth);
            speed = calculateMISpeed(MI);
            calculatePower(Xa, Xg, ourPlayer->batWeight, sweetSpotLenth, player->dpr);
            power = calculateMIPower(MI);
            calculatePosition(acc);
            //			printf("%f, \t",speed);
            SON = calculate_Stop_or_Not(acc, gyro, angle, ourPlayer->orientation_App,
                                        ourPlayer->orientation_Ana, ourPlayer->userBattingHand, MI,
                                        ourPlayer->firmware_version);
            II = calculateImpactIndex(Xg, Yg, Zg, MI, speed);
            if (SON == 1 && II == 0) {
                II = calculateImpactIndex2(Xg, Yg, Zg, MI);
            }
            // Impact R&D Parameters
            ourResult->impactBucket = impactBucket;
            ourResult->DiffXg1 = DiffXg1;
            ourResult->DiffXg2 = DiffXg2;
            ourResult->UpXg3 = UpXg3;
            ourResult->DiffYa = DiffYa;
            ourResult->DiffXa = DiffXa;
            ourResult->DiffZa = DiffZa;
            ourResult->DiffZg = DiffZg;
            ourResult->DiffYg = DiffYg;

            backliftdirection_ana = calculateBackLiftDirectionAverage_Ana(
                    ourPlayer->orientation_Ana, BSI, ourPlayer->userBattingHand);
            backliftdirection_app = calculateBackLiftDirectionAverage_App(
                    ourPlayer->orientation_App, BSI, ourPlayer->userBattingHand);
            backliftdirection_test = calculateBackLiftDirectionAverage_Ana_test(
                    ourPlayer->orientation_Ana, BSI, ourPlayer->userBattingHand);
            /*.......................... FOR TESTING PURPOSE ................................*/
            backliftdirection_app_test_newFirmware = calculateBackLiftDirection_App_Test_NewFirmware_Avg(
                    ourPlayer->orientation_App, BSI, ourPlayer->userBattingHand);
            //			printf("%f \n",backliftdirection_app_test_newFirmware);
            // For Match Mode
            ////            if(ourPlayer->mode == 1)
            //            {
            //                if(fabs(backliftdirection_app_test_newFirmware) > 120)
            //                {
            //                    if(ourPlayer->orientation_App > 180)
            //                    {
            //                        ourPlayer->orientation_App = ourPlayer->orientation_App - 180;
            //                    }
            //                    else
            //                    {
            //                        ourPlayer->orientation_App = ourPlayer->orientation_App + 180;
            //                    }
            //                    backliftdirection_app_test_newFirmware = calculateBackLiftDirection_App_Test_NewFirmware_Avg(ourPlayer->orientation_App,BSI,ourPlayer->userBattingHand);
            //                }
            //            }

            downswingdirection_app_test_newFirmware = calculateDownswingDirection_App_Test_NewFirmware(
                    ourPlayer->orientation_App, SI + 2, ourPlayer->userBattingHand);

            if (fabs(ourPlayer->orientation_Ana - ourPlayer->orientation_App) <
                50) // conditions to improve backlift direction
            {
                if (fabs(backliftdirection_app - backliftdirection_ana) < 30) {
                    backliftdirection_avg = (backliftdirection_ana + backliftdirection_app) / 2;
                } else {
                    backliftdirection_avg = backliftdirection_ana;
                }
            } else {
                backliftdirection_avg = backliftdirection_ana;
            }
            wristbreak = calculateWristBreak(Yg, SI, FTI);
            if (II > 0) {
                satimpact = calculateMISpeed(II);
                powerAtImpact = calculateMIPower(II);
                SHV = typeOfShot(SI, II, FTI);

                if (SHV == 1) {
                    PST = shotAnalysis(gyro, angle, ourPlayer->orientation_App,
                                       ourPlayer->orientation_Ana, SI, II, FTI,
                                       ourPlayer->userBattingHand, ourPlayer->firmware_version);
                } else {
                    if (ourPlayer->firmware_version == 3) {
                        PST = shot_analysis_horizontal_cut_pull_hook_using_new_logic(
                                downswingdirection_app_test_newFirmware, ourPlayer->orientation_App,
                                II, ourPlayer->userBattingHand);
                    } else {
                        PST = shot_analysis_horizontal_pull_or_hook(SHV, Xrotation_unity,
                                                                    Yrotation_unity, SI, II,
                                                                    backliftdirection_test);
                    }
                }
                WBI_UN = calculateWristBreakForUnity(ourPlayer->orientation_Ana,
                                                     ourPlayer->userBattingHand, II);
                maxorimpactdirection_ana = calculateBatDirectionforShotTypeAverage_Ana(
                        ourPlayer->orientation_Ana, II,
                        ourPlayer->userBattingHand);
                impact_angle = calculateImpactAngle(Roll, II, SHV);

                if (ourPlayer->firmware_version == 3) {
                    impactormaxdirection_app_test_newFirmware = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                            ourPlayer->orientation_App, II, ourPlayer->userBattingHand,
                            ourPlayer->firmware_version);
                } else {
                    impactormaxdirection_app_test_newFirmware = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                            ourPlayer->orientation_Ana, II, ourPlayer->userBattingHand,
                            ourPlayer->firmware_version);
                }

                impact_direction = impactormaxdirection_app_test_newFirmware;
                //                printf("impact_direction %f \n",impactormaxdirection_app_test_newFirmware);
                int shottypetochange = PST;
                if (shottypetochange == 1) {
                    PST = change_stright_to_on_drive(shottypetochange, II, WBI_UN,
                                                     impactormaxdirection_app_test_newFirmware);
                }
                ULI = calculateLastIndexforUnityAfterWristBreak(WBI_UN);
                short_k = PST;
                backlift = calculateBackLift(Pitch, BSI, SI, SHV);
                downswing = calculateDownSwing(gyro, Pitch, backlift, short_k, SI, II);
                impact_groundedorair = backlift - downswing;
                time = calculateTimeToImpact(Tg, SI, II);
                maxorimpactdirection_app = calculateBatDirectionforShotTypeAverage_App(
                        ourPlayer->orientation_App, II,
                        ourPlayer->userBattingHand);
                maxorimpactdirection = calculateBatDirectionforShotTypeAverage(
                        ourPlayer->orientation_App, ourPlayer->orientation_Ana, II,
                        ourPlayer->userBattingHand);
                maxorimpactdirection_r = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                        ourPlayer->orientation_Ana, II, ourPlayer->userBattingHand,
                        ourPlayer->firmware_version);
                maxorimpactdirection_90 = calculateBatDirectionforShotTypeAverage_Ana_Test_90(
                        ourPlayer->orientation_Ana, II, ourPlayer->userBattingHand,
                        ourPlayer->firmware_version);
                ballimpactatbat = calculateBallImpactAtBat(II);
                if (II >= FTI) {
                    FTI = FTI + 5;
                    IisG = 1;
                    if (FTI > 130) {
                        FTI = 129;
                    }
                }
                followthrougdirection_app_test_newFirmware = calculateBackLiftDirection_App_Test_NewFirmware(
                        ourPlayer->orientation_App, FTI, ourPlayer->userBattingHand);
                followthroug = calculateFollowThrough(gyro, Yg, Pitch, backlift, downswing, short_k,
                                                      II, FTI, ourPlayer->userBattingHand);
                if (ourPlayer->firmware_version == 3) {
                    calculateUnityData(maxorimpactdirection_app, maxorimpactdirection_ana,
                                       backliftdirection_app_test_newFirmware,
                                       ourPlayer->orientation_App, ourPlayer->orientation_Ana, II,
                                       ourPlayer->userBattingHand, ourPlayer->firmware_version);
                } else {
                    calculateUnityData(maxorimpactdirection_app, maxorimpactdirection_ana,
                                       backliftdirection_test, ourPlayer->orientation_App,
                                       ourPlayer->orientation_Ana, II, ourPlayer->userBattingHand,
                                       ourPlayer->firmware_version);
                }
                calculateUnityDataPosition(ourPlayer->orientation_Ana, ourPlayer->userBattingHand,
                                           MI, SON);
                calculateUnityZpositionFilterDataTo2(short_k, ourPlayer->userBattingHand);
                calculateUnityZpositionFilterData(short_k);
                max_or_impact = II;
                // in case of horizontal bat shots no sense of giving impcat pitch
                if (SHV == 1) {
                    ourResult->impact_Pitch = (Pitch[II]);
                } else {
                    ourResult->impact_Pitch = 0;
                }

                //				printf("%f \n",(Pitch[II]));
            } else {
                time = 0;
                powerAtImpact = 0;
                satimpact = 0;
                SHV = typeOfShot(SI, MI, FTI);
                //                printf(" Horizontal and Vertical %d \n",SHV);
                if (SHV == 1) {
                    PST = shotAnalysis(gyro, angle, ourPlayer->orientation_App,
                                       ourPlayer->orientation_Ana, SI, MI, FTI,
                                       ourPlayer->userBattingHand, ourPlayer->firmware_version);
                } else {
                    if (ourPlayer->firmware_version == 3) {
                        PST = shot_analysis_horizontal_cut_pull_hook_using_new_logic(
                                downswingdirection_app_test_newFirmware, ourPlayer->orientation_App,
                                MI, ourPlayer->userBattingHand);
                    } else {
                        PST = shot_analysis_horizontal_pull_or_hook(SHV, Xrotation_unity,
                                                                    Yrotation_unity, SI, MI,
                                                                    backliftdirection_test);
                    }
                }
                WBI_UN = calculateWristBreakForUnity(ourPlayer->orientation_Ana,
                                                     ourPlayer->userBattingHand, MI);
                ULI = calculateLastIndexforUnityAfterWristBreak(WBI_UN);
                short_k = PST;
                backlift = calculateBackLift(Pitch, BSI, SI, SHV);
                if (SHV == 1) {
                    int pitchIndex = lastPositiveIndexPitchDuringDownSwing(Pitch, SI, MI, FTI, SHV,
                                                                           WBI_UN);
                    downswing = calculateDownSwing(gyro, Pitch, backlift, short_k, SI, pitchIndex);
                    followthroug = calculateFollowThrough(gyro, Yg, Pitch, backlift, downswing,
                                                          short_k, pitchIndex, FTI,
                                                          ourPlayer->userBattingHand);
                    //                    printf(" %d  %d %f \n",pitchIndex,WBI_UN,Pitch[pitchIndex]);
                } else {
                    downswing = calculateDownSwing(gyro, Pitch, backlift, short_k, SI, MI);
                    followthroug = calculateFollowThrough(gyro, Yg, Pitch, backlift, downswing,
                                                          short_k, MI, FTI,
                                                          ourPlayer->userBattingHand);
                }
                maxorimpactdirection_ana = calculateBatDirectionforShotTypeAverage_Ana(
                        ourPlayer->orientation_Ana, MI,
                        ourPlayer->userBattingHand);
                maxorimpactdirection_app = calculateBatDirectionforShotTypeAverage_App(
                        ourPlayer->orientation_App, MI,
                        ourPlayer->userBattingHand);
                maxorimpactdirection = calculateBatDirectionforShotTypeAverage(
                        ourPlayer->orientation_App, ourPlayer->orientation_Ana, MI,
                        ourPlayer->userBattingHand);
                maxorimpactdirection_r = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                        ourPlayer->orientation_Ana, MI, ourPlayer->userBattingHand,
                        ourPlayer->firmware_version);
                maxorimpactdirection_90 = calculateBatDirectionforShotTypeAverage_Ana_Test_90(
                        ourPlayer->orientation_Ana, MI, ourPlayer->userBattingHand,
                        ourPlayer->firmware_version);

                if (ourPlayer->firmware_version == 3) {
                    impactormaxdirection_app_test_newFirmware = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                            ourPlayer->orientation_App, MI, ourPlayer->userBattingHand,
                            ourPlayer->firmware_version);
                } else {
                    impactormaxdirection_app_test_newFirmware = calculateBatDirectionforShotTypeAverage_Ana_Test_R(
                            ourPlayer->orientation_Ana, MI, ourPlayer->userBattingHand,
                            ourPlayer->firmware_version);
                }

                followthrougdirection_app_test_newFirmware = calculateBackLiftDirection_App_Test_NewFirmware(
                        ourPlayer->orientation_App, FTI, ourPlayer->userBattingHand);

                if (ourPlayer->firmware_version == 3) {
                    calculateUnityData(maxorimpactdirection_app, maxorimpactdirection_ana,
                                       backliftdirection_app_test_newFirmware,
                                       ourPlayer->orientation_App, ourPlayer->orientation_Ana, MI,
                                       ourPlayer->userBattingHand, ourPlayer->firmware_version);
                } else {
                    calculateUnityData(maxorimpactdirection_app, maxorimpactdirection_ana,
                                       backliftdirection_test, ourPlayer->orientation_App,
                                       ourPlayer->orientation_Ana, MI, ourPlayer->userBattingHand,
                                       ourPlayer->firmware_version);
                }
                calculateUnityDataPosition(ourPlayer->orientation_Ana, ourPlayer->userBattingHand,
                                           MI, SON);
                calculateUnityZpositionFilterDataTo2(short_k, ourPlayer->userBattingHand);
                calculateUnityZpositionFilterData(short_k);
                max_or_impact = MI;
                ourResult->impact_Pitch = 0;
            }
            shot_type_for_h_v = SHV;
            int firstNegativeIndex = calculateFirstNegativeYgInBack();
            ourResult->downswing_Direction = directionOfBatAtAnyPoint(ourPlayer->orientation_App,
                                                                      ourPlayer->userBattingHand,
                                                                      BSI + 1, SI, SHV, PST,
                                                                      firstNegativeIndex);
            //			printf("%f %d %d %f %f \n",ourResult->downswing_Direction,SHV,PST,Pitch[SI],Roll[SI]);
            batface = calculateBatFace(angle, BSI, ourPlayer->userBattingHand, backlift,
                                       backliftdirection_app_test_newFirmware);
            double speedFac = factorForSpeed(Xg, SI, MI, FTI, sweetSpotLenth);
            //            printf("speedFac %f \n",speedFac);
            //            printf("speed %f,satimpact %f",Xg[MI],Xg[II]);
            if (abs(MI - II) < 5) // if impact and max same then we are adding to both max and
            {
                speed = speed + speedFac;
                satimpact = satimpact + speedFac;
                if (speed > 100) {
                    speed = 97;
                    satimpact = 97;
                }
            } else {
                speed = speedFac + speed;
                if (speed > 100) {
                    speed = 97;
                }
            }
            if (speed < satimpact) // if impact speed is more than
            {
                speed = satimpact;
            }

            powerMax = calculateMIPowerWithOutTime(MI, ourPlayer->batWeight, sweetSpotLenth);
            powerImpact = calculateMIPowerWithOutTime(II, ourPlayer->batWeight, sweetSpotLenth);
            if (powerImpact > powerMax) {
                powerMax = powerImpact;
            }
            efficiency = calculateEfficiency(MI, II, speed, satimpact);
            ourResult->powerMax = floor(powerMax * 100) * 0.01;
            ourResult->powerImpact = floor(powerImpact * 100) * 0.01;
            ourResult->powerAtImpact = floor(powerAtImpact * 100) * 0.01;
            //			printf("%f %f %f %f %d %d  \n",ourResult->powerMax,ourResult->powerImpact,Xg[II],Xg[MI],II,MI);
            ourResult->Power = floor(power * 100) / 100;
            ourResult->Speed = floor(speed * 100) / 100;
            ourResult->speedAtImpact = floor(satimpact * 100) / 100;
            ourResult->Efficiency = efficiency;
            ourResult->Backlift = floor(backlift * 100) / 100;
            ourResult->Downswing = floor(downswing * 100) / 100;
            ourResult->Followthroug = floor(followthroug * 100) / 100;
            ourResult->shotType = short_k;
            ourResult->wrist_break_index = WBI_UN;
            ourResult->vertical_or_horizontal_shot = SHV;
            ourResult->shotType2 = shotType;
            ourResult->timeToImpact = time;
            ourResult->maxIndex = MI;
            ourResult->impactIndex = II;
            ourResult->backlift_start_index = BSI;
            ourResult->follow_through_index = FTI;
            ourResult->unity_last_index = ULI;
            ourResult->batBackliftDirection_Ana = floor(backliftdirection_ana * 100) / 100;
            ourResult->batBackliftDirection_App = floor(backliftdirection_app * 100) / 100;
            ourResult->batBackliftDirection_Avg =
                    floor(backliftdirection_test * 100) / 100; // for ios app
            ourResult->batBackliftDirection_Test = floor(backliftdirection_test * 100) / 100;
            ourResult->batbackliftDirection_Test1 =
                    floor(backliftdirectiontest1 * 100) / 100; // 0 roll 90 pitch
            ourResult->batbackliftDirection_Test2 =
                    floor(backliftdirectiontest2 * 100) / 100; // 0 roll  actual pitch
            ourResult->batbackliftDirection_App_NewFirmware_Test =
                    floor(backliftdirection_app_test_newFirmware * 100) / 100;
            //			printf("%d ,%f ,%f,\n",II,batface,backliftdirection_app_test_newFirmware);
            ourResult->batdownswingDirection_App_NewFirmware_Test =
                    floor(downswingdirection_app_test_newFirmware * 100) / 100;
            ourResult->batimpactDirection_App_NewFirmware_Test =
                    floor(impactormaxdirection_app_test_newFirmware * 100) / 100;
            ourResult->batfollowThroughDirection_App_NewFirmware_Test =
                    floor(followthrougdirection_app_test_newFirmware * 100) / 100;
            ourResult->impact_Angle = impact_angle;
            ourResult->impact_Direction = impact_direction;
            ourResult->impact_GroundedAir = impact_groundedorair;
            double mybackliftdirection = 0;
            if (ourPlayer->firmware_version == 3) {
                mybackliftdirection = floor(backliftdirection_app_test_newFirmware * 100) / 100;
                ourResult->batBackliftDirection =
                        floor(backliftdirection_app_test_newFirmware * 100) / 100;
                //                printf("%f \n",mybackliftdirection);
            } else {
                mybackliftdirection = floor(backliftdirection_test * 100) / 100;
                ourResult->batBackliftDirection = floor(backliftdirection_test * 100) / 100;
            }
            ourResult->batImapctDirection = floor(maxorimpactdirection * 100) / 100;
            ourResult->batImapctDirection_Ana = floor(maxorimpactdirection_ana * 100) / 100;
            ourResult->batImapctDirection_App = floor(maxorimpactdirection_app * 100) / 100;
            ourResult->batImpactDirection_Ana_test_r = floor(maxorimpactdirection_r * 100) / 100;
            ourResult->batImpactDirection_Ana_test_90 = floor(maxorimpactdirection_90 * 100) / 100;
            ourResult->batFace = batface;
            ourResult->g_y_negative = gyro_y_negative;
            ourResult->g_y_negative_greater = gyro_y_negative_greater;
            ourResult->ball_impact_at_bat = ballimpactatbat;
            diff = FTI - BSI;
            ourResult->Yawat90PitchatDownswing =
                    floor(calculataYawWithTiltCompensation_test(0, 90, Xm[SI], Ym[SI], Zm[SI]) *
                          100) / 100;
            ourResult->YawatRealPitchatDownswing =
                    floor(calculataYawWithTiltCompensation_test(0, Pitch[SI], Xm[SI], Ym[SI],
                                                                Zm[SI]) * 100) / 100;
            ourResult->YRota_at_impact = Yrota_at_impact;
            ourResult->Rota_at_downswing = rota_at_downswing;
            ourResult->Rota_at_max_or_impact = rota_at_max_or_impact;
            ourResult->F_Xa = f_ax;
            ourResult->F_Ya = f_ay;
            ourResult->F_Za = f_az;
            ourResult->F_Xg = f_gx;
            ourResult->F_Yg = f_gy;
            ourResult->F_Zg = f_gz;
            ourResult->Roll_at_start_backlift = Roll[BSI];
            ourResult->Pitch_at_start_backlift = Pitch[BSI];
            ourResult->rollAtBacklift_Test = Roll[BSI];
            ourResult->pitchAtBacklift_Test = Pitch[BSI];
            ourResult->rollAtDownswing_Test = Roll[SI];
            ourResult->pitchAtDownswing_Test = Pitch[SI];
            ourResult->rollAtImpact_Test = Roll[max_or_impact];
            ourResult->pitchAtImpact_Test = Pitch[max_or_impact];
            ourResult->rollAtfollowThrough_Test = Roll[FTI];
            ourResult->pitchAtfollowThrough_Test = Pitch[FTI];
            for (i = 0; i < 130; i++) {
                ourResult->Yaw_tilt[i] = floor(Yaw_T[i] * 100) / 100;
                ourResult->Yaw_tilt_90[i] = floor(Yaw_T_90[i] * 100) / 100;
                ourResult->Yaw_tilt_R[i] = floor(Yaw_T_R[i] * 100) / 100;
                ourResult->Yaw_tilt_90_R[i] = floor(Yaw_T_90_R[i] * 100) / 100;
                ourResult->Xrotation_validation[i] = floor(Xrotation_unity[i] * 100) / 100;
                ourResult->Yrotation_validation[i] = floor(Yrotation_unity[i] * 100) / 100;
                ourResult->Zrotation_validation[i] = floor(Zrotation_unity[i] * 100) / 100;
                // printf(" %d  %f %f \n",i,Zg[i],ourResult->Yaw_tilt[i]);
            }
            for (i = 0; i < diff; i++) {
                ourResult->posX[i] = floor(Xpos_Unity_acc[i] * 100) / 100;
                ourResult->posY[i] = floor(Ypos_Unity_acc[i] * 100) / 100;
                ourResult->posZ[i] = floor(Zpos_Unity_acc[i] * 100) / 100;

                ourResult->PitchUnity[i] = floor(PitchX_U[i] * 100) / 100;
                ourResult->YawUnity[i] = floor(YawY_U[i] * 100) / 100;
                ourResult->RollUnity[i] = floor(RollZ_U[i] * 100) / 100;
            }
            d_diff = 2 * diff - 1;
            if (shot_type_for_h_v == 0) {
                if (short_k == 33) {
                    calculateDiffAndSmooth(YawY_U_Filter_With_diff, YawY_U_Filter);
                    for (i = 0; i < 2 * (130 - BSI) - 4; i++) {
                        ourResult->TimeUnityFilter[i] =
                                floor(Time_U_Filter[i + 2 * BSI + 2] * 100) / 100;
                        ourResult->posXFilter[i] = floor(Xpos_Unity_acc[i] * 100) / 100;
                        ourResult->posYFilter[i] = floor(Ypos_Unity_acc[i] * 100) / 100;
                        ourResult->posZFilter[i] = floor(Zpos_Unity_acc2[i] * 100) /
                                                   100; // z index problem so changed to extra array        Zpos_Unity_acc2[i]
                        ourResult->PitchUnityFilter[i] = floor(PitchX_U_Filter[i] * 100) / 100;
                        ourResult->YawUnityFilter[i] =
                                floor(YawY_U_Filter_With_diff[i] * 100) / 100;
                        ourResult->RollUnityFilter[i] = floor(RollZ_U_Filter[i] * 100) / 100;
                    }
                } else {
                    for (i = 0; i < d_diff; i++) {
                        ourResult->TimeUnityFilter[i] =
                                floor(Time_U_Filter[i + 2 * BSI + 2] * 100) / 100;
                        ourResult->posXFilter[i] = floor(Xpos_Unity_acc[i] * 100) / 100;
                        ourResult->posYFilter[i] = floor(Ypos_Unity_acc[i] * 100) / 100;
                        ourResult->posZFilter[i] = floor(Zpos_Unity_acc2[i] * 100) / 100;
                        ourResult->PitchUnityFilter[i] = floor(PitchX_U_Filter[i] * 100) / 100;
                        ourResult->YawUnityFilter[i] = floor(YawY_U_Filter[i] * 100) / 100;
                        ourResult->RollUnityFilter[i] = floor(RollZ_U_Filter[i] * 100) / 100;
                        // printf("%f,%f,%f,%f,%f,%f,%f\n",ourResult->TimeUnityFilter[i],ourResult->posXFilter[i],ourResult->posYFilter[i],ourResult->posZFilter[i],ourResult->PitchUnityFilter[i],ourResult->YawUnityFilter[i],ourResult->RollUnityFilter[i]);
                    }
                }
            } else {
                if (short_k == 3 && WBI_UN != 0) {
                    calculateDiffAndSmooth(YawY_U_Filter_With_diff, YawY_U_Filter);
                    calculatePitchFilterNextValueHigh(PitchX_U_Filter_High_Value, PitchX_U_Filter,
                                                      max_or_impact);
                    if (mybackliftdirection > -80 && mybackliftdirection < 80) {
                        for (i = 0; i < 2 * (ULI - BSI) - 4; i++) {
                            ourResult->TimeUnityFilter[i] =
                                    floor(Time_U_Filter[i + 2 * BSI + 2] * 100) / 100;
                            ourResult->posXFilter[i] = floor(Xpos_Unity_acc[i] * 100) / 100;
                            ourResult->posYFilter[i] = floor(Ypos_Unity_acc[i] * 100) / 100;
                            ourResult->posZFilter[i] = floor(Zpos_Unity_acc2[i] * 100) / 100;
                            ourResult->PitchUnityFilter[i] =
                                    floor(PitchX_U_Filter_High_Value[i] * 100) / 100;
                            ourResult->YawUnityFilter[i] =
                                    floor(YawY_U_Filter_With_diff[i] * 100) / 100;
                            ourResult->RollUnityFilter[i] = floor(RollZ_U_Filter[i] * 100) / 100;
                            //                            printf("%f,%f,%f,%f,%f,%f,%f\n",ourResult->TimeUnityFilter[i],ourResult->posXFilter[i],ourResult->posYFilter[i],ourResult->posZFilter[i],ourResult->PitchUnityFilter[i],ourResult->YawUnityFilter[i],ourResult->RollUnityFilter[i]);
                        }
                    } else {
                        for (i = 0; i < 2 * (ULI - BSI - SI) - 4; i++) {
                            ourResult->TimeUnityFilter[i] =
                                    floor(Time_U_Filter[i + 2 * (BSI + SI) + 2] * 100) / 100;
                            ourResult->posXFilter[i] =
                                    floor(Xpos_Unity_acc[i + 2 * SI] * 100) / 100;
                            ourResult->posYFilter[i] =
                                    floor(Ypos_Unity_acc[i + 2 * SI] * 100) / 100;
                            ourResult->posZFilter[i] =
                                    floor(Zpos_Unity_acc2[i + 2 * SI] * 100) / 100;
                            ourResult->PitchUnityFilter[i] =
                                    floor(PitchX_U_Filter_High_Value[i + 2 * SI] * 100) / 100;
                            ourResult->YawUnityFilter[i] =
                                    floor(YawY_U_Filter_With_diff[i + 2 * SI] * 100) / 100;
                            ourResult->RollUnityFilter[i] =
                                    floor(RollZ_U_Filter[i + 2 * SI] * 100) / 100;
                            //                            printf("%f,%f,%f,%f,%f,%f,%f\n",ourResult->TimeUnityFilter[i],ourResult->posXFilter[i],ourResult->posYFilter[i],ourResult->posZFilter[i],ourResult->PitchUnityFilter[i],ourResult->YawUnityFilter[i],ourResult->RollUnityFilter[i]);
                        }
                    }
                } else {
                    calculateDiffAndSmooth(YawY_U_Filter_With_diff, YawY_U_Filter);
                    calculatePitchFilterNextValueHigh(PitchX_U_Filter_High_Value, PitchX_U_Filter,
                                                      max_or_impact);
                    if (mybackliftdirection > -80 && mybackliftdirection < 80) {
                        for (i = 0; i < d_diff; i++) {
                            ourResult->TimeUnityFilter[i] =
                                    floor(Time_U_Filter[i + 2 * BSI + 2] * 100) / 100;
                            ourResult->posXFilter[i] = floor(Xpos_Unity_acc[i] * 100) / 100;
                            ourResult->posYFilter[i] = floor(Ypos_Unity_acc[i] * 100) / 100;
                            ourResult->posZFilter[i] = floor(Zpos_Unity_acc2[i] * 100) / 100;
                            ourResult->PitchUnityFilter[i] =
                                    floor(PitchX_U_Filter_High_Value[i] * 100) / 100;
                            ourResult->YawUnityFilter[i] =
                                    floor(YawY_U_Filter_With_diff[i] * 100) / 100;
                            ourResult->RollUnityFilter[i] = floor(RollZ_U_Filter[i] * 100) / 100;
                            //                            printf("%.f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",ourResult->TimeUnityFilter[i],ourResult->posXFilter[i],ourResult->posYFilter[i],ourResult->posZFilter[i],ourResult->PitchUnityFilter[i],ourResult->YawUnityFilter[i],ourResult->RollUnityFilter[i]);
                            // printf("%f %f \n",ourResult->posZFilter[i],ourResult->PitchUnityFilter[i]);
                        }
                    } else {
                        for (i = 0; i < d_diff - 2 * SI; i++) {
                            ourResult->TimeUnityFilter[i] =
                                    floor(Time_U_Filter[i + 2 * (BSI + SI) + 2] * 100) / 100;
                            ourResult->posXFilter[i] =
                                    floor(Xpos_Unity_acc[i + 2 * SI] * 100) / 100;
                            ourResult->posYFilter[i] =
                                    floor(Ypos_Unity_acc[i + 2 * SI] * 100) / 100;
                            ourResult->posZFilter[i] =
                                    floor(Zpos_Unity_acc2[i + 2 * SI] * 100) / 100;
                            ourResult->PitchUnityFilter[i] =
                                    floor(PitchX_U_Filter_High_Value[i + 2 * SI] * 100) / 100;
                            ourResult->YawUnityFilter[i] =
                                    floor(YawY_U_Filter_With_diff[i + 2 * SI] * 100) / 100;
                            ourResult->RollUnityFilter[i] =
                                    floor(RollZ_U_Filter[i + 2 * SI] * 100) / 100;
                            //                            printf("%.f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",ourResult->TimeUnityFilter[i],ourResult->posXFilter[i],ourResult->posYFilter[i],ourResult->posZFilter[i],ourResult->PitchUnityFilter[i],ourResult->YawUnityFilter[i],ourResult->RollUnityFilter[i]);
                        }
                    }
                }
            }
            if (ourPlayer->firmware_version == 3) {
                ourResult->unity_data_correct_ornot = calculateDataForUnityCorrectOrNotForVerticalShots(
                        SHV, YawY_U_Filter_With_diff, PitchX_U_Filter_High_Value,
                        backliftdirection_app_test_newFirmware,
                        downswingdirection_app_test_newFirmware, WBI_UN, max_or_impact);
                //                printf("%d \n",ourResult->unity_data_correct_ornot);
            } else {
                ourResult->unity_data_correct_ornot = calculateDataForUnityCorrectOrNotForVerticalShots(
                        SHV, YawY_U_Filter_With_diff, PitchX_U_Filter_High_Value,
                        backliftdirection_test, downswingdirection_app_test_newFirmware, WBI_UN,
                        max_or_impact); // there downswing direction does not make any change
            }
        } else {
            result = 9;
        }
    } else {
        result = 0;
    }
    double rot1 = calculateRotationZ(Zg, II - 1, II + 1);
    //	double rot2 = calculateRotationZ(Zg, II, II+5);
    //	printf("%f %f %d %d %f %f \n",rot1,rot2,II-1,II,Zg[II-1],Zg[II]);

    //	printf("%f %f %f %f %f %f \n",Zrotation_unity[II-5],Zrotation_unity[II],Zrotation_unity[II + 5],Roll[II-5],Roll[II],Roll[II+5]);
    //	printf(" %d %d %d %d %f %f %f  %f  %f \n",SI,MI,MI-SI,II,power,Xg[MI],Xg[II],speed,satimpact);
    //    printf(" %d %d %f %f  %f %f \n",II,MI,speed,satimpact,efficiency,power);
    //	printf("%f \n",ourResult->batBackliftDirection);
    //	printf("%f \n",ourResult->impact_Pitch);
    //    printf("%f,%f \n",ourResult->Speed,ourResult->speedAtImpact);
    //      printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%f,%d,%d,%f,%f,%f \n",ourResult->Power,ourResult->Efficiency,backlift,downswing,followthroug,ourResult->impact_Angle,ourResult->impact_Direction,ourResult->impact_GroundedAir,ourResult->Speed,ourResult->speedAtImpact,MI,II,ourResult->timeToImpact,ourResult->shotType,SHV,ourResult->batBackliftDirection,batface,ourResult->batdownswingDirection_App_NewFirmware_Test);
    //    printf("\n");
    //    printf(" Wrist break %d  %d %d \n",WBI_UN,PST,II);
    //    printf("%f,%f,%f,%f,%f,%f \n",ourResult->Backlift,ourResult->batbackliftDirection_App_NewFirmware_Test,ourResult->batdownswingDirection_App_NewFirmware_Test,ourResult->impact_Angle,ourResult->batFace,ourResult->Pitch_at_start_backlift);
    //    printf("%f,%f,%f,%f,%f,%f,%d,%d \n",ourResult->Speed,ourResult->speedAtImpact,backlift,downswing,followthroug,batface,ourResult->shotType,WBI_UN);

    // printf("%d %f \n",shotType,Roll[max_or_impact]);
    /// this part for run multiple file and save result in csv file.
    //    {
    //        int first_nega = calculateFirstNegativeYgInBackforTesting();
    ////        printf("Negative in Back and Downswing start Index %d,%d \n",first_nega,SI);
    ////        printf("Bat_Downswing_direction %f \n ",ourResult->batdownswingDirection_App_NewFirmware_Test);
    ////        printf("X rotation at Downsing %f \n ",Xrotation_unity[SI]);
    ////        printf("X rotation at max Or Impact %f \n ",Xrotation_unity[max_or_impact]);
    ////        printf("Y rotation at Downsing %f,%f  \n ",Yrotation_unity[SI],Yg[SI]);
    ////        printf("Y rotation at max Or Impact %f,%f \n ",Yrotation_unity[max_or_impact],Yg[max_or_impact]);
    ////        printf("Average of Yg %f \n",calculateAvg(Yg,first_nega,  SI));
    ////        printf("Yaw Difference %f \n ",Yaw[max_or_impact] - ourPlayer->orientation_App);
    ////        printf("Roll at Impact %f \n",Roll[max_or_impact]);
    ////        printf("Pitch at Impact %f \n",Pitch[max_or_impact]);
    //        FILE *fp = fopen("/Users/apple/desktop/Data/Day 1 /cutshots.csv", "a");
    //    fprintf(fp,"\n%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",short_k,first_nega,SI,max_or_impact,Xrotation_unity[SI],Xrotation_unity[max_or_impact],Yrotation_unity[SI],Yrotation_unity[max_or_impact],Yg[SI],Yg[max_or_impact],Roll[max_or_impact],calculateAvg(Roll,max_or_impact,max_or_impact+5 ),calculateAvg(Roll,max_or_impact+5,max_or_impact+10) ,Pitch[max_or_impact],Yaw[max_or_impact],ourPlayer->orientation_App,calculateAvg(Yg,first_nega, SI),ourResult->batdownswingDirection_App_NewFirmware_Test);
    //        fclose(fp);
    //    }

    //    printf("%d %d %d %d \n",MI,FTI,BSI,WBI_UN);
    //    printf("%f \n",Xg[MI]);
    //    printf("Back-lift                      %f \n",ourResult->Backlift );
    //    printf("Power                          %f \n",ourResult->Power );

    //  printf("%f,%f,%f,%f,%f \n",downswing,followthroug,impact_angle,impact_direction,impact_groundedorair);

    /*
     printf("Shot-Type  using Yaw           %d \n",ourResult->shotType);
     printf(" is vertical or horizontal     %d \n",SHV);
     printf("%d \n",MI);
     printf("is unity data correct or not   %d \n",ourResult->unity_data_correct_ornot);
     printf("is vertical or horizontal      %d \n",SHV);
     printf("Wrist Break Index              %d \n",WBI_UN);
     printf("Unity last Index               %d \n",ULI);
     printf("Differenese                    %d \n",SI - BSI);
     printf("Back-lift                      %f \n",ourResult->Backlift );
     printf("Power                          %f \n",ourResult->Power );
     printf("Power at Impact                %f \n",ourResult->powerAtImpact );
     printf("Speed                          %f \n",ourResult->Speed );
     printf("Speed at Impact                %f \n",ourResult->speedAtImpact );
     printf("Efficiency                     %f \n",ourResult->Efficiency );
     printf("Downswing                      %f \n",ourResult->Downswing );
     printf("Follow-Through                 %f \n",ourResult->Followthroug );
     printf("Shot-Type  using Yaw           %d \n",ourResult->shotType );
     printf("Shot-Type  using Roll          %d \n",ourResult->shotType2 );
     printf("T-to-Impact                    %f \n",ourResult->timeToImpact);
     printf("Max Index                      %d \n",ourResult->maxIndex);
     printf("Max Index  Xg                  %f \n",Xg[MI]);
     printf("Impact  index                  %d \n",ourResult->impactIndex);
     printf("Follow through index           %d \n",FTI);
     printf("Follow through  Xg             %f \n",Xg[II]);
     printf("Max Index  Xg                  %f \n",Xg[ourResult->impactIndex]);
     printf("Pitch at impact                %f \n",Yaw[ourResult->impactIndex]);
     printf("batBackLIftdirection_Ana       %f \n",ourResult->batBackliftDirection_Ana);
     printf("batBackLIftdirection_App       %f \n",ourResult->batBackliftDirection_App);
     printf("batBackLIftdirection_Avg       %f \n",ourResult->batBackliftDirection_Avg);
     printf("batBackLIftdirection_test      %f \n",ourResult->batBackliftDirection_Test);
     printf("batImpactdirection             %f \n",ourResult->batImapctDirection);
     printf("batImpactdirection_App         %f \n",ourResult->batImapctDirection_App);
     printf("batImpactdirection_Ana         %f \n",ourResult->batImapctDirection_Ana);
     printf("batImpactdirection_Ana_test_r  %f \n",ourResult->batImpactDirection_Ana_test_r);
     printf("batImpactdirection_Ana_test_90 %f \n",ourResult->batImpactDirection_Ana_test_90 );
     printf("BatFace                        %f \n",ourResult->batFace);
     printf("Ball Impact At Bat             %f \n",ourResult->ball_impact_at_bat);
     printf("G_Y_Negative  for Wrist Break  %d \n",ourResult->g_y_negative);
     printf("G_Y_Negative  for wrist Break  greater then -200  %d \n",ourResult->g_y_negative_greater);
     printf("%d result\n",result);
     */
    return result;
}

struct shotResult *getStruct(int a) {
    struct shotResult *ourResult = getResult();
    return ourResult;
}

int lastPositiveIndexPitchDuringDownSwing(double Pitch[], int downswing_start_index, int max_index,
                                          int follow_end_index, int shottype, int wrist_break) {
    int i = 0, index = follow_end_index;
    if (shottype == 1) {
        if (wrist_break == 0) {
            for (i = max_index - 3; i < follow_end_index; i++) {
                if (Pitch[i] < 0 && Pitch[i + 1] < 0) {
                    index = i - 1;
                    break;
                } else if (Pitch[i] == 0) {
                    index = i - 1;
                    break;
                } else if (Pitch[i] < 10) {
                    index = i - 1;
                    break;
                }
            }
        } else {
            double min = Pitch[max_index - 3];
            for (i = max_index - 3; i < follow_end_index; i++) {
                if (Pitch[i] > 0) {
                    if (Pitch[i] < min) {
                        min = Pitch[i];
                        index = i;
                    }
                } else {
                    if (Pitch[i] > -10) {
                        index = i;
                        break;
                    }
                }
            }
        }
    }
    return index;
}

double directionOfBatAtAnyPoint(double Pitch_Orientation, int Bating_Hand, int backlift_start_index,
                                int downswing_start_index, int horizontal_vertical, int shot_type,
                                int first_Negative_in_Y) {
    double yaw = 0, pitch = 0, roll = 0;
    double direction = 0;
    double roll_1 = Roll[backlift_start_index];
    if (horizontal_vertical == 1) {
        yaw = Yaw[downswing_start_index];
        pitch = Pitch[downswing_start_index];
        roll = Roll[downswing_start_index];
    } else {
        yaw = Yaw[first_Negative_in_Y];
        pitch = Pitch[first_Negative_in_Y];
        roll = Roll[first_Negative_in_Y];
    }
    //    printf("%f \n",Pitch_Orientation);
    //    printf("%f \t",yaw);
    // if roll from backllift start to downswing start going positive to negative or negative to postive
    // the we have to consider the roll is in chagnging mode

    if (roll < 0) {
        if (roll_1 > 0) {
            if (fabs(roll) < 15) {
                roll = fabs(roll);
            }
        }
    } else {
        if (roll_1 < 0) {
            if (fabs(roll) < 15) {
                roll = -roll;
            }
        }
    }

    // Yaw adjustment according to Roll and Pitch
    if (fabs(roll) >= 0 && fabs(roll) <= 5) {
        if (pitch >= 0 && pitch <= 10) {
            yaw = yaw - 0;
        } else if (pitch > 10 && pitch <= 20) {
            yaw = yaw - 0;
        } else if (pitch > 20 && pitch <= 30) {
            yaw = yaw - 0;
        } else if (pitch > 30 && pitch <= 40) {
            yaw = yaw - 0;
        } else if (pitch > 40 && pitch <= 50) {
            yaw = yaw - 0;
        } else if (pitch > 50 && pitch <= 60) {
            yaw = yaw - 0;
        } else if (pitch > 60 && pitch <= 70) {
            yaw = yaw - 0;
        } else if (pitch > 70 && pitch <= 80) {
            yaw = yaw - 0;
        } else if (pitch > 80 && pitch <= 85) {
            yaw = yaw - 0;
        } else if (pitch > 85 && pitch <= 90) {
            yaw = yaw - 0;
        } else if (pitch >= 90 && pitch <= 100) {
            yaw = yaw - 0;
        } else if (pitch > 100 && pitch <= 110) {
            yaw = yaw - 0;
        } else if (pitch > 110 && pitch <= 120) {
            yaw = yaw - 0;
        } else if (pitch > 120 && pitch <= 130) {
            yaw = yaw - 0;
        } else if (pitch > 130 && pitch <= 140) {
            yaw = yaw - 0;
        } else if (pitch > 140 && pitch <= 150) {
            yaw = yaw - 0;
        } else if (pitch > 150 && pitch <= 160) {
            yaw = yaw - 0;
        } else if (pitch > 160 && pitch <= 170) {
            yaw = yaw - 0;
        } else if (pitch > 170 && pitch <= 180) {
            yaw = yaw - 0;
        } else if (pitch > -180 && pitch <= -170) {
            yaw = yaw + 35;
        } else if (pitch > -170 && pitch <= -160) {
            yaw = yaw + 40;
        }
    }
    if (fabs(roll) > 5 && fabs(roll) <= 10) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 30;
            } else {
                yaw = yaw + 30;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 16;
            } else {
                yaw = yaw + 16;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 13;
            } else {
                yaw = yaw + 13;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 5;
            } else {
                yaw = yaw + 5;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 2;
            } else {
                yaw = yaw + 2;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 1;
            } else {
                yaw = yaw - 1;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 3;
            } else {
                yaw = yaw - 3;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 9;
            } else {
                yaw = yaw - 9;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        }
    } else if (fabs(roll) > 10 && fabs(roll) <= 15) {

        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 15;
            } else {
                yaw = yaw + 15;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 14;
            } else {
                yaw = yaw + 14;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 20;
            } else {
                yaw = yaw + 20;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 31;
            } else {
                yaw = yaw + 31;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 15;
            } else {
                yaw = yaw + 15;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 5;
            } else {
                yaw = yaw + 5;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 2;
            } else {
                yaw = yaw + 2;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 1;
            } else {
                yaw = yaw - 1;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 3;
            } else {
                yaw = yaw - 3;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 7;
            } else {
                yaw = yaw - 7;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 7;
            } else {
                yaw = yaw - 7;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 9;
            } else {
                yaw = yaw - 9;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 13;
            } else {
                yaw = yaw - 13;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 170 && pitch <= 175) {
            if (roll > 0) {
                yaw = yaw + 30;
            } else {
                yaw = yaw - 30;
            }
        } else if (pitch > 175 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 45;
            } else {
                yaw = yaw - 45;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 45;
            } else {
                yaw = yaw - 45;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 50;
            } else {
                yaw = yaw - 50;
            }
        }
    } else if (fabs(roll) > 15 && fabs(roll) <= 20) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 50;
            } else {
                yaw = yaw + 50;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 35;
            } else {
                yaw = yaw + 35;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 28;
            } else {
                yaw = yaw + 28;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 20;
            } else {
                yaw = yaw + 20;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 15;
            } else {
                yaw = yaw + 15;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 9;
            } else {
                yaw = yaw + 9;
            }
        } else if (pitch > 60 && pitch <= 70) {

            if (roll > 0) {
                yaw = yaw - 5;
            } else {
                yaw = yaw + 5;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 4;
            } else {
                yaw = yaw + 4;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 10;
            } else {
                yaw = yaw - 10;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 14;
            } else {
                yaw = yaw - 14;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 14;
            } else {
                yaw = yaw - 14;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 17;
            } else {
                yaw = yaw - 17;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 28;
            } else {
                yaw = yaw - 28;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 65;
            } else {
                yaw = yaw - 65;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 65;
            } else {
                yaw = yaw - 65;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        }
    } else if (fabs(roll) > 20 && fabs(roll) <= 25) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 40;
            } else {
                yaw = yaw + 40;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 30;
            } else {
                yaw = yaw + 30;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 20;
            } else {
                yaw = yaw + 20;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 9;
            } else {
                yaw = yaw + 9;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 5;
            } else {
                yaw = yaw + 5;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 4;
            } else {
                yaw = yaw + 4;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 10;
            } else {
                yaw = yaw - 10;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 14;
            } else {
                yaw = yaw - 14;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 21;
            } else {
                yaw = yaw - 21;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 37;
            } else {
                yaw = yaw - 37;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 45;
            } else {
                yaw = yaw - 45;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 75;
            } else {
                yaw = yaw - 75;
            }
        }
    } else if (fabs(roll) > 25 && fabs(roll) <= 30) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 65;
            } else {
                yaw = yaw + 65;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 40;
            } else {
                yaw = yaw + 40;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 33;
            } else {
                yaw = yaw + 33;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 28;
            } else {
                yaw = yaw + 28;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 15;
            } else {
                yaw = yaw + 15;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 10;
            } else {
                yaw = yaw - 10;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 30;
            } else {
                yaw = yaw - 30;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 50;
            } else {
                yaw = yaw - 50;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 75;
            } else {
                yaw = yaw - 75;
            }
        }
    } else if (fabs(roll) > 30 && fabs(roll) <= 35) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 70;
            } else {
                yaw = yaw + 70;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 45;
            } else {
                yaw = yaw + 45;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 35;
            } else {
                yaw = yaw + 35;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 20;
            } else {
                yaw = yaw + 20;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 3;
            } else {
                yaw = yaw + 3;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 10;
            } else {
                yaw = yaw - 10;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 25;
            } else {
                yaw = yaw - 25;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 30;
            } else {
                yaw = yaw - 30;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 55;
            } else {
                yaw = yaw - 55;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 65;
            } else {
                yaw = yaw - 65;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 73;
            } else {
                yaw = yaw - 73;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 73;
            } else {
                yaw = yaw - 73;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 78;
            } else {
                yaw = yaw - 78;
            }
        }
    } else if (fabs(roll) > 35 && fabs(roll) <= 40) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 86;
            } else {
                yaw = yaw + 86;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 68;
            } else {
                yaw = yaw + 68;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 55;
            } else {
                yaw = yaw + 55;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 42;
            } else {
                yaw = yaw + 42;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 33;
            } else {
                yaw = yaw + 33;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 18;
            } else {
                yaw = yaw + 18;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 45;
            } else {
                yaw = yaw - 45;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 55;
            } else {
                yaw = yaw - 55;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 65;
            } else {
                yaw = yaw - 65;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 74;
            } else {
                yaw = yaw - 74;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 74;
            } else {
                yaw = yaw - 74;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 79;
            } else {
                yaw = yaw - 79;
            }
        }
    } else if (fabs(roll) > 40 && fabs(roll) <= 45) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 88;
            } else {
                yaw = yaw + 88;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 70;
            } else {
                yaw = yaw + 70;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 42;
            } else {
                yaw = yaw + 42;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 33;
            } else {
                yaw = yaw + 33;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 20;
            } else {
                yaw = yaw + 20;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 12;
            } else {
                yaw = yaw + 12;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 6;
            } else {
                yaw = yaw + 6;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 3;
            } else {
                yaw = yaw - 3;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 25;
            } else {
                yaw = yaw - 25;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 45;
            } else {
                yaw = yaw - 45;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 57;
            } else {
                yaw = yaw - 57;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 65;
            } else {
                yaw = yaw - 65;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 79;
            } else {
                yaw = yaw - 79;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 79;
            } else {
                yaw = yaw + 79;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 84;
            } else {
                yaw = yaw - 84;
            }
        }
    } else if (fabs(roll) > 45 && fabs(roll) <= 50) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 91;
            } else {
                yaw = yaw + 91;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 68;
            } else {
                yaw = yaw + 68;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 55;
            } else {
                yaw = yaw + 55;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 48;
            } else {
                yaw = yaw + 48;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 29;
            } else {
                yaw = yaw + 29;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 18;
            } else {
                yaw = yaw + 18;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 57;
            } else {
                yaw = yaw - 57;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 60;
            } else {
                yaw = yaw - 60;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 75;
            } else {
                yaw = yaw - 75;
            }
        }
    } else if (fabs(roll) > 50 && fabs(roll) <= 70) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 93;
            } else {
                yaw = yaw + 93;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 72;
            } else {
                yaw = yaw + 72;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 50;
            } else {
                yaw = yaw + 50;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 48;
            } else {
                yaw = yaw + 48;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 29;
            } else {
                yaw = yaw + 29;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 37;
            } else {
                yaw = yaw + 37;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 14;
            } else {
                yaw = yaw - 14;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 18;
            } else {
                yaw = yaw - 18;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 25;
            } else {
                yaw = yaw - 25;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 47;
            } else {
                yaw = yaw - 47;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 58;
            } else {
                yaw = yaw - 58;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 60;
            } else {
                yaw = yaw - 60;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 83;
            } else {
                yaw = yaw - 83;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 83;
            } else {
                yaw = yaw - 83;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 83;
            } else {
                yaw = yaw - 83;
            }
        }
    } else if (fabs(roll) > 70 && fabs(roll) <= 75) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 91;
            } else {
                yaw = yaw + 91;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 80;
            } else {
                yaw = yaw + 80;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 75;
            } else {
                yaw = yaw + 75;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 55;
            } else {
                yaw = yaw + 55;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 48;
            } else {
                yaw = yaw + 48;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 29;
            } else {
                yaw = yaw + 29;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 25;
            } else {
                yaw = yaw + 25;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 35;
            } else {
                yaw = yaw - 35;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 50;
            } else {
                yaw = yaw - 50;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 57;
            } else {
                yaw = yaw - 57;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 60;
            } else {
                yaw = yaw - 60;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        }
    } else if (fabs(roll) > 75 && fabs(roll) <= 80) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 91;
            } else {
                yaw = yaw + 91;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 91;
            } else {
                yaw = yaw + 91;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 55;
            } else {
                yaw = yaw + 55;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 48;
            } else {
                yaw = yaw + 48;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 29;
            } else {
                yaw = yaw + 29;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 18;
            } else {
                yaw = yaw + 18;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 57;
            } else {
                yaw = yaw - 57;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 60;
            } else {
                yaw = yaw - 60;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 80;
            } else {
                yaw = yaw - 80;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 85;
            } else {
                yaw = yaw - 85;
            }
        }
    } else if (fabs(roll) > 80 && fabs(roll) <= 90) {
        if (pitch >= 0 && pitch <= 10) {
            if (roll > 0) {
                yaw = yaw - 91;
            } else {
                yaw = yaw + 91;
            }
        } else if (pitch > 10 && pitch <= 20) {
            if (roll > 0) {
                yaw = yaw - 68;
            } else {
                yaw = yaw + 68;
            }
        } else if (pitch > 20 && pitch <= 30) {
            if (roll > 0) {
                yaw = yaw - 60;
            } else {
                yaw = yaw + 60;
            }
        } else if (pitch > 30 && pitch <= 40) {
            if (roll > 0) {
                yaw = yaw - 55;
            } else {
                yaw = yaw + 55;
            }
        } else if (pitch > 40 && pitch <= 50) {
            if (roll > 0) {
                yaw = yaw - 48;
            } else {
                yaw = yaw + 48;
            }
        } else if (pitch > 50 && pitch <= 60) {
            if (roll > 0) {
                yaw = yaw - 29;
            } else {
                yaw = yaw + 29;
            }
        } else if (pitch > 60 && pitch <= 70) {
            if (roll > 0) {
                yaw = yaw - 18;
            } else {
                yaw = yaw + 18;
            }
        } else if (pitch > 70 && pitch <= 80) {
            if (roll > 0) {
                yaw = yaw - 10;
            } else {
                yaw = yaw + 10;
            }
        } else if (pitch > 80 && pitch <= 85) {
            if (roll > 0) {
                yaw = yaw - 7;
            } else {
                yaw = yaw + 7;
            }
        } else if (pitch > 85 && pitch <= 90) {
            if (roll > 0) {
                yaw = yaw - 1;
            } else {
                yaw = yaw + 1;
            }
        } else if (pitch >= 90 && pitch <= 100) {
            if (roll > 0) {
                yaw = yaw + 2;
            } else {
                yaw = yaw - 2;
            }
        } else if (pitch > 100 && pitch <= 110) {
            if (roll > 0) {
                yaw = yaw + 5;
            } else {
                yaw = yaw - 5;
            }
        } else if (pitch > 110 && pitch <= 120) {
            if (roll > 0) {
                yaw = yaw + 15;
            } else {
                yaw = yaw - 15;
            }
        } else if (pitch > 120 && pitch <= 130) {
            if (roll > 0) {
                yaw = yaw + 20;
            } else {
                yaw = yaw - 20;
            }
        } else if (pitch > 130 && pitch <= 140) {
            if (roll > 0) {
                yaw = yaw + 40;
            } else {
                yaw = yaw - 40;
            }
        } else if (pitch > 140 && pitch <= 150) {
            if (roll > 0) {
                yaw = yaw + 57;
            } else {
                yaw = yaw - 57;
            }
        } else if (pitch > 150 && pitch <= 160) {
            if (roll > 0) {
                yaw = yaw + 60;
            } else {
                yaw = yaw - 60;
            }
        } else if (pitch > 160 && pitch <= 170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > 170 && pitch <= 180) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -180 && pitch <= -170) {
            if (roll > 0) {
                yaw = yaw + 70;
            } else {
                yaw = yaw - 70;
            }
        } else if (pitch > -170 && pitch <= -160) {
            if (roll > 0) {
                yaw = yaw + 75;
            } else {
                yaw = yaw - 75;
            }
        }
    }
    if (yaw > 360) {
        yaw = yaw - 360;
    } else if (yaw < 0) {
        yaw = yaw + 360;
    }
    // printf("Pitch %f Roll %f  yaw %f \t",pitch,roll,yaw);
    double backliftdirection = calculateDirectionForBackliftDirectionTest(Bating_Hand,
                                                                          Pitch_Orientation, yaw);
    if (fabs(backliftdirection) >= 360) {
        backliftdirection = 0;
    }

    if (SHV == 0) {
        //		if(shot_type == 5 ||shot_type == 13)
        //		{
        ////			printf("%f %f %f %d %d \n ",backliftdirection,pitch,roll,SHV,PST);
        //			backliftdirection =  backliftdirection - 180;
        ////			printf("%f %f %f %d %d \n ",backliftdirection,pitch,roll,SHV,PST);
        //		}
        //		else if(shot_type == 10)
        //		{
        //			if(pitch >-170 && pitch<-160)
        //			{
        //				backliftdirection =  backliftdirection;
        //			}
        //			else if(pitch > -100 && pitch< -50 )
        //			{
        //				backliftdirection =  backliftdirection + 180;
        //			}
        //			else if(pitch >-160 && pitch<-100)
        //			{
        //				backliftdirection =  180 - backliftdirection;
        //			}
        ////			printf("%f %f %f %d %d \n ",backliftdirection,pitch,roll,SHV,PST);
        //		}
        int indexHighPitch = indexForHorizontHighPitch(Pitch, BSI, SI);
        int yawMovement = checkYawForBatMovement(Yaw, indexHighPitch, Bating_Hand,
                                                 Pitch_Orientation);
        double dir = 0;
        if (Bating_Hand == 1) {
            dir = calculateDirectionForShotType(Bating_Hand, Pitch_Orientation,
                                                Yaw[indexHighPitch] + 90);
        } else {
            dir = calculateDirectionForShotType(Bating_Hand, Pitch_Orientation,
                                                Yaw[indexHighPitch] - 90);
        }

        //		printf("%f %f %f %f \t",Pitch[indexHighPitch],dir+Yrotation_unity[indexHighPitch],Yrotation_unity[indexHighPitch],Roll[indexHighPitch]);
        backliftdirection = dir + Yrotation_unity[indexHighPitch];
        //		if(yawMovement == 1)
        //		{
        //			backliftdirection = dir + Roll[indexHighPitch];
        //		}
        //		else{
        //			backliftdirection = dir - Roll[indexHighPitch];
        //		}
    }

    return backliftdirection;
}

int indexForHorizontHighPitch(double arr[], int backlift_start_index, int downswing_start_index) {
    int index = downswing_start_index;
    double val = 0;
    for (i = downswing_start_index; i > backlift_start_index; i--) {
        if (arr[i] < val) {
            val = arr[i];
            index = i;
        }
    }

    return index;
}

int checkYawForBatMovement(double arr[], int HighPitchindex, int batting_hand,
                           double orientation_app_or_ana) {
    int index = 0;
    double val = 0;
    double dir = calculateDirectionForShotType(batting_hand, orientation_app_or_ana,
                                               Yaw[HighPitchindex - 1]);
    int countPosi = 0, countNegi = 0;
    double dir2 = 0;
    for (i = HighPitchindex; i < HighPitchindex + 5; i++) {
        dir2 = calculateDirectionForShotType(batting_hand, orientation_app_or_ana, Yaw[i]);
        if (dir > dir2) {
            countNegi = countNegi + 1;
        } else {
            countPosi = countPosi + 1;
        }
    }
    if (countPosi > countNegi) {
        index = 1;
    }
    {
        index = -1;
    }
    return index;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_stancebeam_cc_1lane_data_analytics_AnalyticsEngine_initPlayer(
        JNIEnv *env,
        jobject obj,
        jdouble bat_weight,
        jdouble bat_length,
        jint grip_position,
        jdouble orientation_app,
        jdouble orientation_ana,
        jint user_hand,
        jint firmware_version,
        jdouble gyro_x_threshold,
        jint dpr) {
    initPlayer(bat_weight, bat_length, grip_position, orientation_app, orientation_ana, user_hand,
               firmware_version, gyro_x_threshold, dpr);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_stancebeam_cc_1lane_data_analytics_AnalyticsEngine_kinematics(
        JNIEnv *env,
        jobject obj,
        jdoubleArray acc,
        jdoubleArray gyro,
        jdoubleArray angle,
        jdoubleArray mag,
        jdoubleArray mag_max_min) {
    jdouble *accArray = (*env).GetDoubleArrayElements(acc, nullptr);
    jdouble *gyroArray = (*env).GetDoubleArrayElements(gyro, nullptr);
    jdouble *angleArray = (*env).GetDoubleArrayElements(angle, nullptr);
    jdouble *magArray = (*env).GetDoubleArrayElements(mag, nullptr);
    jdouble *mag_max_minArray = (*env).GetDoubleArrayElements(mag_max_min, nullptr);
    return kinematics(accArray, gyroArray, angleArray, magArray, mag_max_minArray);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_stancebeam_cc_1lane_data_analytics_AnalyticsEngine_getResult(JNIEnv *env, jobject obj) {
    struct shotResult *ourResult = getResult();
    jclass resultClass = (*env).FindClass("com/stancebeam/cc_lane/data/analytics/models/KinematicsResult");
    jmethodID methodId = (*env).GetMethodID(resultClass, "<init>", "(DDDDDDDDDDI)V");
    return (*env).NewObject(
            resultClass,
            methodId,
            (*ourResult).Power,
            (*ourResult).powerAtImpact,
            (*ourResult).Speed,
            (*ourResult).speedAtImpact,
            (*ourResult).Efficiency,
            (*ourResult).Backlift,
            (*ourResult).Downswing,
            (*ourResult).Followthroug,
            (*ourResult).timeToImpact,
            (*ourResult).batFace,
            (*ourResult).shotType
    );
}
