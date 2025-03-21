#include "macros/scrcmd.inc"
#include "generated/distribution_events.h"
#include "res/text/bank/newmoon_island_forest.h"

    .data

    ScriptEntry _000E
    ScriptEntry _005E
    ScriptEntry _0079
    ScriptEntryEnd

_000E:
    ScrCmd_22D 2, 0x4000
    GoToIfEq 0x4000, 0, _0058
    CheckItem ITEM_MEMBER_CARD, 1, 0x4000
    GoToIfEq 0x4000, FALSE, _0058
    CheckDistributionEvent DISTRIBUTION_EVENT_DARKRAI, 0x4000
    GoToIfEq 0x4000, FALSE, _0058
    GoToIfSet 0x158, _0058
    ClearFlag 0x240
    End

_0058:
    SetFlag 0x240
    End

_005E:
    GoToIfSet 142, _006B
    End

_006B:
    SetFlag 0x240
    RemoveObject 0
    ClearFlag 142
    End

_0079:
    LockAll
    PlayFanfare SEQ_SE_CONFIRM
    WaitFanfare SEQ_SE_CONFIRM
    Message 0
    CloseMessage
    SetFlag 142
    StartLegendaryBattle SPECIES_DARKRAI, 50
    ClearFlag 142
    CheckWonBattle 0x800C
    GoToIfEq 0x800C, FALSE, _014C
    CheckDidNotCapture 0x800C
    GoToIfEq 0x800C, TRUE, _00D8
    SetFlag 0x158
_00BC:
    SetFlag 0x241
    GoToIfEq 0x40F8, 2, _00E3
    Message 3
    WaitABXPadPress
    CloseMessage
    ReleaseAll
    End

_00D8:
    Message 1
    CloseMessage
    GoTo _00BC

_00E3:
    Message 2
    CloseMessage
    WaitTime 30, 0x800C
    FadeScreen 6, 3, 0, 0
    WaitFadeScreen
    FadeScreen 6, 3, 1, 0
    WaitFadeScreen
    FadeScreen 6, 3, 0, 0
    WaitFadeScreen
    FadeScreen 6, 3, 1, 0
    WaitFadeScreen
    FadeScreen 6, 6, 0, 0
    WaitFadeScreen
    WaitTime 120, 0x800C
    Warp MAP_HEADER_CANALAVE_CITY_HARBOR_INN, 0, 8, 6, 1
    FadeScreen 6, 6, 1, 0
    WaitFadeScreen
    ReleaseAll
    End

_014C:
    BlackOutFromBattle
    ReleaseAll
    End

    .byte 0
    .byte 0
