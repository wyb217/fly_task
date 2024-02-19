/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2022 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "commander.h"
#include "stabilizer_types.h"
#include "square_task.h"
#define DEBUG_MODULE "TRIANGLE_TASK"
#include "debug.h"

extern SemaphoreHandle_t fly_Semaphore;
extern State state;
extern Stage stage;
static setpoint_t setpoint;
static float height = 0.3;
static void setHoverSetpoint(setpoint_t *setpoint, float vx, float vy, float z, float yawrate)
{
    setpoint->mode.z = modeAbs;
    setpoint->position.z = z;
    setpoint->mode.yaw = modeVelocity;
    setpoint->attitudeRate.yaw = yawrate;
    setpoint->mode.x = modeVelocity;
    setpoint->mode.y = modeVelocity;
    setpoint->velocity.x = vx;
    setpoint->velocity.y = vy;
    setpoint->velocity_body = true;
    commanderSetSetpoint(setpoint, 3);
}

void control_fly_triangle(int fly_case)
{
    switch (fly_case)
    {
    case stage_one:
        for(int i=0;i< 100;i++){
            setHoverSetpoint(&setpoint, -0.865, 0.5, height ,0);
            vTaskDelay(M2T(10));
        }
        DEBUG_PRINT("stage_one_triangle\n");
        break;
    
    case stage_two:
        for(int i=0;i< 100;i++){
            setHoverSetpoint(&setpoint, 0, -1, height ,0);
            vTaskDelay(M2T(10));
        }
        DEBUG_PRINT("stage_two_triangle\n");
        break;
    
    case stage_three:
        for(int i=0;i< 100;i++){
            setHoverSetpoint(&setpoint, 0.865, 0.5, height ,0);
            vTaskDelay(M2T(10));
        }
        DEBUG_PRINT("stage_three_triangle\n");
        break;
    }
}
void triangle_task(void *param)
{
    BaseType_t result = pdFALSE;
    while(1){
        if(result == pdFALSE){
            result = xSemaphoreTake(fly_Semaphore, portMAX_DELAY);
        }else{
            switch (state)
            {
            case flying:
                if(stage == onGround){
                    take_off();
                    stage ++;
                }else{
                    control_fly_triangle(stage);

                    if(stage == stage_three){
                        result = pdFALSE;
                        stage = stage_one;
                        xSemaphoreGive(fly_Semaphore);
                        vTaskDelay(M2T(10));
                    }else stage++;
                }
                break;
            
            case landing:
                land();
                state = idle;
                stage = onGround;
                vTaskDelay(M2T(1000));
                break;
            
            case idle:
                vTaskDelay(M2T(10));
                break;

            }
        }
    }
    
}
