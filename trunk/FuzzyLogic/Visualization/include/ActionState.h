#ifndef __ActionState_h_
#define __ActionState_h_

typedef enum
{
    NORMAL, /**< when driving normally on the road */
    SIMMER, /**< when driving on the roadside */
    EXPLODE /**< when driving on the roadside for too long or going offroad completely */
} ActionState; // TODO: rename

#endif // __ActionState_h_


